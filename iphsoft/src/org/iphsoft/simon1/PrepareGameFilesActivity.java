
package org.iphsoft.simon1;

import java.io.File;
import java.util.Vector;

import org.iphsoft.simon1.GameFileHelper.Language;
import org.iphsoft.simon1.GameFileHelper.ProgressListener;
import org.iphsoft.simon1.stats.StatisticsManager;
import org.iphsoft.simon1.ui.BetterProgressBar;
import org.iphsoft.simon1.util.DialogUtils;
import org.iphsoft.simon1.util.FileUtils;
import org.iphsoft.simon1.util.MyLog;

import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnDismissListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.widget.Toast;

import com.mojotouch.simon.R;

public class PrepareGameFilesActivity extends Activity implements ProgressListener {

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        MyLog.d("PrepareGameFilesActivity: onCreate:");

        super.onCreate(savedInstanceState);

        // Get the language preference
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(this);

        String languageSetting = prefs.getString("pref_language", "en");
        Language language;
        if (languageSetting.equals("en"))
            language = Language.ENGLISH;
        else if (languageSetting.equals("de"))
        {
            String voiceSetting = prefs.getString("pref_voice_mode", null);

            if (voiceSetting.equals("subtitles"))
            {
                language = Language.GERMAN_SUBS;
            }
            else
            {
                language = Language.GERMAN;
            }
        }
        else if (languageSetting.equals("he"))
            language = Language.HEBREW;
        else if (languageSetting.equals("es"))
            language = Language.SPANISH;
        else if (languageSetting.equals("fr"))
            language = Language.FRENCH;
        else if (languageSetting.equals("it"))
            language = Language.ITALIAN;
        else
        {
            language = Language.ENGLISH;
            MyLog.e("PrepareGameFilesActivity: onCreate: something is wrong, unfamiliar language preference");
        }

        if (AndroidPortAdditions.STANDALONE_APK)
        {
            mGameFileHelper = new StandaloneGameFileHelper(AndroidPortAdditions.instance()
                    .getGameFileDir(), language);
        }
        else
        {
            mGameFileHelper = new GameFileHelper(AndroidPortAdditions.instance()
                    .getGameFileDir(), language);
        }

        // Check if the game files exist.
        if (mGameFileHelper.checkThatGameFilesExist())
        {
            // No need for this activity
            gotoNextActivity();
            return;
        }

        setContentView(R.layout.prepare_game_files);

        mPbProgress = (BetterProgressBar) findViewById(R.id.pbProgress);
        // mTxtPercent = (TextView) findViewById(R.id.txtPercent);

        mGameFileHelper.setProgressListener(this);

        /*
         * findViewById(R.id.btnCancel).setOnClickListener(new OnClickListener()
         * {
         * @Override public void onClick(View arg0) {
         * mGameFileHelper.cancel(true); PrepareGameFilesActivity.this.finish();
         * } });
         */

        // Start preparing game files in an AsyncTask
        mGameFileHelper.execute();
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
    public void onProgress(int progress, int max) {

        mPbProgress.setMax(max);
        mPbProgress.setProgress(progress);

        // int percent = 100 * progress / max;
        // mTxtPercent.setText(String.valueOf(percent) + '%');
    }

