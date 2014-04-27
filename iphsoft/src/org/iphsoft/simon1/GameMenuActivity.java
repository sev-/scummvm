
package org.iphsoft.simon1;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;

import org.iphsoft.simon1.AdHelper.AdInfo;
import org.iphsoft.simon1.AndroidPortAdditions.GraphicModeBehavior;
import org.iphsoft.simon1.stats.StatisticsManager;
import org.iphsoft.simon1.ui.ActivityAnimationUtil;
import org.iphsoft.simon1.ui.ExtendedButton;
import org.iphsoft.simon1.ui.ExtendedImageButton;
import org.iphsoft.simon1.ui.ExtendedRadioButton;
import org.iphsoft.simon1.ui.ToggledExtendedButton;
import org.iphsoft.simon1.ui.ToggledExtendedButton.OnToggleListener;
import org.iphsoft.simon1.ui.VideoActivity;
import org.iphsoft.simon1.util.DialogUtils;
import org.iphsoft.simon1.util.IntentUtils;
import org.iphsoft.simon1.util.MyLog;

import android.app.Dialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Environment;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.view.Gravity;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;

import com.mojotouch.simon.R;

public class GameMenuActivity extends PreferenceActivity {

    public static final String EXTRA_MID_GAME = "GameMenuActivity.EXTRA_MID_GAME";
    public static final String EXTRA_FROM_POSTCARD = "GameMenuActivity.EXTRA_FROM_POSTCARD";
    public static final String EXTRA_SAVE_SLOT = "GameMenuActivity.EXTRA_SAVE_SLOT";

