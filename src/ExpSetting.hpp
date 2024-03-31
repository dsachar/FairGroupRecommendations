/*
 * ExpSetting.hpp
 *
 *  Created on: Apr 14, 2015
 *      Author: dimitris
 */

#ifndef SRC_EXPSETTING_HPP_
#define SRC_EXPSETTING_HPP_

#include "ExpMeasurement.hpp"
#include "Groups.hpp"
#include "GroupRatings.hpp"

#include <vector>
#include <map>

using namespace std;

class ExpSetting {
public:

	///////////// parameters

	/// output directory
	string baseDir;

	/// group params
	int numGroups;
	int groupSize;
	int numDistinctUsersInGroups;
	int userBehType;
	string groupsFileName;

	/// group ratings params
	int groupRatingsTrainPrc;
	int numItemsRatedPerGroup;
	int groupBehType;
	bool roundGroupRatings;
	string groupRatingsFileName;


	/// dataset params
	string inputFile;
	float maxRating;
	float relevanceThreshFraction; // items with rating above relevanceThreshFraction*maxRating are considered relevant
	int numEpochs;

	/// methods
	int numMethods;
	map<string, int> methodNameIDs; // methodName : encodedID (used in measurements maps)


	///////////// measurements
	map<int, map<int, ExpMeasurement>> measurements; // repetition:method:measurement


	///////////// result files
	string resultsFile;

	ExpSetting();

	void setBaseDir(string baseDir);
	void setResultsFile(string resultsFile);
	void setInputParameters(Groups& groups, GroupRatings& groupRatings);
	void addMethod(string methodName);

	void addMeasurement(int repetition, string methodName, ExpMeasurement& meas);


	void writeSetting();
	void writeHeader();
	void writeRepetition(int repetition);


};



#endif /* SRC_EXPSETTING_HPP_ */
