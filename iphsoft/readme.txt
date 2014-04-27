README

Instructions for building the Android verion (Google Play) of Simon the Sorcerer: 20th Anniversary edition from the source code archive:

1. Creating the GAME DATA Expension file

	Create OBB expansion file and place it in the path needed according to Google's APK expansion file documentation:
	http://developer.android.com/google/play/expansion-files.html

	Place the English game data files in root folder and other languages in sub-folders: De, De_sub, Fr, Es, It, He

2. Building the native library

	a. Create a "static_libs" dir in the project root.
	b. Use the following link to generate the static library dependencies for ScummVM (MAD and Tremor): http://wiki.scummvm.org/index.php/Compiling_ScummVM/Android
	c. Place the static libraries and the header files in the "static_libs" dir.
	d. Compile the project's native library using Android NDK toolchain.

3. Building the APK

	a. Use the project's root folder as a standard Android project in your IDE

	b. Make sure you have the following 3 library projects in your environment:

	Licensing
	Downloader
	Zip File

	c. Set the project to depend on those 3 libraries.

	d. You need to manually add the following code to DownloadsDB in Google's Downloader library:

	  public static void clearAllData(Context context)
		{
			if (mDownloadsDB != null)
			{
				SQLiteDatabase sqldb = mDownloadsDB.mHelper.getWritableDatabase();

				sqldb.delete(MetadataColumns.TABLE_NAME, "1", null);

				sqldb.close();
				
				mDownloadsDB.mHelper.close();
				
				mDownloadsDB = null;
			}
		}

	e. Add your own tuturial video, font and graphics files for Menu, pop-ups and icon.

	f. Build the APK using the Android toolchain

4. Run the project

