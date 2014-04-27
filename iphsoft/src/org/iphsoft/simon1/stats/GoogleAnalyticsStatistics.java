package org.iphsoft.simon1.stats;

import org.iphsoft.simon1.AndroidPortAdditions;
import org.iphsoft.simon1.ScummVMApplication;
import org.iphsoft.simon1.util.MyLog;

import android.app.Activity;
import android.content.Context;

import com.google.analytics.tracking.android.EasyTracker;
import com.google.analytics.tracking.android.Fields;
import com.google.analytics.tracking.android.GAServiceManager;
import com.google.analytics.tracking.android.GoogleAnalytics;
import com.google.analytics.tracking.android.Logger.LogLevel;
import com.google.analytics.tracking.android.MapBuilder;
import com.google.analytics.tracking.android.Tracker;

public class GoogleAnalyticsStatistics implements IStatistics {
	public GoogleAnalyticsStatistics() {

		Context ctx = ScummVMApplication.getContext();

		// For testing
		// GoogleAnalytics.getInstance(ctx).getLogger().setLogLevel(LogLevel.VERBOSE);

		mTracker = EasyTracker.getInstance(ctx);
	}

	@Override
	public void reportGameStart(SettingsInfo settingsInfo) {
		MyLog.d("GoogleAnalyticsStatistics: reportGameStart: ");

		// Send an app view event, along with all the custom fields for settings
		mTracker.send(MapBuilder
				.createAppView()
				.set(Fields.SCREEN_NAME, "Game Start")
				.set(Fields
						.customDimension(GRAPHIC_SETTING_TYPE_DIMENSION_INDEX),
						settingsInfo.mGraphicModeType)
				.set(Fields.customDimension(GRAPHIC_SETTING_DIMENSION_INDEX),
						settingsInfo.mGraphicMode)
				.set(Fields.customDimension(VOICE_SETTING_DIMENSION_INDEX),
						settingsInfo.mVoiceMode)
				.set(Fields.customDimension(MUSIC_SETTING_DIMENSION_INDEX),
						settingsInfo.mMusicMode)
				.set(Fields.customDimension(CONTROL_SETTING_DIMENSION_INDEX),
						settingsInfo.mControlMode)
				.set(Fields.customDimension(LANGUAGE_SETTING_DIMENSION_INDEX),
						settingsInfo.mLanguage).build());

		// For testing - dispatch immediately
		// GAServiceManager.getInstance().dispatchLocalHits();
	}

	@Override
	public void reportDeviceAndGPU(String device, String gpu) {
		// Send an app view event, along with all the custom fields for settings
		mTracker.send(MapBuilder
				.createAppView()
				.set(Fields.SCREEN_NAME, "Graphic mode initialization")
				.set(Fields.customDimension(DEVICE_NAME_DIMENSION_INDEX),
						device)
				.set(Fields.customDimension(GPU_KIND_DIMENSION_INDEX), gpu)
				.build());
	}

	public void reportActivityStart(Activity activity) {
		mTracker.activityStart(activity);
	}

	public void reportActivityStop(Activity activity) {
		mTracker.activityStop(activity);
	}

	private EasyTracker mTracker;

	private static String TRACKING_ID = "";

	static {
		switch (AndroidPortAdditions.GAME_TYPE) {
		case SIMON1:
			TRACKING_ID = "UA-47478821-1";
			break;
		}
	}

	private static final int GRAPHIC_SETTING_TYPE_DIMENSION_INDEX = 1;
	private static final int GRAPHIC_SETTING_DIMENSION_INDEX = 2;
	private static final int VOICE_SETTING_DIMENSION_INDEX = 3;
	private static final int MUSIC_SETTING_DIMENSION_INDEX = 4;
	private static final int CONTROL_SETTING_DIMENSION_INDEX = 5;
	private static final int LANGUAGE_SETTING_DIMENSION_INDEX = 6;
	private static final int GPU_KIND_DIMENSION_INDEX = 7;
	private static final int DEVICE_NAME_DIMENSION_INDEX = 8;

}
