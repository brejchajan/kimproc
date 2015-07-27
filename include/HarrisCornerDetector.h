//
//  HarrisCornerDetector.h
//  kimproc
//
//  Created by Jan Brejcha on 29.10.14.
//  Copyright (c) 2014 brejchajan. All rights reserved.
//

#ifndef __kimproc__HarrisCornerDetector__
#define __kimproc__HarrisCornerDetector__

#include <stdio.h>
#include <cstdio>
#include <cmath>

#include <Eigen/Dense>

#include "CImg.h"

#include "Image.h"
#include "GaussianSampler.h"
#include "Convolution.h"

class HarrisCornerDetector
{
public:
	static void detect(cimg_library::CImg<unsigned char> &image, int treshold);
};

#endif /* defined(__kimproc__HarrisCornerDetector__) */
