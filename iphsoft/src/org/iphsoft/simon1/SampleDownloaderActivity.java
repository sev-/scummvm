/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.iphsoft.simon1;

import java.io.DataInputStream;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.zip.CRC32;

import org.iphsoft.simon1.stats.StatisticsManager;
import org.iphsoft.simon1.ui.ActivityAnimationUtil;
import org.iphsoft.simon1.ui.BetterProgressBar;
import org.iphsoft.simon1.ui.ExtendedTextView;
import org.iphsoft.simon1.ui.VideoActivity;
import org.iphsoft.simon1.util.DialogUtils;
import org.iphsoft.simon1.util.MyLog;

import android.app.Activity;
import android.app.Dialog;
import android.app.PendingIntent;
import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Messenger;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

import com.android.vending.expansion.zipfile.ZipResourceFile;
import com.android.vending.expansion.zipfile.ZipResourceFile.ZipEntryRO;
import com.google.android.vending.expansion.downloader.Constants;
import com.google.android.vending.expansion.downloader.DownloadProgressInfo;
import com.google.android.vending.expansion.downloader.DownloaderClientMarshaller;
import com.google.android.vending.expansion.downloader.DownloaderServiceMarshaller;
import com.google.android.vending.expansion.downloader.Helpers;
import com.google.android.vending.expansion.downloader.IDownloaderClient;
import com.google.android.vending.expansion.downloader.IDownloaderService;
import com.google.android.vending.expansion.downloader.IStub;
import com.google.android.vending.expansion.downloader.impl.DownloadsDB;
import com.mojotouch.simon.R;

/**
 * This is sample code for a project built against the downloader library. It
 * implements the IDownloaderClient that the client marshaler will talk to as
 * messages are delivered from the DownloaderService.
 */
public class SampleDownloaderActivity extends Activity implements IDownloaderClient {
    private static final String LOG_TAG = "LVLDownloader";
    private BetterProgressBar mPB;

    private static final int FAILURE_DIALOG_ID = 1;
    private static final int CORRUPTION_DIALOG_ID = 2;

    // private TextView mStatusText;
    // private TextView mProgressFraction;
    // private TextView mProgressPercent;
    // private TextView mAverageSpeed;
    // private TextView mTimeRemaining;

    // private View mDashboard;
    // private View mCellMessage;

    private Button mCancelButton;
    private Button mPauseButton;

    private ExtendedTextView mTxtProgress;
    private ExtendedTextView mUpperText;

    // private Button mWiFiSettingsButton;

    private boolean mStatePaused;
    private int mState;

    private boolean mIsRestarting = false;
    private boolean mValidating = false;
    private boolean mCompleted = false;

    private IDownloaderService mRemoteService;

    private IStub mDownloaderClientStub;

    private void setState(int newState) {
        if (mState != newState) {
            mState = newState;
            // mStatusText.setText(Helpers.getDownloaderStringResourceIDFromState(newState));

            MyLog.d("SampleDownloaderActivity: setState: status: "
                    + getString(Helpers.getDownloaderStringResourceIDFromState(newState)));
        }
    }

    private void setButtonPausedState(boolean paused) {
        mStatePaused = paused;
        int stringResourceID = paused ? R.string.text_button_resume :
                R.string.text_button_pause;
        mPauseButton.setText(stringResourceID);
    }

    /**
     * This is a little helper class that demonstrates simple testing of an
     * Expansion APK file delivered by Market. You may not wish to hard-code
     * things such as file lengths into your executable... and you may wish to
     * turn this code off during application development.
     */
    private static class XAPKFile {
        public final boolean mIsMain;
        public final int mFileVersion;
        public final long mFileSize;

        XAPKFile(boolean isMain, int fileVersion, long fileSize) {
            mIsMain = isMain;
            mFileVersion = fileVersion;
            mFileSize = fileSize;
        }
    }

    /**
     * Here is where you place the data that the validator will use to determine
     * if the file was delivered correctly. This is encoded in the source code
     * so the application can easily determine whether the file has been
     * properly delivered without having to talk to the server. If the
     * application is using LVL for licensing, it may make sense to eliminate
     * these checks and to just rely on the server.
     */
    private static final XAPKFile[] xAPKS = {
            new XAPKFile(
                    true, // true signifies a main file
                    16, // the version of the APK that the file was uploaded
                        // against
                    169034320L // the length of the file in bytes
            )
    };

