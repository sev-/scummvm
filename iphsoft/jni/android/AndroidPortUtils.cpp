/*
 * AndroidPortUtils.cpp
 *
 *  Created on: Jan 18, 2013
 *      Author: omergilad
 */

#include "AndroidPortUtils.h"



void AndroidPortUtils::dumpBytesToFile(const byte* bytes, uint32 length, const char* filename)
{
	FILE* file = fopen(filename, "w");

	if (file != NULL)
	{

		// Write all bytes
		uint32 index = 0;
		while (index < length)
		{
			index += fwrite(bytes, sizeof(byte), length - index, file);
		}

		fclose(file);
	}
}


AndroidPortUtils::AndroidPortUtils() {
	// TODO Auto-generated constructor stub

}

AndroidPortUtils::~AndroidPortUtils() {
	// TODO Auto-generated destructor stub
}

