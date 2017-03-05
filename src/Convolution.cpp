//
//  Convolution.cpp
//  kimproc
//
//  Created by Jan Brejcha on 26.10.14.
//  Copyright (c) 2014 brejchajan. All rights reserved.
//

#include "Convolution.h"

void Convolution::convolve1D(Image &image, float * result, unsigned int kernel_size, float * kernel, int direction)
{
	int kernel_w, kernel_h;
	kernel_w = kernel_size;
	kernel_h = 1;
	if (direction == DIR_VERT)
	{
		kernel_h = kernel_size;
		kernel_w = 1;
	}

	unsigned int index = 0;
	unsigned int index1 = 0;
	unsigned char val = 0;
	float res = 0;
	for (int y = 0; y < image.height; ++y)
	{
		for (int x = 0; x < image.width; ++x)
		{
			res = 0;
			index1 = y * image.width + x;
			for (int t = 0; t < kernel_h; ++t)
			{
				for (int s = 0; s < kernel_w; ++s)
				{
					int y_t = y - t;
					int x_s = x - s;
					index = (y_t >= 0 ? y_t : 0) * image.width + (x_s >= 0 ? x_s : 0);
					val = image.data[index];
					res += val * kernel[t * kernel_w + s];
				}
			}
			result[index1] = res;
		}
	}
}

void Convolution::convolve1D(Imagef &image, float * result, unsigned int kernel_size, float * kernel, int direction)
{
	int kernel_w, kernel_h;
	kernel_w = kernel_size;
	kernel_h = 1;
	if (direction == DIR_VERT)
	{
		kernel_h = kernel_size;
		kernel_w = 1;
	}
	
	unsigned int index = 0;
	//unsigned int index1 = 0;
	float val = 0;
	float res = 0;
	for (int y = 0; y < image.height; ++y)
	{
		for (int x = 0; x < image.width; ++x)
		{
			res = 0;
			//index1 = y * image.width + x;
			for (int t = 0; t < kernel_h; ++t)
			{
				for (int s = 0; s < kernel_w; ++s)
				{
					int y_t = y - t;
					int x_s = x - s;
					index = (y_t >= 0 ? y_t : 0) * image.width + (x_s >= 0 ? x_s : 0);
					val = image.data[index];
					res += val * kernel[t * kernel_w + s];
				}
			}
			result[index] = res;
		}
	}
}


void Convolution::convolve1D(CImg<float> &image, CImg<float> &result, unsigned int kernel_size, float * kernel, int direction)
{	
	assert(image.width() == result.width() &&
		   image.height() == result.height() &&
		   image.spectrum() == result.spectrum());
	
	
	
	int kernel_w, kernel_h;
	kernel_w = kernel_size;
	kernel_h = 1;
	if (direction == DIR_VERT)
	{
		kernel_h = kernel_size;
		kernel_w = 1;
	}
	
	int width = image.width();
	int height = image.height();
	
	float *res = new float[image.spectrum()];
	
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			// null the accumulator
			for (int c = 0; c < image.spectrum(); ++c)
			{
				res[c] = 0;
			}
			for (int t = 0; t < kernel_h; ++t)
			{
				for (int s = 0; s < kernel_w; ++s)
				{
					int y_t = y - t;
					int x_s = x - s;
					y_t = y_t >= 0 ? y_t : 0;
					x_s = x_s >= 0 ? x_s : 0;
					for (int c = 0; c < image.spectrum(); ++c)
					{
						res[c] += image(x_s, y_t, 0, c) * kernel[t * kernel_w + s];
					}
				}
			}
			for (int c = 0; c < image.spectrum(); ++c)
			{
				result(x, y, 0, c) = res[c];
				
			}
			
			
		}
	}
	delete [] res;
}
