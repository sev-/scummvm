package org.iphsoft.simon1.util;

import org.iphsoft.simon1.ScummVMApplication;

import android.content.pm.PackageManager.NameNotFoundException;

/**
 * Utility for querying the package version
 * 
 * @author omergilad
 *
 */
public class Version {

	public static String getPackageVersion() {
		try {
			return ScummVMApplication.getContext().getPackageManager()
					.getPackageInfo(ScummVMApplication.getContext().getPackageName(), 0).versionName;
		} catch (NameNotFoundException e) {
			return null;
		}
	}
}
