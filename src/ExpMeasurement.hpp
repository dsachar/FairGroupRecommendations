/*
 * Measurement.hpp
 *
 *  Created on: Apr 14, 2015
 *      Author: dimitris
 */

#ifndef SRC_EXPMEASUREMENT_HPP_
#define SRC_EXPMEASUREMENT_HPP_

#include <string>
#include <map>
#include <vector>

using namespace std;



class ExpMeasurement {
public:

	// rmse
	float rmse_total;
	float rmse_group_max;
	vector<float> rmses;
	vector<float> aes; // absolute errors

	// Kendal tau
	float ktau_avg;
	float ktau_group_min;

	// precision @
	map<int, float> precAt_avg;
	map<int, float> precAt_min;
	float MAP;

	// recall @
	map<int, float> recAt_avg;
	map<int, float> recAt_min;

	// NDCG @ (normalized discounted cumulative gain)
	map<int, float> NDCGAt_avg;
	map<int, float> NDCGAt_min;

	vector<int> posRelevants;
	map<int, int> RelevantAtPos;

	static const int numPos = 10;
	int RelevantAtPosArray[numPos] = {0};


	static string getMetricString();
	string getMeasurementString();
	static string getMetricTabsString();

	string getMeasurementRelAtNString();

	static string getMetricAtNString();
	string getMeasurementAtNString();
	static string getMetricAtNTabsString();

};



#endif /* SRC_EXPMEASUREMENT_HPP_ */
