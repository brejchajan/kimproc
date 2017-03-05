//
//  SingleImageHazeRemoval.h
//  kimproc
//
//  Created by Jan Brejcha on 15.07.15.
//  Copyright (c) 2015 brejchajan. All rights reserved.
//

#ifndef __kimproc__SingleImageHazeRemoval__
#define __kimproc__SingleImageHazeRemoval__

#include <stdio.h>
#include <algorithm>
#include <climits>
#include <string>
#include <cmath>
#include <vector>
#include <iostream>
//#include <ctime>

#include <Eigen/Dense>
#include <Eigen/SparseCore>
#include <Eigen/IterativeLinearSolvers>
#include <Eigen/SparseLU>

#include "CImg.h"

#define PATCH_SIZE 15
#define OMEGA 0.95
#define T0 0.1
#define BETA 

#define TIME_DEBUG

using namespace cimg_library;
/**
 @brief SingleImageHazeRemoval implements haze removal method from 
 Single Image Haze Removal Using Dark Channel Prior, by He, Sun, Tang, CVPR 09.
 */
class SingleImageHazeRemoval
{
	
public:
	/**
	 @param image			the CImg image to be dehazed
	 @param _output_name	name of the output file
	 */
	SingleImageHazeRemoval(CImg<unsigned char> &image, std::string _output_name);
	
	/**
	 Entry method that runs dehazing process
	 */
	void dehaze();
	
private:
	
	CImg<unsigned char> &input_image;
	std::string output_name;
	
	CImg<unsigned char> dark_channel;
	CImg<unsigned char> transEst;
	
	/**
	 Calculation of dark channel prior on input_image
	 */
	template <typename T>
	void darkChannel(CImg<T> &_input_image, CImg<T> &_dark_channel)
	{
		int w = _input_image.width();
		int h = _input_image.height();
		T p_min;
		T c_min;
		
		//clock_t begin = clock();
		
		for (int y = 0; y < h; ++y)
		{
			for (int x = 0; x < w; ++x)
			{
				p_min = 255;
				for (int py = y; py < std::min(y + PATCH_SIZE, h); ++py)
				{
					for (int px = x; px < std::min(x + PATCH_SIZE, w); ++px)
					{
						c_min = std::min(std::min(_input_image(px, py, 0, 0),
												  _input_image(px, py, 0, 1)),
										 _input_image(px, py, 0, 2));
						
						p_min = std::min(p_min, c_min);
						if (p_min == 0)
							break;
					}
					if (p_min == 0)
						break;
				}
				_dark_channel(x, y, 0, 0) = p_min;
			}
		}
		
		//clock_t end = clock();
		//double elapsed_ms = double(end - begin) / CLOCKS_PER_SEC;
		//printf("dark channel took: %f\n", elapsed_ms);
		
	}
	
	/**
	 Calculation of atmospheric light from darkChannel
	 */
	Eigen::Vector3i atmosphericLight();
	
	void transmissionEstimate(Eigen::Vector3i atmLight);
	
	void getRadiance(Eigen::Vector3i atmLight, CImg<unsigned char> &trans, CImg<unsigned char> &out_rad);
	
	CImg<double> depthMap(CImg<unsigned char> &transmission);
	
	std::vector<int> sort_indices(const unsigned char *v, int size) {
		
		// initialize original index locations
		std::vector<int> idx(size);
		for (int i = 0; i != size; ++i) idx[i] = i;
			
		// sort indexes based on comparing values in v
		sort(idx.begin(), idx.end(),
		   [v](size_t i1, size_t i2) {return v[i1] < v[i2];});
			
		return idx;
	}
	
	/**
	 Each column of matrix m is random vector
	 */
	Eigen::MatrixXd cov(Eigen::MatrixXd m);
	
	/**
	 @param im		the image from which the pixels into matrix are extracted
	 @param window	window size, must be odd number greater than one.
	 @param cx		center of the window in x dimension
	 @param cy		center of the window in y dimension
	 @return		columnwise matrix of pixels from given window, each column
					of the matrix is one pixel (r, g, b) vector.
	 */
	Eigen::MatrixXd windowFlatMatrix(CImg<unsigned char> &im, int window, int cx, int cy);
	
	Eigen::MatrixXd vecFromTransmission(CImg<unsigned char> &trans);
	
	Eigen::MatrixXd rgbVec(CImg<unsigned char> &im, int x, int y);
	
	CImg<unsigned char> vecToImg(Eigen::MatrixXd &vec, int w, int h);
	
	Eigen::SparseMatrix<double> sparseDiagonal(int size, double val);
	
	CImg<unsigned char> matte();

};

#endif /* defined(__kimproc__SingleImageHazeRemoval__) */
