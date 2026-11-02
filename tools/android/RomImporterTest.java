package io.github.genissonemilio.minishcap;
import java.io.*;
import java.nio.file.*;
import java.util.Arrays;
import java.util.zip.*;

/** Runs on the host JVM without the Android SDK or a copyrighted game in CI. */
public class RomImporterTest {
    static int cases;
    static void reject(byte[] input,File target) throws Exception {
        byte[] before=Files.readAllBytes(target.toPath());
        try { RomImporter.importRom(new ByteArrayInputStream(input),target); throw new AssertionError("accepted invalid ROM"); }
        catch(IOException expected) { }
        if(!Arrays.equals(before,Files.readAllBytes(target.toPath()))) throw new AssertionError("replaced prior ROM on failure");
        if(target.getParentFile().listFiles().length!=1) throw new AssertionError("left temporary import files");
        cases++;
    }
    static byte[] zip(String name,byte[] content) throws Exception {
        ByteArrayOutputStream bytes=new ByteArrayOutputStream();
        try(ZipOutputStream out=new ZipOutputStream(bytes)) { out.putNextEntry(new ZipEntry(name)); out.write(content); out.closeEntry(); }
        return bytes.toByteArray();
    }
    public static void main(String[] args) throws Exception {
        Path folder=Files.createTempDirectory("minish-import-test");
        File dest=folder.resolve("game.gba").toFile();
        try {
            Files.write(dest.toPath(),new byte[]{1,2,3});
            reject(new byte[0],dest);
            reject(new byte[16777216],dest); // Correct size alone is insufficient.
            reject(new byte[16777217],dest);
            reject(zip("../../outside.gba",new byte[16]),dest);
            reject(zip("readme.txt",new byte[]{1}),dest);
            reject(zip("padding.txt",new byte[32*1024*1024+1]),dest);
            reject(new byte[]{'P','K',3,4,0,0},dest);
            for(String path:args) {
                byte[] valid=Files.readAllBytes(Path.of(path));
                RomImporter.Region imported=RomImporter.importRom(new ByteArrayInputStream(valid),dest);
                if(dest.length()!=RomImporter.SIZE || imported!=RomImporter.identify(dest))
                    throw new AssertionError("valid owner ROM import/identification failed");
                if(valid[0xaf]=='P' && imported!=RomImporter.Region.EUROPE)
                    throw new AssertionError("EU ROM selected USA corpus");
                if(valid[0xaf]=='E' && imported!=RomImporter.Region.USA)
                    throw new AssertionError("USA ROM selected EU corpus");
                if(RomImporter.importRom(new ByteArrayInputStream(zip("game.gba",valid)),dest)!=imported)
                    throw new AssertionError("ZIP region differs from raw region");
                ByteArrayOutputStream bytes=new ByteArrayOutputStream();
                try(ZipOutputStream out=new ZipOutputStream(bytes)) {
                    for(String name:new String[]{"one.gba","two.gba"}) {
                        out.putNextEntry(new ZipEntry(name));out.write(valid);out.closeEntry();
                    }
                }
                reject(bytes.toByteArray(),dest);
                cases+=2;
            }
            if(RomImporter.Region.USA.savePath.equals(RomImporter.Region.EUROPE.savePath))
                throw new AssertionError("regions share a save file");
            File library=folder.resolve("roms").toFile();
            for(String path:args) {
                RomImporter.Region imported;
                try(InputStream in=new FileInputStream(path)) { imported=RomImporter.importToLibrary(in,library); }
                File installed=new File(library,"minishcap_"+imported.id+".gba");
                if(RomImporter.identify(installed)!=imported) throw new AssertionError("wrong library path/identity");
                cases++;
            }
            if(args.length>0) {
                int count=library.listFiles().length;
                try {
                    RomImporter.importToLibrary(new ByteArrayInputStream(new byte[16]),library);
                    throw new AssertionError("invalid library import accepted");
                } catch(IOException expected) { }
                if(library.listFiles().length!=count) throw new AssertionError("failed library import left files");
                for(File file:library.listFiles()) RomImporter.identify(file);
                cases++;
            }
            System.out.println(cases+" import checks passed");
        } finally {
            try(java.util.stream.Stream<Path> paths=Files.walk(folder)) {
                for(Path path:paths.sorted(java.util.Comparator.reverseOrder()).toArray(Path[]::new)) Files.delete(path);
            }
        }
    }
}
