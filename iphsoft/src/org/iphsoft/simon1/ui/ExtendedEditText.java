
package org.iphsoft.simon1.ui;

import com.mojotouch.simon.R;

import android.content.Context;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.util.TypedValue;
import android.widget.EditText;

public class ExtendedEditText extends EditText {

    public ExtendedEditText(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

        init(context);
    }

    public ExtendedEditText(Context context, AttributeSet attrs) {
        super(context, attrs);

        init(context);
    }

    public ExtendedEditText(Context context) {
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
        setBackgroundResource(R.drawable.text_input);

        // Set text size
        setTextSize(TypedValue.COMPLEX_UNIT_SP, 18);
        
        setTextColor(getResources().getColor(R.color.black));

    }
}
