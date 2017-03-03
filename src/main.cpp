/**
* @Author: Jan Brejcha <janbrejcha>
* @Date:   2014-10-25T22:58:03+02:00
* @Email:  ibrejcha@fit.vutbr.cz, brejchaja@gmail.com
* @Project: static system IS51
* @Last modified by:   janbrejcha
* @Last modified time: 2017-02-13T08:15:37+01:00
*/



//
//  main.cpp
//  kimproc
//
//  Created by Jan Brejcha on 25.10.14.
//  Copyright (c) 2014 brejchajan. All rights reserved.
//
#include "main.h"

#include "HarrisCornerDetector.h"
#include "SingleImageHazeRemoval.h"
#include "argumentparser.h"

#include "CImg.h"
#include <cstdio>
#include <vector>
#include <iostream>
#include <xlocale.h>

using namespace argpar;
using namespace std;


ArgumentParser buildArgumentParser(int argc, const char *argv[])
{
	ArgumentParser ap("kimproc", "Kaeru Image Processing Framework.");

	vector<Parameter> inpar;
	Parameter input_im("file path", "Path of the input image file");
	inpar.push_back(input_im);
	Argument input("i", "input-image", inpar, "Input image to be processed");

	vector<Parameter> outpar;
	Parameter output_im("file path", "Path of the output image file");
	outpar.push_back(output_im);
	Argument output("o", "output-image", outpar, "Output image name.");

	Parameter threshold("threshold", "threshold int value to be used for harris keypoint detection. Default value is 10 000.");
	vector<Parameter> hpar;
	hpar.push_back(threshold);

	Argument harris("h", "harris", hpar, "Calculates harris corners and visualizes them as red dots in the output image.", true);

	vector<Parameter> dpar;
	Argument dehaze("dh", "dehaze-HST09", dpar, "Implements article: Single Image Haze Removal Using Dark Channel Prior by He, Sun, Tung from CVPR 09.", true);
	
	vector<Parameter> s_par;
	s_par.push_back(Parameter("stitched image", "Image to be stitched with the "
							  "input image."));
	s_par.push_back(Parameter("mask", "Mask defining the pixels of the "
							  "stitched image to be used for stitching. "
							  "Black (zero) -> do not use, otherwise use."));
	Argument stitch("s", "stitch", s_par, "Stitches The portion of the "
				   "<stitched image> defined by <mask> with the input image.",
				   true);

	ap.addArgument(input);
	ap.addArgument(output);
	ap.addArgument(harris);
	ap.addArgument(dehaze);
	ap.addArgument(stitch);

	return ap;
}

void png16test(const char *input_image)
{
    cimg_library::CImg<int> src(input_image);
    for (int y = 0; y < src.height(); ++y)
	{
		for (int x = 0; x < src.width(); ++x)
		{
			float r = src(x,y,0,0); // First channel RED
            std::cout << r << ", ";
        }
        std::cout << std::endl;
    }
}


int main(int argc, const char *argv[])
{
	ArgumentParser ap = buildArgumentParser(argc, argv);

	if (ap.parse(argc, argv))
	{
		string input_image = ap.resultByShortname("i")[0];
		string output_path = ap.resultByShortname("o")[0];
		Argument *harrisArg = ap.argumentByName("harris");
		Argument *matchArg = ap.argumentByName("match-sift");
		bool harris = harrisArg->exists();
		bool dehaze = ap.argumentByShortname("dh");

		//Options parsed.
		//Load the image for processing
		cimg_library::CImg<unsigned char> src(input_image.c_str());

        png16test(input_image.c_str());

		if (harris)
		{
			int harris_threshold = atoi(harrisArg->getResult()[0].c_str());
			HarrisCornerDetector::detect(src, harris_threshold);
		}
		if (dehaze)
		{
			SingleImageHazeRemoval sihr(src, output_path);
			sihr.dehaze();
		}
		if (matchArg->exists())
		{
			
		}
		
		//Save the final image.
		src.save(output_path.c_str());

	}

	return EXIT_SUCCESS;

}
