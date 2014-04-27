/*
 * Interpolator.h
 *
 *  Created on: Jul 9, 2013
 *      Author: omergilad
 */

#ifndef INTERPOLATOR_H_
#define INTERPOLATOR_H_

#include <memory>

class Interpolator {
public:
	Interpolator();
	virtual ~Interpolator();

	virtual float interpolate(float linearValue) = 0;
};

typedef std::shared_ptr<Interpolator> InterpolatorPtr;

#endif /* INTERPOLATOR_H_ */
