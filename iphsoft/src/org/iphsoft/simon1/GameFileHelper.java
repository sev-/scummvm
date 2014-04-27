
package org.iphsoft.simon1;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import org.iphsoft.simon1.AndroidPortAdditions.GameType;
import org.iphsoft.simon1.util.MyLog;

import android.os.AsyncTask;

import com.android.vending.expansion.zipfile.APKExpansionSupport;
import com.android.vending.expansion.zipfile.ZipResourceFile;
import com.android.vending.expansion.zipfile.ZipResourceFile.ZipEntryRO;

public class GameFileHelper extends AsyncTask<Void, Integer, Boolean> {

    public enum Language
    {
        ENGLISH("En/"), GERMAN("De/"), GERMAN_SUBS("De_sub/"), HEBREW("He/"), SPANISH(
                "Es/"), FRENCH("Fr/"), ITALIAN("It/"), RUSSIAN("Ru/"), ENGLISH_SUBS("En_sub/");

        public String getInnerFolderName()
        {
            return mInnerFolderName;
        }

        private Language(String innerFolderName)
        {
            mInnerFolderName = innerFolderName;
        }

        private String mInnerFolderName;
    }

    public interface ProgressListener
    {
        public void onProgress(int progress, int max);

        public void onPostExecute(boolean result);
    }

    public boolean checkThatGameFilesExist()
    {
        MyLog.d("GameFileHelper: checkThatGameFilesExist: ");

        File gameFolder = new File(mDestGameFolder);
        if (!gameFolder.exists() || !gameFolder.isDirectory())
        {
            // Folder doesn't exist
            MyLog.d("GameFileHelper: checkThatGameFilesExist: folder " + mDestGameFolder
                    + " does not exist");

            return false;
        }

        File[] gameFiles = gameFolder.listFiles();
        long totalSize = 0;
        for (File file : gameFiles)
        {
            totalSize += file.length();
        }

        long expectedSize = AndroidPortAdditions.instance().getGameFilesSize();
        if (CHECK_GAME_FILE_SIZE && totalSize != expectedSize)
        {
            // Size does not match
            MyLog.d("GameFileHelper: checkThatGameFilesExist: folder's size is " + totalSize
                    + ", does not match expected size " + expectedSize);

            return false;
        }

        MyLog.d("GameFileHelper: checkThatGameFilesExist: total " + totalSize + " expected "
                + expectedSize);

        return true;
    }

    public GameFileHelper(String destGameFolder, Language language) {

        mDestGameFolder = destGameFolder;
        mLanguage = language;
    }

    public void setProgressListener(ProgressListener listener)
    {
        mListener = listener;
    }

    @Override
    protected Boolean doInBackground(Void... arg0) {

        try {
            prepareGameFilesForLanguage();
        } catch (IOException e) {
            MyLog.e("GameFileHelper: doInBackground: " + e);
            e.printStackTrace();

            return false;
        }

        return true;
    }

    @Override
    protected void onPostExecute(Boolean result) {

        if (mListener != null)
        {
            mListener.onPostExecute(result);
        }
    }

    @Override
    protected void onProgressUpdate(Integer... values) {

        if (mListener != null)
        {
            // Notify the external listener on progress
            mListener.onProgress(values[0], values[1]);
        }
    }
    
    protected File initGameDirectory()
    {
        File destGameFolderFile = new File(mDestGameFolder);
        if (destGameFolderFile.exists())
        {
            File[] files = destGameFolderFile.listFiles();
            if (files != null)
            {
                for (File f : files)
                {
                    f.delete();
                }
            }

            destGameFolderFile.delete();
        }

        // Create a new directory
        destGameFolderFile.mkdirs();
        
        return destGameFolderFile;
    }

