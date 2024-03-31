/*
 * GroupRatings.cpp
 *
 *  Created on: Mar 20, 2015
 *      Author: dimitris
 */

#include "GroupRatings.hpp"
#include "UserRatings.hpp"
#include "Groups.hpp"
#include "MatrixFactorization.hpp"

#include <fstream>
#include <sstream>
#include <cfloat>
#include <random>

#include <cstdio>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


GroupRatings::GroupRatings() {

}

void GroupRatings::readFrom(){

}

void GroupRatings::createSynth(Groups &groups, UserRatings& userRatings, int numItemsRatedPerGroup, int groupBehType, int trainPercentage, bool roundGroupRatings, int maxNumGroupRatings) {
	this->groups = groups;
	this->numItemsRatedPerGroup = numItemsRatedPerGroup;
	this->groupBehType = groupBehType;
	this->groupsFile = groups.groupsFile;
	this->trainPercentage = trainPercentage;
	this->roundGroupRatings = roundGroupRatings;

	setGroupRatingsFile();
	createGroupRatingsForRandomItems(userRatings, maxNumGroupRatings);
	splitAndWriteGroupRatings();
}



void GroupRatings::createMultipleSynth(Groups &groups, UserRatings& userRatings, int numItemsRatedPerGroup, int groupBehType, int trainPercentage, bool roundGroupRatings, int maxNumGroupRatings) {
	this->groups = groups;
	this->numItemsRatedPerGroup = numItemsRatedPerGroup;
	this->groupBehType = groupBehType;
	this->groupsFile = groups.groupsFile;
	this->trainPercentage = trainPercentage;
	this->roundGroupRatings = roundGroupRatings;

	setGroupRatingsFile();
	createGroupRatingsForRandomItems(userRatings, maxNumGroupRatings);
	splitAndWriteMultipleGroupRatings();
}


void GroupRatings::setGroupRatingsFile() {
	string groupsFileWOExtension = fs::path(groupsFile).parent_path().string() + "/" + fs::path(groupsFile).stem().string();
	cout << "groupsFile: " << groupsFile << endl;
	cout << "groupsFileWOExtension: " << groupsFileWOExtension << endl;
	groupRatingsFile = groupsFileWOExtension + "_GB" + to_string(groupBehType) + "_I" + to_string(numItemsRatedPerGroup) + "_r" + to_string(roundGroupRatings) + "_p" + to_string(trainPercentage);
	cout << "groupRatingsFile: " << groupRatingsFile << endl;
}

void GroupRatings::createGroupRatingsForCommonItems(UserRatings& userRatings, int maxNumGroupRatings) {
	numGroupRatings = 0;

	for (auto& groupUserPair : groups.groupUserWeights){ // for each group
		if (numGroupRatings > maxNumGroupRatings) break;
		int groupID = groupUserPair.first;
//		cout << "group " << groupUserPair.first << endl;
		map<int,float> userWeights = groupUserPair.second;
		set<int> commonRatedItems;
		bool firstUser = true;
		for (auto userWeightPair : userWeights){ // for each user in group
			int user = userWeightPair.first;
//			cout << "user " << user << " has rated items: ";
			set<int> userRatedItems;
			for (auto& ratedItemPair : userRatings.ratings[user]) {
				int ratedItem = ratedItemPair.first;
				userRatedItems.insert(ratedItem);
//				cout << ratedItem << " ";
			}
//			cout << endl;
			if (firstUser) {
				firstUser = false;
				commonRatedItems = userRatedItems;
			}
			else {
				set<int> intersection;
				set_intersection(commonRatedItems.begin(), commonRatedItems.end(), userRatedItems.begin(), userRatedItems.end(), std::inserter(intersection,intersection.begin()));
				commonRatedItems = intersection;
			}

		}
		if (commonRatedItems.size() == 0){
//			cout << "no commonly rated items in this group" << endl;
			continue;
		}
//		cout << "commonRatedItems: ";
//		for (int commonRatedItem : commonRatedItems){
//			cout << commonRatedItem << " ";
//		}
//		cout << endl;

		for (int commonRatedItem : commonRatedItems){  // for each common rated item by the group
			if (numGroupRatings > maxNumGroupRatings) break;

			float minRating = numeric_limits<float>::infinity();
			float sumWeightedRatings = 0;
			float sumWeights = 0;
			float avgRating = 0;

			for (auto userWeightPair : userWeights){
				int userID = userWeightPair.first;
				float weight = groups.groupUserWeights[groupID][userID];
				if (groupBehType == AVG) {
					weight = 1;
				}

				sumWeights += weight;
				sumWeightedRatings += weight * userRatings.ratings[userID][commonRatedItem];
				avgRating += userRatings.ratings[userID][commonRatedItem];

				if (minRating > userRatings.ratings[userID][commonRatedItem]) {
					minRating = userRatings.ratings[userID][commonRatedItem];
				}

			}

			avgRating /= userWeights.size();

			float groupRating;
			if (sumWeights == 0) {
				groupRating = 0;
			}
			else {
				groupRating = sumWeightedRatings / sumWeights;
			}

			if (groupBehType == MIN) {
				groupRating = (minRating < numeric_limits<float>::infinity()) ? minRating : 0;
			}

//			cout << "groupRating " << groupRating << " averageRating " << avgRating << endl;
			ratings[groupID][commonRatedItem] = groupRating;
			numGroupRatings++;
		}


	}

	cout << "created " << numGroupRatings << " group ratings" << endl;

}


