
package org.iphsoft.simon1.util;

import com.mojotouch.simon.R;

import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.graphics.drawable.ColorDrawable;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

/**
 * Utils for simplifying dialog creation
 * 
 * @author omergilad
 */
public class DialogUtils {

    public static interface InputListener
    {
        public void onInputReceived(String input);
    }

    public static Dialog createNegativePositiveDialog(Context context,
            int negative, int positive, int message, final OnClickListener negativeListener,
            final OnClickListener positiveListener) {

        return createNegativePositiveDialog(context, negative, positive, message, negativeListener,
                positiveListener, null, "");
    }

    public static Dialog createNegativePositiveDialog(Context context,
            int negative, int positive, int message, final OnClickListener negativeListener,
            final OnClickListener positiveListener, final InputListener inputListener,
            String initialInput) {

        Dialog dlg = new Dialog(context);
        Window window = dlg.getWindow();

        window.setLayout(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
        window.setGravity(Gravity.CENTER);

        window.requestFeature(Window.FEATURE_NO_TITLE);

        window.setBackgroundDrawable(new ColorDrawable(0x00000000));

        dlg.setContentView(R.layout.game_dialog);

        TextView txtTitle = (TextView) dlg.findViewById(R.id.txtTitle);
        txtTitle.setText(message);

        dlg.findViewById(R.id.imgSpinner).setVisibility(View.GONE);
        dlg.findViewById(R.id.lvList).setVisibility(View.GONE);
   //     dlg.findViewById(R.id.separator1).setVisibility(View.GONE);
   //     dlg.findViewById(R.id.separator2).setVisibility(View.GONE);


        final EditText etInput = (EditText) dlg.findViewById(R.id.etInput);
        etInput.setLines(1);
        etInput.setMaxLines(1);
        etInput.setSingleLine(true);
        etInput.setText(initialInput);
        if (inputListener == null)
        {
            // No need for input
            etInput.setVisibility(View.GONE);
        }

        Button btnNegative = (Button) dlg.findViewById(R.id.btnNegative);
        Button btnPositive = (Button) dlg.findViewById(R.id.btnPositive);

        if (negative != 0)
        {
            btnNegative.setText(negative);
            btnNegative.setTag(dlg);
            btnNegative.setOnClickListener(new View.OnClickListener() {

                @Override
                public void onClick(View btn) {

                    DialogInterface dlgInterface = (DialogInterface) btn.getTag();

                    if (negativeListener != null)
                    {
                        negativeListener.onClick(dlgInterface, 0);
                    }

                    dlgInterface.dismiss();
                }
            });
        }
        else
        {
            btnNegative.setVisibility(View.GONE);
            dlg.findViewById(R.id.fillerView1).setVisibility(View.GONE);
        }

        if (positive != 0)
        {
            btnPositive.setText(positive);
            btnPositive.setTag(dlg);
            btnPositive.setOnClickListener(new View.OnClickListener() {

                @Override
                public void onClick(View btn) {
                    DialogInterface dlgInterface = (DialogInterface) btn.getTag();

                    if (positiveListener != null)
                    {
                        positiveListener.onClick(dlgInterface, 1);
                    }

                    if (inputListener != null)
                    {
                        inputListener.onInputReceived(etInput.getText().toString());
                    }
                    
                    dlgInterface.dismiss();
                }
            });
        }
        else
        {
            btnPositive.setVisibility(View.GONE);
            dlg.findViewById(R.id.fillerView1).setVisibility(View.GONE);
            
            // Remove the negative button margin
            LinearLayout.LayoutParams lp = (LinearLayout.LayoutParams) btnNegative.getLayoutParams();
            lp.leftMargin = 0;
            btnNegative.setLayoutParams(lp);
        }

        return dlg;
    }

    public static Dialog createProgressDialog(Context context, int message) {

        Dialog dlg = new Dialog(context);
        Window window = dlg.getWindow();

        window.setLayout(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
        window.setGravity(Gravity.CENTER);

        window.requestFeature(Window.FEATURE_NO_TITLE);

        window.setBackgroundDrawable(new ColorDrawable(0x00000000));

        dlg.setContentView(R.layout.game_dialog);

        TextView txtTitle = (TextView) dlg.findViewById(R.id.txtTitle);
        txtTitle.setText(message);
        
        View imgSpinner = dlg.findViewById(R.id.imgSpinner);
        Animation rotation = AnimationUtils.loadAnimation(context, R.anim.rotate);
        imgSpinner.startAnimation(rotation);

        dlg.findViewById(R.id.etInput).setVisibility(View.GONE);
        dlg.findViewById(R.id.btnPositive).setVisibility(View.GONE);
        dlg.findViewById(R.id.btnNegative).setVisibility(View.GONE);
        dlg.findViewById(R.id.lvList).setVisibility(View.GONE);
        dlg.findViewById(R.id.layoutButtons).setVisibility(View.GONE);
    //    dlg.findViewById(R.id.separator1).setVisibility(View.GONE);
    //    dlg.findViewById(R.id.separator2).setVisibility(View.GONE);

        return dlg;
    }

    public static Dialog createListDialog(Context context,
            int negative, int message, final OnClickListener selectionListener, CharSequence[] items) {

        Dialog dlg = new Dialog(context);
        Window window = dlg.getWindow();

        window.setLayout(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
        window.setGravity(Gravity.CENTER);

        window.requestFeature(Window.FEATURE_NO_TITLE);

        window.setBackgroundDrawable(new ColorDrawable(0x00000000));

        dlg.setContentView(R.layout.game_dialog);

        TextView txtTitle = (TextView) dlg.findViewById(R.id.txtTitle);
        txtTitle.setText(message);

        ListView lvList = (ListView) dlg.findViewById(R.id.lvList);
        
        // Adding header to make the ListView display an additional divider on top
        View invisible1 = new View(context);
        invisible1.setLayoutParams(new AbsListView.LayoutParams(0, 0));
        lvList.addHeaderView(invisible1);
        
        lvList.setTag(dlg);
        lvList.setAdapter(new ListDialogAdapter(context, items));
        lvList.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> view, View arg1, int pos, long arg3) {
                DialogInterface dlgInterface = (DialogInterface) view.getTag();
                dlgInterface.dismiss();
                
                if (selectionListener != null)
                {
                    // Subtracting 1 because of the header view
                    selectionListener.onClick(dlgInterface, pos - 1);
                }
            }
        });
        
       

        
        dlg.findViewById(R.id.imgSpinner).setVisibility(View.GONE);
        dlg.findViewById(R.id.etInput).setVisibility(View.GONE);
        dlg.findViewById(R.id.btnPositive).setVisibility(View.GONE);

        Button btnNegative = (Button) dlg.findViewById(R.id.btnNegative);

        if (negative != 0)
        {
            btnNegative.setText(negative);
            btnNegative.setTag(dlg);
            btnNegative.setOnClickListener(new View.OnClickListener() {

                @Override
                public void onClick(View btn) {

                    DialogInterface dlgInterface = (DialogInterface) btn.getTag();
                    dlgInterface.dismiss();
                }
            });
        }
        else
        {
            btnNegative.setVisibility(View.GONE);
        }
        
        // Remove the negative button margin
        LinearLayout.LayoutParams lp = (LinearLayout.LayoutParams) btnNegative.getLayoutParams();
        lp.leftMargin = 0;
        btnNegative.setLayoutParams(lp);
        
        dlg.findViewById(R.id.fillerView1).setVisibility(View.GONE);

        return dlg;
    }

    private static class ListDialogAdapter extends ArrayAdapter<CharSequence>
    {

        public ListDialogAdapter(Context context, CharSequence[] objects) {
            super(context, R.layout.dialog_list_row, R.id.txtValue, objects);
        }

    }
}
