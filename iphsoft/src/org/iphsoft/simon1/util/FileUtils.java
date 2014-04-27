
package org.iphsoft.simon1.util;

import android.os.Environment;
import android.os.StatFs;

public class FileUtils {

    public static long getBytesAvailableOnExternalStorage() {
        StatFs stat = new StatFs(Environment.getExternalStorageDirectory().getPath());
        long bytes = (long) stat.getBlockSize() * (long) stat.getAvailableBlocks();
        
        MyLog.d("FileUtils: getBytesAvailableOnExternalStorage: bytes available " + bytes);
        
        return bytes;
    }
}
