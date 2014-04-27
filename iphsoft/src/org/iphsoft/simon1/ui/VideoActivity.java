/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.iphsoft.simon1.ui;

import org.iphsoft.simon1.GameMenuActivity;
import org.iphsoft.simon1.util.MyLog;

import android.app.Activity;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.net.Uri;
import android.os.Bundle;
import android.view.WindowManager;
import android.widget.VideoView;

import com.mojotouch.simon.R;

/**
 * Utility activity for displaying video
 */
public class VideoActivity extends Activity {

    public static final String EXTRA_VIDEO_RESOURCE = "EXTRA_VIDEO_RESOURCE";

    private int mVideoResource;
    private VideoView mVideoView;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.video_activity);

        mVideoResource = getIntent().getIntExtra(EXTRA_VIDEO_RESOURCE, 0);
        MyLog.d("VideoActivity: onCreate: " + mVideoResource);

        mVideoView = (VideoView) findViewById(R.id.video_view);
        String uri = "android.resource://" + getPackageName() + "/" + mVideoResource;

        mVideoView.setVideoURI(Uri.parse(uri));
        mVideoView.requestFocus();

        mVideoView.setOnCompletionListener(new OnCompletionListener() {

            @Override
            public void onCompletion(MediaPlayer arg0) {
                VideoActivity.this.finish();
                ActivityAnimationUtil.makeActivityFadeTransition(VideoActivity.this);
            }
        });


    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        super.onPause();

        mVideoView.pause();
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();

        mVideoView.start();
    }
}
