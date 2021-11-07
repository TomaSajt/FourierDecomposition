#pragma once
/* ---------------------------------------------------------------------------
** Basic implementation of Cooley-Tukey FFT algorithm in C++
**
** Author: Darko Lukic <lukicdarkoo@gmail.com>
** -------------------------------------------------------------------------*/

#ifndef FFT_h
#define FFT_h
#define _USE_MATH_DEFINES


#include <cmath>
#include <complex>
#include <vector>

extern void fft(std::vector<int> x_in,
	std::vector<std::complex<double>> x_out,
	int N);
void fft_rec(std::vector<std::complex<double>> x, int N);

#endif