
package org.iphsoft.simon1;

import java.util.Locale;

import org.iphsoft.simon1.util.MyLog;

import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.view.View;
import android.view.View.OnClickListener;
import com.mojotouch.simon.R;

public class SettingsActivity extends PreferenceActivity {

    public static final int DOES_NOT_REQUIRES_RESTART = 0;
    public static final int REQUIRES_RESTART = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
/*
        if (AndroidPortAdditions.instance().getFirstUse())
        {
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

            MyLog.d("SettingsActivity: onCreate: locale language: "
                    + Locale.getDefault().getLanguage());
            MyLog.d("SettingsActivity: onCreate: default language pref: " + defaultLanguagePref);
        }*/

        addPreferencesFromResource(R.xml.preferences);
        setContentView(R.layout.settings);

        View btnOk = findViewById(R.id.btnOk);

        // Don't show the OK button if it's not the first use
     //   if (!AndroidPortAdditions.instance().getFirstUse())
            btnOk.setVisibility(View.GONE);

        findViewById(R.id.btnOk).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                Intent intent = new Intent(SettingsActivity.this, PrepareGameFilesActivity.class);
                startActivity(intent);
                finish();

                AndroidPortAdditions.instance().setFirstUse(false);
            }
        });

        // Save the current settings before changes
        mPreviousMusicSetting = prefs.getString("pref_music_mode", null);
        mPreviousVoiceSetting = prefs.getString("pref_voice_mode", null);
        mPreviousLanguageSetting = prefs.getString("pref_language", null);
        mPreviousScalerSetting = prefs.getString("pref_scaler_option", null);
    }

    @Override
    public void onBackPressed() {

        // Check for setting changes
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        if (!AndroidPortAdditions.instance().getFirstUse() &&
                (!mPreviousMusicSetting.equals(prefs.getString("pref_music_mode", null)) ||
                        !mPreviousVoiceSetting.equals(prefs.getString("pref_voice_mode", null)) ||
                        !mPreviousLanguageSetting.equals(prefs.getString("pref_language", null)) ||
                !mPreviousScalerSetting.equals(prefs.getString("pref_scaler_option", null))))
        {
            showDialog(RESTART_DIALOG_ID);
        }
        else
        {
            super.onBackPressed();
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == RESTART_DIALOG_ID)
        {
            Builder builder = new Builder(this);
            builder.setMessage(R.string.restart_needed);
            builder.setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {

                public void onClick(DialogInterface arg0, int arg1) {

                    resetLanguageIfNeeded();
                    SettingsActivity.this.setResult(REQUIRES_RESTART);
                    finish();
                }
            });

            builder.setNegativeButton(R.string.no, new DialogInterface.OnClickListener() {

                public void onClick(DialogInterface arg0, int arg1) {

                    resetLanguageIfNeeded();
                    SettingsActivity.this.setResult(DOES_NOT_REQUIRES_RESTART);
                    finish();
                }
            });

            return builder.create();
        }

        return super.onCreateDialog(id);
    }

    private boolean checkLocale(String languageCode)
    {
        // Compare the language code to the default locale's language code
        return (Locale.getDefault().getLanguage().equals(new Locale(languageCode, "", "")
                .getLanguage()));
    }

    private void resetLanguageIfNeeded()
    {
        // If the language was changed, we need to reset the expected language
        // size so it will recreate the files
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        if (!mPreviousLanguageSetting.equals(prefs.getString("pref_language", null)))
        {
            AndroidPortAdditions.instance().setGameFilesSize(-1);
        }
    }

    private String mPreviousMusicSetting;
    private String mPreviousVoiceSetting;
    private String mPreviousLanguageSetting;
    private String mPreviousScalerSetting;

    private static final int RESTART_DIALOG_ID = 1;
}
