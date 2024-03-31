/*
 * POEvaluation.hpp
 *
 *  Created on: Oct 6, 2017
 *      Author: dimitris
 */

#ifndef SRC_POEVALUATION_HPP_
#define SRC_POEVALUATION_HPP_


#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <random>
using namespace std;

#include "UserRatings.hpp"
#include "ParetoOptimal.hpp"
#include "Evaluator.hpp"
#include "SinglePOEvaluation.hpp"
#include "AggMetrics.hpp"


class POEvaluation {
public:

	ParetoOptimal po;
	UserRatings& ur;

	string resultFile;

	int numUsers; // num of users in groups
	int numItems; // num of items for evaluation
	int numRelevant; // num of items considered relevant
	int numRepetitions; // num of groups to create

	int groupType; // 0 is random groups, 1 is low similarity groups, 2 is high similarity groups

	int numMethods = 7; // AVG, MIN, MAX, PRD, CTK(ours), BORDA, MED-RANK

	int numCriteria; // AVG, MIN, MAX, PRD + one per user


	int topK; // topK num for CTK; typically topK = numRelevant

	vector<string> criteriaNames;
	vector<string> methodNames;

	vector<AggMetrics> methodAggMetrics; // AggMetrics for each method


	POEvaluation(UserRatings& ratings);

	void doRepetitions();
	void doRepetitions_NEW();
	void doRepsFromFile();
	void doEvaluation();

	string getSettingString();

	void printRecommendations(string methodName, map<int, float> scores);

};



#endif /* SRC_POEVALUATION_HPP_ */
