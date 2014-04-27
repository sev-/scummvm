
package org.iphsoft.simon1.ui;

import android.content.Context;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.widget.TextView;

public class ExtendedTextView extends TextView {

    public ExtendedTextView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

        init(context);
    }

    public ExtendedTextView(Context context, AttributeSet attrs) {
        super(context, attrs);

        init(context);
    }

    public ExtendedTextView(Context context) {
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
    }

}
