
package org.iphsoft.simon1.util;

import android.os.Build;

/**
 * Utility for getting device models
 * 
 * @author omergilad
 */
public class DeviceIdentifier {

    public enum DeviceModel
    {
        Galaxy_S1("GT-I9000"), Galaxy_S2("GT-I9100"), Galaxy_S3("GT-I9300"), Unknown("");

        public static DeviceModel getFromModelString(String modelString)
        {
            for (DeviceModel model : DeviceModel.values())
            {
                if (model.mModelString.equalsIgnoreCase(modelString))
                {
                    return model;
                }
            }

            return Unknown;
        }

        private DeviceModel(String modelString)
        {
            mModelString = modelString;
        }

        private String mModelString;
    }

    public static DeviceModel getDeviceModel()
    {
        String model = Build.MODEL;
        return DeviceModel.getFromModelString(model);
    }
}