    /**
     * Go through each of the APK Expansion files defined in the structure above
     * and determine if the files are present and match the required size. Free
     * applications should definitely consider doing this, as this allows the
     * application to be launched for the first time without having a network
     * connection present. Paid applications that use LVL should probably do at
     * least one LVL check that requires the network to be present, so this is
     * not as necessary.
     * 
     * @return true if they are present.
     */
    boolean expansionFilesDelivered() {
        for (XAPKFile xf : xAPKS) {
            String fileName = Helpers.getExpansionAPKFileName(this, xf.mIsMain, xf.mFileVersion);

            // Keep the main extension file for later reference
            if (xf.mIsMain)
            {
                AndroidPortAdditions.instance().setAPKExtensionFileVersion(xf.mFileVersion);
            }

            MyLog.d("SampleDownloaderActivity: expansionFilesDelivered: filename " + fileName);

            if (!Helpers.doesFileExist(this, fileName, xf.mFileSize, false))
                return false;
        }
        return true;
    }

    private void deleteOldExpansionFiles()
    {
        // Get our current expansion file names
        ArrayList<String> currentExpansionFileNames = new ArrayList<String>();
        for (XAPKFile xf : xAPKS) {
            currentExpansionFileNames.add(Helpers.getExpansionAPKFileName(this, xf.mIsMain,
                    xf.mFileVersion));
        }

        // Get all files in the current directory that match the naming
        // convention
        String path = Helpers.getSaveFilePath(this);
        File parent = new File(path);
        if (parent.exists())
        {
            String[] filesInDir = parent.list();

            // Delete every .obb file that doesn't have the current expansion
            // name
            for (String filename : filesInDir)
            {
                if (filename.endsWith(".obb") && !currentExpansionFileNames.contains(filename))
                {
                    new File(path + File.separator + filename).delete();
                }
            }
        }
    }

    /**
     * Calculating a moving average for the validation speed so we don't get
     * jumpy calculations for time etc.
     */
    static private final float SMOOTHING_FACTOR = 0.005f;

    /**
     * Used by the async task
     */
    private boolean mCancelValidation;

