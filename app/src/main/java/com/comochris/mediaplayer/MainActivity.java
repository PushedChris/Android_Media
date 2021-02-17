package com.comochris.mediaplayer;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.Log;
import android.view.Window;
import android.view.WindowManager;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.comochris.mediaplayer.FFMediaPlayer;
import com.comochris.mediaplayer.StoragePermission;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private FFMediaPlayer mVideoSurface;
    private RelativeLayout mRootView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // hide stauts bar
        StoragePermission.verifyStoragePermissions(this);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        setContentView(R.layout.activity_main);

        mRootView = (RelativeLayout) findViewById(R.id.video_surface_layout);

        // creat surfaceview
        mVideoSurface = new FFMediaPlayer(this);
        mRootView.addView(mVideoSurface);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mVideoSurface.pausePlayer();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mVideoSurface.resumePlayer();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mVideoSurface.stopPlayer();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        switch (requestCode){
            case 1:
                if(resultCode == RESULT_OK){
                    Log.v("mediaplayer","anchor:" + data.getData().toString());
                }
        }
        Log.v("mediaplayer","anchor_2");
    }

    private void pickVedio(){
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("video/mp4");
        startActivityForResult(intent,1);
        Log.v("mediaplayer","anchor_1");
    }
}


