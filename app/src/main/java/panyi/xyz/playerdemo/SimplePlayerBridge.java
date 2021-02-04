package panyi.xyz.playerdemo;

import android.content.res.AssetManager;

/**
 * Nativr Bridge
 */
public class SimplePlayerBridge {
    static {
        System.loadLibrary("simple_player");
    }

    public static native String sayHello(int code);

    /**
     * 初始化OpenSL
     */
    public static native void createEngine();

    public static native void shutdown();

    public static native void createBufferQueueAudioPlayer(int sampleRate, int samplesPerBuf);

    public static native void createAssetAudioPlayer(AssetManager assetManager, String filename);

    public static native void setPlayingAssetAudioPlayer(boolean isPlaying);
}
