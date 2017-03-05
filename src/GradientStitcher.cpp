//
//  GradientStitcher.cpp
//  kimproc
//
//  Created by Jan Brejcha on 03.03.17.
//
//

#include "GradientStitcher.h"

GradientStitcher::GradientStitcher(string input_path,
								   string stitch_path,
								   string mask_path)
{
	input_img = CImg<float>(input_path.c_str()).normalize(0, 1);
	CImg<float> stitch_img = CImg<float>(stitch_path.c_str()).normalize(0, 1);
	CImg<float> mask_img = CImg<float>(mask_path.c_str()).normalize(0, 1);
	
	assert(input_img.height() == stitch_img.height() &&
		   input_img.width() == stitch_img.width());
	assert(input_img.height() == mask_img.height() &&
		   input_img.width() == mask_img.width());
	
	int width = input_img.width();
	int height = input_img.height();
	int spectrum = input_img.spectrum();
	
	mask_bbox = calculateMaskBBox(mask_img);
	
	CImg<float> G_x = CImg<float>(width, height, 1, spectrum);
	CImg<float> G_y = CImg<float>(width, height, 1, spectrum);
	
	//std::cout << "width: " << G_x.width() << ", height: " << G_x.width() << ", spectrum: " << spectrum << std::endl;
	
	CImg<float> stitch_G_x = CImg<float>(width, height, 1, spectrum);
	CImg<float> stitch_G_y = CImg<float>(width, height, 1, spectrum);
	
	float kernel[2] = {1, -1};
	Convolution::convolve1D(input_img, G_x, 2, kernel, DIR_HORIZ);
	Convolution::convolve1D(input_img, G_y, 2, kernel, DIR_VERT);
	Convolution::convolve1D(stitch_img, stitch_G_x, 2, kernel, DIR_HORIZ);
	Convolution::convolve1D(stitch_img, stitch_G_y, 2, kernel, DIR_VERT);
	
	paste(stitch_G_x, G_x, mask_img);
	paste(stitch_G_y, G_y, mask_img);
	setMaskBorderZero(mask_img, stitch_G_x);
	setMaskBorderZero(mask_img, stitch_G_y);
	paste(stitch_img, input_img, mask_img);
	
	//second derivative
	Convolution::convolve1D(G_x, stitch_G_x, 2, kernel, DIR_HORIZ);
	Convolution::convolve1D(G_y, stitch_G_y, 2, kernel, DIR_VERT);
	
	//make stitch_G_x is equal to divergence of vector field (G_x, G_y)
	//div G = dG_x/dx + dG_y/dy;
	stitch_G_x += stitch_G_y;
	setBorder(stitch_G_x, 0, 1);
	div_G = stitch_G_x;
	
	CImg<float> output_img = CImg<float>(width, height, 1, spectrum);
}


CImg<float> GradientStitcher::stitchGaussSeidel(float tolerance,
												bool display_calculation)
{
	
	CImg<float> crop_input = input_img.get_crop(mask_bbox.x1, mask_bbox.y1,
												mask_bbox.x2, mask_bbox.y2);
	CImg<float> output_img_crop = CImg<float>(crop_input);
	
	div_G.crop(mask_bbox.x1, mask_bbox.y1, mask_bbox.x2, mask_bbox.y2);
	//setBorder(div_G, 0, 1);
	setBorderConditions(input_img, div_G);
	//div_G.display();
	
	gaussSeidel(crop_input, div_G, output_img_crop, tolerance, display_calculation);
	//conjugateGradient(crop_input, div_G, output_img_crop);
	clamp(output_img_crop, 0, 1);
	
	//cout << "Stitching complete." << std::endl;
	//paste smaller stitcher image into the original one
	CImg<float> output_img = CImg<float>(input_img);
	paste(output_img_crop, output_img, mask_bbox.x1, mask_bbox.y1);
	
	return output_img;
}

void GradientStitcher::gaussSeidel(CImg<float> &input,
								   CImg<float> &div_G,
								   CImg<float> &output,
								   float tolerance,
								   bool display_calculation)
{
	int height = input.height();
	int width = input.width();
	int spectrum = input.spectrum();
	//iterate until convergence
	
	CImg<float> *in = &input;
	CImg<float> *out = &output;
	CImg<float> *temp = NULL;
	
	CImgDisplay disp_orig;
	CImgDisplay disp;
	
	if (display_calculation)
	{
		disp_orig.display(input);
	}
	
	while (true)
	{
		for (int y = 1; y < height-1; ++y)
		{
			for (int x = 1; x < width-1; ++x)
			{
				for (int c = 0; c < spectrum; ++c)
				{
					(*out)(x, y, 0, c) =	0.25 * ((*in)(x+1, y, 0, c) +
													(*in)(x-1, y, 0, c) +
													(*in)(x, y+1, 0, c) +
													(*in)(x, y-1, 0, c) -
													div_G(x, y, 0, c));
				}
			}
		}
		//cout << "iter: " << i << endl;
		temp = out;
		out = in;
		in = temp;
		temp = NULL;
		if (display_calculation)
		{
			disp.display(*in);
		}
		if (ImageUtil::allclose(*in, *out, tolerance))
		{
			break;
		}
	}
}


