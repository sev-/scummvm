/*
 * AccelerateInterpolator.cpp
 *
 *  Created on: Jul 9, 2013
 *      Author: omergilad
 */

#include "DeccelerateInterpolator.h"
#include <cmath>


DeccelerateInterpolator::DeccelerateInterpolator() {
	// TODO Auto-generated constructor stub

}

DeccelerateInterpolator::~DeccelerateInterpolator() {
	// TODO Auto-generated destructor stub
}



float DeccelerateInterpolator::interpolate(float linearValue)
{
	return sqrt(linearValue);
}


