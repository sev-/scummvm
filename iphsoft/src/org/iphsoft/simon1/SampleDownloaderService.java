/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.iphsoft.simon1;

import com.google.android.vending.expansion.downloader.impl.DownloaderService;

/**
 * This class demonstrates the minimal client implementation of the
 * DownloaderService from the Downloader library.
 */
public class SampleDownloaderService extends DownloaderService {
    // stuff for LVL
	
	private static String BASE64_PUBLIC_KEY_BETA = "nothing";
	private static String BASE64_PUBLIC_KEY_PRODUCTION = "nothing";
	
    static {
    	switch (AndroidPortAdditions.GAME_TYPE)
    	{
    	case SIMON1:
    		BASE64_PUBLIC_KEY_BETA = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAiUo7Z1nP209bvgWTn1oS+jkJuZNG4C4MElhNcr1kZVySY6626pdmiA5zH94MHd5VngsRUAPTrow/YRhql0pv3VVkNvNto34E5FhE12pMYpTwFlw0wIPptACKij7xIZwBfM6u3Cv0sUVuzMZ34QokmPYLwj4e+AbKbXx6TxGof1c3gfddPtrIlH+m6YvIurT/zD9ljOHdClBNnNbzMnWM+O2kn3SIhGK9WO0dpmjiUNmg5PyNpiurjrPoyHQ8aBdgJRWZJxuUyK0Z3k3CT9U+aae+0q+EwvAbKM+qdGPXCEFbACCTXbjng5Aa6mmxCd0zMgGl8Mbeqgw0hgiBu8JgNQIDAQAB";
    		BASE64_PUBLIC_KEY_PRODUCTION = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvMcG9rXPBFdPdZNao+KBsbRUS33fYER4wfBXkIl72BjZd84lZdxWazm8YgzHAEBeez26bfSxjC6/eR6oUxB2PYrFnXVOnKF25oPUnc1p/BwY7ohc+u+srLIOFGejANkMvKPuXHKp3QbOyqdC3IhXPJ79kJQXAyGaCuQHrZbxJoGBw1eupFPB91R2fmxC0ChifO6NpsvuaR/LO9cuLWesb+gKYv5B0Dl7tOGLXGAcvUUuJ6fZLntSsoO+UZrL2R87qfMrvsl5BdLKJ6bj8S+BGyLsTx3jJ6CNxupFz9ueDS/HEeNc8noooip18QYbQEJSE1fJG7LiwVOS7CwU6HN04QIDAQAB";
    		break;
    	case SIMON2:
    	case FOTAQ:
    	case INDY_FOA:
    		
    	}
    }
    
    private static final String BASE64_PUBLIC_KEY = BASE64_PUBLIC_KEY_PRODUCTION;
 
    
    
    
    
    // used by the preference obfuscater
    private static final byte[] SALT = new byte[] {
            65, 4, -52, -64, 1, 100,
            -30, -3, 44, 31, 32, -15, -82, 9, -16, 2, -10, 102, 84, 55
    };

    /**
     * This public key comes from your Android Market publisher account, and it
     * used by the LVL to validate responses from Market on your behalf.
     */
    @Override
    public String getPublicKey() {
        return BASE64_PUBLIC_KEY;
    }

    /**
     * This is used by the preference obfuscater to make sure that your
     * obfuscated preferences are different than the ones used by other
     * applications.
     */
    @Override
    public byte[] getSALT() {
        return SALT;
    }

    /**
     * Fill this in with the class name for your alarm receiver. We do this
     * because receivers must be unique across all of Android (it's a good idea
     * to make sure that your receiver is in your unique package)
     */
    @Override
    public String getAlarmReceiverClassName() {
        return SampleAlarmReceiver.class.getName();
    }

}
