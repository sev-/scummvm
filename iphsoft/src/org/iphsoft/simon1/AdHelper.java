package org.iphsoft.simon1;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import org.iphsoft.simon1.network.HttpRequest;
import org.iphsoft.simon1.network.HttpRequest.HttpRequestType;
import org.iphsoft.simon1.network.HttpRequestExecutor;
import org.iphsoft.simon1.network.HttpResponse;
import org.iphsoft.simon1.util.BackgroundHandler;
import org.iphsoft.simon1.util.MyLog;
import org.json.JSONArray;
import org.json.JSONException;

import com.mojotouch.simon.R;

/**
 * Helper class for game advertisements: Checks the web for ad policy, caches
 * the result and chooses a random ad.
 * 
 * @author omergilad
 * 
 */
public class AdHelper {

	public static class AdInfo {
		public String appPackage;
		public int adIconResourceId;
	}

	/**
	 * Start the web check in the background - will update the local policy once
	 * successful
	 */
	public static void startBackgroundAdCheck() {

		MyLog.d("AdHelper: startBackgroundAdCheck: ");

		// Get check URL
		String url = getCheckUrl();

		// Construct an HTTP request
		HttpRequest httpReq = new HttpRequest(url, HttpRequestType.GET);

		// Start executing in the background (will update the app shared prefs
		// once finished)
		new HttpRequestExecutor(httpReq, BackgroundHandler.instance(),
				sHttpListener).execute();
	}

	/**
	 * 
	 * @return Get a randomized ad to display, according to latest policy
	 */
	public static AdInfo getAdToDisplay() {

		MyLog.d("AdHelper: getAdToDisplay: ");

		String game = AndroidPortAdditions.instance().getAdGame();

		// Return the ad information
		AdInfo adInfo = new AdInfo();
		if (game.equals("simon1")) {
			adInfo.appPackage = SIMON1_PACKAGE;
			adInfo.adIconResourceId = R.drawable.ad_simon1;
		} else if (game.equals("simon2")) {
			adInfo.appPackage = SIMON2_PACKAGE;
			adInfo.adIconResourceId = R.drawable.ad_simon2;
		} else if (game.equals("fotaq")) {
			adInfo.appPackage = FOTAQ_PACKAGE;
			adInfo.adIconResourceId = R.drawable.ad_fotaq;
		} else if (game.equals("ite")) {
			adInfo.appPackage = INHERIT_THE_EARTH_PACKAGE;
			adInfo.adIconResourceId = R.drawable.ad_ite;
		} else if (game.equals("t7g")) {
			adInfo.appPackage = THE_7TH_GUEST_PACKAGE;
			adInfo.adIconResourceId = R.drawable.ad_t7g;
		} else {
			return null;
		}

		return adInfo;
	}

	private static String getCheckUrl() {
		String fileName;
		switch (AndroidPortAdditions.GAME_TYPE) {
		case SIMON1:
			fileName = SIMON1_FILE;
			break;
		case SIMON2:
			fileName = SIMON2_FILE;
			break;
		default:
			return null;
		}

		return CHECK_URL_BASE + fileName;
	}

	private static void handleHttpResponse(HttpResponse response) {
		if (response.getResponseCode() != HttpResponse.RESPONSE_CODE_HTTP_OK) {
			MyLog.w("AdHelper: handleHttpResponse: bad response code "
					+ response.getResponseCode());
			return;
		}

		// Parse the JSON array in the response
		String jsonString = new String(response.getResponseData());

		try {
			JSONArray jsonArray = new JSONArray(jsonString);

			MyLog.d("AdHelper: handleHttpResponse: json array: "
					+ jsonArray.toString());

			String game;
			if (jsonArray.length() == 0) {
				game = "";
			} else {
				// Choose a random game
				game = jsonArray.getString(Math.abs(new Random()
						.nextInt()) % jsonArray.length());
			}

			// Store the choice in shared prefs
			AndroidPortAdditions.instance().setAdGame(game);

			MyLog.d("AdHelper: handleHttpResponse: new ad game: " + game);

		} catch (JSONException e) {
			MyLog.e("AdHelper: handleHttpResponse: error parsing JSON: "
					+ e.getMessage());
			e.printStackTrace();
		}
	}

	private static HttpRequestExecutor.HttpResponseListener sHttpListener = new HttpRequestExecutor.HttpResponseListener() {

		@Override
		public void onResponse(HttpResponse response) {

			handleHttpResponse(response);
		}
	};

	private static final String CHECK_URL_BASE = "http://www.mojo-touch.com/ads-android/";

	private static final String SIMON1_FILE = "simon1.json";
	private static final String SIMON2_FILE = "simon2.json";

	private static final String SIMON1_PACKAGE = "com.mojotouch.simon";
	private static final String SIMON2_PACKAGE = "com.mojotouch.simon2";
	private static final String FOTAQ_PACKAGE = "com.mojotouch.fotaq";
	private static final String INHERIT_THE_EARTH_PACKAGE = "com.mojotouch.ite";
	private static final String THE_7TH_GUEST_PACKAGE = "com.mojotouch.t7g";

}