    /**
     * Go through each of the Expansion APK files and open each as a zip file.
     * Calculate the CRC for each file and return false if any fail to match.
     * 
     * @return true if XAPKZipFile is successful
     */
    void validateXAPKZipFiles() {

        MyLog.d("SampleDownloaderActivity: validateXAPKZipFiles: ");

        mValidating = true;
        // mAverageSpeed.setVisibility(View.GONE);
        // mTimeRemaining.setVisibility(View.GONE);

        AsyncTask<Object, DownloadProgressInfo, Boolean> validationTask = new AsyncTask<Object, DownloadProgressInfo, Boolean>() {

            @Override
            protected void onPreExecute() {
                // mDashboard.setVisibility(View.VISIBLE);
                // mCellMessage.setVisibility(View.GONE);
                // mStatusText.setText(R.string.text_verifying_download);
                /*
                 * mPauseButton.setOnClickListener(new View.OnClickListener() {
                 * @Override public void onClick(View view) { mCancelValidation
                 * = true; } });
                 * mPauseButton.setText(R.string.text_button_cancel_verify);
                 */
                MyLog.d("SampleDownloaderActivity.validateXAPKZipFiles().new AsyncTask() {...}: onPreExecute: ");

                mTxtProgress.setVisibility(View.INVISIBLE);
                mUpperText.setText(R.string.expansion_downloader_upper_text_validating);

                super.onPreExecute();
            }

            @Override
            protected Boolean doInBackground(Object... params) {

                MyLog.d("SampleDownloaderActivity.validateXAPKZipFiles().new AsyncTask() {...}: doInBackground: ");

                for (XAPKFile xf : xAPKS) {
                    String fileName = Helpers.getExpansionAPKFileName(
                            SampleDownloaderActivity.this,
                            xf.mIsMain, xf.mFileVersion);
                    if (!Helpers.doesFileExist(SampleDownloaderActivity.this, fileName,
                            xf.mFileSize, false))
                        return false;
                    fileName = Helpers
                            .generateSaveFileName(SampleDownloaderActivity.this, fileName);
                    ZipResourceFile zrf;
                    byte[] buf = new byte[1024 * 256];
                    try {
                        zrf = new ZipResourceFile(fileName);
                        ZipEntryRO[] entries = zrf.getAllEntries();

                        float averageVerifySpeed = 0;

                        // Counter for the progress bar
                        int entryCount = 0;

                        /**
                         * Then calculate a CRC for every file in the Zip file,
                         * comparing it to what is stored in the Zip directory.
                         * Note that for compressed Zip files we must extract
                         * the contents to do this comparison.
                         */
                        for (ZipEntryRO entry : entries) {

                            ++entryCount;

                            if (-1 != entry.mCRC32) {
                                long length = entry.mUncompressedLength;
                                CRC32 crc = new CRC32();
                                DataInputStream dis = null;
                                try {
                                    dis = new DataInputStream(
                                            zrf.getInputStream(entry.mFileName));

                                    while (length > 0) {
                                        int seek = (int) (length > buf.length ? buf.length
                                                : length);
                                        dis.readFully(buf, 0, seek);
                                        crc.update(buf, 0, seek);
                                        length -= seek;

                                        if (mCancelValidation)
                                            return true;
                                    }
                                    if (crc.getValue() != entry.mCRC32) {
                                        Log.e(Constants.TAG,
                                                "CRC does not match for entry: "
                                                        + entry.mFileName);
                                        Log.e(Constants.TAG,
                                                "In file: " + entry.getZipFileName());
                                        return false;
                                    }
                                } finally {
                                    if (null != dis) {
                                        dis.close();
                                    }
                                }
                            }

                            this.publishProgress(
                                    new DownloadProgressInfo(
                                            entries.length,
                                            entryCount,
                                            0,
                                            0)
                                    );
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                        return false;
                    }
                }
                return true;
            }

            @Override
            protected void onProgressUpdate(DownloadProgressInfo... values) {
                onDownloadProgress(values[0]);
                super.onProgressUpdate(values);
            }

            @Override
            protected void onPostExecute(Boolean result) {
                if (result) {
                    MyLog.d("SampleDownloaderActivity.validateXAPKZipFiles().new AsyncTask() {...}: onPostExecute: validated");

                    moveToNextActivity();
                } else {

                    MyLog.e("SampleDownloaderActivity.validateXAPKZipFiles().new AsyncTask() {...}: onPostExecute: corrupt files");

                    showDialog(CORRUPTION_DIALOG_ID);
                }
                super.onPostExecute(result);
            }

        };
        validationTask.execute(new Object());
    }

    /**
     * If the download isn't present, we initialize the download UI. This ties
     * all of the controls into the remote service calls.
     */
    private void initializeDownloadUI() {

        MyLog.d("SampleDownloaderActivity: initializeDownloadUI: ");

        mDownloaderClientStub = DownloaderClientMarshaller.CreateStub
                (this, SampleDownloaderService.class);
        setContentView(R.layout.expansion_downloader);

        mPB = (BetterProgressBar) findViewById(R.id.progressBar);
        mTxtProgress = (ExtendedTextView) findViewById(R.id.txtProgress);
        mUpperText = (ExtendedTextView) findViewById(R.id.upperText);

        /*
         * mStatusText = (TextView) findViewById(R.id.statusText);
         * mProgressFraction = (TextView) findViewById(R.id.progressAsFraction);
         * mProgressPercent = (TextView)
         * findViewById(R.id.progressAsPercentage); mAverageSpeed = (TextView)
         * findViewById(R.id.progressAverageSpeed); mTimeRemaining = (TextView)
         * findViewById(R.id.progressTimeRemaining); mDashboard =
         * findViewById(R.id.downloaderDashboard); mCellMessage =
         * findViewById(R.id.approveCellular);
         */
        mPauseButton = (Button) findViewById(R.id.pauseButton);
        // mCancelButton = (Button) findViewById(R.id.cancelButton);

        // mWiFiSettingsButton = (Button) findViewById(R.id.wifiSettingsButton);

        mPauseButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mStatePaused) {
                    mRemoteService.requestContinueDownload();
                } else {
                    mRemoteService.requestPauseDownload();
                }
                setButtonPausedState(!mStatePaused);
            }
        });

        // Setup tutorial button
        findViewById(R.id.tutorialButton).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                startActivity(new Intent(SampleDownloaderActivity.this, VideoActivity.class)
                        .putExtra(
                                VideoActivity.EXTRA_VIDEO_RESOURCE, R.raw.tutorial));
                ActivityAnimationUtil.makeActivityFadeTransition(SampleDownloaderActivity.this);
            }
        });
    }

    /**
     * Called when the activity is first create; we wouldn't create a layout in
     * the case where we have the file and are moving to another activity
     * without downloading.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        MyLog.d("SampleDownloaderActivity: onCreate: ");

        // Set the parent directory for storing the game files, and also check
        // USB storage availability
        // (This is the entry point activity)
        File externalDir = getExternalFilesDir(null);
        if (externalDir != null)
        {
            AndroidPortAdditions.instance().setGameFileParentDir(externalDir.getAbsolutePath());
        }
        else
        {
            MyLog.e("SampleDownloaderActivity: onCreate: no external storage");

            // Show a dialog
            Dialog dlg = DialogUtils.createNegativePositiveDialog(this, 0, R.string.ok,
                    R.string.storage_unavailable, null, null);
            dlg.setOnDismissListener(new OnDismissListener() {
                
                @Override
                public void onDismiss(DialogInterface arg0) {
                 // Finish
                    SampleDownloaderActivity.this.finish();            
                }
            });
            
            dlg.show();
            
            return;
        }

        /**
         * Before we do anything, are the files we expect already here and
         * delivered (presumably by Market) For free titles, this is probably
         * worth doing. (so no Market request is necessary)
         */
        if (!AndroidPortAdditions.STANDALONE_APK && !expansionFilesDelivered()) {

            MyLog.d("SampleDownloaderActivity: onCreate: downloading expansion");

            initializeDownloadUI();

            // Delete old remains if found
            deleteOldExpansionFiles();

            try {
                Intent launchIntent = SampleDownloaderActivity.this
                        .getIntent();
                Intent intentToLaunchThisActivityFromNotification = new Intent(
                        SampleDownloaderActivity
                        .this, SampleDownloaderActivity.this.getClass());
                intentToLaunchThisActivityFromNotification.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK |
                        Intent.FLAG_ACTIVITY_CLEAR_TOP);
                intentToLaunchThisActivityFromNotification.setAction(launchIntent.getAction());

                if (launchIntent.getCategories() != null) {
                    for (String category : launchIntent.getCategories()) {
                        intentToLaunchThisActivityFromNotification.addCategory(category);
                    }
                }

                // Build PendingIntent used to open this activity from
                // Notification
                PendingIntent pendingIntent = PendingIntent.getActivity(
                        SampleDownloaderActivity.this,
                        0, intentToLaunchThisActivityFromNotification,
                        PendingIntent.FLAG_UPDATE_CURRENT);
                // Request to start the download
                int startResult = DownloaderClientMarshaller.startDownloadServiceIfRequired(this,
                        pendingIntent, SampleDownloaderService.class);

                if (startResult != DownloaderClientMarshaller.NO_DOWNLOAD_REQUIRED) {
                    // The DownloaderService has started downloading the files,
                    // show progress
                    initializeDownloadUI();
                    return;
                }

                // otherwise, download not needed so we fall through
            } catch (NameNotFoundException e) {
                Log.e(LOG_TAG, "Cannot find own package! MAYDAY!");
                e.printStackTrace();
            }

        }

        MyLog.d("SampleDownloaderActivity: onCreate: no need to download");

        // Check if the game activity is alive in this process
        if (AndroidPortAdditions.instance().isGameActivityAlive())
        {
            // Go to the game activity instead of the splash and menu flow
            startActivity(new Intent(this, ScummVMActivity.class)
                    .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK));
            finish();
        }
        else
        {
            moveToNextActivity();
        }
    }

    /**
     * Connect the stub to our service on start.
     */
    @Override
    protected void onStart() {
        if (null != mDownloaderClientStub) {
            mDownloaderClientStub.connect(this);
        }
        super.onStart();
        
        StatisticsManager.getStatistics().reportActivityStart(this);
    }

    /**
     * Disconnect the stub from our service on stop
     */
    @Override
    protected void onStop() {
        if (null != mDownloaderClientStub) {
            mDownloaderClientStub.disconnect(this);
        }
        super.onStop();
        
        StatisticsManager.getStatistics().reportActivityStop(this);
    }

    /**
     * Critical implementation detail. In onServiceConnected we create the
     * remote service and marshaler. This is how we pass the client information
     * back to the service so the client can be properly notified of changes. We
     * must do this every time we reconnect to the service.
     */
    @Override
    public void onServiceConnected(Messenger m) {
        mRemoteService = DownloaderServiceMarshaller.CreateProxy(m);
        mRemoteService.onClientUpdated(mDownloaderClientStub.getMessenger());
    }

    /**
     * The download state should trigger changes in the UI --- it may be useful
     * to show the state as being indeterminate at times. This sample can be
     * considered a guideline.
     */
    @Override
    public void onDownloadStateChanged(int newState) {

        MyLog.d("SampleDownloaderActivity: onDownloadStateChanged: state " + newState);

        if (mCompleted)
        {
            MyLog.d("SampleDownloaderActivity: onDownloadStateChanged: already completed");
            return;
        }

        setState(newState);
        boolean showDashboard = true;
        boolean showCellMessage = false;
        boolean paused;
        boolean indeterminate;
        boolean failure = false;
        switch (newState) {
            case IDownloaderClient.STATE_IDLE:
                // STATE_IDLE means the service is listening, so it's
                // safe to start making calls via mRemoteService.
                paused = false;
                indeterminate = true;
                break;
            case IDownloaderClient.STATE_CONNECTING:
            case IDownloaderClient.STATE_FETCHING_URL:
                showDashboard = true;
                paused = false;
                indeterminate = true;
                break;
            case IDownloaderClient.STATE_DOWNLOADING:
                paused = false;
                showDashboard = true;
                indeterminate = false;
                break;

            case IDownloaderClient.STATE_FAILED_CANCELED:
            case IDownloaderClient.STATE_FAILED:
            case IDownloaderClient.STATE_FAILED_FETCHING_URL:
            case IDownloaderClient.STATE_FAILED_UNLICENSED:
            case IDownloaderClient.STATE_FAILED_SDCARD_FULL:
                paused = true;
                showDashboard = false;
                indeterminate = false;
                failure = true;
                break;
            case IDownloaderClient.STATE_PAUSED_NEED_CELLULAR_PERMISSION:
            case IDownloaderClient.STATE_PAUSED_WIFI_DISABLED_NEED_CELLULAR_PERMISSION:
                showDashboard = false;

                // Don't pause on cellular connection, user can pause at will
                mRemoteService.setDownloadFlags(IDownloaderService.FLAGS_DOWNLOAD_OVER_CELLULAR);
                mRemoteService.requestContinueDownload();
                paused = false;

                // paused = true;

                indeterminate = false;
                showCellMessage = true;
                break;

            case IDownloaderClient.STATE_PAUSED_BY_REQUEST:
                paused = true;
                indeterminate = false;
                break;
            case IDownloaderClient.STATE_PAUSED_ROAMING:
            case IDownloaderClient.STATE_PAUSED_SDCARD_UNAVAILABLE:
                paused = true;
                indeterminate = false;
                break;
            case IDownloaderClient.STATE_COMPLETED:
                showDashboard = false;
                paused = false;
                indeterminate = false;
                mCompleted = true;

                validateXAPKZipFiles();
                return;
            default:
                paused = true;
                indeterminate = true;
                showDashboard = true;
        }
        int newDashboardVisibility = showDashboard ? View.VISIBLE : View.GONE;
        /*
         * if (mDashboard.getVisibility() != newDashboardVisibility) {
         * mDashboard.setVisibility(newDashboardVisibility); } int
         * cellMessageVisibility = showCellMessage ? View.VISIBLE : View.GONE;
         * if (mCellMessage.getVisibility() != cellMessageVisibility) {
         * mCellMessage.setVisibility(cellMessageVisibility); }
         */

        // Don't show pause\resume when showing cellular message
        // mPauseButton.setVisibility(showCellMessage ? View.GONE :
        // View.VISIBLE);

        if (failure)
        {
            showDialog(FAILURE_DIALOG_ID);
        }
        else
        {
            // mPB.setIndeterminate(indeterminate);
            setButtonPausedState(paused);
        }
    }

    /**
     * Sets the state of the various controls based on the progressinfo object
     * sent from the downloader service.
     */
    @Override
    public void onDownloadProgress(DownloadProgressInfo progress) {
        /*
         * mAverageSpeed.setText(getString(R.string.kilobytes_per_second,
         * Helpers.getSpeedString(progress.mCurrentSpeed)));
         * mTimeRemaining.setText(getString(R.string.time_remaining,
         * Helpers.getTimeRemaining(progress.mTimeRemaining)));
         */

        if (!mValidating)
        {
            mPB.setMax(10000);
            mPB.setProgress((int) ((progress.mOverallProgress / (double) progress.mOverallTotal) * 10000));

            mTxtProgress.setText(Helpers.getDownloadProgressString
                    (progress.mOverallProgress, progress.mOverallTotal));
        }
        else
        {
            mPB.setMax((int) (progress.mOverallTotal));
            mPB.setProgress((int) (progress.mOverallProgress));
            /*
             * mProgressPercent.setText(Long.toString(progress.mOverallProgress
             * 100 / progress.mOverallTotal) + "%");
             * mProgressFraction.setText(getString(R.string.files_fraction,
             * progress.mOverallProgress, progress.mOverallTotal));
             */

        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == FAILURE_DIALOG_ID)
        {
            int failureMessage;
            switch (mState)
            {
                case IDownloaderClient.STATE_FAILED:
                    failureMessage = R.string.failure_message_unknown;
                    break;
                case IDownloaderClient.STATE_FAILED_CANCELED:
                    failureMessage = R.string.failure_message_cancelled;
                    break;
                case IDownloaderClient.STATE_FAILED_FETCHING_URL:
                    failureMessage = R.string.failure_message_connection;
                    break;
                case IDownloaderClient.STATE_FAILED_UNLICENSED:
                    failureMessage = R.string.failure_message_license;
                    break;
                case IDownloaderClient.STATE_FAILED_SDCARD_FULL:
                    failureMessage = R.string.failure_message_sdcard;
                    break;
                default:
                    failureMessage = R.string.failure_message_unknown;
            }

            Dialog dlg = DialogUtils.createNegativePositiveDialog(this, 0, R.string.ok,
                    failureMessage, null, null);

            dlg.setOnDismissListener(new OnDismissListener() {

                @Override
                public void onDismiss(DialogInterface arg0) {
                    SampleDownloaderActivity.this.finish();

                }
            });

            return dlg;
        }
        else if (id == CORRUPTION_DIALOG_ID)
        {
            Dialog dlg = DialogUtils.createNegativePositiveDialog(this, 0, R.string.ok,
                    R.string.text_validation_failed, null, null);

            dlg.setOnDismissListener(new OnDismissListener() {

                @Override
                public void onDismiss(DialogInterface arg0) {
                    restartActivity();
                }
            });

            return dlg;
        }
        return super.onCreateDialog(id);
    }

    @Override
    protected void onDestroy() {
        this.mCancelValidation = true;
        super.onDestroy();

        if (mIsRestarting) {
            startActivity(new Intent(SampleDownloaderActivity.this,
                    SampleDownloaderActivity.class));
            ActivityAnimationUtil.makeActivityNullTransition(this);

        }

    }

    private void restartActivity()
    {
        MyLog.d("SampleDownloaderActivity: restartActivity: ");

        // Delete corrupt expansion files
        for (XAPKFile xf : xAPKS) {
            String fileName = Helpers.getExpansionAPKFileName(
                    SampleDownloaderActivity.this,
                    xf.mIsMain, xf.mFileVersion);
            if (Helpers.doesFileExist(SampleDownloaderActivity.this, fileName,
                    xf.mFileSize, false))
            {
                fileName = Helpers
                        .generateSaveFileName(SampleDownloaderActivity.this,
                                fileName);

                File file = new File(fileName);
                file.delete();
            }
        }

        // Force the download DB to clear itself due to library bugs
        DownloadsDB.clearAllData(this);
        stopService(new Intent(this, SampleDownloaderService.class));

        // Restart this activity
        mIsRestarting = true;
        finish();
        ActivityAnimationUtil.makeActivityNullTransition(this);
    }

    private void moveToNextActivity()
    {
        MyLog.d("SampleDownloaderActivity: moveToNextActivity: ");

        // Force the download DB to clear itself due to library bugs
        DownloadsDB.clearAllData(this);
        stopService(new Intent(this, SampleDownloaderService.class));

        // Force the downloader service to stop
        startActivity(new Intent(this, SplashActivity.class));
        finish();
        ActivityAnimationUtil.makeActivityNullTransition(this);

    }
}
