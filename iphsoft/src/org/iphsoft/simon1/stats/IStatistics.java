package org.iphsoft.simon1.stats;

import android.app.Activity;

public interface IStatistics
{
	public class SettingsInfo
	{
		public String mGraphicModeType;
		public String mGraphicMode;
		public String mVoiceMode;
		public String mMusicMode;
		public String mControlMode;
		public String mLanguage;
	}
	
	public void reportGameStart(SettingsInfo settingsInfo);
		
	public void reportDeviceAndGPU(String device, String gpu);
	
	public void reportActivityStart(Activity activity);
	
	public void reportActivityStop(Activity activity);
}
