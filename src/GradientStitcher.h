//
//  GradientStitcher.hpp
//  kimproc
//
//  Created by Jan Brejcha on 03.03.17.
//
//

#ifndef GradientStitcher_h
#define GradientStitcher_h

#include <cstdio>
#include <string>
#include <assert.h>
#include <iostream>
#include <cmath>
#include <thread>

#include <Eigen/Dense>
#include <Eigen/SparseCore>
#include <Eigen/IterativeLinearSolvers>
#include <Eigen/SparseLU>

#include "Convolution.h"
#include "ImageUtil.h"

// must be at end (after Eigen), because Eigen defines Success as well as X11 does.
#include "CImg.h"

using namespace std;
using namespace cimg_library;

class GradientStitcher {
private:
	typedef Eigen::Triplet<double> Triplet;
	
	CImg<float> input_img;
	
	CImg<float> div_G;
	
	ImageUtil::BBox mask_bbox;
	
	
	/**
	 @brief		Copies pixels from src to dst selected by mask.
	 All three images need to have same resolution.
	 @param	 src	pixels to copy.
	 @param	 dst	image to paste pixels from src into.
	 @param	 mask	defines which pixels to copy. Pixels corresponding to
	 pixels with mask with value greater than zero (non-black)
	 pixels will be copied.
	 */
	void paste(const CImg<float> &src,
			   CImg<float> &dst, const
			   CImg<float> &mask);
	
	/**
	 @brief	Pastes smaller image into larger image starting at given coords.
	 @param src		Source image to be pasted.
	 @param dst		Destimation image to be updated.
	 @param	x		x coordinate of the top left corner where the src will be
	 pasted.
	 @param	y		y coordinate of the top left corner where the src will be
	 pasted.
	 */
	void paste(CImg<float> &src, CImg<float> &dst,
			   int x, int y);
	
	/**
	 @brief	solves iteratively discrete Poisson Equation using Gauss-Seidel
	 method.
	 @param	input	the input image
	 @param div_G	divergence of gradient vector field
	 @param output	the final image
	 @param num_steps	the number of iterations of the computation.
	 */
	void gaussSeidel(CImg<float> &input,
					 CImg<float> &div_G,
					 CImg<float> &output,
					 float tolerance,
					 bool display_calculation = false);
	
	/**
	 @brief	Sets border of the image to arbitrary value.
	 @param img		input image where to set border values.
	 @param val		value to set the border to.
	 */
	void setBorder(CImg<float> &img, float val, int border_size);
	
	void keepBorder(CImg<float> &img, float val);
	
	void clamp(CImg<float> &img, int min, int max);
	
	
	/**
	 Not working yet, probably need to have values in <0,1>
	 */
	void conjugateGradient(CImg<float> &input_img,
						   CImg<float> &div_G,
						   CImg<float> &output);
	
	
	void addLaplaceToTripletList(vector<Triplet> *tripletList,
								 CImg<float> &input_img,
								 int row, int x, int y, int c,
								 double m);
	
	/**
	 @brief Sets the border of tha mask to zero in the image.
	 */
	void setMaskBorderZero(CImg<float> &mask,
						   CImg<float> img);
	
	/**
	 Sets the border conditions of divergence of vector field (Gx, Gy)
	 from the input image.
	 @param input_img	The image to take border pixels from.
	 @param div_G		the divergence of vector field (Gx, Gy) to which
						the border will be set.
	 */
	static void setBorderConditions(CImg<float> &input_img,
									CImg<float> &div_G);
	
	/**
	 @brief Calculatex bounding box around the masked area (pixels>0) in the
			mask. This is later internally used to stitch area under the bbox
			only to solve smaller problem.
	 @param mask	the mask for which the bounding box is calculated.
	 */
	ImageUtil::BBox calculateMaskBBox(CImg<float> &mask);
	
	void cgComputeThread(vector<Triplet> *tripletList, CImg<float> &div_G, CImg<float> &output, int c);
	
	
public:
	
	
	/**
	 @brief		Gradient stitcher stitches the input image with the portion
				of stitch_img defined in mask.
	 @param		input_path	path of the input image.
	 @param		stitch_path	path of the image to be stitched.
	 @param		mask_path	path of the mask that defines the area of the
				stitch_img to be used for stitching with the input_img.
				Pixels with black color (0) in red channel are not used,
				other pixels are used.
	 */
	GradientStitcher(string input_path, string stitch_path, string mask_path);

	
	
	/**
	 @brief Calculates the stitching using Gauss-Seidel iterative method.
	 @param	output_img	reference to the output image.
	 @param tolerance	Tolerance on error of the result. Default 0.0001.
	 @param	display_calculation	if set to true, the iterations of the method
						are shown with the use of CImgDisplay.
	 */
	CImg<float> stitchGaussSeidel(float tolerance = 0.0001f,
								  bool display_calculation = true);
};

#endif /* GradientStitcher_h */
