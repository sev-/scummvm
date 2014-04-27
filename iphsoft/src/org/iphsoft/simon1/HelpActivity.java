
package org.iphsoft.simon1;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;

import android.app.Activity;
import android.os.Bundle;
import android.view.KeyEvent;
import android.webkit.WebView;
import com.mojotouch.simon.R;

public class HelpActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);

        setContentView(R.layout.help);

        wvHelp = (WebView) findViewById(R.id.wvHelp);

        try {
            // Load the local HTML file
            ByteArrayOutputStream bytes = new ByteArrayOutputStream();
            InputStream is = getAssets().open(
                    "help/help.html");
            byte[] buf = new byte[1024];
            int count;
            while ((count = is.read(buf)) != -1) {
                bytes.write(buf, 0, count);
            }
            is.close();

            mBaseHtml = new String(bytes.toByteArray(), "utf-16");

            wvHelp.loadDataWithBaseURL("file:///android_asset/help/",
                    mBaseHtml, "text/html", null, null);

        } catch (Exception e) {
            e.printStackTrace();
            finish();
        }

    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {

        // Override back button behavior for WebView
        if (keyCode == KeyEvent.KEYCODE_BACK && wvHelp.canGoBack())
        {
            wvHelp.loadDataWithBaseURL("file:///android_asset/help/",
                    mBaseHtml, "text/html", null, null);
            wvHelp.clearHistory();
            return true;
        }

        return super.onKeyUp(keyCode, event);
    }

    private WebView wvHelp;
    
    private String mBaseHtml;
}
