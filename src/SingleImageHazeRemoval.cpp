//
//  SingleImageHazeRemoval.cpp
//  kimproc
//
//  Created by Jan Brejcha on 15.07.15.
//  Copyright (c) 2015 brejchajan. All rights reserved.
//

#include "SingleImageHazeRemoval.h"

#define CLOCK_PER_MS CLOCKS_PER_SEC/1000.0
SingleImageHazeRemoval::SingleImageHazeRemoval(CImg<unsigned char> &image, std::string _output_name)
: input_image(image), output_name(_output_name)
{
	dark_channel = CImg<unsigned char>(image.width(), image.height(), 1, 1);
}

/**
 Entry method that runs dehazing process
 */
void SingleImageHazeRemoval::dehaze()
{
	darkChannel<unsigned char>(input_image, dark_channel);
	dark_channel.save((output_name + "_darkChannel.png").c_str());
	Eigen::Vector3i atm = atmosphericLight();
	
	transmissionEstimate(atm);
	
	CImg<unsigned char> rad_est(input_image.width(), input_image.height(), 1, 3);
	getRadiance(atm, transEst, rad_est);
	rad_est.save((output_name + "_radEst.png").c_str());
	
	CImg<unsigned char> trans = matte();
	trans.save((output_name + "_trans.png").c_str());
	CImg<unsigned char> rad(input_image.width(), input_image.height(), 1, 3);
	getRadiance(atm, trans, rad);
	rad.save((output_name + "_rad.png").c_str());

	CImg<unsigned char> depth = depthMap(trans);
	depth.save((output_name + "_depth.png").c_str());
}

/**
 Calculation of atmospheric light from darkChannel
 */
Eigen::Vector3i SingleImageHazeRemoval::atmosphericLight()
{
	int w = input_image.width();
	int h = input_image.height();
	
	int pixels = w * h;
	int pixels_2 = pixels * 2;
	//take 0.1% of all pixels
	int numpx = (int)floor((double)pixels / 1000.0);
	unsigned char *data = dark_channel.data();
	unsigned char *im = input_image.data();
	std::vector<int> idx = sort_indices(data, pixels);
	
	Eigen::Vector3i atm(0, 0, 0);
	for (int i = pixels - 1; i >= pixels - numpx; --i)
	{
		int index = idx[i];
		atm += Eigen::Vector3i(im[index], im[index + pixels], im[index + pixels_2]);
	}
	atm = atm / numpx;
	
	return atm;
}

void SingleImageHazeRemoval::transmissionEstimate(Eigen::Vector3i atmLight)
{
	int w = input_image.width();
	int h = input_image.height();
	
	//normalize input image by atmospheric light
	CImg<double> normalized(w, h, 1, 3);
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			normalized(x, y, 0, 0) = (double)input_image(x, y, 0, 0) / (double)atmLight.x();
			normalized(x, y, 0, 1) = (double)input_image(x, y, 0, 1) / (double)atmLight.y();
			normalized(x, y, 0, 2) = (double)input_image(x, y, 0, 2) / (double)atmLight.z();
		}
	}
	CImg<double> _transEst = CImg<double>(w, h, 1, 1);
	transEst = CImg<unsigned char>(w, h, 1, 1);
	darkChannel<double>(normalized, _transEst);
	
	//find max
	double maximum = 0.0;
	double val;
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			val = _transEst(x, y, 0, 0);
			if (val > maximum)
				maximum = val;
		}
	}
	
	_transEst = (maximum - 0.95 * _transEst);
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			transEst(x, y, 0, 0) = (unsigned char)floor((_transEst(x, y, 0, 0) / maximum) * 255.0);
		}
	}
}