    public static final int RESULT_NONE = 0;
    public static final int RESULT_REQUEST_RESTART = 1;
    public static final int RESULT_REQUEST_SAVE = 2;
    public static final int RESULT_REQUEST_LOAD = 3;
    public static final int RESULT_REQUEST_RESTART_LOAD = 4;
    public static final int RESULT_REQUEST_QUIT = 5;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);

        setContentView(R.layout.game_menu);

        // Initialize the preference from XML
        addPreferencesFromResource(R.xml.preferences);

        mMenuFrame = (FrameLayout) findViewById(R.id.menu_frame);
        mSubTitleView = findViewById(R.id.subtitle_view);

        Intent launchingIntent = getIntent();
        mMidGame = launchingIntent.getBooleanExtra(EXTRA_MID_GAME, false);

        switchStateMain();

        if (AndroidPortAdditions.instance().getFirstUse())
        {
            // Set setting defaults
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);

            // If this is the first use, set the default language according to
            // device locale
            String defaultLanguagePref = "en";

            if (checkLocale("de"))
                defaultLanguagePref = "de";
            if (checkLocale("he"))
                defaultLanguagePref = "he";
            if (checkLocale("es"))
                defaultLanguagePref = "es";
            if (checkLocale("fr"))
                defaultLanguagePref = "fr";
            if (checkLocale("it"))
                defaultLanguagePref = "it";

            prefs.edit().putString("pref_language", defaultLanguagePref).commit();

            AndroidPortAdditions.instance().setFirstUse(false);

            // WORKAROUND: for German version - make sure we use the
            // "voice only option"
            germanVersionForceVoiceOnly();

            MyLog.d("GameMenuActivity: onCreate: locale language: "
                    + Locale.getDefault().getLanguage());
            MyLog.d("GameMenuActivity: onCreate: default language pref: " + defaultLanguagePref);

            // Set the appropriate graphic mode or the device
            GraphicModeBehavior behavior = AndroidPortAdditions.instance().getGraphicModeBehavior();

            // We set using the ListPreference interface since the current
            // activity is coupled with it.
            // Otherwise, options won't show the value.
            ListPreference pref = (ListPreference) getPreferenceManager().findPreference(
                    "pref_scaler_option");
            switch (behavior)
            {
                case SOFTWARE_SCALER:
                    pref.setValue("scaling_option_software");
                    AndroidPortAdditions.instance().setShaderTested(true);
                    break;
                case HARDWARE_SCALER:
                    pref.setValue("scaling_option_shader");
                    AndroidPortAdditions.instance().setShaderTested(true);
                    break;
                case TEST_HARDWARE_SCALER:
                    pref.setValue("scaling_option_shader");
                    AndroidPortAdditions.instance().setShaderTested(false);
                    break;
                case LQ_HARDWARE_SCALER:
                    pref.setValue("scaling_option_lq_shader");
                    AndroidPortAdditions.instance().setShaderTested(true);
                    break;
                case TEST_LQ_HARDWARE_SCALER:
                    pref.setValue("scaling_option_lq_shader");
                    AndroidPortAdditions.instance().setShaderTested(false);
                    break;
            }
        }

        if (AndroidPortAdditions.USE_SHADER_FILES)
        {
            // TESTING: copy shader and video to ext. storage
            copyAssetToExternalStorage("fragment.glsl");
            copyAssetToExternalStorage("vertex.glsl");
            copyAssetToExternalStorage("lq_fragment.glsl");
            copyAssetToExternalStorage("lq_vertex.glsl");
        }
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

    private void copyAssetToExternalStorage(String asset)
    {
        // If file exists, return
        if (new File(Environment.getExternalStorageDirectory() + File.separator + asset).exists())
        {
            return;
        }

        InputStream is;
        try {
            is = getAssets().open(asset);
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return;
        }

        copyInputStreamToExternalStorage(is, asset);
    }

    private void copyRawFileExternalStorage(int fileResource, String name)
    {
        // If file exists, return
        if (new File(Environment.getExternalStorageDirectory() + File.separator + name).exists())
        {
            return;
        }

        InputStream is = getResources().openRawResource(fileResource);

        copyInputStreamToExternalStorage(is, name);
    }

    private void copyInputStreamToExternalStorage(InputStream is, String name)
    {
        try {
            FileOutputStream os = new FileOutputStream(Environment.getExternalStorageDirectory()
                    .getPath() + File.separator + name);

            byte[] buf = new byte[1024 * 64];
            int res;
            while ((res = is.read(buf)) != -1)
            {
                os.write(buf, 0, res);
            }

            os.flush();
            os.close();
            is.close();

        } catch (FileNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    private void showTutorialPromptDialog()
    {
        Dialog dlg = DialogUtils.createNegativePositiveDialog(this, R.string.no, R.string.yes,
                R.string.tutorial_prompt, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface arg0, int arg1) {

                        // If pressed "no", start new game
                        startActivity(new Intent(GameMenuActivity.this,
                                PrepareGameFilesActivity.class));
                        finish();
                        ActivityAnimationUtil.makeActivityFadeTransition(GameMenuActivity.this);

                    }
                }, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface arg0, int arg1) {

                        // If pressed "yes", show tutorial
                        startActivity(new Intent(GameMenuActivity.this, VideoActivity.class)
                                .putExtra(
                                        VideoActivity.EXTRA_VIDEO_RESOURCE, R.raw.tutorial));
                        ActivityAnimationUtil.makeActivityFadeTransition(GameMenuActivity.this);
                    }
                });

        dlg.show();
    }

    private void showLoadDialog()
    {
        // Generate a list of save slots to display
        List<String> allSaveSlots = AndroidPortAdditions.instance().getSaveSlots();
        mSaveSlotIndexes = AndroidPortAdditions.instance().getSaveSlotsIndexes();

        List<String> displayedSaveSlots = new ArrayList<String>();

        for (Integer i : mSaveSlotIndexes)
        {
            displayedSaveSlots.add(allSaveSlots.get(i));
        }

        Dialog dlg = DialogUtils.createListDialog(this, R.string.cancel, R.string.choose_load_slot,
                new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dlg, int which) {
                        // Convert index to save slot
                        int slot = mSaveSlotIndexes.get(which) + 1;

                        requestLoadWithRestart(slot);
                    }
                }, displayedSaveSlots.toArray(new String[0]));

        dlg.show();
    }

    private boolean checkLocale(String languageCode)
    {
        // Compare the language code to the default locale's language code
        return (Locale.getDefault().getLanguage().equals(new Locale(languageCode, "", "")
                .getLanguage()));
    }

    private void requestLoadWithRestart(int slot)
    {
        MyLog.d("GameMenuActivity: requestLoadWithRestart: " + slot);

        if (mMidGame)
        {
            // Request the game activity to restart and load
            Intent data = new Intent().putExtra(EXTRA_SAVE_SLOT, slot);

            setResult(RESULT_REQUEST_RESTART_LOAD, data);
        }
        else
        {
            // Start the game activity with a load slot
            Intent loadIntent = new Intent(this, PrepareGameFilesActivity.class);
            loadIntent.putExtra(ScummVMActivity.EXTRA_STARTING_SAVE_SLOT, slot);
            startActivity(loadIntent);
            ActivityAnimationUtil.makeActivityFadeTransition(GameMenuActivity.this);
        }

        finish();
        ActivityAnimationUtil.makeActivityFadeTransition(GameMenuActivity.this);
    }

    private void switchStateMain()
    {
    	MyLog.d("GameMenuActivity: switchStateMain: ");
    	
        mMenuState = MenuState.MAIN;

        // Inflate the main layout
        mMenuFrame.removeAllViews();
        mSubTitleView.setVisibility(View.VISIBLE);

        if (mMainMenuView == null)
        {
        	mMainMenuView = View.inflate(this, R.layout.main_menu_frame, null);
        	mMainMenuView.setLayoutParams(new LayoutParams(LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT));
        }
        
        mMenuFrame.addView(mMainMenuView);
        
        // Setup continue button
        Button btnContinue = (Button) findViewById(R.id.btnContinue);
        btnContinue.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {

                if (mMidGame)
                {
                    // Just finish the menu and go back to game
                    setResult(RESULT_NONE);
                    finish();
                    ActivityAnimationUtil.makeActivityFadeTransition(GameMenuActivity.this);

                }
                else
                {
                    // Load the last save
                    requestLoadWithRestart(AndroidPortAdditions.instance().getLastSaveSlot());
                }

            }
        });

        // Disallow continue if there is no last save and game is not in
        // progress
        if (!mMidGame && AndroidPortAdditions.instance().getLastSaveSlot() == -1)
        {
            btnContinue.setEnabled(false);
        }

        // Setup new game button
        findViewById(R.id.btnNewGame).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                if (mMidGame)
                {
                    // Show confirmation dialog for new game
                    Dialog dlg = DialogUtils.createNegativePositiveDialog(GameMenuActivity.this, R.string.no, R.string.yes, R.string.new_game_confirmation, null, new DialogInterface.OnClickListener() {
                        
                        @Override
                        public void onClick(DialogInterface arg0, int arg1) {
                            // Start a new game
                            setResult(RESULT_REQUEST_RESTART);   
                            finish();
                            ActivityAnimationUtil.makeActivityFadeTransition(GameMenuActivity.this);
                        }
                    });
                    
                    dlg.show();
                }
                else
                {
                    // Show tutorial prompt on first time, if tutorial wasn't
                    // shown
                    if (!AndroidPortAdditions.instance().wasTutorialOffered())
                    {
                        AndroidPortAdditions.instance().setTutorialOffered(true);
                        showTutorialPromptDialog();
                        return;
                    }

                    startActivity(new Intent(GameMenuActivity.this, PrepareGameFilesActivity.class));
                    finish();
                    ActivityAnimationUtil.makeActivityFadeTransition(GameMenuActivity.this);
                }
            }
        });

        // Setup save button
        Button btnSave = (Button) findViewById(R.id.btnSave);
        btnSave.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                Intent data = new Intent().putExtra(EXTRA_FROM_POSTCARD, getIntent()
                        .getBooleanExtra(EXTRA_FROM_POSTCARD, false));
                setResult(RESULT_REQUEST_SAVE, data);
                finish();
                ActivityAnimationUtil.makeActivityFadeTransition(GameMenuActivity.this);

            }
        });
        btnSave.setEnabled(mMidGame);

        // Setup load button
        ExtendedButton btnLoad = (ExtendedButton) findViewById(R.id.btnLoad);
        btnLoad.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {

                showLoadDialog();
            }
        });

        // Disable load button if there are no saves
        if (AndroidPortAdditions.instance().getSaveSlotsIndexes().size() == 0)
        {
            btnLoad.setEnabled(false);
        }

        // Setup quit button
        final ExtendedButton btnExit = (ExtendedButton)findViewById(R.id.btnExit);
        btnExit.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                if (mMidGame)
                {
                    setResult(RESULT_REQUEST_QUIT);
                }

                finish();
                ActivityAnimationUtil.makeActivityFadeTransition(GameMenuActivity.this);

            }
        });

        // Setup tutorial button
        findViewById(R.id.btnTutorial).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {

                AndroidPortAdditions.instance().setTutorialOffered(true);

                startActivity(new Intent(GameMenuActivity.this, VideoActivity.class).putExtra(
                        VideoActivity.EXTRA_VIDEO_RESOURCE, R.raw.tutorial));
                ActivityAnimationUtil.makeActivityFadeTransition(GameMenuActivity.this);
            }
        });

        // Setup options button
        findViewById(R.id.btnOptions).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                // Save the current settings before changes
                SharedPreferences prefs = PreferenceManager
                        .getDefaultSharedPreferences(GameMenuActivity.this);
                mPreviousMusicSetting = prefs.getString("pref_music_mode", null);
                mPreviousVoiceSetting = prefs.getString("pref_voice_mode", null);
                mPreviousLanguageSetting = prefs.getString("pref_language", null);
                mPreviousScalerSetting = prefs.getString("pref_scaler_option", null);

                switchStateOptions();
            }
        });
        
        // Setup ad button
        final ExtendedImageButton btnAd = (ExtendedImageButton)findViewById(R.id.btnAd);
        
        // Get the ad information and adjust the button accordingly
        final AdInfo adInfo = AdHelper.getAdToDisplay();
        if (adInfo != null)
        {
        	btnAd.setImageResource(adInfo.adIconResourceId);
        	
        	btnAd.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View arg0) {
                	IntentUtils.startAppStoreActivity(GameMenuActivity.this, adInfo.appPackage);
                }
            });
        	
        	// Adjust the ad button to be the same height as other buttons
			mMenuFrame.getViewTreeObserver().addOnGlobalLayoutListener(new OnGlobalLayoutListener() {
    			
    			@Override
    			public void onGlobalLayout() {
    				MyLog.d("GameMenuActivity.switchStateMain().new OnGlobalLayoutListener() {...}: onGlobalLayout: height " + btnExit.getHeight());
    				btnAd.setAdjustViewBounds(true);
    				btnAd.setMaxHeight(btnExit.getHeight());
    				mMenuFrame.getViewTreeObserver().removeGlobalOnLayoutListener(this);
    			}
    		});
        }
        else
        {
        	// No ad to display - remove the button
        	btnAd.setVisibility(View.GONE);
        }
    }

    private void switchStateOptions()
    {
        mMenuState = MenuState.OPTIONS_LEVEL1;

        // Inflate the main layout
        mMenuFrame.removeAllViews();
        mSubTitleView.setVisibility(View.VISIBLE);

        View.inflate(this, R.layout.settings_frame, mMenuFrame);

        findViewById(R.id.btnBack).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                backMenuState();
            }
        });

        findViewById(R.id.btnMusic).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                switchStatePreference(getPreferenceManager().findPreference("pref_music_mode"));
            }
        });

        findViewById(R.id.btnVoice).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                switchStatePreference(getPreferenceManager().findPreference("pref_voice_mode"));
            }
        });

        findViewById(R.id.btnLanguage).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                switchStatePreference(getPreferenceManager().findPreference("pref_language"));
            }
        });

        findViewById(R.id.btnGraphics).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                switchStatePreference(getPreferenceManager().findPreference("pref_scaler_option"));
            }
        });

        findViewById(R.id.btnControl).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                switchStatePreference(getPreferenceManager().findPreference("pref_control_mode"));
            }
        });

        findViewById(R.id.btnAbout).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                startActivity(new Intent(GameMenuActivity.this, AboutActivity.class));
                ActivityAnimationUtil.makeActivityNullTransition(GameMenuActivity.this);
            }
        });
    }

    private void switchStatePreference(Preference pref)
    {
        mMenuState = MenuState.OPTIONS_LEVEL2;

        mMenuFrame.removeAllViews();
        mSubTitleView.setVisibility(View.GONE);

        // Handle ListPreference
        if (pref instanceof ListPreference)
        {
            final ListPreference listPref = (ListPreference) pref;

            // Check the number of entries
            CharSequence[] entries = listPref.getEntries();

            if (entries.length > 1)
            {
                // Use a radiogroup layout
                View.inflate(this, R.layout.setting_list_frame, mMenuFrame);

                // Populate the layout
                int checkedIndex = -1;
                RadioGroup radioGroup = (RadioGroup) findViewById(R.id.settingRadioGroup);
                for (int i = 0; i < entries.length; ++i)
                {
                    ExtendedRadioButton radioButton = new ExtendedRadioButton(this);
                    radioButton.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT,
                            LayoutParams.WRAP_CONTENT));
                    radioButton.setGravity(Gravity.CENTER);
                    radioButton.setText(entries[i]);
                    radioButton.setId(i);
                    radioGroup.addView(radioButton);

                    // WORKAROUND: for German version - disable the option for
                    // "voice and subtitles"
                    SharedPreferences prefs = PreferenceManager
                            .getDefaultSharedPreferences(this);
                    if (prefs.getString("pref_language", "en").equals("de"))
                    {
                        if (entries[i]
                                .equals(getString(R.string.voice_mode_voice_subtitles)))
                        {
                            radioButton.setEnabled(false);
                        }
                    }

                    // Save the checked item
                    if (listPref.getEntry().equals(entries[i]))
                    {
                        checkedIndex = i;
                    }
                }

                // Set the initial checked item
                radioGroup.check(checkedIndex);

                // Set click listener
                radioGroup.setOnCheckedChangeListener(new OnCheckedChangeListener() {

                    @Override
                    public void onCheckedChanged(RadioGroup view, int checkedId) {
                        listPref.setValueIndex(checkedId);
                    }
                });
            }
            else
            {
                // Use a toggle button
                View.inflate(this, R.layout.setting_toggle_frame, mMenuFrame);

                ToggledExtendedButton toggleButton = (ToggledExtendedButton) findViewById(R.id.toggleButton);
                toggleButton.setLabels(Arrays.asList(entries));
                for (int i = 0; i < entries.length; ++i)
                {
                    if (entries[i].equals(listPref.getEntry()))
                    {
                        // Set the currently selected setting
                        toggleButton.setLabelIndex(i);
                    }
                }

                toggleButton.setOnToggleListener(new OnToggleListener() {

                    @Override
                    public void onToggle(int index) {
                        // Update the setting
                        listPref.setValueIndex(index);
                    }
                });
            }
        }

        findViewById(R.id.btnBack).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {

                // WORKAROUND: for German version - make sure we use the
                // "voice only option"
                germanVersionForceVoiceOnly();

                backMenuState();
            }
        });

        // Show prompt for graphic settings if needed
        if (!AndroidPortAdditions.instance().getGraphicSettingPromptShown()
                && pref.getKey().equalsIgnoreCase("pref_scaler_option"))
        {
            Dialog dlg = DialogUtils.createNegativePositiveDialog(this, 0, R.string.ok, R.string.graphic_setting_message, null, null);
            dlg.show();
            
            AndroidPortAdditions.instance().setGraphicSettingPromptShown(true);
        }
    }

    private void resetLanguageIfNeeded()
    {

        // If the language was changed, we need to reset the expected language
        // size so it will recreate the files.
        // Also consider the case of using German language and changing voice
        // mode.
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        String currentLanguage = prefs.getString("pref_language", null);
        String currentVoiceMode = prefs.getString("pref_voice_mode", null);
        
        MyLog.d("GameMenuActivity: resetLanguageIfNeeded: language: current " + currentLanguage + " previous " + mPreviousLanguageSetting);
        MyLog.d("GameMenuActivity: resetLanguageIfNeeded: voice mode: current " + currentVoiceMode + " previous " + mPreviousVoiceSetting);

        if (!mPreviousLanguageSetting.equals(currentLanguage)
                || (currentLanguage.equals("de") && !mPreviousVoiceSetting.equals(currentVoiceMode)))
        {
            MyLog.d("GameMenuActivity: resetLanguageIfNeeded: resetting language");
            AndroidPortAdditions.instance().setGameFilesSize(-1);
        }

        MyLog.d("GameMenuActivity: resetLanguageIfNeeded: previous language: "
                + mPreviousLanguageSetting
                + " current: " + prefs.getString("pref_language", null) + " previous voice mode "
                + mPreviousVoiceSetting + " current " + currentVoiceMode);
    }

    private boolean checkSettingChanges()
    {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);

    	// Remove "auto graphics" flag if graphic mode changed (for statistics)
    	if (!mPreviousScalerSetting.equals(prefs.getString("pref_scaler_option", null)))
    	{
    		MyLog.d("GameMenuActivity: checkSettingChanges: graphic mode change, removing 'auto' flag");
    		AndroidPortAdditions.instance().setAutoGraphicMode(false);
    	}
    	
        // Check for setting changes
        if (!AndroidPortAdditions.instance().getFirstUse() &&
                (!mPreviousMusicSetting.equals(prefs.getString("pref_music_mode", null)) ||
                        !mPreviousVoiceSetting.equals(prefs.getString("pref_voice_mode", null)) ||
                        !mPreviousLanguageSetting.equals(prefs.getString("pref_language", null)) ||
                !mPreviousScalerSetting.equals(prefs.getString("pref_scaler_option", null))))
        {
            return true;
        }

        return false;
    }

    private void backMenuState()
    {
        if (mMenuState == MenuState.OPTIONS_LEVEL2)
        {
            switchStateOptions();
        }
        else if (mMenuState == MenuState.OPTIONS_LEVEL1)
        {
            resetLanguageIfNeeded();
            if (checkSettingChanges())
            {
                showDialog(RESTART_DIALOG_ID);
            }
            else
            {
                switchStateMain();
            }
        }
        else
        {
            finish();
            ActivityAnimationUtil.makeActivityFadeTransition(GameMenuActivity.this);
        }
    }

    @Override
    public void onBackPressed() {

        // Implement the artificial menu backstack
        backMenuState();
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == RESTART_DIALOG_ID)
        {

            Dialog dlg = DialogUtils.createNegativePositiveDialog(this, 0, R.string.ok,
                    R.string.restart_needed, null, new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface arg0, int arg1) {

                        }
                    });

            dlg.setOnDismissListener(new OnDismissListener() {

                @Override
                public void onDismiss(DialogInterface arg0) {
                    switchStateMain();
                }
            });

            return dlg;
        }

        return super.onCreateDialog(id);
    }

    private void germanVersionForceVoiceOnly()
    {

        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(ScummVMApplication.getContext());
        if (prefs.getString("pref_language", "en").equals("de")
                && prefs.getString("pref_voice_mode", "voice_subtitles").equals("voice_subtitles"))
        {
            ListPreference pref = (ListPreference) getPreferenceManager().findPreference(
                    "pref_voice_mode");
            pref.setValue("voice");
        }
    }

    private boolean mMidGame;

    private List<Integer> mSaveSlotIndexes;

    private FrameLayout mMenuFrame;
    
    // Used for preventing re-inflating of the main menu, due to performance glitch in ad icon
    private View mMainMenuView;

    private View mSubTitleView;
    
    private enum MenuState
    {
        MAIN, OPTIONS_LEVEL1, OPTIONS_LEVEL2;
    }

    private MenuState mMenuState = MenuState.MAIN;

    private String mPreviousMusicSetting;
    private String mPreviousVoiceSetting;
    private String mPreviousLanguageSetting;
    private String mPreviousScalerSetting;

    private static final int RESTART_DIALOG_ID = 1;

}
