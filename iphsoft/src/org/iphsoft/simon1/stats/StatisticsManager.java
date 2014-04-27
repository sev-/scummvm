package org.iphsoft.simon1.stats;

/**
 * Facade for statistics operations
 */
public class StatisticsManager
{
	public static synchronized IStatistics getStatistics()
	{
		if (sInstance == null)
		{
			// Choosing the use the Google Analytics implementation
			sInstance = new GoogleAnalyticsStatistics();
		}
		
		return sInstance;
	}

	private static IStatistics sInstance = null;
}
