package org.iphsoft.simon1.network;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;

import org.iphsoft.simon1.util.MyLog;

/**
 * Wrapper for simple HTTP requests.
 * 
 * @author omergilad
 *
 */
public class HttpRequest {

	public enum HttpRequestType {
		GET, POST
	}

	/**
	 * 
	 * @param url The request URL
	 * @param type The type of the request
	 */
	public HttpRequest(String url, HttpRequestType type) {
		MyLog.d("HttpRequest: HttpRequest: " + url + " " + type);
		
		mUrl = url;
		mType = type;
	}
	
	/**
	 * Perform the request
	 * 
	 * @return The relevant HTTP response
	 * @throws MalformedURLException
	 */
	public HttpResponse execute() throws MalformedURLException {
		
		HttpResponse response = new HttpResponse(null,
				HttpResponse.RESPONSE_CODE_NO_CONNECTION, this);
		
		try {			
			HttpURLConnection connection = (HttpURLConnection) new URL(mUrl).openConnection();
			connection.setConnectTimeout(TIMEOUT);
			connection.setReadTimeout(TIMEOUT);
			
			response.setResponseCode(connection.getResponseCode());
			
			MyLog.d("HttpRequest: execute: responseCode " + response.getResponseCode());
			
			if (response.getResponseCode() == HttpResponse.RESPONSE_CODE_HTTP_OK)
			{
				InputStream is = new BufferedInputStream(connection.getInputStream());
				ByteArrayOutputStream baos = new ByteArrayOutputStream();
				
				int oneByte;
				while ((oneByte = is.read()) != -1)
				{
					baos.write(oneByte);
				}
				
				response.setResponseData(baos.toByteArray());
			}
			
			
		} catch (IOException e) {
			MyLog.d("HttpRequest: execute: " + e);
			e.printStackTrace();
		}

		return response;
	}

	public String getUrl() {
		return mUrl;
	}

	public void setUrl(String url) {
		this.mUrl = url;
	}

	public HttpRequestType getType() {
		return mType;
	}

	public void setType(HttpRequestType type) {
		this.mType = type;
	}

	private String mUrl;
	private HttpRequestType mType;
	
	private static final int TIMEOUT = 10000;
}