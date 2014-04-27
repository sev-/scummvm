package org.iphsoft.simon1;

import java.io.File;

import android.app.Application;
import android.content.Context;

public class ScummVMApplication extends Application {
	public final static String ACTION_PLUGIN_QUERY = "org.iphsoft.simon1.action.PLUGIN_QUERY";
	public final static String EXTRA_UNPACK_LIBS = "org.iphsoft.simon1.extra.UNPACK_LIBS";
	public final static String EXTRA_VERSION = "org.iphsoft.simon1.extra.VERSION";

	private static File _cache_dir;

	private static Context _context;

	@Override
	public void onCreate() {
		super.onCreate();

		_context = this;

		// This is still on /data :(
		_cache_dir = getCacheDir();
		// This is mounted noexec :(
		// cache_dir = new File(Environment.getExternalStorageDirectory(),
		// "/.ScummVM.tmp");
		// This is owned by download manager and requires special
		// permissions to access :(
		// cache_dir = Environment.getDownloadCacheDirectory();
	}

	public static File getLastCacheDir() {
		return _cache_dir;
	}

	public static Context getContext() {
		return _context;
	}
}
