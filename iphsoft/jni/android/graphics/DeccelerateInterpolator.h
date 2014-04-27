/*
 * AccelerateInterpolator.h
 *
 *  Created on: Jul 9, 2013
 *      Author: omergilad
 */

#ifndef DECCELERATEINTERPOLATOR_H_
#define DECCELERATEINTERPOLATOR_H_

#include "Interpolator.h"


class DeccelerateInterpolator: public Interpolator {
public:
	DeccelerateInterpolator();
	virtual ~DeccelerateInterpolator();

	virtual float interpolate(float linearValue);

};

#endif /* DECCELERATEINTERPOLATOR_H_ */
