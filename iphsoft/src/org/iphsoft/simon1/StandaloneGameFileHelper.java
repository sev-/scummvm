package org.iphsoft.simon1;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.iphsoft.simon1.util.MyLog;

/**
 * This is a version of the GameFileHelper that install from a standalone APK (without OBB expansion)
 * 
 * @author omergilad
 */
public class StandaloneGameFileHelper extends GameFileHelper {

    public StandaloneGameFileHelper(String destGameFolder, Language language) {
        super(destGameFolder, language);
        // TODO Auto-generated constructor stub
    }

    @Override
    protected void prepareGameFilesForLanguage() throws IOException {
        
        
        MyLog.d("StandaloneGameFileHelper: prepareGameFilesForLanguage: " + mDestGameFolder + " "
                + mLanguage.name());

        MyLog.d("StandaloneGameFileHelper: prepareGameFilesForLanguage: preparing an empty directory");

        // First, make sure we have a new empty directory.
        File destGameFolderFile = initGameDirectory();

        MyLog.d("StandaloneGameFileHelper: prepareGameFilesForLanguage: extracting game files");
       
        // Obtain all root entries
        String[] rootEntries = ScummVMApplication.getContext().getAssets().list(BASE_ASSET_DIR);

        // Obtain all language override entries
        String subFolder = mLanguage.getInnerFolderName();
        subFolder = subFolder.substring(0,  subFolder.length() - 1);
        String[] languageEntries = ScummVMApplication.getContext().getAssets().list(BASE_ASSET_DIR + File.separator + subFolder);

        int progressMax = rootEntries.length + languageEntries.length;
        int progress = 0;
        publishProgress(progress, progressMax);
        
        MyLog.d("prepareGameFilesForLanguage: root entries: " + rootEntries.length);
        MyLog.d("prepareGameFilesForLanguage: language entries: " + languageEntries.length);

        // Copy all root entries
        mCopyBuffer = new byte[65536];
        for (String entry : rootEntries)
        {
            // Check if the entry is one of the language folders
            boolean ignore = false;
            for (Language lang : Language.values())
            {
                if ((entry + File.separator).equalsIgnoreCase(lang.getInnerFolderName()))
                {
                    ignore = true;
                }
            }
            
            if (ignore)
            {
                continue;
            }
            
            
            String assetName = BASE_ASSET_DIR + File.separator + entry;
            
            MyLog.d("prepareGameFilesForLanguage: prepareGameFilesForLanguage: root entry " + assetName);

            if (isCancelled())
            {
                // Allow for interrupt
                return;
            }
            
            copyAsset(destGameFolderFile, assetName);
            
            ++progress;
            publishProgress(progress, progressMax);
        }

        // Copy all language entries
        for (String entry : languageEntries)
        {
            String assetName = BASE_ASSET_DIR + File.separator + mLanguage.getInnerFolderName() + entry;

            MyLog.d("prepareGameFilesForLanguage: prepareGameFilesForLanguage: language entry " + assetName);

            if (isCancelled())
            {
                // Allow for interrupt
                return;
            }
            
            copyAsset(destGameFolderFile, assetName);
            
            ++progress;
            publishProgress(progress, progressMax);
        } 

        // Sum the size of all copied files
        long totalGamefilesSize = 0;
        File[] gameFiles = destGameFolderFile.listFiles();
        for (File f : gameFiles)
        {
            totalGamefilesSize += f.length();
        }
        
        MyLog.d("StandaloneGameFileHelper: prepareGameFilesForLanguage: totalGamefilesSize "  + totalGamefilesSize);


        AndroidPortAdditions.instance().setGameFilesSize(totalGamefilesSize);    
    }
    
    private void copyAsset(File dest, String asset) throws IOException
    {
        // Create the dest file
        String destFilename = asset.substring(asset.lastIndexOf(File.separator) + 1);
       
        // If the file exists, override it
        OutputStream fos = new FileOutputStream(dest.getAbsolutePath() + File.separator + destFilename, false);
        InputStream is = ScummVMApplication.getContext().getAssets().open(asset);

        // Copy
        int read = 0;
        while ((read = is.read(mCopyBuffer)) != -1)
        {
            fos.write(mCopyBuffer, 0, read);
            
            if (isCancelled())
            {
                // Allow for interrupt
                fos.close();
                is.close();
                return;
            }
        }
        
        fos.flush();

        fos.close();
        is.close();
    }
    
    
    private static final String BASE_ASSET_DIR = "Simon1_data_all";

}