    protected void prepareGameFilesForLanguage()
            throws IOException
    {
        MyLog.d("GameFileHelper: prepareGameFilesForLanguage: " + mDestGameFolder + " "
                + mLanguage.name());

        MyLog.d("GameFileHelper: prepareGameFilesForLanguage: preparing an empty directory");

        // First, make sure we have a new empty directory.
       File destGameFolderFile = initGameDirectory();
       
        MyLog.d("GameFileHelper: prepareGameFilesForLanguage: extracting game files");

        // Extract the needed files from the archive
        ZipResourceFile gameArchive = APKExpansionSupport.getAPKExpansionZipFile(
                ScummVMApplication.getContext(), AndroidPortAdditions.instance()
                        .getAPKExtensionFileVersion(),
                0);

        if (gameArchive == null)
        {
            throw new IOException("Unable to open game archive");
        }

        ZipEntryRO[] entries = gameArchive.getAllEntries();

        int progressMax = entries.length;
        int progress = 0;
        publishProgress(progress, progressMax);

        MyLog.d("GameFileHelper: prepareGameFilesForLanguage: entries: " + entries.length);

        mCopyBuffer = new byte[65536];
        for (ZipEntryRO entry : entries)
        {
            MyLog.d("GameFileHelper: prepareGameFilesForLanguage: entry " + entry.mFileName);

            if (isCancelled())
            {
                // Allow for interrupt
                return;
            }

            String entryName = entry.mFileName;

            // Strip the top level dir
            String strippedEntryName = entryName.substring(BASE_ARCHIVE_DIR.length());

            // Check to see if it's a top-level file (no slashes)
            if (strippedEntryName.indexOf('/') < 0)
            {
                // Contains only the top level directory, copy it
                String destFile = destGameFolderFile.getAbsolutePath() + "/" + strippedEntryName;
                copyFile(destFile, entry, gameArchive, false);
            }
            else
            {
                // Check if the file belongs the the language folder we need
                String languagePathPrefix = mLanguage.getInnerFolderName();
                if (strippedEntryName.indexOf(languagePathPrefix) == 0)
                {
                    strippedEntryName = strippedEntryName.substring(languagePathPrefix.length());

                    if (strippedEntryName.length() > 0)
                    {
                        String destFile = destGameFolderFile.getAbsolutePath() + "/"
                                + strippedEntryName;
                        copyFile(destFile, entry, gameArchive, true);
                    }
                }
            }

            ++progress;
            publishProgress(progress, progressMax);
        }

        mCopyBuffer = null;

        // Sum the size of all copied files
        long totalGamefilesSize = 0;
        File[] gameFiles = destGameFolderFile.listFiles();
        for (File f : gameFiles)
        {
            totalGamefilesSize += f.length();
        }
        
        MyLog.d("GameFileHelper: prepareGameFilesForLanguage: totalGamefilesSize "  + totalGamefilesSize);

        AndroidPortAdditions.instance().setGameFilesSize(totalGamefilesSize);
    }

    private void copyFile(String destFilename, ZipEntryRO entry, ZipResourceFile resourceFile,
            boolean override)
            throws IOException
    {
        MyLog.d("GameFileHelper: copyFile: " + destFilename + " " + override);

        if (!override && (new File(destFilename).exists()))
        {
            // Skip that file
            return;
        }

        // If the file exists, override it
        FileOutputStream fos = new FileOutputStream(destFilename, false);

        DataInputStream dis = new DataInputStream(resourceFile.getInputStream(entry.mFileName));

        long length = entry.mUncompressedLength;
        int seek;

        while (length > 0)
        {
            if (isCancelled())
            {
                // Allow for interrupt
                fos.close();
                dis.close();
                return;
            }

            seek = (int) (length > mCopyBuffer.length ? mCopyBuffer.length
                    : length);
            dis.readFully(mCopyBuffer, 0, seek);

            fos.write(mCopyBuffer, 0, seek);

            length -= seek;

        }

        fos.flush();

        fos.close();
        dis.close();
    }


    protected String mDestGameFolder;
    protected Language mLanguage;
    protected ProgressListener mListener;
    protected byte[] mCopyBuffer;

    private static final String BASE_ARCHIVE_DIR = "Simon1_data_all/";
    
    private static final boolean CHECK_GAME_FILE_SIZE = AndroidPortAdditions.GAME_TYPE == GameType.SIMON1 ? true : false;
}
