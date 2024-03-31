/*
 * AggMetrics.hpp
 *
 *  Created on: Oct 6, 2017
 *      Author: dimitris
 */

#ifndef SRC_AGGMETRICS_HPP_
#define SRC_AGGMETRICS_HPP_


#include <vector>
#include <cmath>
#include <iostream>


#include "Metrics.hpp"

using namespace std;


class AggMetrics {
public:

	vector<string> criteria; // names of criteria
	int numCriteria; // criterion = aggregation strategy = ground truth
	int numRepetitions; // num of random repetitions

	vector<Metrics> allMetrics; // numCriteria * numRepetitions Metrics objects; iter1:crit1, iter1:crit2, ...

	vector<Metrics> avgOverRepsMetrics; // numCriteria Metrics objects


	Metrics avgMetrics;
	Metrics worstMetrics;
	Metrics bestMetrics;

	vector<Metrics> avgOverCriteriaMetrics; // numRepetitions Metrics objects
	vector<Metrics> worstOverCriteriaMetrics; // numRepetitions Metrics objects
	vector<Metrics> bestOverCriteriaMetrics; // numRepetitions Metrics objects


	AggMetrics();

	void averageOverRepetititions(); // create avgOverRepsMetrics

	void aggregateOverCriteria(); // create avgOverCriteriaMetrics, worstOverCriteriaMetrics, bestOverCriteriaMetrics

	void computeMarginals(); // create avgMetrics, worstMetrics, bestMetrics



	string getMarginalsString(int step = 5, int stopAt = -1);

};


#endif /* SRC_AGGMETRICS_HPP_ */
