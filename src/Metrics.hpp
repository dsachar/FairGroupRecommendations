/*
 * Metrics.hpp
 *
 *  Created on: Oct 6, 2017
 *      Author: dimitris
 */

#ifndef SRC_METRICS_HPP_
#define SRC_METRICS_HPP_


#include <map>
#include <sstream>
#include <cassert>


using namespace std;

class Metrics {
public:

//	float RMSE; // not used

	// ranking metrics: these are wrt some ground truth
	float KendalTau;
	float KendalTauTopK;
	float AP; // average precision
	map<int, float> prec; // precision @
	map<int, float> rec; // recall @
	map<int, float> NDCG; // NDCG @ (normalized discounted cumulative gain)

	// fairness metrics: these are independent of the ground truth; but to minimize code changes, we just repeat them for every ground truth
	map<int, float> fairness; // Fairness @
	map<int, float> avgBestRankScore; // avgBestRankScore @
	map<int, float> avgWorstRankScore; // avgWorstRankScore @
	map<int, int> minNumGood;

	// metrics from RecSys'17
	float sum_utility;
	float min_utility;
	float max_utility;
	float variance;
	float jain_fairness;
	float minmax;

	string getMetricsString(int step = 5, int stopAt = -1);
	string getMetricsResultString();
	static string getMetricsResultHeaderString();
	static string getMetricsResultFillerString();

	Metrics operator+(const Metrics& other);
};


Metrics operator* (float x, const Metrics& y);
Metrics worst (const Metrics& one, const Metrics& other);
Metrics best (const Metrics& one, const Metrics& other);


#endif /* SRC_METRICS_HPP_ */
