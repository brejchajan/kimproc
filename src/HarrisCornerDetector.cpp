//
//  HarrisCornerDetector.cpp
//  kimproc
//
//  Created by Jan Brejcha on 29.10.14.
//  Copyright (c) 2014 brejchajan. All rights reserved.
//

#include "HarrisCornerDetector.h"

using namespace Eigen;
void HarrisCornerDetector::detect(cimg_library::CImg<unsigned char> &src,
								  int treshold)
{
	if (treshold == -1)
		treshold = 10000;
	
	int width = src.width();
	int height = src.height();
	cimg_library::CImg<unsigned char> gray1(width,height,src.depth(),1);
	//to greyscale
	unsigned char r, g, b, gr1 = 0;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			r = src(x,y,0,0); // First channel RED
			g = src(x,y,0,1); // Second channel GREEN
			b = src(x,y,0,2); // Third channel BLUE
			
			gr1 = round(0.299*((double)r) + 0.587*((double)g) + 0.114*((double)b));
			gray1(x,y,0,0) = gr1;
		}
	}
	
	Image image;
	image.data = gray1.data(0, 0);
	image.width = width;
	image.height = height;
	
	
	float kernel[2] = {-1, 1};
	float *der_x = (float *)malloc(sizeof(float) * image.width * image.height);
	float *der_y = (float *)malloc(sizeof(float) * image.width * image.height);
	Convolution::convolve1D(image, der_x, 2, kernel, DIR_HORIZ);
	Convolution::convolve1D(image, der_y, 2, kernel, DIR_VERT);
	
	float *Ixx = (float *)malloc(sizeof(float) * image.width * image.height);
	float *Ixy = (float *)malloc(sizeof(float) * image.width * image.height);
	float *Iyy = (float *)malloc(sizeof(float) * image.width * image.height);
	
	
	int i = 0;
	float derx, dery;
	for (int i = 0; i < image.height * image.width; ++i)
	{
		derx = der_x[i];
		dery = der_y[i];
		Ixx[i] = derx * derx;
		Ixy[i] = derx * dery;
		Iyy[i] = dery * dery;
	}
	
	Imagef imIxx;
	imIxx.data = Ixx;
	imIxx.width = image.width;
	imIxx.height = image.height;
	
	Imagef imIxy;
	imIxy.data = Ixy;
	imIxy.width = image.width;
	imIxy.height = image.height;
	
	Imagef imIyy;
	imIyy.data = Iyy;
	imIyy.width = image.width;
	imIyy.height = image.height;
	
	
	float ker[5];
	GaussianSampler::gaussian1D(0.0, 1.0, 5, ker);
	
	//convolve with larger gaussian
	//xx
	Convolution::convolve1D(imIxx, Ixx, 5, ker, DIR_HORIZ);
	Convolution::convolve1D(imIxx, Ixx, 5, ker, DIR_VERT);
	
	//xy
	Convolution::convolve1D(imIxy, Ixy, 5, ker, DIR_HORIZ);
	Convolution::convolve1D(imIxy, Ixy, 5, ker, DIR_VERT);
	
	//yy
	Convolution::convolve1D(imIyy, Iyy, 5, ker, DIR_HORIZ);
	Convolution::convolve1D(imIyy, Iyy, 5, ker, DIR_VERT);
	
	
	MatrixXf A(2, 2);
	float R, tr;
	float k = 0.00001;
	for (int y = 0; y < image.height; ++y)
	{
		for (int x = 0; x < image.width; ++x)
		{
			i = y * image.width + x;
			A(0, 0) = Ixx[i];
			A(0, 1) = Ixy[i];
			A(1, 0) = Ixy[i];
			A(1, 1) = Iyy[i];
			
			tr = A.trace();
			R = A.determinant() - k * tr*tr;
			
			if (R > treshold)
			{
				src(x, y, 0, 0) = 255;
			}
		}
	}
	
	//free memory
	free(der_x);
	free(der_y);
	free(Ixx);
	free(Ixy);
	free(Iyy);
}
