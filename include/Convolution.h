//
//  Convolution.h
//  kimproc
//
//  Created by Jan Brejcha on 26.10.14.
//  Copyright (c) 2014 brejchajan. All rights reserved.
//

#ifndef __kimproc__Convolution__
#define __kimproc__Convolution__

#include <stdio.h>

#include "Image.h"

#define DIR_VERT 0
#define DIR_HORIZ 1

class Convolution
{
public:
	static void convolve1D(Image &image, float * result, unsigned int kernel_size, float * kernel, int direction);
	
	static void convolve1D(Imagef &image, float * result, unsigned int kernel_size, float * kernel, int direction);
};

#endif /* defined(__kimproc__Convolution__) */
