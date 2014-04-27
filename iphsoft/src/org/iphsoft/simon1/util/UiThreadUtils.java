package org.iphsoft.simon1.util;

import org.iphsoft.simon1.ScummVMApplication;

import android.os.Handler;
import android.widget.Toast;

/**
 * Utils for dealing with the UI thread
 * 
 * @author omergilad
 *
 */
public class UiThreadUtils {

	/**
	 * Show a toast from anywhere, saving the need to get a context or UI thread.
	 * 
	 * @param message
	 * @param duration
	 */
	public static void showBackgroundToast(final int message, final int duration) {

		new Handler(ScummVMApplication.getContext().getMainLooper())
				.post(new Runnable() {

					public void run() {

						Toast.makeText(ScummVMApplication.getContext(), message,
								duration).show();

					};
				});
	}
}
