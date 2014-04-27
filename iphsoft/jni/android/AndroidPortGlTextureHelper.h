/*
 * AndroidPortGlTextureHelper.h
 *
 *  Created on: Feb 3, 2013
 *      Author: omergilad
 */

#ifndef ANDROIDPORTGLTEXTUREHELPER_H_
#define ANDROIDPORTGLTEXTUREHELPER_H_

#include "AndroidBitmap.h"

#define GL_TEXTURE_WIDTH 1024
#define GL_TEXTURE_HEIGHT 1024
#define GL_TEXTURE_GRID_SIZE 64

class AndroidPortGlTextureHelper {
public:
	AndroidPortGlTextureHelper();
	virtual ~AndroidPortGlTextureHelper();

	void allocateBitmapInGlTexture(AndroidBitmap* bitmap);

private:

	bool attemptToAllocateSpaceInGrid(uint16 x, uint16 y, uint16 w, uint16 h);

	bool mAllocationGrid[GL_TEXTURE_WIDTH / GL_TEXTURE_GRID_SIZE]
	                     [GL_TEXTURE_HEIGHT * GL_TEXTURE_GRID_SIZE];
	uint16 mGridW;
	uint16 mGridH;

};

#endif /* ANDROIDPORTGLTEXTUREHELPER_H_ */