void GradientStitcher::conjugateGradient(CImg<float> &_input_img,
										 CImg<float> &div_G,
										 CImg<float> &output)
{
	int width = _input_img.width();
	int height = _input_img.height();
	int spectrum = _input_img.spectrum();
	int dim = width * height;
	
	//set border conditions on div_G according to the input
	//div_G.display();
	
	
	vector< vector<Triplet>* > colorTripletList;
	
	for (int c = 0; c < spectrum; ++c)
	{
		std::vector<Triplet> *tripletList = new vector<Triplet>();
		tripletList->reserve(2 * dim);
		colorTripletList.push_back(tripletList);
	}
	
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			for (int c = 0; c < spectrum; ++c)
			{
				vector<Triplet> *tripletList = colorTripletList[c];
				int row = y * width + x;
				int x_m1 = x - 1;
				int x_p1 = x + 1;
				int y_m1 = y - 1;
				int y_p1 = y + 1;
				if (x_m1 >= 0)
				{
					addLaplaceToTripletList(tripletList, _input_img, row, x_m1, y, c, 1);
				}
				if (x_p1 < width)
				{
					addLaplaceToTripletList(tripletList, _input_img, row, x_p1, y, c, 1);
				}
				if (y_m1 >= 0)
				{
					addLaplaceToTripletList(tripletList, _input_img, row, x, y_m1, c, 1);
				}
				if (y_p1 < height)
				{
					addLaplaceToTripletList(tripletList, _input_img, row, x, y_p1, c, 1);
				}
				addLaplaceToTripletList(tripletList, _input_img, row, x, y, c, -4);
				
			}
		}
	}
	
	for (int c = 0; c < spectrum; ++c)
	{
		vector<Triplet> *tripletList = colorTripletList[c];
		Eigen::MatrixXd b = ImageUtil::img2Vec(div_G, c);
		
		Eigen::SparseMatrix<double> A(dim, dim);
		A.setFromTriplets(tripletList->begin(), tripletList->end());
		//cout << A << endl;
		Eigen::ConjugateGradient<Eigen::SparseMatrix<double>> cg;
		cout << "computing A using Eigen" << endl;
		cg.compute(A);
		cout << "solving Poisson Equation" << endl;
		Eigen::MatrixXd out = cg.solve(b);
		ImageUtil::vec2Img(out, output, c);
	}
	
	
}

void GradientStitcher::cgComputeThread(vector<Triplet> *tripletList, CImg<float> &div_G, CImg<float> &output, int c)
{
	int dim = div_G.width() * div_G.height();
	Eigen::MatrixXd b = ImageUtil::img2Vec(div_G, c);
	//Eigen::MatrixXd guess = ImageUtil::img2Vec(_input_img);
	
	Eigen::SparseMatrix<double> A(dim, dim);
	A.setFromTriplets(tripletList->begin(), tripletList->end());
	//cout << A << endl;
	Eigen::ConjugateGradient<Eigen::SparseMatrix<double>> cg;
	cout << "computing A using Eigen" << endl;
	cg.compute(A);
	cout << "solving Poisson Equation" << endl;
	Eigen::MatrixXd out = cg.solve(b);
	ImageUtil::vec2Img(out, output, c);
}

void GradientStitcher::addLaplaceToTripletList(vector<Triplet> *tripletList,
											   CImg<float> &input_img,
											   int row, int x, int y, int c,
											   double m)
{
	int col = y * input_img.width() + x;
	tripletList->push_back(Triplet(col, row, m /** input_img(x, y, 0, c)*/));
}


void GradientStitcher::setBorder(CImg<float> &img, float val, int border_size)
{
	int width = img.width();
	int height = img.height();
	int spectrum = img.spectrum();
	for (int y = 0; y < border_size; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			for (int c = 0; c < spectrum; ++c)
			{
				img(x, y, 0, c) = val;
			}
		}
	}
	for (int y = height - 1 - border_size; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			for (int c = 0; c < spectrum; ++c)
			{
				img(x, y, 0, c) = val;
			}
		}
	}
	
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < border_size; ++x)
		{
			for (int c = 0; c < spectrum; ++c)
			{
				img(x, y, 0, c) = val;
			}
		}
		for (int x = width - 1 - border_size; x < width; ++x)
		{
			for (int c = 0; c < spectrum; ++c)
			{
				img(x, y, 0, c) = val;
			}
		}
	}
}


void GradientStitcher::keepBorder(CImg<float> &img, float val)
{
	int width = img.width();
	int height = img.height();
	int spectrum = img.spectrum();
	for (int y = 1; y < height - 1; ++y)
	{
		for (int x = 1; x < width - 1; ++x)
		{
			for (int c = 0; c < spectrum; ++c)
			{
				img(x, y, 0, c) = val;
			}
		}
	}
}