void SingleImageHazeRemoval::getRadiance(Eigen::Vector3i atmLight, CImg<unsigned char> &trans, CImg<unsigned char> &out_rad)
{
	int w = input_image.width();
	int h = input_image.height();
	
	double ax = atmLight.x() / 255.0;
	double ay = atmLight.y() / 255.0;
	double az = atmLight.z() / 255.0;
	double transmission;
	double t;
	
	CImg<double> _out_rad(out_rad.width(), out_rad.height(), 1, 3);
	double maxr = 0;
	double maxg = 0;
	double maxb = 0;
	double minr = 100000;
	double ming = 100000;
	double minb = 100000;
	
	double r, g, b;
	
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			transmission = ((double)trans(x, y, 0, 0))/255.0;
			//std::cout << "trans: " << transmission << std::endl;
			t = std::max(transmission, T0);
			r = (((input_image(x, y, 0, 0) / 255.0) - ax)/t + ax);
			g = (((input_image(x, y, 0, 1) / 255.0) - ay)/t + ay);
			b = (((input_image(x, y, 0, 2) / 255.0) - az)/t + az);
			maxr = std::max(r, maxr);
			maxg = std::max(g, maxg);
			maxb = std::max(b, maxb);
			minr = std::min(r, minr);
			ming = std::min(g, ming);
			minb = std::min(b, minb);
			
			_out_rad(x, y, 0, 0) = r;
			_out_rad(x, y, 0, 1) = g;
			_out_rad(x, y, 0, 2) = b;
		}
	}
	maxr = maxr - minr;
	maxg = maxg - ming;
	maxb = maxb - minb;
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			r = ((_out_rad(x, y, 0, 0) - minr) / maxr) * 255.0;
			g = ((_out_rad(x, y, 0, 1)  - ming) / maxg) * 255.0;
			b = ((_out_rad(x, y, 0, 2)  - minb) / maxb) * 255.0;
			r = r < 0 ? 0 : r;
			g = g < 0 ? 0 : g;
			b = b < 0 ? 0 : b;
			out_rad(x, y, 0, 0) = (unsigned char)floor(r);
			out_rad(x, y, 0, 1) = (unsigned char)floor(g);
			out_rad(x, y, 0, 2) = (unsigned char)floor(b);
		}
	}
}

CImg<double> SingleImageHazeRemoval::depthMap(CImg<unsigned char> &transmission)
{
	CImg<double> depth_map = CImg<double>(transmission);
	log(depth_map);
	return depth_map;
}


Eigen::MatrixXd SingleImageHazeRemoval::cov(Eigen::MatrixXd m)
{
	m.transposeInPlace();
	Eigen::MatrixXd centered = m.rowwise() - m.colwise().mean();
	Eigen::MatrixXd cov = (centered.adjoint() * centered) / double(m.rows());
	return cov;
}

CImg<unsigned char> SingleImageHazeRemoval::matte()
{
	int w = input_image.width();
	int h = input_image.height();
	int dim = w * h;
	Eigen::SparseMatrix<double> L(dim, dim);
	int window = 3;
	int w_pixels = window * window;
	double inv_w_pixels = 1.0 / (double)w_pixels;
	double eps = 0.00000001;
	double lam = 0.0001;
	double eps_inv_w_pixels = eps * inv_w_pixels;
	Eigen::MatrixXd epsid = eps_inv_w_pixels * Eigen::MatrixXd::Identity(window, window);
	
	int side = (window - 1) / 2;
	
	typedef Eigen::Triplet<double> T;
	std::vector<T> tripletList;
	tripletList.reserve(2 * dim);
	int lx, ly;
	int j_s, i_s;
	double k_d;
	Eigen::MatrixXd var_f;
	Eigen::MatrixXd var_s;
	Eigen::MatrixXd one(1, 1);
	one(0, 0) = 1;
	Eigen::MatrixXd k_dm(1, 1);
	
	std::map<std::pair<int, int>, Eigen::MatrixXd> res;
	clock_t begin = clock();
	k_d = 0.0;
	res.clear();
	Eigen::MatrixXd val;
	
	//Matting Laplacian matrix construction
	for (int j = 1; j < h - 1; ++j)
	{
		for (int i = 1; i < w - 1; ++i)
		{
			Eigen::MatrixXd wf = windowFlatMatrix(input_image, window, i, j);
			Eigen::MatrixXd w_mean = wf.rowwise().mean();
			Eigen::MatrixXd w_cov = cov(wf);
			Eigen::MatrixXd w_inv_cov_id = (w_cov + epsid).inverse();
			j_s = j - side;
			i_s = i - side;
			for (int fy = j_s; fy < j_s + window; ++fy)
			{
				for (int fx = i_s; fx < i_s + window; ++fx)
				{
					lx = fy * w + fx;
					for (int sy = j_s; sy < j_s + window; ++sy)
					{
						for (int sx = i_s; sx < i_s + window; ++sx)
						{
							ly = sy * w + sx;
							k_d = lx == ly ? 1.0 : 0.0;
							k_dm(0, 0) = k_d;
							var_f = rgbVec(input_image, fx, fy) - w_mean;
							var_f.transposeInPlace();
							var_s = rgbVec(input_image, sx, sy) - w_mean;
						
							val = (k_dm - (inv_w_pixels * (one + var_f * w_inv_cov_id * var_s)));
							
							tripletList.push_back(T(lx, ly, val(0, 0)));
						}
					}
					
				}
			}
		}
	}
	L.setFromTriplets(tripletList.begin(), tripletList.end());
#ifdef TIME_DEBUG
	clock_t end = clock();
	double elapsed_ms = double(end - begin) / CLOCKS_PER_SEC;
	printf("laplacian construction took: %f\n", elapsed_ms);
#endif
	begin = clock();
	
	Eigen::MatrixXd t = vecFromTransmission(transEst);
	Eigen::MatrixXd b = lam * t;
	Eigen::SparseMatrix<double> speye = sparseDiagonal(dim, lam);
	//solve the system
	Eigen::SparseMatrix<double> A = L + speye;
	Eigen::ConjugateGradient<Eigen::SparseMatrix<double>> cg;
	cg.compute(A);
	Eigen::MatrixXd matte_t = cg.solve(b);
	
	CImg<unsigned char> trans = vecToImg(matte_t, w, h);
	
#ifdef TIME_DEBUG
	end = clock();
	elapsed_ms = double(end - begin) / CLOCKS_PER_SEC;
	printf("linear system solution took: %f\n", elapsed_ms);
#endif
	return trans;
}

