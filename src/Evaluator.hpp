/*
 * Evaluator.hpp
 *
 *  Created on: 20 Mar 2017
 *      Author: dimitris
 */

#ifndef SRC_EVALUATOR_HPP_
#define SRC_EVALUATOR_HPP_




#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <tuple>
#include <cmath>


#include "Item.hpp"
#include "Metrics.hpp"


using namespace std;




class Evaluator {
public:


	map<int, float> truth; // item:score; in this case score is a rating
	map<int, float> estim; // item:score


	multimap<float, int, std::greater<float>> Rtruth; // score:item in decreasing score
	multimap<float, int, std::greater<float>> Rrelev; // score:item in decreasing score; subset of Rtruth with scores greater than relevanceThresh
	multimap<float, int, std::greater<float>> Restim; // score:item in decreasing score


	int numRelevant;

	Metrics metrics;

	Evaluator();

	Evaluator(map<int, float> truth, map<int, float> estim, int numRelevant = -1);

	void computeRankingMetrics();

	void computeFairness(vector<Item> items, int numUsers, int numItems);

	void computeSatisfactionFunctions(vector<Item> items, int numUsers, int numItems);

	void computeKendallTau(int maxRank = -1);
	void computeKendallTauTopK(int topK = -1);
	float getKendallTauTopK(int topK = -1);

	void computeMetricsAtRank();

};





#endif /* SRC_EVALUATOR_HPP_ */
