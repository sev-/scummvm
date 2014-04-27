package org.iphsoft.simon1;

import java.util.ArrayList;
import java.util.List;

import org.iphsoft.simon1.stats.StatisticsManager;
import org.iphsoft.simon1.util.CsvUtils;
import org.iphsoft.simon1.util.MyLog;

import android.content.Context;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageManager;
import android.os.Build;
import android.preference.PreferenceManager;

public class AndroidPortAdditions
{

	public enum GraphicModeBehavior
	{
		SOFTWARE_SCALER, HARDWARE_SCALER, TEST_HARDWARE_SCALER, LQ_HARDWARE_SCALER, TEST_LQ_HARDWARE_SCALER
	}

	public enum AppStore
	{
		GOOGLE_PLAY, AMAZON, SAMSUNG
	}

	public enum GameType
	{
		SIMON1(0), SIMON2(1), FOTAQ(2), INDY_FOA(3);

		public int getValue()
		{
			return mValue;
		}

		private GameType(int value)
		{
			mValue = value;
		}

		private int mValue;
	}

	public static GameType GAME_TYPE = GameType.SIMON1;

	// Used for testing shaders
	public static final boolean USE_SHADER_FILES = false;

	public static final AppStore APP_STORE = AppStore.GOOGLE_PLAY;

	// True if the game files are inside the APK
	public static final boolean STANDALONE_APK = (APP_STORE == AppStore.GOOGLE_PLAY) ? false
			: true;

	private static String sTempGameFileDir;
	static
	{
		switch (GAME_TYPE)
		{
		case SIMON1:
			sTempGameFileDir = "/simon1";
			break;
		case SIMON2:
			sTempGameFileDir = "/simon2";
			break;
		case FOTAQ:
			sTempGameFileDir = "/fotaq";
			break;
		case INDY_FOA:
			sTempGameFileDir = "/indy";
			break;
		}
	}

	public static final String GAME_FILE_DIR = sTempGameFileDir;

	public static final int GAME_OPTION_NONE = 0;
	public static final int GAME_OPTION_SAVE = 1;
	public static final int GAME_OPTION_LOAD = 2;
	public static final int GAME_OPTION_EXIT = 3;

	public static final int SCALING_OPTION_SHADER = 0;
	public static final int SCALING_OPTION_SOFT = 1;
	public static final int SCALING_OPTION_NONE = 2;
	public static final int SCALING_OPTION_LQ_SHADER = 3;

	public static final int GAME_EVENT_SCALER_FALLBACK = 0;
	public static final int GAME_EVENT_SHOULD_TEST_SHADER = 1;
	public static final int GAME_EVENT_SHADER_TEST_SUCCESS = 2;
	public static final int GAME_EVENT_SHADER_TEST_FAILURE = 3;
	public static final int GAME_EVENT_LOAD_SUCCESS = 4;
	public static final int GAME_EVENT_LOAD_FAILURE = 5;
	public static final int GAME_EVENT_SAVE_SUCCESS = 6;
	public static final int GAME_EVENT_SAVE_FAILURE = 7;
	public static final int GAME_EVENT_SAVE_CANCELLED = 8;
	public static final int GAME_EVENT_HEBREW_TITLE_GONE = 9;
	public static final int GAME_EVENT_POSTCARD_SHOWN = 10;
	public static final int GAME_EVENT_SHOW_MENU = 11;
	public static final int GAME_EVENT_SCALER_LQ_FALLBACK = 12;
	public static final int GAME_EVENT_USE_ULTRA_MODE = 13;

	public static synchronized AndroidPortAdditions instance()
	{
		if (sInstance == null)
			sInstance = new AndroidPortAdditions();

		return sInstance;
	}

	public String getAppName()
	{
		Context appContext = ScummVMApplication.getContext();
		PackageManager pm = appContext.getPackageManager();
		CharSequence appName = pm.getApplicationLabel(appContext
				.getApplicationInfo());
		return appName.toString();
	}

