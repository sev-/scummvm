/*
 * AccelerateInterpolator.h
 *
 *  Created on: Jul 9, 2013
 *      Author: omergilad
 */

#ifndef ACCELERATEINTERPOLATOR_H_
#define ACCELERATEINTERPOLATOR_H_

#include "Interpolator.h"


class AccelerateInterpolator: public Interpolator {
public:
	AccelerateInterpolator();
	virtual ~AccelerateInterpolator();

	virtual float interpolate(float linearValue);

};

#endif /* ACCELERATEINTERPOLATOR_H_ */
