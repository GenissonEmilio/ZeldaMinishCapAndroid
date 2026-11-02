package io.github.genissonemilio.minishcap;

import org.gbarecomp.GbaGameActivity;
import android.content.SharedPreferences;
import android.content.Intent;
import java.util.ArrayList;
import java.util.Arrays;

public class ZeldaGameActivity extends GbaGameActivity {
    private final GameAlbumBridge gameAlbum=new GameAlbumBridge(this);
    private boolean albumOpening;
    @Override public void onResume() { super.onResume();albumOpening=false; }
    // JNI copies one clean, color-corrected frame. PNG encoding stays off the game thread.
    public void captureFrame(byte[] rgb,int width,int height,int area,int room,int x,int y) {
        gameAlbum.save(rgb,width,height,area,room,x,y,0);
    }
    public void loadGameAlbum(int page,long token) { gameAlbum.loadPage(page,token); }
    public void loadGamePhoto(String id,long token) { gameAlbum.loadPhoto(id,token); }
    public void saveGamePhoto(byte[] rgb,int width,int height,int area,int room,int x,int y,long token) {
        gameAlbum.save(rgb,width,height,area,room,x,y,token);
    }
    public void showAlbum() {
        runOnUiThread(()->{
            if(!albumOpening && !isFinishing() && !isDestroyed()) {
                albumOpening=true;
                startActivity(new Intent(this,ZeldaAlbumActivity.class).putExtra("from_game",true));
            }
        });
    }
    @Override protected void onDestroy() { gameAlbum.close();super.onDestroy(); }
    // Called by the native settings menu on its game thread; preferences are thread-safe.
    public void persistPixelMode(int mode) {
        getSharedPreferences("minish",MODE_PRIVATE).edit().putInt("pixel_mode",mode==0?0:1).apply();
    }
    @Override protected String[] getArguments() {
        SharedPreferences p=getSharedPreferences("minish",MODE_PRIVATE);
        RomImporter.Region region="eu".equals(getIntent().getStringExtra("rom_region"))?
            RomImporter.Region.EUROPE:RomImporter.Region.USA;
        ArrayList<String> args=new ArrayList<>(Arrays.asList(
            "--minish-region",region.id,
            "--rom",getFilesDir()+"/"+region.romPath(),
            "--bios",getFilesDir()+"/bios/open_gba_bios.firmware",
            "--save",getFilesDir()+"/"+region.savePath,
            "--window","--bios-skip-intro",
            "--volume",p.getBoolean("sound",true)?"100":"0",
            "--touch-size",String.valueOf(p.getInt("touch_size",100)),
            "--touch-opacity",String.valueOf(p.getInt("touch_opacity",70)),
            "--pixel-mode",String.valueOf(p.getInt("pixel_mode",1)),
            region.configPath));
        // ADB QA builds alone expose the local observer and dummy audio sink.
        if ((getApplicationInfo().flags & android.content.pm.ApplicationInfo.FLAG_DEBUGGABLE)!=0
                && getIntent().getBooleanExtra("qa",false)) {
            args.add("--tcp-observe"); args.add("19842"); args.add("--qa-muted");
            if(getIntent().getBooleanExtra("qa_strict",false)) args.add("--qa-strict");
        }
        return args.toArray(new String[0]);
    }
}
