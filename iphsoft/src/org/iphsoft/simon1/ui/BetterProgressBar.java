
package org.iphsoft.simon1.ui;

import com.mojotouch.simon.R;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.Gravity;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;

public class BetterProgressBar extends FrameLayout {

    public BetterProgressBar(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init(context);
    }

    public BetterProgressBar(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    public BetterProgressBar(Context context) {
        super(context);
        init(context);
    }

    public void setProgress(int progress)
    {
        mProgress = progress;
        updateProgress();
    }

    public void setMax(int max)
    {
        mMax = max;
        updateProgress();
    }

    private void init(Context context)
    {
        // Set progress background
        ImageView progressBackground = new ImageView(context);
        progressBackground.setImageResource(R.drawable.loaderout);
        progressBackground.setScaleType(ScaleType.FIT_XY);
        progressBackground.setLayoutParams(new FrameLayout.LayoutParams(LayoutParams.MATCH_PARENT,
                LayoutParams.WRAP_CONTENT, Gravity.CENTER));
        addView(progressBackground);

        // Add foreground view
        mProgressDrawable = getResources().getDrawable(R.drawable.progress);
        ImageView progressForeground = new ImageView(context);
        progressForeground.setImageDrawable(mProgressDrawable);
        progressForeground.setScaleType(ScaleType.FIT_XY);
        progressForeground.setLayoutParams(new FrameLayout.LayoutParams(LayoutParams.MATCH_PARENT,
                LayoutParams.WRAP_CONTENT, Gravity.CENTER));
        addView(progressForeground);
    }

    private void updateProgress()
    {
        if (mMax == 0)
        {
            // Prevent division by zero
            mProgressDrawable.setLevel(0);
        }
        else
        {
            mProgressDrawable.setLevel((mProgress * 10000) / mMax);
        }
        invalidate();
    }

    private Drawable mProgressDrawable;

    private int mProgress = 0;
    private int mMax = 0;
}