void GroupRatings::createGroupRatingsForRandomItems(UserRatings& userRatings, int maxNumGroupRatings) {
	numGroupRatings = 0;


	// factorize userRatings matrix to make predictions when necessary
	MatrixFactorization mf_u;
	mf_u.setInputData(userRatings.ratings, userRatings.userIDs, userRatings.itemIDs, userRatings.ratingsFile);
	mf_u.SGD_train(FACTORS, 0.005, 0.001, 20); // FIXME: change epochs to 20


	random_device rd;
	mt19937 mt_rand(rd());
	if (globalRandomSeed != -1) {
		mt_rand.seed(globalRandomSeed);
	}


	for (auto& groupUserPair : groups.groupUserWeights){ // for each group
		if (numGroupRatings > maxNumGroupRatings) break;
		int groupID = groupUserPair.first;

//		cout << "groupID " << groupID << endl;


		// determine the weights for users
		bool kingChosen = false;
		int usersLeft = groupUserPair.second.size();
		map<int, float> weights; // userID, weight

		for (auto userWeightPair : groupUserPair.second){ // for each user in group
			int userID = userWeightPair.first;
			float weight = userWeightPair.second;
			if (groupBehType == AVG) {
				weight = 1;
			}
			if (groupBehType == ABS) {
				if (!kingChosen) {
					// nominate current user for king
					uniform_int_distribution<> unif(0, groupUserPair.second.size()-1);

//					if (! ( rand() % groupUserPair.second.size() ) ) {
					if ( unif(mt_rand) == 0 ) {
						kingChosen = true;
						weight = 1; // king's opinion matters
					}
					else {
						weight = 0;
					}
				}
				else {
					weight = 0;
				}
				if (!kingChosen && usersLeft==1) {
					weight = 1;
				}
			}

			weights[userID] = weight;
//			cout << "user " << userID << " in group " << groupID << " has weight " << weight << endl;
			usersLeft--;
		}


		for (int i=0; i<numItemsRatedPerGroup; i++) {
			// choose random item
			uniform_int_distribution<> unif(0, userRatings.itemIDs.size()-1);
			int randOffset = unif(mt_rand);

			set<int>::const_iterator iter(userRatings.itemIDs.begin());
			advance(iter, randOffset);
			int itemID = *iter;

//			cout << "itemID " << itemID << endl;

			float groupRating = 0;
			float sumWeights = 0;
			float minRating = numeric_limits<float>::infinity();
			float maxRating = - numeric_limits<float>::infinity();

			map<int,float> userWeights = groupUserPair.second;

			for (auto userWeightPair : userWeights){ // for each user in group
				int userID = userWeightPair.first;

				float weight = weights[userID];

//				cout << "userID " << userID << endl;

				float rating;
				if (userRatings.ratings[userID].count(itemID)) { // get actual
					rating = userRatings.ratings[userID][itemID];
				}
				else { // predict
					rating = mf_u.getEstimate(userID, itemID);
				}
//				cout << "rating = " << rating << endl;
				if (rating < minRating) {
					minRating = rating;
				}
				if (rating > maxRating) {
					maxRating = rating;
				}
				groupRating += weight * rating;
				sumWeights += weight;
			}


			if (sumWeights == 0) {
				groupRating = 0;
			}
			else {
				groupRating /= sumWeights;
			}

			if (groupBehType == MIN) {
				groupRating = (minRating < numeric_limits<float>::infinity()) ? minRating : 0;
			}
			if (groupBehType == MAX) {
				cout << "minRating: " << minRating << " maxRating: " << maxRating << " avgRating: " << groupRating << endl;
				groupRating = (maxRating > - numeric_limits<float>::infinity()) ? maxRating : 5;
			}


			if (roundGroupRatings) {
				ratings[groupID][itemID] = round(groupRating); // round value
			}
			else {
				ratings[groupID][itemID] = groupRating; // don't truncate/round
			}
			numGroupRatings++;

		}

	}
}



