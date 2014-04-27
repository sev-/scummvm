package org.iphsoft.simon1.network;

/**
 * Wrapper for a simple HTTP response
 * 
 * @author omergilad
 *
 */
public class HttpResponse {

	public static final int RESPONSE_CODE_NO_CONNECTION = -1;
	public static final int RESPONSE_CODE_HTTP_OK = 200;
	
	public HttpResponse(byte[] responseData, int responseCode, HttpRequest request)
	{
		mResponseData = responseData;
		mResponseCode = responseCode;
		mRequest = request;
	}
	
	public byte[] getResponseData() {
		return mResponseData;
	}
	
	public void setResponseData(byte[] responseData) {
		this.mResponseData = responseData;
	}
	
	public int getResponseCode() {
		return mResponseCode;
	}
	
	public void setResponseCode(int responseCode) {
		this.mResponseCode = responseCode;
	}
	
	public HttpRequest getHttpRequest()
	{
		return mRequest;
	}
	
	public void setHttpRequest(HttpRequest request)
	{
		mRequest = request;
	}
	
	@Override
	public String toString() {

		return String.valueOf(mResponseCode);
	}
	
	private byte[] mResponseData;
	private int mResponseCode;
	private HttpRequest mRequest;
}