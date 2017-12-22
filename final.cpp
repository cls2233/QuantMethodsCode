// final.cpp - G5260 final exam
////
#include "G5260.h"
#include "xll/xll.h"
#include "fms_black.h"

using namespace xll;
using namespace fms;

// A _binary call option_ with strike k and maturity t pays b(x) = 1(x > k) at t.
// Assume call options are available at strikes k_ and _k where k_ < k < _k
// having prices c_ and _c respectively. Write a function that gives the
// price of an option with payoff 0 for x <= k_, 1 for x >= _k and is
// linear from (k_, 0) to (_k, 1).

// So essentially what we want to do is price the binary option as a call spread that is scaled
// to match the payoff of 1 if x > k. That is, price of the approximate binary is:
// 1/(_k - k_) * ( c_ - _c )

#pragma warning(disable: 4100)
inline double approximate_binary(double k_, double c_, double _k, double _c)
{
	return 1 / (_k - k_) * (c_ - _c);
}

// The forward value of a binary call is E 1(F > k) = P(Z > z) where
// z = (s^2/2 + log(k/f))/s, F = f exp(sZ - s^2/2), and Z is standard normal.
// Use fms::black::call to calculate the difference between the approximate
// and the true value using f = k = 100, s = 0.01, k_ = 99, and _k = 101.
//// if s = 0.01, then sigma = 0.01 and t = 1

xll::test approximate_binary_test([]() {
	double f = 100, k = 100, s = 0.01, k_ = 99, _k = 101;
	double c_ = fms::black::call(f, s, k_, 1); ////we can just use s as sigma in the formula
	double _c = fms::black::call(f, s, _k, 1);
	double ab = approximate_binary(k_, c_, _k, _c);
	double bc = 1 - fms::prob::normal::cdf((s*s / 2 + log(k / f)) / s);
	double eps = 0.0001; 
	ensure(ab - bc == eps);
});

// The Bachelier model posits the stock price at time t is F_t = f + sigma B_t. 
// The forward call value is E max{F_t - k, 0} = (f - k) N(d) + sigma sqrt(t) n(d),
// where d = (f - k)/(sigma sqrt(t)), N is the standard normal cumulative distribution
// and n is the standard normal probability density function.

//// Implement a call valuation formula
inline double bachelier_call(double f, double sigma, double k, double t)
{
	double d = (f - k) / (sigma * sqrt(t));
	return (f - k) * fms::prob::normal::cdf(d) + sigma * sqrt(t) * fms::prob::normal::pdf(d);
}

////

AddIn xai_bachelier_call(
	Function(XLL_DOUBLE, L"?xll_bachelier_call", L"BACHELIER.CALL")
	.Arg(XLL_DOUBLE, L"f", L"is the current price")
	.Arg(XLL_DOUBLE, L"sigma", L"is the implied volatility")
	.Arg(XLL_DOUBLE, L"k", L"is the strike price")
	.Arg(XLL_DOUBLE, L"t", L"is the time to expiration")
	.Category(CATEGORY)
	.FunctionHelp(L"Returns the call price according to the Bachelier model.")
);
double WINAPI xll_bachelier_call(double f, double sigma, double k, double t)
{
#pragma XLLEXPORT
	double result;
	try {
		result = bachelier_call(f, sigma, k, t);
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());

		return 0;
	}

	return result;
}

// Implemented in excel.