CImg<unsigned char> SingleImageHazeRemoval::vecToImg(Eigen::MatrixXd &vec, int w, int h)
{
	int dim = w * h;
	if (dim != vec.size())
	{
		throw std::runtime_error("This vector does not have right number of elements to fill image of this size.");
	}
	int x, y;
	CImg<unsigned char> img(w, h, 1);
	double val;
	for (int i = 0; i < vec.size(); ++i)
	{
		x = i % w;
		y = i / w;
		//std::cout << vec(i) << std::endl;
		val = vec(i);
		val = val > 255.0 ? 255.0 : val;
		val = val < 0.0 ? 0.0 : val;
		img(x, y, 0, 0) = (unsigned char)floor(val);
	}	
	return img;
}

Eigen::MatrixXd SingleImageHazeRemoval::vecFromTransmission(CImg<unsigned char> &trans)
{
	int w = trans.width();
	int h = trans.height();
	
	Eigen::MatrixXd vec(w * h, 1);
	
	for (int j = 0; j < h; ++j)
	{
		for (int i = 0; i < w; ++i)
		{
			vec(j * w + i, 0) = (double)trans(i, j, 0, 0);
		}
	}
	
	return vec;
}


Eigen::MatrixXd SingleImageHazeRemoval::windowFlatMatrix(CImg<unsigned char> &im, int window, int cx, int cy)
{
	int side = (window - 1)/2;
	int sx = cx - side;
	int sy = cy - side;
	int maxx = sx + window;
	int maxy = sy + window;
	int dim = window * window;
	Eigen::MatrixXd m(3, dim);
	int ind = 0;
	for (int i = sx; i < maxx; ++i)
	{
		for (int j = sy; j < maxy; ++j)
		{
			m(0, ind) = ((double)im(i, j, 0, 0))/255.0;
			m(1, ind) = ((double)im(i, j, 0, 1))/255.0;
			m(2, ind) = ((double)im(i, j, 0, 2))/255.0;
			ind++;
		}
	}
	
	return m;
}

Eigen::SparseMatrix<double> SingleImageHazeRemoval::sparseDiagonal(int size, double val)
{
	Eigen::SparseMatrix<double> res(size, size);
	typedef Eigen::Triplet<double> T;
	std::vector<T> tripletList;
	for (int i = 0; i < size; ++i)
	{
		tripletList.push_back(T(i, i, val));
	}
	res.setFromTriplets(tripletList.begin(), tripletList.end());
	return res;
}

Eigen::MatrixXd SingleImageHazeRemoval::rgbVec(CImg<unsigned char> &im, int x, int y)
{
	Eigen::MatrixXd vec(3, 1);
	vec(0, 0) = ((double)im(x, y, 0, 0)/255.0);
	vec(1, 0) = ((double)im(x, y, 0, 1)/255.0);
	vec(2, 0) = ((double)im(x, y, 0, 2)/255.0);
	
	return vec;
}
