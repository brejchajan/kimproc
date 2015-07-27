//
//  GaussianSampler.h
//  kimproc
//
//  Created by Jan Brejcha on 26.10.14.
//  Copyright (c) 2014 brejchajan. All rights reserved.
//

#ifndef __kimproc__GaussianSampler__
#define __kimproc__GaussianSampler__

#include <cmath>
#include <stdexcept>
#include <list>

class GaussianSampler{
public:
	static void gaussian1D(double mi, double sigma, unsigned int size, float *kernel);

};

#endif /* defined(__kimproc__GaussianSampler__) */
