package io.github.genissonemilio.minishcap;

import java.io.*;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

/** Bounded, transactional import: ZIP entries are streamed, never extracted by name. */
public final class RomImporter {
    public static final long SIZE = 16777216;
    public static final String SHA1 = "b4bd50e4131b027c334547b4524e2dbbd4227130";
    public enum Region {
        USA("usa", "USA", SHA1, "saves/minishcap.sav", "variants/minishcap/game.toml"),
        EUROPE("eu", "Europe", "cff199b36ff173fb6faf152653d1bccf87c26fb7",
            "saves/minishcap_eu.sav", "variants/minishcap_eu/game.toml");
        public final String id, label, sha1, savePath, configPath;
        public String romPath() { return "roms/minishcap_"+id+".gba"; }
        Region(String id,String label,String sha1,String savePath,String configPath) {
            this.id=id; this.label=label; this.sha1=sha1; this.savePath=savePath; this.configPath=configPath;
        }
    }
    private RomImporter() {}
    public static Region importRom(InputStream source, File destination) throws IOException {
        return importRom(source,destination,false);
    }
    public static Region importToLibrary(InputStream source,File directory) throws IOException {
        return importRom(source,new File(directory,"unused.gba"),true);
    }
    private static Region importRom(InputStream source,File destination,boolean library) throws IOException {
        File parent = destination.getParentFile();
        if (!parent.isDirectory() && !parent.mkdirs()) throw new IOException("Não foi possível criar a biblioteca.");
        File pending = File.createTempFile("import-", ".pending", parent);
        try (BufferedInputStream input = new BufferedInputStream(source)) {
            Region region=null;
            input.mark(4);
            int a=input.read(), b=input.read(); input.reset();
            if (a=='P' && b=='K') {
                try (ZipInputStream zip = new ZipInputStream(input)) {
                    boolean found=false;
                    int entries=0;
                    long skipped=0;
                    ZipEntry e;
                    byte[] buffer=new byte[65536];
                    while ((e=zip.getNextEntry()) != null) {
                        if (++entries>256) throw new IOException("ZIP com arquivos demais.");
                        String name=e.getName().toLowerCase(java.util.Locale.ROOT);
                        if (!e.isDirectory() && (name.endsWith(".gba") || name.endsWith(".agb"))) {
                            if (found) throw new IOException("Selecione um ZIP com apenas uma ROM de GBA.");
                            region=verifyAndCopy(zip,pending); found=true;
                        } else {
                            int n;
                            while ((n=zip.read(buffer))!=-1) {
                                skipped+=n;
                                if(skipped>32*1024*1024L) throw new IOException("Conteúdo extra do ZIP excede o limite.");
                            }
                        }
                        zip.closeEntry();
                    }
                    if(!found) throw new IOException("Nenhuma ROM .gba encontrada no ZIP.");
                }
            } else region=verifyAndCopy(input,pending);
            File target=library?new File(parent,"minishcap_"+region.id+".gba"):destination;
            Files.move(pending.toPath(),target.toPath(),StandardCopyOption.REPLACE_EXISTING,StandardCopyOption.ATOMIC_MOVE);
            return region;
        } finally { pending.delete(); }
    }
    public static Region identify(File rom) throws IOException {
        if(!rom.isFile() || rom.length()!=SIZE) throw incompatible();
        try(InputStream in=new FileInputStream(rom)) { return verifyAndCopy(in,null); }
    }
    private static IOException incompatible() {
        return new IOException("ROM incompatível. Use The Minish Cap (USA ou Europe), original, sem patches (16 MiB).");
    }
    private static Region verifyAndCopy(InputStream input,File destination) throws IOException {
        MessageDigest digest;
        try { digest=MessageDigest.getInstance("SHA-1"); }
        catch(NoSuchAlgorithmException impossible) { throw new IOException(impossible); }
        long size=0;
        try(FileOutputStream output=destination==null?null:new FileOutputStream(destination)) {
            byte[] data=new byte[65536]; int n;
            while((n=input.read(data))!=-1) {
                size+=n;
                if(size>SIZE) throw incompatible();
                digest.update(data,0,n); if(output!=null) output.write(data,0,n);
            }
            if(output!=null) output.getFD().sync();
        }
        StringBuilder hex=new StringBuilder();
        for(byte b:digest.digest()) hex.append(String.format("%02x",b&255));
        if(size==SIZE) for(Region region:Region.values())
            if(region.sha1.equals(hex.toString())) return region;
        throw incompatible();
    }
}
