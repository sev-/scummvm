
package org.iphsoft.simon1.ui;

import java.util.List;

import com.mojotouch.simon.R;
import org.iphsoft.simon1.util.MyLog;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;

public class ToggledExtendedButton extends ExtendedButton {

    public interface OnToggleListener
    {
        public void onToggle(int index);
    }
    
    public ToggledExtendedButton(Context context) {
        super(context);
    }

    public ToggledExtendedButton(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public ToggledExtendedButton(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void setLabels(List<CharSequence> labels)
    {
        mLabels = labels;
        updateLabel();
    }
    
    public void setOnToggleListener(OnToggleListener listener)
    {
        mListener = listener;
    }

    public void setLabelIndex(int index)
    {
        mLabelIndex = index;
        updateLabel();
    }

    public int getLabelIndex()
    {
        return mLabelIndex;
    }

    private void updateLabel()
    {
        if (mLabels == null || mLabelIndex >= mLabels.size())
        {
            MyLog.e("ToggledExtendedButton: updateLabel: bad index or no labels");
            return;
        }
        
        setText(mLabels.get(mLabelIndex));
    }

    protected void init(Context context)
    {
        super.init(context);

        // Add arrows on the sides
        setCompoundDrawablesWithIntrinsicBounds(context.getResources().getDrawable(R.drawable.arrowl), null, context
                .getResources().getDrawable(R.drawable.arrowr), null);
        setCompoundDrawablePadding(10);
        
        // Set click behavior
        setOnClickListener(new OnClickListener() {
            
            @Override
            public void onClick(View arg0) {
                if (mLabels == null)
                {
                    MyLog.e("ToggledExtendedButton: updateLabel: no labels");

                    return;
                }
                
                // Increase index
                ++mLabelIndex;
                if (mLabelIndex >= mLabels.size())
                {
                    mLabelIndex = 0;
                }
                
                // Update
                updateLabel();
                
                if (mListener != null)
                {
                    mListener.onToggle(mLabelIndex);
                }
            }
        });
    }

    private List<CharSequence> mLabels;
    private int mLabelIndex = 0;
    private OnToggleListener mListener;
}
