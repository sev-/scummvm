
package org.iphsoft.simon1.util;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.List;

/**
 * Utility for comma separated values
 * 
 * @author omergilad
 */
public class CsvUtils
{
    public static String intListToCsv(List<Integer> values)
    {
        String string = new String();
        for (int i : values)
        {
            string += i;
            string += ',';
        }

        if (string.endsWith(","))
            string = string.substring(0, string.length() - 1);

        return string;
    }

    public static List<Integer> csvToIntList(String csv)
    {
        String[] values = csv.split(",");
        List<Integer> result = new ArrayList<Integer>(values.length);

        if (csv.equals(""))
        {
            return result;
        }

        for (int i = 0; i < values.length; ++i)
        {
            result.add(Integer.parseInt(values[i]));
        }

        return result;
    }

    public static String longListToCsv(List<Long> values)
    {
        String string = new String();
        for (long i : values)
        {
            string += i;
            string += ',';
        }

        if (string.endsWith(","))
            string = string.substring(0, string.length() - 1);

        return string;
    }

    public static List<Long> csvToLongList(String csv)
    {
        String[] values = csv.split(",");
        List<Long> result = new ArrayList<Long>(values.length);

        if (csv.equals(""))
        {
            return result;
        }

        for (int i = 0; i < values.length; ++i)
        {
            result.add(Long.parseLong(values[i]));
        }

        return result;
    }

    public static String stringListToCsv(List<String> values)
    {
        String string = new String();
        for (String s : values)
        {
            try {
                string += URLEncoder.encode(s, "UTF-8");
            } catch (UnsupportedEncodingException e) {
                MyLog.e("CsvUtils: stringListToCsv: " + e);
                e.printStackTrace();

                string += s;
            }
            string += ',';
        }

        if (string.endsWith(","))
            string = string.substring(0, string.length() - 1);

        return string;
    }

    public static List<String> csvToStringList(String csv)
    {
        String[] values = csv.split(",");
        List<String> result = new ArrayList<String>(values.length);

        if (csv.equals(""))
        {
            return result;
        }

        for (int i = 0; i < values.length; ++i)
        {
            try {
                result.add(URLDecoder.decode(values[i], "UTF-8"));
            } catch (UnsupportedEncodingException e) {
                MyLog.e("CsvUtils: csvToStringList: " + e);
                e.printStackTrace();
                
                result.add(values[i]);
            }
        }

        return result;
    }
}
