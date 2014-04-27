package org.iphsoft.simon1.network;

import java.net.MalformedURLException;

import org.iphsoft.simon1.util.MyLog;

import android.os.Handler;

/**
 * Utility for executing Http requests on a handler. Supports retries.
 * 
 * @author omergilad
 */
public class HttpRequestExecutor {
	public interface HttpResponseListener {
		public void onResponse(HttpResponse response);
	}

	public HttpRequestExecutor(HttpRequest request, Handler handler,
			HttpResponseListener listener) {
		mRequest = request;
		mHandler = handler;
		mListener = listener;
	}

	/**
	 * Execute the request on the handler, up to the max retries
	 */
	public void execute() {
		MyLog.d("HttpRequestExecutor: execute: ");

		mHandler.post(mRunnable);
	}

	private void executeAttempt() {
		MyLog.d("HttpRequestExecutor: executeAttempt: retry number "
				+ mRetryNum);

		HttpResponse response = null;
		try {
			// Attempt to execute the request
			response = mRequest.execute();
		} catch (MalformedURLException e) {
			MyLog.e("HttpRequestThread: run: " + e);
			e.printStackTrace();

			if (mListener != null) {
				mListener.onResponse(null);
			}
		}

		// Check for end condition
		if (response == null
				|| response.getResponseCode() == HttpResponse.RESPONSE_CODE_HTTP_OK
				|| mRetryNum == MAX_RETRIES) {
			MyLog.d("HttpRequestExecutor: executeAttempt: notify listener");

			if (mListener != null) {
				mListener.onResponse(response);
			}
		} else {
			MyLog.d("HttpRequestExecutor: executeAttempt: failed, more retries left");

			// Failure - more retries left
			++mRetryNum;
			mHandler.postDelayed(mRunnable, DELAY);
		}
	}

	protected HttpRequest mRequest;
	protected Handler mHandler;
	protected HttpResponseListener mListener;

	private Runnable mRunnable = new Runnable() {

		public void run() {
			executeAttempt();
		}
	};

	private int mRetryNum = 1;

	private static final int MAX_RETRIES = 2;
	private static final int DELAY = 2000;
}