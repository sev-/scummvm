package org.iphsoft.simon1.ui;

import com.mojotouch.simon.R;

import android.app.Activity;

public class ActivityAnimationUtil
{
	/*
	 * Convenience for setting activity transitions for the splash screen
	 */
	public static void makeSplashActivityFadeTransition(Activity activity)
	{
		activity.overridePendingTransition(R.anim.splash_fade_in, R.anim.splash_fade_out);
	}
	
	/*
     * Convenience for setting activity transitions for the app
     */
    public static void makeActivityFadeTransition(Activity activity)
    {
        activity.overridePendingTransition(R.anim.activity_fade_in, R.anim.activity_fade_out);
    }
    
    /*
     * Convenience for setting activity transitions for the app
     */
    public static void makeActivityNullTransition(Activity activity)
    {
        activity.overridePendingTransition(0, 0);
    }
}
