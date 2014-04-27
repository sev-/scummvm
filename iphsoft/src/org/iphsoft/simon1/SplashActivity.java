
package org.iphsoft.simon1;

import org.iphsoft.simon1.stats.StatisticsManager;
import org.iphsoft.simon1.ui.ActivityAnimationUtil;
import org.iphsoft.simon1.util.TestGLActivity;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.Animation.AnimationListener;
import android.view.animation.AnimationUtils;
import com.mojotouch.simon.R;

public class SplashActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.splash);
        
        // Start a background check for ad policy
        AdHelper.startBackgroundAdCheck();

        // Start fade out animation after a delay
        mHandler = new Handler();
        mAnimateRunnable = new Runnable() {

            @Override
            public void run() {
                // Start the animation, and switch to the next activity when it
                // ends
                View insideView = findViewById(R.id.splashCenterLayout);

                Animation fadeoutAnim = AnimationUtils.loadAnimation(SplashActivity.this,
                        R.anim.splash_inside_fade_out);

                fadeoutAnim.setAnimationListener(new AnimationListener() {

                    @Override
                    public void onAnimationStart(Animation arg0) {
                    }

                    @Override
                    public void onAnimationRepeat(Animation arg0) {
                    }

                    @Override
                    public void onAnimationEnd(Animation arg0) {
                        nextActivity();
                    }
                });

                insideView.startAnimation(fadeoutAnim);
            }
        };

        if (AndroidPortAdditions.instance().getFirstUse())
        {
            // If this is the first run, launch an activity to query the
            // GL_RENDERER string
            startActivityForResult(new Intent(this, TestGLActivity.class), GL_TEST_REQUEST_CODE);
        }
        else
        {
            // If this is not the first run, queue the runnable normally

            mHandler.postDelayed(mAnimateRunnable, DELAY);
        }
    }

    @Override
    protected void onDestroy() {

        // Remove all pending callbacks (in case user pressed back and next
        // activity wasn't launched yet)
        mHandler.removeCallbacks(mAnimateRunnable);
        super.onDestroy();
    }
    
    
    @Override
    protected void onStart() {
    	super.onStart();
    	
    	StatisticsManager.getStatistics().reportActivityStart(this);
    }
    
    @Override
    protected void onStop() {
    	super.onStop();
    	
    	StatisticsManager.getStatistics().reportActivityStop(this);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {

        // Check for the GL_RENDERER result
        if (requestCode == GL_TEST_REQUEST_CODE && resultCode == RESULT_OK && data != null)
        {
            String glRenderer = data.getStringExtra(TestGLActivity.EXTRA_GL_RENDERER);
            if (glRenderer != null)
            {
                AndroidPortAdditions.instance().setGlRendererPref(glRenderer);
            }
        }

        // Queue the splash animation
        mHandler.postDelayed(mAnimateRunnable, DELAY);

        super.onActivityResult(requestCode, resultCode, data);
    }

    private void nextActivity()
    {
        startActivity(new Intent(this, GameMenuActivity.class));
        finish();
        ActivityAnimationUtil.makeSplashActivityFadeTransition(this);
    }

    private Handler mHandler;
    private Runnable mAnimateRunnable;

    private static final int DELAY = 1500;

    private static final int GL_TEST_REQUEST_CODE = 1;
}