void GroupRatings::splitAndWriteGroupRatings() {

	int numRatingsForTrain = numGroupRatings * trainPercentage / 100;


	string groupRatingsFileTrain = groupRatingsFile + ".train";
	string groupRatingsFileTest = groupRatingsFile + ".test";

//	cout << "groupRatingsFileTrain: " << groupRatingsFileTrain << endl;
//	cout << "groupRatingsFileTest: " << groupRatingsFileTest << endl;

	ofstream train;
	train.open(groupRatingsFileTrain.c_str(), ios::out);
	ofstream test;
	test.open(groupRatingsFileTest.c_str(), ios::out);

	// groupRatingsTrainPrc : numItemsRatedPerGroup : groupBehType : groupsFile
//	train << trainPercentage <<  " " << numItemsRatedPerGroup << " " << groupBehType << " " << groupsFile << endl;
//	test << trainPercentage <<  " " << numItemsRatedPerGroup << " " << groupBehType << " " << groupsFile << endl;


	random_device rd;
	mt19937 mt_rand(rd());
	if (globalRandomSeed != -1) {
		mt_rand.seed(globalRandomSeed);
	}


	int left = numRatingsForTrain;

	for (auto& groupItemPair : ratings){
		int group = groupItemPair.first;
		for (auto& itemRatePair : groupItemPair.second) {
			int item = itemRatePair.first;
			float rate = itemRatePair.second;

			if (left > 0) {

				uniform_int_distribution<> unif(0, 100);

//				int flip = (int)(100.0 * rand() / (RAND_MAX + 1.0)) + 1;
				int flip = unif(mt_rand);
//				cout << "flip: " << flip << endl;

				if (flip < trainPercentage) {
					train << group << " " << item << " " << rate << endl;
					left--;
				}
				else {
					test << group << " " << item << " " << rate << endl;
				}

			}
			else {
				test << group << " " << item << " " << rate << endl;
			}


		}
	}

	train.close();
	test.close();

}



