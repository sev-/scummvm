package org.iphsoft.simon1.util;

import org.iphsoft.simon1.AndroidPortAdditions;

import com.mojotouch.simon.R;
import com.mojotouch.simon.R.string;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;

/**
 * Utils for dealing with system intents - launching system apps and so on
 * 
 * @author omergilad
 */
public class IntentUtils {
	public static void showMailApp(Context context, String subject,
			String body, String email, boolean fromService) {
		Intent sendIntent = new Intent(Intent.ACTION_SEND);
		sendIntent.setType("plain/text");
		sendIntent.putExtra(Intent.EXTRA_SUBJECT, subject);
		sendIntent.putExtra(Intent.EXTRA_TEXT, body);
		sendIntent.putExtra(Intent.EXTRA_EMAIL, new String[] { email });
		sendIntent = Intent.createChooser(sendIntent, "Select mail app");
		context.startActivity(sendIntent);
	}

	/**
	 * Show a URL in the native browser
	 * 
	 * @param url
	 * @param context
	 */
	public static void showBrowserApp(Context context, String url) {
		Intent i = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
		context.startActivity(i);
	}

	public static void showSmsApp(Context context, String body, String to,
			boolean fromService) {
		Intent sendIntent = new Intent(Intent.ACTION_VIEW);

		String uriString;
		if (to == null) {
			uriString = "sms:";
		} else {
			uriString = "smsto:" + to;
		}

		sendIntent.setData(Uri.parse(uriString));

		if (to != null) {
			sendIntent.putExtra("address", to);
		}
		sendIntent.putExtra("sms_body", body);

		context.startActivity(sendIntent);
	}

	public static void startAppStoreActivity(Context context, String packageName) {
		switch (AndroidPortAdditions.APP_STORE) {
		case GOOGLE_PLAY:
			context.startActivity(new Intent(Intent.ACTION_VIEW, Uri
					.parse(String.format(
							context.getString(R.string.google_play_market_url),
							packageName))));
			break;
		case AMAZON:
			context.startActivity(new Intent(Intent.ACTION_VIEW, Uri
					.parse(String.format(context
							.getString(R.string.amazon_market_url),
							packageName))));
			break;
		case SAMSUNG:
			Intent intent = new Intent();
			intent.setData(Uri.parse(String.format(
					context.getString(R.string.samsung_market_url),
					packageName)));
			context.startActivity(intent);
			break;
		default:
			MyLog.e("IntentUtils: startAppStoreActivity: unknown appstore");
		}
	}

}
