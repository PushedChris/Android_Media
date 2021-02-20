package com.comochris.mediaplayer;

import android.content.Context;
import android.os.Environment;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class FFMediaPlayer extends SurfaceView implements SurfaceHolder.Callback{
    private static final String TAG = "mediaplayer";
    private String TEST_FILE_TFCARD = "/storage/emulated/0/DCIM/Camera/4k_test.mp4";
//    private String TEST_FILE_TFCARD = "/storage/emulated/0/DCIM/Camera/abc.mp4";
    static {
        System.loadLibrary("player-ffmpeg");
    }
    public static String nativeGetFFmpegVersion() {
        return GetFFmpegVersion();
    }

    public FFMediaPlayer(Context context) {
        super(context);
        Log.v(TAG, "mediaplayer");
        Log.v(TAG, Environment.getDataDirectory().toString());
        getHolder().addCallback(this);
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width,
                               int height) {
        Log.v(TAG, "surfaceChanged, format is " + format + ", width is "
                + width + ", height is " + height);
        //setSize(1920,1080);
        setSize(3840,2160);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.v(TAG, "surfaceCreated");
        initPlayer();
        setSurface(holder.getSurface());
        openMedia(TEST_FILE_TFCARD);//3 thread
        startPlayer();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.v(TAG, "surfaceDestroyed");
        stopPlayer();
        closeMedia();
        destoryPlayer();
    }

    private static native String GetFFmpegVersion();
    public native int initPlayer();
    public native int destoryPlayer();
    public native int startPlayer();
    public native int stopPlayer();
    public native int openMedia(String filePath);
    public native int closeMedia();
    public native int setSurface(Surface view);
    public  native int setSize(int width,int height);

}