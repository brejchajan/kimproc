//
//  Image.h
//  kimproc
//
//  Created by Jan Brejcha on 26.10.14.
//  Copyright (c) 2014 brejchajan. All rights reserved.
//

#ifndef __kimproc__Image__
#define __kimproc__Image__

#include <stdio.h>

struct Image
{
	unsigned char * data;
	unsigned int width;
	unsigned int height;
};

struct Imagef
{
	float * data;
	unsigned int width;
	unsigned int height;
};

#endif /* defined(__kimproc__Image__) */
