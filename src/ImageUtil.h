//
//  ImageUtil.hpp
//  kimproc
//
//  Created by Jan Brejcha on 03.03.17.
//
//

#ifndef ImageUtil_h
#define ImageUtil_h

#include <Eigen/Core>

#include <cstdio>
#include <assert.h>
#include <cmath>
#include <iostream>


#include "CImg.h"

using namespace cimg_library;

class ImageUtil {
	
public:
	
	struct BBox{
		int x1, x2, y1, y2;
		
		BBox()
		:x1(0), x2(0), y1(0), y2(0){}
		
		/**
		 Creates new bounding box from top left x1, y1 and bottom right x2, y2
		 @param x1	top left x
		 @param y1	top left y
		 @param x2	bottom right x
		 @param	y2	bottom right y
		 */
		BBox(int x1, int y1, int x2, int y2)
		:x1(x1), x2(x2), y1(y1), y2(y2){}
	};
	
	
	template<typename T>
	static Eigen::MatrixXd img2Vec(CImg<T> &img, int channel)
	{
		int w = img.width();
		int h = img.height();
		
		Eigen::MatrixXd vec(w * h, 1);
		
		for (int j = 0; j < h; ++j)
		{
			for (int i = 0; i < w; ++i)
			{
				vec(j * w + i, 0) = (double)img(i, j, 0, channel);
			}
		}
		
		return vec;
	}
	
	/**
	 @brief	Fills the image out_img with values from vector of length 
			width * height of the image.
	 @param channel		defines the channel into which the values are stored.
	 */
	
	static void vec2Img(Eigen::MatrixXd &vec, CImg<float> &out_img, int channel)
	{
		int w = out_img.width();
		int h = out_img.height();
		int dim = w * h;
		if (dim != vec.size())
		{
			throw std::runtime_error("This vector does not have right number "
									 "of elements to fill image of this size.");
		}
		int x, y;
		for (int i = 0; i < vec.size(); ++i)
		{
			x = i % w;
			y = i / w;
			out_img(x, y, 0, channel) = (float)vec(i);
		}
	}
	
	template<typename T>
	static bool allclose(CImg<T> &first, CImg<T> &second, T val)
	{
		int w = first.width();
		int h = first.height();
		int d = first.depth();
		int s = first.spectrum();
		assert(w == second.width() && h == second.height() &&
			   d == second.depth() && s == second.spectrum());
		
		for (int y = 0; y < h; ++y)
		{
			for (int x = 0; x < w; ++x)
			{
				for (int g = 0; g < d; ++g)
				{
					for (int c = 0; c < s; ++c)
					{
						double test = std::abs(first(x, y, g, c) - second(x, y, g, c));
						if (test > val)
						{
							//std::cout << "violated: x: " << x << ", y: "
							//<< y << ", err: " << std::abs(first(x, y, g, c) -
							//second(x, y, g, c)) << std::endl;
							return false;
						}
					}
				}
			}
		}
		return true;
	}

	
private:
	
	ImageUtil(){}
	
};

#endif /* ImageUtil_h */
