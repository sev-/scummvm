
package org.iphsoft.simon1;

import org.iphsoft.simon1.stats.StatisticsManager;
import org.iphsoft.simon1.ui.ActivityAnimationUtil;
import org.iphsoft.simon1.util.IntentUtils;
import org.iphsoft.simon1.util.Version;

import android.app.Activity;
import android.os.Bundle;
import android.text.Html;
import android.text.method.LinkMovementMethod;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.TextView;

import com.mojotouch.simon.R;

public class AboutActivity extends Activity {

    // Workaround for string resource limitations
    private static final String ABOUT_STRING_2 = "<br>Published and Developed by MojoTouch.<br>All Rights Reserved.<br><br>Lead Developer: Omer Gilad<br>UI graphics by Zach Sigal,<br><i>I &lt;M&gt; Duck</i>";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.about);

        TextView txtAbout1 = (TextView) findViewById(R.id.txtAbout1);
        TextView txtAbout2 = (TextView) findViewById(R.id.txtAbout2);
        TextView txtAbout3 = (TextView) findViewById(R.id.txtAbout3);
        TextView txtAbout4 = (TextView) findViewById(R.id.txtAbout4);
        TextView txtAbout5 = (TextView) findViewById(R.id.txtAbout5);

        // Add the about string arguments
        String about1 = getString(R.string.about1, Version.getPackageVersion());
        txtAbout1.setText(about1);

        txtAbout2.setText(Html.fromHtml(ABOUT_STRING_2));

        // Activate links
        txtAbout3.setMovementMethod(LinkMovementMethod.getInstance());
        txtAbout4.setMovementMethod(LinkMovementMethod.getInstance());
        txtAbout5.setMovementMethod(LinkMovementMethod.getInstance());

        // Set image links
        findViewById(R.id.imgMojotouch).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                IntentUtils.showBrowserApp(AboutActivity.this,
                        getResources().getString(R.string.mojotouch_url));

            }
        });

        findViewById(R.id.imgScummVM).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                IntentUtils.showBrowserApp(AboutActivity.this,
                        getResources().getString(R.string.scummvm_url));

            }
        });

        findViewById(R.id.imgWoodcock).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                IntentUtils.showBrowserApp(AboutActivity.this,
                        getResources().getString(R.string.james_woodcock_url));

            }
        });

        // Back button
        findViewById(R.id.btnBack).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                finish();
                ActivityAnimationUtil.makeActivityNullTransition(AboutActivity.this);

            }
        });

        // "rate us" button
        findViewById(R.id.btnRate).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                IntentUtils.startAppStoreActivity(AboutActivity.this, getPackageName());

            }
        });
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
}
