package org.iphsoft.simon1;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.iphsoft.simon1.stats.StatisticsManager;
import org.iphsoft.simon1.stats.IStatistics.SettingsInfo;
import org.iphsoft.simon1.ui.ActivityAnimationUtil;
import org.iphsoft.simon1.util.Appirater;
import org.iphsoft.simon1.util.DialogUtils;
import org.iphsoft.simon1.util.DialogUtils.InputListener;
import org.iphsoft.simon1.util.MyLog;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.text.format.DateFormat;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.ImageView;
import android.widget.Toast;
import com.mojotouch.simon.R;

public class ScummVMActivity extends Activity implements
		ScummVM.OnCompletionListener {

	public final static String EXTRA_STARTING_SAVE_SLOT = "org.iphsoft.simon1.EXTRA_STARTING_SAVE_SLOT";

	/* Establish whether the hover events are available */
	// private static boolean _hoverAvailable;
	/*
	 * static { try { MouseHelper.checkHoverAvailable(); // this throws
	 * exception if we're // on too old version _hoverAvailable = true; } catch
	 * (Throwable t) { _hoverAvailable = false; } }
	 */

	private class MyScummVM extends ScummVM {
		private boolean usingSmallScreen() {
			// Multiple screen sizes came in with Android 1.6. Have
			// to use reflection in order to continue supporting 1.5
			// devices :(
			DisplayMetrics metrics = new DisplayMetrics();
			getWindowManager().getDefaultDisplay().getMetrics(metrics);

			try {
				// This 'density' term is very confusing.
				int DENSITY_LOW = metrics.getClass().getField("DENSITY_LOW")
						.getInt(null);
				int densityDpi = metrics.getClass().getField("densityDpi")
						.getInt(metrics);
				return densityDpi <= DENSITY_LOW;
			} catch (Exception e) {
				return false;
			}
		}

		public MyScummVM(SurfaceHolder holder) {
			super(ScummVMActivity.this.getAssets(), holder);

			// Enable ScummVM zoning on 'small' screens.
			// FIXME make this optional for the user
			// disabled for now since it crops too much
			// enableZoning(usingSmallScreen());
		}

		@Override
		protected void getDPI(float[] values) {
			DisplayMetrics metrics = new DisplayMetrics();
			getWindowManager().getDefaultDisplay().getMetrics(metrics);

			values[0] = metrics.xdpi;
			values[1] = metrics.ydpi;
		}

		@Override
		protected void displayMessageOnOSD(String msg) {
			Log.i(LOG_TAG, "OSD: " + msg);
			Toast.makeText(ScummVMActivity.this, msg, Toast.LENGTH_LONG).show();
		}

		@Override
		protected void setWindowCaption(final String caption) {
			runOnUiThread(new Runnable() {
				public void run() {
					setTitle(caption);
				}
			});
		}

		@Override
		protected String[] getPluginDirectories() {
			String[] dirs = new String[1];
			dirs[0] = ScummVMApplication.getLastCacheDir().getPath();
			return dirs;
		}

		@Override
		protected void showVirtualKeyboard(final boolean enable) {
			runOnUiThread(new Runnable() {
				public void run() {
					showKeyboard(enable);
				}
			});
		}

		@Override
		protected void onGameOption(final int option) {
			runOnUiThread(new Runnable() {

				@Override
				public void run() {
					switch (option) {
					case AndroidPortAdditions.GAME_OPTION_SAVE:
						mSaveLoadFromPostcard = true;
						showSaveDialog();
						break;
					case AndroidPortAdditions.GAME_OPTION_LOAD:
						showLoadDialog();
						break;
					case AndroidPortAdditions.GAME_OPTION_EXIT:
						showDialog(DIALOG_ID_EXIT);
						break;
					}

				}
			});
		}

		@Override
		protected void onGameDisplayStarted() {
			// For the hebrew version, we display the splash screen right after
			// game display starts (to cover the vendor logo)
			if (mLanguagePref.equalsIgnoreCase("he") && mAutoloadSlot == -1) {
				// Run on the UI thread after delay
				Handler uiHandler = new Handler(getMainLooper());
				uiHandler.postDelayed(new Runnable() {

					@Override
					public void run() {
						displaySplashScreenHebrewVersion();
					}
				}, SPLASH_SCREEN_START_TIME_HEBREW_VERSION_MILLIS);
			}
		}

		@Override
		protected void gameEventJNIToJava(final int type) {

			MyLog.d("ScummVMActivity.MyScummVM: gameEventJNIToJava: " + type);

			runOnUiThread(new Runnable() {

				@Override
				public void run() {

					switch (type) {
					case AndroidPortAdditions.GAME_EVENT_SCALER_FALLBACK:
						// Set the scaling setting to software, and show a
						// toast
						AndroidPortAdditions.instance().setScalingOption(
								AndroidPortAdditions.SCALING_OPTION_SOFT);
						Toast.makeText(ScummVMActivity.this,
								R.string.graphic_setting_fallback_toast,
								Toast.LENGTH_LONG).show();
						AndroidPortAdditions.instance().setShaderTested(true);

						break;
					case AndroidPortAdditions.GAME_EVENT_SCALER_LQ_FALLBACK:
						// Set the scaling setting to LQ, and show a
						// toast
						AndroidPortAdditions.instance().setScalingOption(
								AndroidPortAdditions.SCALING_OPTION_LQ_SHADER);
						Toast.makeText(ScummVMActivity.this,
								R.string.graphic_setting_fallback_toast,
								Toast.LENGTH_LONG).show();
						AndroidPortAdditions.instance().setShaderTested(true);
						break;
					case AndroidPortAdditions.GAME_EVENT_SHADER_TEST_SUCCESS:

						// Shader test was successful
						AndroidPortAdditions.instance().setShaderTested(true);
						break;
					case AndroidPortAdditions.GAME_EVENT_SHADER_TEST_FAILURE:

						// Shader test failed - change the setting to
						// software
						// scaling and inform the user

						AndroidPortAdditions.instance().setScalingOption(
								AndroidPortAdditions.SCALING_OPTION_SOFT);
						Toast.makeText(ScummVMActivity.this,
								R.string.graphic_setting_fallback_toast,
								Toast.LENGTH_LONG).show();

						AndroidPortAdditions.instance().setShaderTested(true);
						break;
					case AndroidPortAdditions.GAME_EVENT_LOAD_SUCCESS:

						hideLoadProgressDialog();

						Toast.makeText(ScummVMActivity.this,
								R.string.load_success, Toast.LENGTH_LONG)
								.show();

						// We can possibly show the Appirater dialog here
						Appirater.appLaunched(ScummVMActivity.this);

						break;
					case AndroidPortAdditions.GAME_EVENT_LOAD_FAILURE:

						hideLoadProgressDialog();

						Toast.makeText(ScummVMActivity.this,
								R.string.load_failure, Toast.LENGTH_LONG)
								.show();

						break;

					case AndroidPortAdditions.GAME_EVENT_SAVE_SUCCESS:

						hideSaveProgressDialog();

						AndroidPortAdditions.instance().setSaveSlot(
								mChosenSaveSlot, mChosenSaveName);

						// Reset the game time without saving
						mGameTimeWithoutSave = 0;
						mLastResumeTimestamp = 0;

						// SaveReminderHelper.instance().setLastSaveTime(
						// System.currentTimeMillis());

						Toast.makeText(ScummVMActivity.this,
								R.string.save_success, Toast.LENGTH_LONG)
								.show();

						break;

					case AndroidPortAdditions.GAME_EVENT_SAVE_FAILURE:

						hideSaveProgressDialog();

						Toast toast = Toast.makeText(ScummVMActivity.this,
								R.string.save_failure, Toast.LENGTH_LONG);
						toast.setGravity(Gravity.CENTER, 0, 0);
						toast.show();

						break;

					case AndroidPortAdditions.GAME_EVENT_HEBREW_TITLE_GONE:

						// Clear the splash screen if the game doesn't
						// display he hebrew title anymore
						cleanSplash();

						break;

					case AndroidPortAdditions.GAME_EVENT_POSTCARD_SHOWN:

						showMenu(true);
						break;

					case AndroidPortAdditions.GAME_EVENT_SHOW_MENU:

						showMenu(false);
						break;
					}
				}
			});
		}

		@Override
		protected String[] getSysArchives() {
			return new String[0];
		}

	}

	private MyScummVM _scummvm;
	private ScummVMEvents _events;
	// private MouseHelper _mouseHelper;
	private Thread _scummvm_thread;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		mUiHandler = new Handler();

		AndroidPortAdditions.instance().onGameActivityCreate();

		/*
		 * // Turn off soft buttons on Android 4.0+ try { Field
		 * lowProfileFlagField =
		 * View.class.getField("SYSTEM_UI_FLAG_LOW_PROFILE"); if
		 * (lowProfileFlagField != null) { int lowProfileFlag =
		 * (Integer)lowProfileFlagField.get(View.class); Method
		 * setSystemUiVisibility =
		 * getWindow().getDecorView().getClass().getMethod
		 * ("setSystemUiVisibility", Integer.TYPE); if (setSystemUiVisibility !=
		 * null) { setSystemUiVisibility.invoke(getWindow().getDecorView(),
		 * lowProfileFlag); } } } catch (Exception e) { // TODO Auto-generated
		 * catch block e.printStackTrace(); }
		 */

		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		setVolumeControlStream(AudioManager.STREAM_MUSIC);

		setContentView(R.layout.main);
		takeKeyEvents(true);

		// This is a common enough error that we should warn about it
		// explicitly.
		if (!Environment.getExternalStorageDirectory().canRead()
				|| !Environment.getExternalStorageDirectory().canWrite()) {

			DialogUtils.createNegativePositiveDialog(this, R.string.quit, 0,
					R.string.no_sdcard, new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int which) {
							finish();
							ActivityAnimationUtil
									.makeActivityFadeTransition(ScummVMActivity.this);

						}
					}, null).show();

			return;
		}

		SharedPreferences prefs = PreferenceManager
				.getDefaultSharedPreferences(this);

		mImgSplash = (ImageView) findViewById(R.id.imgSplash);

		SurfaceView main_surface = (SurfaceView) findViewById(R.id.main_surface);

		main_surface.requestFocus();

		getFilesDir().mkdirs();

		// Store savegames on external storage if we can, which means they're
		// world-readable and don't get deleted on uninstall.
		String savePath = Environment.getExternalStorageDirectory() + "/"
				+ AndroidPortAdditions.instance().getAppName() + "/Saves/";
		File saveDir = new File(savePath);
		saveDir.mkdirs();
		if (!saveDir.isDirectory()) {
			// If it doesn't work, show an error and quit
			Toast.makeText(this, R.string.no_sdcard, Toast.LENGTH_LONG);
			finish();
			ActivityAnimationUtil
					.makeActivityFadeTransition(ScummVMActivity.this);

			return;
		}

		// Start ScummVM
		_scummvm = new MyScummVM(main_surface.getHolder());

		// Receive callback for ScummVM completion
		_scummvm.setOnCompletionListener(this);

		ArrayList<String> args = new ArrayList<String>();

		args.add("ScummVM");
		args.add("--config=" + getFileStreamPath("scummvmrc").getPath());
		args.add("--path=" + AndroidPortAdditions.instance().getGameFileDir());
		args.add("--gui-theme=scummmodern");
		args.add("--savepath=" + savePath);

		// Edit command line args according to user selection
		// music mode
		String musicMode = prefs.getString("pref_music_mode", "enhanced");
		if (musicMode.equals("enhanced")) {
			args.add("--use-music=0");
		} else if (musicMode.equals("original")) {
			args.add("--use-music=1");
		} else if (musicMode.equals("none")) {
			args.add("--use-music=2");
		}

		// subtitles
		String voiceMode = prefs
				.getString("pref_voice_mode", "voice_subtitles");
		if (voiceMode.equals("voice_subtitles")) {
			args.add("--subtitles");
		} else if (voiceMode.equals("subtitles")) {
			args.add("--subtitles");
			args.add("--speech_mute");
		}

		// Language option
		mLanguagePref = prefs.getString("pref_language", null);
		String languageOption = "en";
		if (mLanguagePref != null) {
			if (mLanguagePref.equals("de"))
				languageOption = "de";
			else if (mLanguagePref.equals("he"))
				languageOption = "hb";
			else if (mLanguagePref.equals("es"))
				languageOption = "es";
			else if (mLanguagePref.equals("fr"))
				languageOption = "fr";
			else if (mLanguagePref.equals("it"))
				languageOption = "it";
		}

		args.add("--language=" + languageOption);

		// Set the slot to auto-load if there is
		mAutoloadSlot = getIntent().getIntExtra(EXTRA_STARTING_SAVE_SLOT, -1);
		MyLog.d("ScummVMActivity: onCreate: autoload slot: " + mAutoloadSlot);
		_scummvm.setAutoLoadSlot(mAutoloadSlot);
		if (mAutoloadSlot != -1) {
			showLoadProgressDialog();
		} else {
			// We can possibly show the Appirater dialog here
			Appirater.appLaunched(this);
		}

		// Set the game as Simon1
		switch (AndroidPortAdditions.GAME_TYPE) {
		case SIMON1:
			args.add("simon1");
			break;
		case SIMON2:
			args.add("simon2");
			break;
		case FOTAQ:
			args.add("queen");
			break;
		case INDY_FOA:
			args.add("atlantis");
			break;
		}

		String[] argsArray = args.toArray(new String[args.size()]);

		MyLog.d("ScummVMActivity: onCreate: args: "
				+ Arrays.toString(argsArray));

		_scummvm.setArgs(argsArray);

		_events = new ScummVMEvents(this, _scummvm);

		main_surface.setOnKeyListener(_events);
		main_surface.setOnTouchListener(_events);

		_scummvm_thread = new Thread(_scummvm, "ScummVM");

		// For hebrew version (with no auto-load), start the ScummVM thread
		// normally
		// For other versions, start it only after the splash screen
		if (mAutoloadSlot != -1) {
			_scummvm_thread.start();
			cleanSplash();
		} else if (mLanguagePref.equalsIgnoreCase("he")) {
			_scummvm_thread.start();
		} else {
			displaySplashScreenDefaultBehavior();
		}

		
		// Send game start statistics with all settings info
		SettingsInfo settingInfo = new SettingsInfo();
		settingInfo.mGraphicModeType = AndroidPortAdditions.instance().getAutoGraphicMode() ? "auto" : "manual";
		settingInfo.mGraphicMode = prefs.getString("pref_scaler_option", "unknown");
		settingInfo.mVoiceMode = voiceMode;
		settingInfo.mMusicMode = musicMode;
		settingInfo.mControlMode = prefs.getString("pref_control_mode", "unknown");
		settingInfo.mLanguage = mLanguagePref;
		
		StatisticsManager.getStatistics().reportGameStart(settingInfo);
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
	public void onResume() {
		Log.d(ScummVM.LOG_TAG, "onResume");

		super.onResume();

		// Support full-screen immersive mode on Android 4.4 and up
		if (Build.VERSION.SDK_INT >= 19) {
			supportFullScreenImmersiveMode();
		}

		if (_scummvm != null)
			_scummvm.setPause(false);

		// SaveReminderHelper.instance().setGamePaused(false);

		// Update the touch mode according to settings
		SharedPreferences prefs = PreferenceManager
				.getDefaultSharedPreferences(this);
		String controlMode = prefs.getString("pref_control_mode",
				"control_mode_touch");
		if (controlMode.equals("control_mode_touch")) {
			_scummvm.setTouchpadMode(false);
		} else if (controlMode.equals("control_mode_classic")) {
			_scummvm.setTouchpadMode(true);
		} else {
			MyLog.e("ScummVMActivity: onPause: unknown control mode: "
					+ controlMode);
		}

		mLastResumeTimestamp = System.currentTimeMillis();
	}

	@Override
	public void onPause() {
		Log.d(ScummVM.LOG_TAG, "onPause");

		super.onPause();

		if (_scummvm != null)
			_scummvm.setPause(true);

		if (mLastResumeTimestamp != 0) {
			mGameTimeWithoutSave += (System.currentTimeMillis() - mLastResumeTimestamp);
		}

		MyLog.d("ScummVMActivity: onPause: game time without save "
				+ mGameTimeWithoutSave);

		// Display save reminder if pausing this activity and a certain game
		// time has passed without saving
		if (mGameTimeWithoutSave >= MIN_GAP_FOR_SAVE_REMINDER) {
			String appName = AndroidPortAdditions.instance().getAppName();
			String message = ScummVMActivity.this.getString(
					R.string.save_game_reminder, appName);

			Toast.makeText(ScummVMActivity.this, message, Toast.LENGTH_LONG)
					.show();
		}

		// SaveReminderHelper.instance().setGamePaused(true);
		// SaveReminderHelper.instance().setLastPauseTime(System.currentTimeMillis());
	}

	@Override
	public void onDestroy() {
		Log.d(ScummVM.LOG_TAG, "onDestroy");

		AndroidPortAdditions.instance().onGameActivityDestroy();

		super.onDestroy();
		/*
		 * if (_events != null) {
		 * MyLog.d("ScummVMActivity: onDestroy: sending quit event");
		 * _events.sendQuitEvent(); try {
		 * MyLog.d("ScummVMActivity: onDestroy: joining ScummVM thread"); // 5s
		 * timeout _scummvm_thread.join(5000); } catch (InterruptedException e)
		 * { Log.i(ScummVM.LOG_TAG, "Error while joining ScummVM thread", e); }
		 * } _scummvm = null;
		 */

		MyLog.d("ScummVMActivity: onDestroy: end");

	}

	@Override
	public boolean onTrackballEvent(MotionEvent e) {
		if (_events != null)
			return _events.onTrackballEvent(e);

		return false;
	}

	private void showKeyboard(boolean show) {
		SurfaceView main_surface = (SurfaceView) findViewById(R.id.main_surface);
		InputMethodManager imm = (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);

		if (show)
			imm.showSoftInput(main_surface, InputMethodManager.SHOW_IMPLICIT);
		else
			imm.hideSoftInputFromWindow(main_surface.getWindowToken(),
					InputMethodManager.HIDE_IMPLICIT_ONLY);
	}

	/*
	 * @Override public boolean onCreateOptionsMenu(Menu menu) { MenuInflater
	 * inflater = getMenuInflater(); inflater.inflate(R.menu.main_menu, menu);
	 * return true; }
	 */
	/*
	 * @Override public boolean onOptionsItemSelected(MenuItem item) { switch
	 * (item.getItemId()) { case R.id.settings: startActivityForResult(new
	 * Intent(this, SettingsActivity.class), SETTINGS_ACTIVITY_REQUEST_CODE);
	 * return true; case R.id.help: startActivity(new Intent(this,
	 * HelpActivity.class)); return true; case R.id.save: showSaveDialog();
	 * return true; case R.id.load: showLoadDialog(); return true; default:
	 * return super.onOptionsItemSelected(item); } }
	 */

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {

		// Restart if needed (settings changed)
		if (requestCode == GAME_MENU_REQUEST_CODE) {
			int loadSlot = -1;

			if (data != null) {
				loadSlot = data.getIntExtra(GameMenuActivity.EXTRA_SAVE_SLOT,
						-1);
			}

			switch (resultCode) {
			case GameMenuActivity.RESULT_REQUEST_RESTART:
				restart(-1);
				return;
			case GameMenuActivity.RESULT_REQUEST_SAVE:
				mSaveLoadFromPostcard = false;

				showSaveDialog();
				return;
			case GameMenuActivity.RESULT_REQUEST_QUIT:
				// We show a confirmation dialog before exiting
				showDialog(DIALOG_ID_EXIT);
				return;
			case GameMenuActivity.RESULT_REQUEST_LOAD:

				if (_scummvm.checkLoadConditions()) {
					// If we can load mid-game, do it
					_scummvm.loadGame(loadSlot);
				} else {
					// If not, restart
					restart(loadSlot);
				}

				return;
			case GameMenuActivity.RESULT_REQUEST_RESTART_LOAD:
				restart(loadSlot);

				return;

			}
		}

		super.onActivityResult(requestCode, resultCode, data);
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {

		if (keyCode == KeyEvent.KEYCODE_BACK) {
			// We show a confirmation dialog before exiting
			showDialog(DIALOG_ID_EXIT);
			return true;
		} else if (keyCode == KeyEvent.KEYCODE_MENU) {
			// Show the game menu
			showMenu(false);
			return true;
		}

		return super.onKeyUp(keyCode, event);
	}

	@Override
	protected Dialog onCreateDialog(int id) {

		switch (id) {
		case DIALOG_ID_EXIT:

			Dialog dlg = DialogUtils.createNegativePositiveDialog(this,
					R.string.no, R.string.yes,
					R.string.exit_confirmation_message, null,
					new OnClickListener() {

						@Override
						public void onClick(DialogInterface arg0, int arg1) {
							exit();
							finish();
							ActivityAnimationUtil
									.makeActivityFadeTransition(ScummVMActivity.this);

						}
					});

			return dlg;

		default:
			return super.onCreateDialog(id);

		}

	}

	private void showMenu(boolean fromPostcard) {
		Intent gameMenu = new Intent(this, GameMenuActivity.class);
		gameMenu.putExtra(GameMenuActivity.EXTRA_MID_GAME, true);
		gameMenu.putExtra(GameMenuActivity.EXTRA_FROM_POSTCARD, fromPostcard);
		startActivityForResult(gameMenu, GAME_MENU_REQUEST_CODE);

		ActivityAnimationUtil.makeActivityFadeTransition(this);
	}

	private void showSaveDialog() {
		mSaveSlots = AndroidPortAdditions.instance().getSaveSlots()
				.toArray(new String[0]);

		Dialog dlg = DialogUtils.createListDialog(this, R.string.cancel,
				R.string.choose_save_slot, new OnClickListener() {

					@Override
					public void onClick(DialogInterface dlg, int which) {
						// Convert to slot number
						mChosenSaveSlot = which + 1;

						// Proceed to edit save name
						showSaveNameDialog();

						dlg.dismiss();
					}
				}, mSaveSlots);

		dlg.show();
	}

	private void showLoadDialog() {

		// Generate a list of save slots to display
		List<String> allSaveSlots = AndroidPortAdditions.instance()
				.getSaveSlots();
		mSaveSlotIndexes = AndroidPortAdditions.instance()
				.getSaveSlotsIndexes();
		if (mSaveSlotIndexes.size() == 0) {
			// Nothing to load
			Toast.makeText(this, R.string.no_saved_games, Toast.LENGTH_LONG)
					.show();
			return;
		}
		List<String> displayedSaveSlots = new ArrayList<String>();

		for (Integer i : mSaveSlotIndexes) {
			displayedSaveSlots.add(allSaveSlots.get(i));
		}

		Dialog dlg = DialogUtils.createListDialog(this, R.string.cancel,
				R.string.choose_load_slot, new OnClickListener() {

					@Override
					public void onClick(DialogInterface dlg, int which) {
						// Load the game (convert index to save slot)
						_scummvm.loadGame(mSaveSlotIndexes.get(which) + 1);
						dlg.dismiss();
					}
				}, displayedSaveSlots.toArray(new String[0]));

		dlg.show();
	}

	private void showSaveNameDialog() {
		// Setup the initial input, depending on whether the save slot is
		// existing or new
		String initialInput;
		List<Integer> indexes = AndroidPortAdditions.instance()
				.getSaveSlotsIndexes();
		if (indexes.contains(mChosenSaveSlot - 1)) {
			// Show the current save name
			initialInput = mSaveSlots[mChosenSaveSlot - 1];
		} else {

			initialInput = "";

			// Show time and date
			// initialInput = DateFormat.format("MMM dd yyyy h:mmaa",
			// System.currentTimeMillis())
			// .toString();
		}

		Dialog dlg = DialogUtils.createNegativePositiveDialog(this,
				R.string.cancel, R.string.ok, R.string.enter_save_name,
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int whichButton) {
						// Canceled.
						dialog.dismiss();
					}
				}, null, new InputListener() {
					public void onInputReceived(String input) {

						mChosenSaveName = input.toUpperCase();
						showSaveProgressDialog();
						_scummvm.saveGame(mChosenSaveSlot,
								mSaveLoadFromPostcard);
					}
				}, initialInput);

		dlg.show();
	}

	private void showSaveProgressDialog() {
		mSaveProgressDialog = DialogUtils.createProgressDialog(this,
				R.string.save_progress_dialog);
		mSaveProgressDialog.show();
	}

	private void hideSaveProgressDialog() {
		if (mSaveProgressDialog != null && mSaveProgressDialog.isShowing()) {
			mSaveProgressDialog.dismiss();
		}
	}

	private void showLoadProgressDialog() {
		mLoadProgressDialog = DialogUtils.createProgressDialog(this,
				R.string.load_progress_dialog);
		mLoadProgressDialog.show();
	}

	private void hideLoadProgressDialog() {
		if (mLoadProgressDialog != null && mLoadProgressDialog.isShowing()) {
			mLoadProgressDialog.dismiss();
		}
	}

	private void exit() {
		MyLog.d("ScummVMActivity: exit: ");

		mExiting = true;

		if (_scummvm != null) {
			_scummvm.setOnCompletionListener(null);
		}

		if (_events != null) {
			_events.sendQuitEvent();
			try {
				MyLog.d("ScummVMActivity: exit: joining");
				_scummvm_thread.join();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		_scummvm = null;

		mLastResumeTimestamp = 0;
		mGameTimeWithoutSave = 0;

		// SaveReminderHelper.instance().stopReminders();
	}

	private void restart(int loadSlot) {
		MyLog.d("ScummVMActivity: restart: ");

		_scummvm.setRestarting();
		_scummvm.setPause(false);
		exit();
		finish();

		Intent loadIntent = new Intent(this, PrepareGameFilesActivity.class);
		if (loadSlot != -1) {
			loadIntent.putExtra(ScummVMActivity.EXTRA_STARTING_SAVE_SLOT,
					loadSlot);
		}
		startActivity(loadIntent);
		ActivityAnimationUtil.makeActivityFadeTransition(ScummVMActivity.this);

	}

	private void displaySplashScreenDefaultBehavior() {
		MyLog.d("ScummVMActivity: displaySplashScreenDefaultBehavior: ");

		mImgSplash.setVisibility(View.VISIBLE);

		new Handler().postDelayed(new Runnable() {

			@Override
			public void run() {
				_scummvm_thread.start();
				cleanSplash();

			}
		}, SPLASH_SCREEN_TIME_MILLIS);
	}

	private void displaySplashScreenHebrewVersion() {
		MyLog.d("ScummVMActivity: displaySplashScreenHebrewVersion: ");

		mImgSplash.setVisibility(View.VISIBLE);
	}

	private void cleanSplash() {
		mImgSplash.setVisibility(View.GONE);
		mImgSplash.setImageBitmap(null);
	}

	/**
	 * Called when ScummVM finished running
	 */
	@Override
	public void onComplete() {

		MyLog.d("ScummVMActivity: onComplete: ");

		runOnUiThread(new Runnable() {

			@Override
			public void run() {
				if (!mExiting) {
					// If we didn't intentionally exit, the game ended - show
					// the game menu
					exit();
					finish();
					startActivity(new Intent(ScummVMActivity.this,
							GameMenuActivity.class).putExtra(
							GameMenuActivity.EXTRA_MID_GAME, false));
					ActivityAnimationUtil
							.makeActivityFadeTransition(ScummVMActivity.this);
				}
			}
		});

	}

	@SuppressLint("NewApi")
	private void supportFullScreenImmersiveMode() {
		MyLog.d("ScummVMActivity: supportFullScreenImmersiveMode: ");

		// Get the needed flags by reflection and use them
		try {
			final int immersiveFlag = View.class.getField(
					"SYSTEM_UI_FLAG_IMMERSIVE_STICKY").getInt(null);
			final int hideNavigationFlag = View.class.getField(
					"SYSTEM_UI_FLAG_HIDE_NAVIGATION").getInt(null);

			// Set the flags to the window decor view
			getWindow().getDecorView().setSystemUiVisibility(
					immersiveFlag | hideNavigationFlag);

			// Set a callback to be called when visibility changes (workaround
			// for volume keys)
			getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(
					new View.OnSystemUiVisibilityChangeListener() {
						@Override
						public void onSystemUiVisibilityChange(int visibility) {
							if ((visibility & (immersiveFlag | hideNavigationFlag)) == 0) {
								mUiHandler
										.removeCallbacks(mHideSystemUiCallback);
								mUiHandler.postDelayed(mHideSystemUiCallback,
										HIDE_SYSTEM_UI_DELAY_MILLI);
							}
						}
					});

		} catch (Exception e) {
			e.printStackTrace();
			MyLog.e("ScummVMActivity: supportFullScreenImmersiveMode: couldn't support immersive mode by reflection");
		}
	}

	private final Runnable mHideSystemUiCallback = new Runnable() {
		@Override
		public void run() {
			supportFullScreenImmersiveMode();
		}
	};

	private Handler mUiHandler;

	private String[] mSaveSlots;
	private List<Integer> mSaveSlotIndexes;

	private int mChosenSaveSlot = 0;
	private String mChosenSaveName = "";

	private ImageView mImgSplash;

	private String mLanguagePref;

	private int mAutoloadSlot = -1;

	private Dialog mSaveProgressDialog;
	private Dialog mLoadProgressDialog;

	private boolean mSaveLoadFromPostcard = false;

	private long mGameTimeWithoutSave = 0;
	private long mLastResumeTimestamp = 0;

	private boolean mExiting = false;

	private static final int GAME_MENU_REQUEST_CODE = 1;

	private static final int DIALOG_ID_EXIT = 1;

	private static final long SPLASH_SCREEN_TIME_MILLIS = 3000;

	private static final long SPLASH_SCREEN_TIME_HEBREW_VERSION_MILLIS = 6000;
	private static final long SPLASH_SCREEN_START_TIME_HEBREW_VERSION_MILLIS = 0;

	public static final long MIN_GAP_FOR_SAVE_REMINDER = 5 * 60 * 1000;

	private static final int HIDE_SYSTEM_UI_DELAY_MILLI = 500;

}
