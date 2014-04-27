
package org.iphsoft.simon1.ui;

import org.iphsoft.simon1.util.MyLog;

import android.content.Context;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Typeface;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.widget.ImageButton;

import com.mojotouch.simon.R;

public class ExtendedImageButton extends ImageButton {

    public ExtendedImageButton(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

        init(context);
    }

    public ExtendedImageButton(Context context, AttributeSet attrs) {
        super(context, attrs);

        init(context);
    }

    public ExtendedImageButton(Context context) {
        super(context);

        init(context);
    }

    protected void init(Context context)
    {
        // Set button drawable
        setBackgroundResource(R.drawable.button_selector);
        
        setPadding(getPaddingLeft() / 2, getPaddingTop() / 2, getPaddingRight() / 2, getPaddingBottom() / 2);
    }

    @Override
    public void setEnabled(boolean enabled) {
        // TODO Auto-generated method stub
        super.setEnabled(enabled);
    }
}
