/*
 * GroupPredictor.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: dimitris
 */

#ifndef SRC_GROUPPREDICTOR_HPP_
#define SRC_GROUPPREDICTOR_HPP_

#include "ExpSetting.hpp"
#include "ExpMeasurement.hpp"


#include <iostream>
#include <map>
#include <tuple>



using namespace std;



class GroupPredictor {
public:

	ExpSetting& expSet;

	map<int,map<int,float>> predR; // group:item:rate
	map<int,map<int,float>> evalR; // group:item:rate
	map<int,map<int,float>> trainR; // group:item:rate

	set<int> itemIDs;


	string groupRatingsFile;

	GroupPredictor(ExpSetting& expSet);

	void readRatings(string groupRatingsFile, string userRatingsFile);

	void resetPredictions();

	ExpMeasurement evaluate();

	void writeGroupRatingPredictions(string predictionsFile);

	virtual void predictRatings(string ratingsFile, string groupsFile, string groupRatingsFile) {};
	virtual string const methodName() { return "GroupPredictor"; }

	float computeKendalTau(map<int, int> A, map<int, int> B, int maxRank=-1);

	void computeRankingMetrics(ExpMeasurement& meas);
	tuple<map<int, float>, map<int, float>, map<int, float>, float, int> computePrecisionAndNDCG(multimap <float, int, std::greater<float>>& rankedRelevantItems, multimap <float, int, std::greater<float>>& rankedPredictedItems); // returns N:precision@N and N:NDCG@N for every N multiple of 5

};



#endif /* SRC_GROUPPREDICTOR_HPP_ */
