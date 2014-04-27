
package org.iphsoft.simon1.ui;

import com.mojotouch.simon.R;
import org.iphsoft.simon1.util.MyLog;

import android.content.Context;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.util.TypedValue;
import android.widget.RadioButton;

public class ExtendedRadioButton extends RadioButton {

    public ExtendedRadioButton(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init(context);
    }

    public ExtendedRadioButton(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    public ExtendedRadioButton(Context context) {
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

        setButtonDrawable(R.drawable.radiobutton_selector);
        setBackgroundDrawable(null);

        setTextColor(getResources().getColorStateList(R.drawable.button_text_color));
        setTextSize(TypedValue.COMPLEX_UNIT_SP, 14);

        int padding = (int) (getResources().getDimension(R.dimen.radio_button_padding)
                / getResources().getDisplayMetrics().density);

        setPadding(getPaddingLeft() + padding, padding, padding, padding);
    }
}
