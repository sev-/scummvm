package org.iphsoft.simon1.util;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;

/**
 * Convenience for posting background runnables to a singleton Handler
 * 
 * @author omergilad
 */
public class BackgroundHandler extends Handler
{
	public static synchronized BackgroundHandler instance()
	{
		if (sInstance == null)
		{
			sHandlerThread = new HandlerThread("background_handler");
			sHandlerThread.start();
			sInstance = new BackgroundHandler(sHandlerThread.getLooper());
		}
		
		return sInstance;
	}
	
	private BackgroundHandler(Looper looper)
	{
		super(looper);
	}
	
	private static HandlerThread sHandlerThread;
	private static BackgroundHandler sInstance;
}