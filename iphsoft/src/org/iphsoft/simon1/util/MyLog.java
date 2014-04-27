package org.iphsoft.simon1.util;

import android.util.Log;

/**
 * Wrapper for the system logger
 * 
 * @author omergilad
 *
 */
public class MyLog {

	public static void v(String msg)
	{
		//Log.v(TAG, msg);
	}
	
	public static void d(String msg)
	{
			//Log.d(TAG, msg);
	}
	
	public static void i(String msg)
	{
		Log.i(TAG, msg);
	}
	
	public static void w(String msg)
	{
		Log.w(TAG, msg);
	}
	
	public static void e(String msg)
	{
		Log.e(TAG, msg);
	}
	
	private static String TAG = "Simon";
}
