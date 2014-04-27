
package org.iphsoft.simon1.ui;

import com.mojotouch.simon.R;
import org.iphsoft.simon1.util.MyLog;

import android.content.Context;
import android.content.res.Configuration;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.TypedValue;
import android.widget.Button;

public class ExtendedButton extends Button {

    public ExtendedButton(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

        init(context);
    }

    public ExtendedButton(Context context, AttributeSet attrs) {
        super(context, attrs);

        init(context);
    }

    public ExtendedButton(Context context) {
        super(context);

        init(context);
    }

    protected void init(Context context)
    {
        if (!isInEditMode())
        {
            // Set font
            Typeface tf = Typeface.createFromAsset(context.getAssets(),
                    "scarecrow.ttf");
            setTypeface(tf);
        }

        // Set button drawable
        setBackgroundResource(R.drawable.button_selector);

        // Set text size
        float density = getResources().getDisplayMetrics().density;
        setTextSize(getResources().getDimensionPixelSize(R.dimen.button_text_size)
                / density);

        setTextColor(getResources().getColorStateList(R.drawable.button_text_color));

        setShadowLayer(1.0f, 2.0f, 2.0f, getResources().getColor(R.color.button_shadow));

        // Workaround for 9-patch sucky padding behavior on mdpi - just set the padding manually
        int densityDpi = getResources().getDisplayMetrics().densityDpi;
        int screenSize = getResources().getConfiguration().screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK;
        if (densityDpi == DisplayMetrics.DENSITY_MEDIUM && screenSize == Configuration.SCREENLAYOUT_SIZE_NORMAL)
        {
            MyLog.d("ExtendedButton: init: mdpi workaround");
            setPadding(10, 2, 10, 0);
        }
        
        setLines(1);
        setMaxLines(1);
        setSingleLine(true);

        setEnabledStateLook();
    }

    @Override
    public void setEnabled(boolean enabled) {
        // TODO Auto-generated method stub
        super.setEnabled(enabled);

        setEnabledStateLook();
    }

    protected void setEnabledStateLook()
    {
        // int colorResource = isEnabled() ? R.color.black : R.color.grey;
        // setTextColor(getContext().getResources().getColor(colorResource));
    }
}