void GradientStitcher::paste(const CImg<float> &src,
							 CImg<float> &dst, const
							 CImg<float> &mask)
{
	assert(src.height() == dst.height() &&
		   src.width() == dst.width());
	assert(src.height() == mask.height() &&
		   src.width() == mask.width());
	assert(src.spectrum() == dst.spectrum());
	
	int height = src.height();
	int width = src.width();
	
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			if (mask(x, y, 0, 0) > 0)
			{
				for (int i = 0; i < src.spectrum(); ++i)
				{
					dst(x, y, 0, i) = src(x, y, 0, i);
				}
			}
		}
	}
}


void GradientStitcher::setMaskBorderZero(CImg<float> &mask,
										 CImg<float> img)
{
	float kernel[2] = {1, -1};
	int width = mask.width();
	int height = mask.height();
	CImg<float> gmask = CImg<float>(width, height,
									mask.depth(), mask.spectrum());
	Convolution::convolve1D(mask, gmask, 2, kernel, DIR_HORIZ);
	Convolution::convolve1D(mask, gmask, 2, kernel, DIR_VERT);
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			if (gmask(x, y, 0, 0) > 0)
			{
				for (int c = 0; c < img.spectrum(); ++c)
				{
					img(x, y, 0, c);
				}
			}
		}
	}
}

ImageUtil::BBox GradientStitcher::calculateMaskBBox(CImg<float> &mask)
{
	int width = mask.width();
	int height = mask.height();
	int x1 = width;
	int y1 = height;
	int x2, y2 = 0;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			if (mask(x, y, 0, 0) > 0)
			{
				if (x < x1)
					x1 = x;
				if (x > x2)
					x2 = x;
				if (y < y1)
					y1 = y;
				if (y > y2)
					y2 = y;
			}
		}
	}
	
	//we do not want tight bbox, we want 50 px greater bbox, but only if
	//it does not violate the position of the bbox in the image.
	int mindx_1 = min(x1, 50);
	int mindx_2 = min(width - x2 - 1, 50);
	int mindy_1 = min(y1, 50);
	int mindy_2 = min(height - y2 - 1, 50);
	
	x1 -= mindx_1; x2 += mindx_2; y1 -= mindy_1; y2 += mindy_2;
	return ImageUtil::BBox(x1, y1, x2, y2);
}

void GradientStitcher::paste(CImg<float> &src, CImg<float> &dst,
								int x, int y)
{
	int width = src.width();
	int height = src.height();
	assert(x + width <= dst.width() && y + height <= dst.height());
	assert(src.spectrum() == dst.spectrum());
	
	for (int y1 = 0; y1 < height; ++y1)
	{
		for (int x1 = 0; x1 < width; ++x1)
		{
			for (int c = 0; c < src.spectrum(); ++c)
			{
				dst(x1 + x, y1 + y, 0, c) = src(x1, y1, 0, c);
			}
		}
	}
}

void GradientStitcher::clamp(CImg<float> &img, int min, int max)
{
	int width = img.width();
	int height = img.height();
	
	for (int y1 = 0; y1 < height; ++y1)
	{
		for (int x1 = 0; x1 < width; ++x1)
		{
			for (int c = 0; c < img.spectrum(); ++c)
			{
				float val = img(x1, y1, 0, c);
				img(x1, y1, 0, c) = img(x1, y1, 0, c) >= max ? max : val;
				img(x1, y1, 0, c) = img(x1, y1, 0, c) <= min ? min : val;
			}
		}
	}
}

void GradientStitcher::setBorderConditions(CImg<float> &input_img,
										   CImg<float> &div_G)
{
	int iwidth = input_img.width();
	int iheight = input_img.height();
	int width = div_G.width();
	int height = div_G.height();
	int spectrum = div_G.spectrum();
	
	int y = 0;
	for (int x = 0; x < width; ++x)
	{
		for (int c = 0; c < spectrum; ++c)
		{
			int y_m1 = max(y - 1, 0);
			div_G(x, y, 0, c) = div_G(x, y, 0, c) - input_img(x, y_m1, 0, c);
		}
	}
	y = height - 1;
	for (int x = 0; x < width; ++x)
	{
		for (int c = 0; c < spectrum; ++c)
		{
			int y_p1 = min(y + 1, iheight - 1);
			div_G(x, y, 0, c) = div_G(x, y, 0, c) - input_img(x, y_p1, 0, c);
		}
	}
	int x = 0;
	for (int y = 0; y < height; ++y)
	{
		for (int c = 0; c < spectrum; ++c)
		{
			int x_m1 = max(x - 1, 0);
			div_G(x, y, 0, c) = div_G(x, y, 0, c) - input_img(x_m1, y, 0, c);
		}
	}
	x = width - 1;
	for (int y = 0; y < height; ++y)
	{
		for (int c = 0; c < spectrum; ++c)
		{
			int x_p1 = min(x + 1, iwidth - 1);
			div_G(x, y, 0, c) = div_G(x, y, 0, c) - input_img(x_p1, y, 0, c);
		}
	}
}
