/*
 * TriadModel.cpp
 *
 *  Created on: Apr 1, 2015
 *      Author: dimitris
 */


#include "TriadModel.hpp"

#include <random>



void TriadModel::setInputData(Groups& groups, UserRatings& userRatings, GroupRatings& groupRatings) {
	userIDs = userRatings.userIDs;
	itemIDs = userRatings.itemIDs;

	// get user ratings
	for (auto& leftRightPair : userRatings.ratings) {
		int userID = leftRightPair.first;
		for (auto& rightRatingPair : leftRightPair.second) {
			int itemID = rightRatingPair.first;
			float rate = rightRatingPair.second;
			MatrixEntry r(userID, itemID, rate);
			this->userRatingsList.push_back(r);
		}
	}

	// get group ratings
	for (auto& leftRightPair : groupRatings.ratings) {
		int groupID = leftRightPair.first;
		for (auto& rightRatingPair : leftRightPair.second) {
			int itemID = rightRatingPair.first;
			float rate = rightRatingPair.second;
			MatrixEntry r(groupID, itemID, rate);
			this->groupRatingsList.push_back(r);
		}
	}

	// get group info
	for (auto& groupUserPair : groups.groupUserWeights) {
		int groupID = groupUserPair.first;
		set<int> userIDs;
		for (auto& userWeightPair : groupUserPair.second) {
			int userID = userWeightPair.first;
			userIDs.insert(userID);
		}
		groupUserIDs[groupID] = userIDs;
	}

}



void TriadModel::SGD_train(int K, double eta, double lambda, int epochs) {

	this->K = K;
	initializeModel(1.0);

	int epoch = 1;


	random_device rd;
	mt19937 mt_rand(rd());
	if (globalRandomSeed != -1) {
		mt_rand.seed(globalRandomSeed);
	}



	float coinProb = (float) userRatingsList.size() / (userRatingsList.size() + groupRatingsList.size());
//	cout << "coinProb=" << coinProb << endl;
	bernoulli_distribution distrib(coinProb);

	while(epoch <= epochs) {
		cerr << "\repoch " << epoch;
		shuffle(userRatingsList.begin(), userRatingsList.end(), mt_rand);
		shuffle(groupRatingsList.begin(), groupRatingsList.end(), mt_rand);

		vector<MatrixEntry>::iterator ur_iter = userRatingsList.begin();
		vector<MatrixEntry>::iterator gr_iter = groupRatingsList.begin();

		while (ur_iter != userRatingsList.end() || gr_iter != groupRatingsList.end()) {

			if (distrib(mt_rand) && ur_iter != userRatingsList.end()) { // take a sample from user ratings
				MatrixEntry ur = *ur_iter;
				ur_iter++;
				int userID = get<0>(ur);
				int itemID = get<1>(ur);
				float rate = get<2>(ur);

				double pred = getUserRatingEstimate(userID, itemID);
				double error_u = pred - rate; // IMPORTANT

				for (int f=0; f<K; f++) { // for each factor
					userMatrix[userID][f] -= 2 * eta * (error_u * itemMatrix[itemID][f] + lambda * userMatrix[userID][f]);
					itemMatrix[itemID][f] -= 2 * eta * (error_u * userMatrix[userID][f] + lambda * itemMatrix[itemID][f]);
				}

			}
			else if(gr_iter != groupRatingsList.end()) { // take a sample from group ratings
				MatrixEntry gr = *gr_iter;
				gr_iter++;
				int groupID = get<0>(gr);
				int itemID = get<1>(gr);
				float rate = get<2>(gr);
				int groupSize = groupUserIDs[groupID].size();

				double pred = getGroupRatingEstimate(groupID, itemID);
				double error_g = pred - rate; // IMPORTANT

				// update userMatrix
				for (int userID : groupUserIDs[groupID]) { // for each user in group
					for (int f=0; f<K; f++) { // for each factor
						userMatrix[userID][f] -= 2 * eta * (error_g * userBehavior[userID] * itemMatrix[itemID][f] / groupSize + lambda * userMatrix[userID][f]);
					}
				}

				// update itemMatrix
				for (int f=0; f<K; f++) { // for each factor
					float sum = 0;
					for (int userID : groupUserIDs[groupID]) { // for each user in group
						sum += userBehavior[userID] * userMatrix[userID][f];
					}
					itemMatrix[itemID][f] -= 2 * eta * (error_g * sum / groupSize  + lambda * itemMatrix[itemID][f]);
				}

				// update userBehavior
				for (int userID : groupUserIDs[groupID]) { // for each user in group
					float sum = 0;
					for (int f=0; f<K; f++) { // for each factor
						sum += userMatrix[userID][f] * itemMatrix[itemID][f];
					}
					userBehavior[userID] -= 2 * eta * (error_g * sum / groupSize +  lambda * userBehavior[userID]);
				}
			}
		}
		epoch++;
	}
	cerr << ": ";
	double obj = objectiveFunction(lambda);
	cerr << "objective=" << obj << " group ratings rmse=" << getGroupRatingsRMSE() << endl;

}


double TriadModel::objectiveFunction(double lambda) {
	return getUserRatingsSSE() + getGroupRatingsSSE() + lambda * (norm(userMatrix) + norm(itemMatrix) + norm(userBehavior) );
}


double TriadModel::getUserRatingsSSE() {
	double sse = 0;
	for (MatrixEntry r : userRatingsList) {
		int userID = get<0>(r);
		int itemID = get<1>(r);
		float rate = get<2>(r);

		double pred = getUserRatingEstimate(userID, itemID);
		double error = pred - rate;

		sse += error*error;
	}

	return sse;
}


double TriadModel::getUserRatingsRMSE() {
	return sqrt (getUserRatingsSSE() / userRatingsList.size());
}


double TriadModel::getGroupRatingsSSE() {
	double sse = 0;
	for (MatrixEntry r : groupRatingsList) {
		int groupID = get<0>(r);
		int itemID = get<1>(r);
		float rate = get<2>(r);

		double pred = getGroupRatingEstimate(groupID, itemID);
		double error = pred - rate;

		sse += error*error;
	}

	return sse;
}


double TriadModel::getGroupRatingsRMSE() {
	return sqrt (getGroupRatingsSSE() / groupRatingsList.size());
}


double TriadModel::getUserRatingEstimate(int userID, int itemID) {
	return innerProduct(userMatrix[userID], itemMatrix[itemID]);
}


double TriadModel::getGroupRatingEstimate(int groupID, int itemID) {
	double pred = 0;
	for (int userID : groupUserIDs[groupID]) {
		pred += userBehavior[userID] * getUserRatingEstimate(userID, itemID);
	}
	pred /= groupUserIDs[groupID].size();

	return pred;
}


void TriadModel::initializeModel(double val) {
	for (int userID : userIDs) {
		Colvec vec(K, val);
		userMatrix[userID] = vec;
		userBehavior[userID] = 0.5;
	}

	for (int itemID : itemIDs) {
		Colvec vec(K, val);
		itemMatrix[itemID] = vec;
	}

}