    @Override
    public void onPostExecute(boolean result) {

        MyLog.d("PrepareGameFilesActivity: onPostExecute: " + result);

        if (result)
        {
            gotoNextActivity();
        }
        else if (!Environment.getExternalStorageDirectory().canRead()
                || !Environment.getExternalStorageDirectory().canWrite())
        {
            // No access to external storage
            Toast.makeText(this, R.string.unable_to_create_game_files_connect_external_storage,
                    Toast.LENGTH_LONG).show();
            finish();
        }
        else if (FileUtils.getBytesAvailableOnExternalStorage() < MIN_FREE_SPACE)
        {
            // Not enough space
            showDialog(NO_FREE_SPACE_DLG_ID);
        }
        else
        {
            showDialog(DOWNLOAD_GAME_FILES_DLG_ID);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        mGameFileHelper.setProgressListener(null);
    }

    @Override
    public void onBackPressed() {
        mGameFileHelper.cancel(true);
        super.onBackPressed();
    }

    @Override
    protected Dialog onCreateDialog(int id) {

        if (id == DOWNLOAD_GAME_FILES_DLG_ID)
        {
            // Prompt for downloading game files
            Dialog dlg = DialogUtils.createNegativePositiveDialog(this, R.string.no, R.string.yes,
                    R.string.unable_to_create_game_files_error_dialog,
                    new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dlg, int arg1) {

                            MyLog.d("PrepareGameFilesActivity.onCreateDialog(...).new OnClickListener() {...}: onClick: no");
                            // Pressed "no"
                            dlg.dismiss();

                        }
                    },
                    new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dlg, int arg1) {
                            MyLog.d("PrepareGameFilesActivity.onCreateDialog(...).new OnClickListener() {...}: onClick: yes");
                            // pressed "yes"

                            // Delete previous game files
                            String[] expansionFiles = getAPKExpansionFiles(
                                    PrepareGameFilesActivity.this,
                                    AndroidPortAdditions.instance()
                                            .getAPKExtensionFileVersion(), 0);
                            boolean result = false;
                            if (expansionFiles != null && expansionFiles.length >= 1)
                            {
                                result = new File(expansionFiles[0]).delete();

                                if (!result)
                                {
                                    MyLog.e("PrepareGameFilesActivity: Could not delete previous APK expansion: "
                                            + expansionFiles[0]);
                                }

                            }

                            // Re-download
                            startActivity(new Intent(PrepareGameFilesActivity.this,
                                    SampleDownloaderActivity.class));

                            dlg.dismiss();
                        }
                    });

            dlg.setOnDismissListener(new OnDismissListener() {

                @Override
                public void onDismiss(DialogInterface arg0) {
                    PrepareGameFilesActivity.this.finish();
                }
            });

            return dlg;
        }
        else if (id == NO_FREE_SPACE_DLG_ID)
        {
            // Show a message
            Dialog dlg = DialogUtils.createNegativePositiveDialog(this, 0, R.string.ok,
                    R.string.prepare_game_files_no_space_message, null, new OnClickListener() {

                        @Override
                        public void onClick(DialogInterface arg0, int arg1) {
                            PrepareGameFilesActivity.this.finish();
                        }
                    });
            
            return dlg;
        }

        return super.onCreateDialog(id);
    }

    /**
     * Returns the APK expension files with full path. NOTE: this method was
     * extracted from the downloader support library.
     * 
     * @param ctx
     * @param mainVersion
     * @param patchVersion
     * @return
     */
    String[] getAPKExpansionFiles(Context ctx, int mainVersion, int patchVersion) {
        String packageName = ctx.getPackageName();
        Vector<String> ret = new Vector<String>();
        if (Environment.getExternalStorageState().equals(
                Environment.MEDIA_MOUNTED)) {
            // Build the full path to the app's expansion files
            File root = Environment.getExternalStorageDirectory();
            File expPath = new File(root.toString() + EXP_PATH + packageName);

            // Check that expansion file path exists
            if (expPath.exists()) {
                if (mainVersion > 0) {
                    String strMainPath = expPath + File.separator + "main." + mainVersion + "."
                            + packageName + ".obb";
                    File main = new File(strMainPath);
                    if (main.isFile()) {
                        ret.add(strMainPath);
                    }
                }
                if (patchVersion > 0) {
                    String strPatchPath = expPath + File.separator + "patch." + mainVersion + "."
                            + packageName + ".obb";
                    File main = new File(strPatchPath);
                    if (main.isFile()) {
                        ret.add(strPatchPath);
                    }
                }
            }
        }
        String[] retArray = new String[ret.size()];
        ret.toArray(retArray);
        return retArray;
    }

    private void gotoNextActivity()
    {
        // Start the game activity, with intent extras forwarded
        Intent gameActivity = new Intent(this, ScummVMActivity.class);
        gameActivity.putExtras(getIntent());
        startActivity(gameActivity);
        finish();
    }

    private BetterProgressBar mPbProgress;
    // private TextView mTxtPercent;

    private GameFileHelper mGameFileHelper;

    private static final int DOWNLOAD_GAME_FILES_DLG_ID = 1;
    private static final int NO_FREE_SPACE_DLG_ID = 2;

    private final static String EXP_PATH = "/Android/obb/";

    private final static long MIN_FREE_SPACE = 1024 * 1024 * 100;
}
