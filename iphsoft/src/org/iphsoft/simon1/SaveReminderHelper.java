
package org.iphsoft.simon1;

import org.iphsoft.simon1.util.MyLog;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.widget.Toast;
import com.mojotouch.simon.R;

public class SaveReminderHelper {

    public static synchronized SaveReminderHelper instance()
    {
        if (sInstance == null)
        {
            sInstance = new SaveReminderHelper();
        }

        return sInstance;
    }

    public void startReminders()
    {
        ScummVMApplication.getContext().registerReceiver(mAlarmReceiver,
                new IntentFilter(ACTION_UPDATE));

        AlarmManager am = (AlarmManager) ScummVMApplication.getContext().getSystemService(
                Context.ALARM_SERVICE);

        am.setRepeating(AlarmManager.RTC, System.currentTimeMillis()
                + REMINDER_INTERVAL, REMINDER_INTERVAL,
                mPendingIntent);

    }

    public void stopReminders()
    {
        AlarmManager am = (AlarmManager) ScummVMApplication.getContext().getSystemService(
                Context.ALARM_SERVICE);
        am.cancel(mPendingIntent);

        ScummVMApplication.getContext().unregisterReceiver(mAlarmReceiver);
    }
    
    public long getLastSaveTime()
    {
        return mLastSaveTime;
    }

    public void setLastSaveTime(long millis)
    {
        mLastSaveTime = millis;
    }

    public void setLastPauseTime(long millis)
    {
        mLastPauseTime = millis;
    }

    public void setGamePaused(boolean isPaused)
    {
        mIsPaused = isPaused;
    }

    private SaveReminderHelper()
    {
        mPendingIntent = PendingIntent.getBroadcast(ScummVMApplication.getContext(), 0, new Intent(
                ACTION_UPDATE), 0);

    }

    private class AlarmReceiver extends BroadcastReceiver
    {
        @Override
        public void onReceive(Context context, Intent intent) {

            MyLog.d("SaveReminderHelper.AlarmReceiver: onReceive: ");
            
            // Check some conditions for displaying the reminder
            if (mIsPaused)
            {
                // If the game is paused, we won't remind if the time between
                // the last save and last pause was small enough
                if (mLastPauseTime != 0 && mLastSaveTime != 0 && mLastPauseTime - mLastSaveTime < MIN_GAP_FOR_REMINDER)
                {
                    MyLog.d("SaveReminderHelper.AlarmReceiver: onReceive: skipping reminder, paused state");
                    return;
                }
            }
            else
            {
                if (mLastSaveTime != 0 && System.currentTimeMillis() - mLastSaveTime < MIN_GAP_FOR_REMINDER)
                {
                    MyLog.d("SaveReminderHelper.AlarmReceiver: onReceive: skipping rmeinder, active state");
                    return;
                }
            }

            // Show a toast with a save reminder
            Context appContext = ScummVMApplication.getContext();
            String appName = AndroidPortAdditions.instance().getAppName();
            String message = appContext.getString(R.string.save_game_reminder, appName);

            // TODO disable for now, implemented more simply
        //    Toast.makeText(appContext, message, Toast.LENGTH_LONG).show();   
        }
    }

    // Timestamps for checking if the game was played for long enough to remind
    // of saving
    private long mLastSaveTime = 0;
    private long mLastPauseTime = 0;

    private AlarmReceiver mAlarmReceiver = new AlarmReceiver();

    private PendingIntent mPendingIntent;

    private boolean mIsPaused;

    private static SaveReminderHelper sInstance;

    private static final String ACTION_UPDATE = "org.iphsoft.simon1.ACTION_UPDATE";

    public static final long REMINDER_INTERVAL = 60 * 60 * 1000;
    public static final long MIN_GAP_FOR_REMINDER = 5 * 60 * 1000;

}
