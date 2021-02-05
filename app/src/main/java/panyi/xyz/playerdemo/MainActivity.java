package panyi.xyz.playerdemo;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.widget.Button;

import java.io.File;
import java.util.List;
import me.rosuh.filepicker.config.FilePickerManager;

public class MainActivity extends AppCompatActivity {

    boolean created = false;

    static boolean isPlayingAsset = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        findViewById(R.id.select_file_btn).setOnClickListener((v)->{
            selectFile();
            //playMusic();
        });

        Button btn = findViewById(R.id.select_file_btn);
        btn.setText(SimplePlayerBridge.sayHello(0));

        SimplePlayerBridge.createEngine();
//        int sampleRate = 0;
//        int bufSize = 0;
//        AudioManager myAudioMgr = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
//        String nativeParam = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
//        sampleRate = Integer.parseInt(nativeParam);
//        nativeParam = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
//        bufSize = Integer.parseInt(nativeParam);
//
//        SimplePlayerBridge.createBufferQueueAudioPlayer(sampleRate , bufSize);


    }

    void playMusic(String filepath){
        created = SimplePlayerBridge.createAssetAudioPlayer(filepath , new File(filepath).length());

        if (created) {
            isPlayingAsset = !isPlayingAsset;
            SimplePlayerBridge.setPlayingAssetAudioPlayer(isPlayingAsset);
        }
    }


    @Override
    protected void onDestroy() {
        SimplePlayerBridge.shutdown();
        super.onDestroy();
    }

    private void selectFile(){
        FilePickerManager.INSTANCE.from(this).forResult(FilePickerManager.REQUEST_CODE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == FilePickerManager.REQUEST_CODE && resultCode == RESULT_OK) {
            List<String> paths = FilePickerManager.INSTANCE.obtainData();
            if (paths != null && paths.size() > 0) {
                final String filePath = paths.get(0);
                System.out.println("filepath = " + filePath);
                playMusic(filePath);
            }
        }
    }
}