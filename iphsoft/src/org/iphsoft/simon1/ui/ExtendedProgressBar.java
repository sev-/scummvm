
package org.iphsoft.simon1.ui;

import com.mojotouch.simon.R;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.ProgressBar;

public class ExtendedProgressBar extends ProgressBar {

    public ExtendedProgressBar(Context context) {
        super(context);

        init(context);
    }

    public ExtendedProgressBar(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);

    }

    public ExtendedProgressBar(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init(context);

    }

    protected void init(Context context)
    {
        // Set background
        setBackgroundResource(R.drawable.loaderout);

        // Set progress drawable
        setProgressDrawable(context.getResources().getDrawable(R.drawable.progress));
    }
}
