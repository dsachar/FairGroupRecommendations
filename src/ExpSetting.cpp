/*
 * ExpSetting.cpp
 *
 *  Created on: Apr 15, 2015
 *      Author: dimitris
 */


#include <fstream>

#include "ExpSetting.hpp"


ExpSetting::ExpSetting() {
	numMethods = 0;
}


void ExpSetting::setResultsFile(string resultsFile){
	this->resultsFile = resultsFile;
}

void ExpSetting::setBaseDir(string baseDir) {
	this->baseDir = baseDir;
	setResultsFile(baseDir + "/results.res");
}


void ExpSetting::setInputParameters(Groups& groups, GroupRatings& groupRatings) {
	numGroups = groups.numGroups;
	groupSize = groups.groupSize;
	numDistinctUsersInGroups = groups.numDistinctUsersInGroups;
	userBehType = groups.userBehType;
	groupsFileName = groups.groupsFile;

	groupRatingsTrainPrc = groupRatings.trainPercentage;
	numItemsRatedPerGroup = groupRatings.numItemsRatedPerGroup;
	groupBehType = groupRatings.groupBehType;
	groupRatingsFileName = groupRatings.groupRatingsFile;
}


void ExpSetting::addMethod(string methodName) {
	methodNameIDs[methodName] = numMethods++;
}


void ExpSetting::addMeasurement(int repetition, string methodName, ExpMeasurement& meas) {
	int methodID = methodNameIDs[methodName];
	measurements[repetition][methodID] = meas;
}


void ExpSetting::writeSetting() {
	ofstream rout;
	rout.open(resultsFile.c_str(), ios::app);

	rout << inputFile << " " << groupsFileName << " " << groupRatingsFileName << endl;


	rout << "numGroups\t groupSize\t numDistinctUsersInGroups\t userBehType" << endl;
	rout << numGroups << "\t" << groupSize << "\t" << numDistinctUsersInGroups << "\t" << UserBehTypeNames[userBehType] << endl;

	rout << "groupRatingsTrainPrc\t numItemsRatedPerGroup\t groupBehType\t roundGroupRatings" << endl;
	rout << groupRatingsTrainPrc << "\t" << numItemsRatedPerGroup << "\t" << GroupAggTypeNames[groupBehType] << "\t" << (roundGroupRatings? "true" : "false") << endl;

	rout << "relevanceThreshFraction" << endl;
	rout << relevanceThreshFraction << endl;

	rout.close();


	string resultsAtNFile = resultsFile + "_AtN";
	rout.open(resultsAtNFile.c_str(), ios::app);

	rout << inputFile << endl;

	rout << "numGroups\t groupSize\t numDistinctUsersInGroups\t userBehType" << endl;
	rout << numGroups << "\t" << groupSize << "\t" << numDistinctUsersInGroups << "\t" << UserBehTypeNames[userBehType] << endl;

	rout << "groupRatingsTrainPrc\t numItemsRatedPerGroup\t groupBehType\t roundGroupRatings" << endl;
	rout << groupRatingsTrainPrc << "\t" << numItemsRatedPerGroup << "\t" << GroupAggTypeNames[groupBehType] << "\t" << (roundGroupRatings? "true" : "false") << endl;

	rout << "relevanceThreshFraction" << endl;
	rout << relevanceThreshFraction << endl;

	rout.close();
}


void ExpSetting::writeHeader() {
	ofstream rout;
	rout.open(resultsFile.c_str(), ios::app);

	for (int i=0; i<methodNameIDs.size(); i++) {
		string methodName;
		for (auto& methodNameID : methodNameIDs) {
			if (methodNameID.second == i) {
				methodName = methodNameID.first;
				break;
			}
		}
		rout << methodName << ExpMeasurement::getMetricTabsString();
	}
	rout << endl;

	for (int i=0; i<methodNameIDs.size(); i++) {
		rout << ExpMeasurement::getMetricString() << "\t";
	}
	rout << endl;
	rout.close();



	string resultsAtNFile = resultsFile + "_AtN";
	rout.open(resultsAtNFile.c_str(), ios::app);

	for (int i=0; i<methodNameIDs.size(); i++) {
		string methodName;
		for (auto& methodNameID : methodNameIDs) {
			if (methodNameID.second == i) {
				methodName = methodNameID.first;
				break;
			}
		}
		rout << methodName << ExpMeasurement::getMetricAtNTabsString();
	}
	rout << endl;

	for (int i=0; i<methodNameIDs.size(); i++) {
		rout << ExpMeasurement::getMetricAtNString() << "\t";
	}
	rout << endl;
	rout.close();


	string resultsRelAtNFile = resultsFile + "_RelAtN";
	rout.open(resultsRelAtNFile.c_str(), ios::app);

	for (int i=0; i<methodNameIDs.size(); i++) {
		string methodName;
		for (auto& methodNameID : methodNameIDs) {
			if (methodNameID.second == i) {
				methodName = methodNameID.first;
				break;
			}
		}
		rout << methodName << ExpMeasurement::getMetricAtNTabsString();
	}
	rout << endl;
	rout.close();

}


void ExpSetting::writeRepetition(int repetition) {
	ofstream rout;
	rout.open(resultsFile.c_str(), ios::app);

	for (int i=0; i<methodNameIDs.size(); i++) {
		rout << measurements[repetition][i].getMeasurementString() << "\t";
	}
	rout << endl;
	rout.close();


	string resultsRawFile = resultsFile + "_raw";
	rout.open(resultsRawFile.c_str(), ios::app);
	for (int g=0; g<measurements[repetition][0].aes.size(); g++) {

		for (int i=0; i<methodNameIDs.size(); i++) {
			rout << measurements[repetition][i].aes[g] << "\t";
		}
		rout << endl;

	}
	rout << endl;
	rout.close();


	string resultsAtNFile = resultsFile + "_AtN";
	rout.open(resultsAtNFile.c_str(), ios::app);

	for (int i=0; i<methodNameIDs.size(); i++) {
		rout << measurements[repetition][i].getMeasurementAtNString() << "\t";
	}
	rout << endl;
	rout.close();

	string resultsRelAtNFile = resultsFile + "_RelAtN";
	rout.open(resultsRelAtNFile.c_str(), ios::app);

	for (int i=0; i<methodNameIDs.size(); i++) {
		rout << measurements[repetition][i].getMeasurementRelAtNString() << "\t";
	}
	rout << endl;
	rout.close();
}

