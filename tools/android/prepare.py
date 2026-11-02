#!/usr/bin/env python3
"""Verify owner ROM and generate the native corpus. Never commits game data."""
import argparse
import hashlib
import pathlib
import shutil
import subprocess
import tempfile
import zipfile

ROOT = pathlib.Path(__file__).resolve().parents[2]
ROM_SHA = "b4bd50e4131b027c334547b4524e2dbbd4227130"
EU_SHA = "cff199b36ff173fb6faf152653d1bccf87c26fb7"
BIOS_SHA = "2e41f9d30ecdf1c4540edf89ac16f49bf327edb7"

def publish_generated(source, destination):
    """Keep unchanged file mtimes so incremental native builds retain their cache."""
    destination.mkdir(parents=True, exist_ok=True)
    files = {p.name: p for p in source.iterdir() if p.is_file()}
    changed = 0
    for name, path in files.items():
        target = destination / name
        if target.exists() and target.read_bytes() == path.read_bytes():
            continue
        with tempfile.NamedTemporaryFile(dir=destination, delete=False) as out:
            out.write(path.read_bytes())
        pathlib.Path(out.name).replace(target)
        changed += 1
    for target in destination.iterdir():
        if target.is_file() and target.suffix in {".c", ".cpp", ".h", ".json"} and target.name not in files:
            target.unlink()
    print(f"{destination.relative_to(ROOT)}: {changed} changed files")

def load_rom(path):
    if zipfile.is_zipfile(path):
        with zipfile.ZipFile(path) as archive:
            candidates = [e for e in archive.infolist() if not e.is_dir() and e.filename.lower().endswith((".gba", ".agb"))]
            if len(candidates) != 1 or candidates[0].file_size != 16777216:
                raise ValueError("ZIP must contain exactly one 16 MiB USA or Europe .gba file")
            data = archive.read(candidates[0])
    else:
        if path.stat().st_size != 16777216:
            raise ValueError("ROM must be 16 MiB")
        data = path.read_bytes()
    if hashlib.sha1(data).hexdigest() not in {ROM_SHA, EU_SHA}:
        raise ValueError("ROM SHA-1 mismatch: unmodified Minish Cap (USA or Europe) required")
    return data

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--rom", type=pathlib.Path, action="append", required=True,
                   help="USA or Europe ROM/ZIP; repeat to generate both corpora")
    p.add_argument("--tool", type=pathlib.Path, default=ROOT / "build/host/gba_recompile")
    args = p.parse_args()
    inputs = [(load_rom(path)) for path in args.rom]
    firmware = ROOT / "android/firmware/open_gba_bios.firmware"
    if hashlib.sha1(firmware.read_bytes()).hexdigest() != BIOS_SHA:
        raise ValueError("Open firmware integrity check failed")
    if not args.tool.is_file():
        raise ValueError("Build the host gba_recompile tool first; see ANDROID.md")
    def run(*opts):
        subprocess.run([str(args.tool.resolve()), *opts], cwd=ROOT, check=True)
    (ROOT / "build").mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="codegen-", dir=ROOT / "build") as temp:
        staging = pathlib.Path(temp)
        run("--bios", str(firmware), "--config", "config/open_bios.toml", "--out", str(staging / "bios"))
        generated = []
        for data in inputs:
            sha = hashlib.sha1(data).hexdigest()
            europe = sha == EU_SHA
            region = "eu" if europe else "usa"
            rom = ROOT / f"roms/minishcap_{region}.gba"
            rom.parent.mkdir(exist_ok=True)
            if not rom.exists() or hashlib.sha1(rom.read_bytes()).hexdigest() != sha:
                with tempfile.NamedTemporaryFile(dir=rom.parent, delete=False) as out:
                    out.write(data)
                pathlib.Path(out.name).replace(rom)
            directory = staging / region
            if europe:
                opts = ["--config", "symbols/eu/minishcap_eu.toml",
                        "--config", "symbols/eu/minishcap_eu_symbols.toml",
                        "--symbols", "symbols/eu/imported_symbols.tsv",
                        "--data-symbols", "symbols/eu/imported_data_symbols.tsv",
                        "--output-prefix", "eu_"]
            else:
                opts = ["--config", "symbols/minishcap.toml",
                        "--config", "symbols/minishcap_reviewed.toml",
                        "--symbols", "symbols/imported_symbols.tsv",
                        "--data-symbols", "symbols/imported_data_symbols.tsv"]
            run("--rom", str(rom), *opts, "--max-functions", "100000", "--out", str(directory))
            dispatch = (directory / ("eu_dispatch_table.cpp" if europe else "dispatch_table.cpp")).read_text()
            for pc in (("03006164", "030065AC") if europe else ("0300643C", "030065A8")):
                if f"0x{pc}u" not in dispatch:
                    raise ValueError(f"Missing RAM continuation {pc}; generation incomplete")
            generated.append((directory, ROOT / "generated/eu" if europe else ROOT / "generated"))
        publish_generated(staging / "bios", ROOT / "generated/open_bios")
        for source, destination in generated:
            publish_generated(source, destination)

if __name__ == "__main__":
    main()
