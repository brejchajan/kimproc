//
//  GaussianSampler.cpp
//  kimproc
//
//  Created by Jan Brejcha on 26.10.14.
//  Copyright (c) 2014 brejchajan. All rights reserved.
//

#include "GaussianSampler.h"

void GaussianSampler::gaussian1D(double mi, double sigma, unsigned int size, float *kernel)
{
	if (sigma < 0)
		throw std::runtime_error("Standard deviation sigma has to be greater \
								 than 0.");
	
	const double e = std::exp(1.0);
	const double PI2sq = sqrt(2 * M_PI);
	const double a = 1/(sigma * PI2sq);
	const double sigma2sq = 2 * (sigma * sigma);
	double x_mi = 0.0;
	double gauss = 0.0;
	const double step = 0.001;
	
	double sample_size = 6.0 / (double)size;
	double x = -3.0;
	double c = 0.0;
	int index = 0;
	kernel[0] = 0.0;
	while (x <= 3.0)
	{
		if (c > sample_size)
		{
			c = 0.0;
			kernel[++index] = 0.0;
		}
		x_mi = x - mi;
		gauss = a * pow(e, (-((x_mi*x_mi)/(sigma2sq))));
		kernel[index] += gauss;
		x += step;
		c += step;
	}
	
	//normalize to have corners == 1
	double coef = 1.0/kernel[0];
	for (int i = 0; i < size; ++i)
	{
		kernel[i] = (int)round(kernel[i] * coef);
	}
	
	//calculate eucleidian norm
	double norm = 0.0;
	for (int i = 0; i < size; ++i)
	{
		norm += kernel[i];
	}
	//norm = sqrt(norm);
	for (int i = 0; i < size; ++i)
	{
		kernel[i] /= norm;
	}
}



