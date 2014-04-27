/*
 * AccelerateInterpolator.cpp
 *
 *  Created on: Jul 9, 2013
 *      Author: omergilad
 */

#include "AccelerateInterpolator.h"
#include <cmath>


AccelerateInterpolator::AccelerateInterpolator() {
	// TODO Auto-generated constructor stub

}

AccelerateInterpolator::~AccelerateInterpolator() {
	// TODO Auto-generated destructor stub
}



float AccelerateInterpolator::interpolate(float linearValue)
{
	return pow(linearValue, 2);
}