	public boolean getAutoGraphicMode()
	{
		return PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).getBoolean(
				AUTO_GRAPHIC_OPTION_PREF, true);
	}
	
	public void setAutoGraphicMode(boolean value)
	{
		PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).edit().putBoolean(
				AUTO_GRAPHIC_OPTION_PREF, value).commit();
	}

	public int getScalingOption()
	{
		String scaleOption = PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).getString(SCALING_OPTION_PREF,
				"scaling_option_shader");

		if (scaleOption.equals("scaling_option_shader"))
		{
			return SCALING_OPTION_SHADER;
		}
		if (scaleOption.equals("scaling_option_lq_shader"))
		{
			return SCALING_OPTION_LQ_SHADER;
		}
		if (scaleOption.equals("scaling_option_software"))
		{
			return SCALING_OPTION_SOFT;
		}
		if (scaleOption.equals("scaling_option_none"))
		{
			return SCALING_OPTION_NONE;
		}

		throw new IllegalStateException("invalid scaling option " + scaleOption);
	}

	public void setScalingOption(int option)
	{
		String scaleOption;
		switch (option)
		{
		case SCALING_OPTION_SHADER:
			scaleOption = "scaling_option_shader";
			break;
		case SCALING_OPTION_LQ_SHADER:
			scaleOption = "scaling_option_lq_shader";
			break;
		case SCALING_OPTION_SOFT:
			scaleOption = "scaling_option_software";
			break;
		case SCALING_OPTION_NONE:
			scaleOption = "scaling_option_none";
			break;
		default:
			throw new IllegalStateException("invalid scaling option " + option);
		}

		PreferenceManager
				.getDefaultSharedPreferences(ScummVMApplication.getContext())
				.edit().putString(SCALING_OPTION_PREF, scaleOption).commit();
	}

	public boolean getFirstUse()
	{
		return PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).getBoolean(FIRST_USE_PREF,
				true);
	}

	public void setFirstUse(boolean value)
	{
		Editor editor = PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).edit();
		editor.putBoolean(FIRST_USE_PREF, value);
		editor.commit();
	}

	public boolean wasTutorialOffered()
	{
		return PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).getBoolean(
				OFFERED_TUTORIAL_PREF, false);
	}

	public void setTutorialOffered(boolean value)
	{
		Editor editor = PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).edit();
		editor.putBoolean(OFFERED_TUTORIAL_PREF, value);
		editor.commit();
	}

	public String getGlRendererPref()
	{
		return PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext())
				.getString(GL_RENDERER_PREF, "");
	}

	public void setGlRendererPref(String value)
	{
		Editor editor = PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).edit();
		editor.putString(GL_RENDERER_PREF, value);
		editor.commit();
	}

	public boolean getShaderTested()
	{
		return PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).getBoolean(SHADER_TESTED_PREF,
				false);
	}

	public void setShaderTested(boolean value)
	{
		Editor editor = PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).edit();
		editor.putBoolean(SHADER_TESTED_PREF, value);
		editor.commit();
	}
	
	public String getAdGame()
	{
		return PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).getString(
				AD_POLICY_PREF, "");
	}

	public void setAdGame(String game)
	{
		Editor editor = PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).edit();
		editor.putString(AD_POLICY_PREF, game);
		editor.commit();
	}

	public boolean getGraphicSettingPromptShown()
	{
		return PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).getBoolean(
				GRAPHIC_SETTING_PROMPT_PREF, false);
	}

	public void setGraphicSettingPromptShown(boolean value)
	{
		Editor editor = PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).edit();
		editor.putBoolean(GRAPHIC_SETTING_PROMPT_PREF, value);
		editor.commit();
	}

	public int getLastSaveSlot()
	{
		return PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext())
				.getInt(LAST_SAVE_SLOT_PREF, -1);
	}

	public List<String> getSaveSlots()
	{
		// Get the save slots preference as a list of strings
		return CsvUtils.csvToStringList(PreferenceManager
				.getDefaultSharedPreferences(ScummVMApplication.getContext())
				.getString(SAVE_SLOTS_PREF, sDefaultSaveSlots));
	}

	public List<Integer> getSaveSlotsIndexes()
	{
		// Get the save slots preference as a list of strings
		return CsvUtils.csvToIntList(PreferenceManager
				.getDefaultSharedPreferences(ScummVMApplication.getContext())
				.getString(SAVE_SLOTS_INDEXES_PREF, ""));
	}

	public void setSaveSlot(int slot, String name)
	{
		if (slot < 1 || slot > MAX_SAVE_SLOTS)
		{
			throw new IllegalArgumentException(
					"Illegal save slot, must be 0 - " + MAX_SAVE_SLOTS);
		}

		// Modify the slot to the chosen name
		List<String> saveSlots = getSaveSlots();
		saveSlots.set(slot - 1, name);

		// Add the slot index if needed
		Integer index = Integer.valueOf(slot - 1);
		List<Integer> slotIndexes = getSaveSlotsIndexes();
		if (!slotIndexes.contains(index))
		{
			slotIndexes.add(index);
		}

		// Commit to preferences
		Editor editor = PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).edit();
		editor.putString(SAVE_SLOTS_PREF, CsvUtils.stringListToCsv(saveSlots));
		editor.putString(SAVE_SLOTS_INDEXES_PREF,
				CsvUtils.intListToCsv(slotIndexes));
		// Update the last save slot
		editor.putInt(LAST_SAVE_SLOT_PREF, slot);
		editor.commit();
	}

	public long getGameFilesSize()
	{
		return PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).getLong(GAME_FILES_SIZE_PREF,
				-1);
	}

	public void setGameFilesSize(long value)
	{
		Editor editor = PreferenceManager.getDefaultSharedPreferences(
				ScummVMApplication.getContext()).edit();
		editor.putLong(GAME_FILES_SIZE_PREF, value);
		editor.commit();
	}

	public String getGameFileDir()
	{
		return sGameFileDir;
	}

	public void setGameFileParentDir(String parentDir)
	{
		sGameFileDir = parentDir + GAME_FILE_DIR;
	}

	public int getAPKExtensionFileVersion()
	{
		return sExtensionFileVersion;
	}

	public void setAPKExtensionFileVersion(int version)
	{
		sExtensionFileVersion = version;
	}

	/**
	 * Used for keeping track of whether the game activity is alive in this
	 * process
	 * 
	 * @return
	 */
	public boolean isGameActivityAlive()
	{
		return (mGameActivityCount > 0);
	}

	public void onGameActivityCreate()
	{
		++mGameActivityCount;
	}

	public void onGameActivityDestroy()
	{
		--mGameActivityCount;
	}

	public boolean checkUltraModeSupport()
	{
		String model = Build.MODEL;
		// Check if the device is in our ultra mode supported models
		for (String s : DeviceConfigurations.DEVICES_USING_ULTRA_MODE)
		{
			if (s.equalsIgnoreCase(model))
			{
				return true;
			}
		}

		return false;
	}

	public GraphicModeBehavior getGraphicModeBehavior()
	{
		String model = Build.MODEL;
		String deviceGlRenderer = getGlRendererPref();

		MyLog.d("AndroidPortAdditions: getGraphicModeBehavior: model " + model);
		MyLog.d("AndroidPortAdditions: getGraphicModeBehavior: GL_RENDERER "
				+ deviceGlRenderer);
		
		// Send statistics of device and GPU
		StatisticsManager.getStatistics().reportDeviceAndGPU(model, deviceGlRenderer);

		// Check if the device is in our hardware scaler supported models
		for (String s : DeviceConfigurations.DEVICES_USING_HARDWARE_SCALER)
		{
			if (s.equalsIgnoreCase(model))
			{
				// Check if the device also has a supported GPU
				for (String s2 : DeviceConfigurations.GPU_KINDS_USING_HARDWARE_SCALER_TEST)
				{
					if (s2.equalsIgnoreCase(deviceGlRenderer))
					{
						return GraphicModeBehavior.HARDWARE_SCALER;
					}
				}
			}
		}

		// Check if the device is in our LQ hardware scaler supported models
		for (String s : DeviceConfigurations.DEVICES_USING_LQ_HARDWARE_SCALER)
		{
			if (s.equalsIgnoreCase(model))
			{
				return GraphicModeBehavior.LQ_HARDWARE_SCALER;
			}
		}

		// Check if the device GL_RENDERER (GPU) is in our HW test list
		for (String s : DeviceConfigurations.GPU_KINDS_USING_HARDWARE_SCALER_TEST)
		{
			if (s.equalsIgnoreCase(deviceGlRenderer))
			{
				return GraphicModeBehavior.TEST_HARDWARE_SCALER;
			}
		}

		// Check if the GPU is in the SW scaler list
		for (String s : DeviceConfigurations.GPU_KINDS_USING_SOFTWARE_SCALER)
		{
			if (s.equalsIgnoreCase(deviceGlRenderer))
			{
				return GraphicModeBehavior.SOFTWARE_SCALER;
			}
		}

		// Default to LQ hardware scaler
		return GraphicModeBehavior.TEST_LQ_HARDWARE_SCALER;
	}

	private AndroidPortAdditions()
	{

	}

	private static AndroidPortAdditions sInstance;

	private String sGameFileDir;

	private int sExtensionFileVersion;

	private int mGameActivityCount = 0;

	private static final String OFFERED_TUTORIAL_PREF = "offered_tutorial_pref";

	private static final String FIRST_USE_PREF = "first_use_pref";
	private static final String SHADER_TESTED_PREF = "shader_tested_pref";
	private static final String GRAPHIC_SETTING_PROMPT_PREF = "grapihc_setting_prompt_pref";

	private static final String GAME_FILES_SIZE_PREF = "game_files_size_pref";
	private static final String SAVE_SLOTS_PREF = "save_slots_pref";
	private static final String SAVE_SLOTS_INDEXES_PREF = "save_slots_indexes_pref";
	private static final String LAST_SAVE_SLOT_PREF = "last_save_slot_pref";

	private static final String AUTO_GRAPHIC_OPTION_PREF = "pref_auto_graphic_option";

	private static final String SCALING_OPTION_PREF = "pref_scaler_option";

	private static final String GL_RENDERER_PREF = "pref_gl_renderer";
	
	private static final String AD_POLICY_PREF = "preF_ad_policy";


	private static final int MAX_SAVE_SLOTS = 8;

	// Initialize the default formation of save slots
	private static String sDefaultSaveSlots;
	static
	{
		ArrayList<String> saveSlots = new ArrayList<String>();
		for (int i = 0; i < MAX_SAVE_SLOTS; ++i)
		{
			saveSlots.add("EMPTY SLOT " + String.valueOf(i + 1));
		}

		sDefaultSaveSlots = CsvUtils.stringListToCsv(saveSlots);
	}

}