void GroupRatings::splitAndWriteMultipleGroupRatings() {



	int numRatingsAvailableForTrain = numGroupRatings * trainPercentage / 100;
	int numRatingsInTest = numGroupRatings - numRatingsAvailableForTrain;

	int numOfSplits = 10; // hard-coded


	int numGroups = groups.numGroups;
	int numGroupsInTrain = numGroups * trainPercentage / 100;
	int numGroupsInTest = numGroups - numGroupsInTrain;


	// create a train/test for each split

	for (int split=0; split <= numOfSplits; split++) {
		int numRatingsInTrain = split * numRatingsAvailableForTrain / numOfSplits;

		string groupRatingsFileTrain = groupRatingsFile + "_SPL" + to_string(split) + ".train";
		string groupRatingsFileTest = groupRatingsFile + "_SPL" + to_string(split) + ".test";
		//	cout << "groupRatingsFileTrain: " << groupRatingsFileTrain << endl;
		//	cout << "groupRatingsFileTest: " << groupRatingsFileTest << endl;

		ofstream train;
		train.open(groupRatingsFileTrain.c_str(), ios::out);
		ofstream test;
		test.open(groupRatingsFileTest.c_str(), ios::out);



		random_device rd;
		mt19937 mt_rand(rd());
		if (globalRandomSeed != -1) {
			mt_rand.seed(globalRandomSeed);
		}



		for (auto& groupItemPair : ratings){
			int group = groupItemPair.first;

			int numRatingsForTrain = groupItemPair.second.size() * trainPercentage / 100; // trainPercentage% for train
			int numRatingsInTrain = split * numRatingsForTrain / numOfSplits; // among numRatingsForTrain keep only split/numOfSplits of them
			int numRatingsInTest = groupItemPair.second.size() - numRatingsForTrain; // 100-trainPercentage % for test
//			cout << "numRatingsForTrain: " << numRatingsForTrain << endl;
//			cout << "numRatingsInTrain: " << numRatingsInTrain << endl;
//			cout << "numRatingsInTest: " << numRatingsInTest << endl;

//			continue;

			int trainLeft = numRatingsInTrain;
			int testLeft = numRatingsInTest;

			for (auto& itemRatePair : groupItemPair.second) {
				int item = itemRatePair.first;
				float rate = itemRatePair.second;

				if (group <= numGroupsInTrain) { // for all groups with ID less than numGroupsInTrain
					train << group << " " << item << " " << rate << endl;
				}
				else {
					// decide on whether it's train or test
					uniform_int_distribution<> unif(0, 100);
					int flip = unif(mt_rand);
					if (flip < trainPercentage) { // for train
						// decide on whether to include in train

						uniform_int_distribution<> unif(0, numOfSplits);
						int flip = unif(mt_rand);

						if (flip < split) {
							train << group << " " << item << " " << rate << endl;
							trainLeft--;
						}

					}
					else { // for test
						if (testLeft > 0) {
							test << group << " " << item << " " << rate << endl;
							testLeft--;
						}
					}
				}

			}
		}

		train.close();
		test.close();
	}
}



//void GroupRatings::writeGroupRatings() {
//	ofstream gr_out;
//	gr_out.open(groupRatingsFile.c_str(), ios::out);
//	// groupRatingsTrainPrc : numItemsRatedPerGroup : groupBehType : groupsFile
//	gr_out << groupRatingsTrainPrc <<  " " << numItemsRatedPerGroup << " " << groupBehType << " " << groupsFile << endl;
//	for (auto& groupItemPair : ratings){
//		int group = groupItemPair.first;
//		for (auto& itemRatePair : groupItemPair.second) {
//			int item = itemRatePair.first;
//			float rate = itemRatePair.second;
//			gr_out << group << " " << item << " " << rate << endl;
//		}
//	}
//	gr_out.close();
//}




void GroupRatings::readGroupRatings(string fileName){
	groupRatingsFile = fs::path(fileName).parent_path().string() + "/" + fs::path(fileName).stem().string();
//	cout << "groupRatingsFile: " << groupRatingsFile << endl;

	ifstream rin(fileName.c_str());
	string line;

//	getline(rin, line);
//    istringstream iss(line);
	// groupRatingsTrainPrc : numItemsRatedPerGroup : groupBehType : groupsFile
//	iss >> trainPercentage >> numItemsRatedPerGroup >> groupBehType >> groupsFile;
//	cout << "read groupsFile " << groupsFile << endl;

	set<int> items;
	numGroupRatings = 0;
	while (getline(rin, line)) {
	    istringstream iss(line);
	    int group, item;
	    float rate;
		if ( !(iss >> group) ) {
			break;
		}
		if ( !(iss >> item) ) {
			break;
		}
		items.insert(item);
		if ( !(iss >> rate) ) {
			break;
		}
//		cout << "group: " << group << " item: " << item << " rating: " << rate << endl;
		ratings[group][item] = rate;
		numGroupRatings++;
	}
	rin.close();
//	cout << "read " << numGroupRatings << " group ratings from " << fileName << endl;
}
