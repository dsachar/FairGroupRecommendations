/*
 * GroupPredictor.cpp
 *
 *  Created on: Mar 31, 2015
 *      Author: dimitris
 */

#include "GroupPredictor.hpp"
#include "GroupRatings.hpp"
#include "UserRatings.hpp"

#include <cmath>
#include <fstream>

#include <boost/math/special_functions/sign.hpp>

GroupPredictor::GroupPredictor(ExpSetting &setting) : expSet(setting) {
}


void GroupPredictor::resetPredictions() {
	predR.clear();
}


void GroupPredictor::readRatings(string groupRatingsFile, string userRatingsFile) {
	this->groupRatingsFile = groupRatingsFile;

	string evalFile = groupRatingsFile + ".test";
	GroupRatings evalRatings;
	evalRatings.readGroupRatings(evalFile);
	evalR = evalRatings.ratings;

	string trainFile = groupRatingsFile + ".train";
	GroupRatings trainRatings;
	trainRatings.readGroupRatings(trainFile);
	trainR = trainRatings.ratings;

	UserRatings userRatings;
	userRatings.readRatings(userRatingsFile);
	this->itemIDs = userRatings.itemIDs;
}


ExpMeasurement GroupPredictor::evaluate() {
	writeGroupRatingPredictions(groupRatingsFile + "_" + methodName() + ".pred");

	ExpMeasurement meas;

	// compute rmse
	int count = 0;
	double rmse = 0;
	double ae;
	double maxGroupRmse = - numeric_limits<double>::infinity();
	for (auto& groupItemPair : evalR){
		double groupRmse = 0;
		int numRatedItems = 0;
		int group = groupItemPair.first;
		for (auto& itemRatePair : groupItemPair.second) {
			int item = itemRatePair.first;
			numRatedItems++;
			float actual = itemRatePair.second;
			ae = predR[group][item] - actual;
			meas.aes.push_back(ae);
			rmse += (predR[group][item] - actual) * (predR[group][item] - actual);
			groupRmse += (predR[group][item] - actual) * (predR[group][item] - actual);
			count++;
		}
		groupRmse = sqrt(groupRmse/numRatedItems);
		meas.rmses.push_back(groupRmse);
//		cout << "group " << group << " RMSE " << groupRmse << endl;
		if (groupRmse > maxGroupRmse) {
			maxGroupRmse = groupRmse;
		}
	}
//	cout << "max group RMSE " << maxGroupRmse << endl;
	rmse = sqrt(rmse/count);

	meas.rmse_group_max = maxGroupRmse;
	meas.rmse_total = rmse;

	// compute Kendal Tau
	float avgKendalTau = 0;
	float minKendalTau = numeric_limits<float>::infinity();
	int numGroups = 0;
	for (auto& groupItemPair : evalR){
		int group = groupItemPair.first;
		numGroups++;
		multimap<float, int, std::greater<float>> rateItemActual; // rate:item
		multimap<float, int, std::greater<float>> rateItemPred; // rate:item
		map<int, int> itemRankActual; // item:rank
		map<int, int> itemRankPred; // item:rank
		for (auto& itemRatePair : groupItemPair.second) {
			int item = itemRatePair.first;
			float actual = itemRatePair.second;
			float pred = predR[group][item];
			rateItemActual.insert(make_pair(actual, item));
			rateItemPred.insert(make_pair(pred, item));
		}
		int rank = 1;
		for (auto& rateItem : rateItemActual) {
			int itemID = rateItem.second;
			itemRankActual.insert(make_pair(itemID, rank++));
		}
		rank = 1;
		for (auto& rateItem : rateItemPred) {
			int itemID = rateItem.second;
			itemRankPred.insert(make_pair(itemID, rank++));
		}
		float kendalTau = computeKendalTau(itemRankActual, itemRankPred, -1);
		if (kendalTau < minKendalTau) {
			minKendalTau = kendalTau;
		}
		avgKendalTau += kendalTau;
//		cout << "kendalTau for group " << group << " = " << kendalTau << endl;

	}
	avgKendalTau = avgKendalTau / numGroups;

//	cout << "average Kendal Tau " << avgKendalTau << endl;
//	cout << "minimum Kendal Tau " << minKendalTau << endl;

	meas.ktau_avg = avgKendalTau;
	meas.ktau_group_min = minKendalTau;

	computeRankingMetrics(meas);

	return meas;
}


float GroupPredictor::computeKendalTau(map<int, int> A, map<int, int> B, int maxRank) {
//	cout << "A.size() = " << A.size() << " B.size() = " << B.size() << endl;
	assert(A.size() == B.size());
	int N = A.size();

	if (maxRank != -1) {
		N = (N > maxRank) ? maxRank : N ;
	}

	int numer = 0;
	int denom = N * (N - 1) / 2;

	vector<int> vecA, vecB;
	for (auto& itemRankPair : A) {
		vecA.push_back(itemRankPair.second);
	}

	for (auto& itemRankPair : B) {
		vecB.push_back(itemRankPair.second);
	}

	for (int i=1; i<N; i++) {
		for (int j=0; j<i; j++) {
			numer += boost::math::sign(vecA[i] - vecA[j]) * boost::math::sign(vecB[i] - vecB[j]);
		}
	}

	return (float) numer/denom;
}

void GroupPredictor::writeGroupRatingPredictions(string predictionsFile) {
	ofstream gr_out;
	gr_out.open(predictionsFile.c_str(), ios::out);
	for (auto& groupItemPair : predR){
		int group = groupItemPair.first;
		for (auto& itemRatePair : groupItemPair.second) {
			int item = itemRatePair.first;
			float rate = itemRatePair.second;
			gr_out << group << " " << item << " " << rate << endl;
		}
	}
	gr_out.close();
}



void GroupPredictor::computeRankingMetrics(ExpMeasurement& meas) {


	float relevanceThresh = expSet.relevanceThreshFraction * expSet.maxRating;
	cout << "relevance threshold " << relevanceThresh << endl;

	int numListsEvaluated = 0;

	map<int, int> numPrecsAt;
	map<int, int> numRecsAt;
	map<int, int> numNDCGsAt;

	meas.MAP = 0; // initialize mean average precision

	for (auto& groupItemPair : evalR){
		int groupID = groupItemPair.first;

		// obtain ranking of relevant items
		multimap <float, int, std::greater<float>> rateItemRelevant;
		for (auto& itemRating : groupItemPair.second) {
			int itemID = itemRating.first;
			float rating = itemRating.second;
			if (rating >= relevanceThresh) { // an item is relevant if it has high rating
				rateItemRelevant.insert(make_pair(rating, itemID));
			}
		}
//		cout << "relevant items " << rateItemRelevant.size() << endl;

		// construct predicted ranking of all items in evaluation
		multimap <float, int, std::greater<float>> rateItemPredicted;

//		cout << "predR[groupID].size() = " << predR[groupID].size() << endl;
//		cout << "rateItemPredicted.size() = " << rateItemPredicted.size() << endl;

		for (auto& itemRating : predR[groupID]) {
			rateItemPredicted.insert(make_pair(itemRating.second, itemRating.first));
		}
//		cout << "evaluated items " << rateItemPredicted.size() << endl;
//		for (auto& rateItem : rateItemPredicted) {
//			cout << "item " << rateItem.second << " rating " << rateItem.first << endl;
//		}

		tuple<map<int, float>, map<int, float>, map<int, float>, float, int> metrics;

		if (rateItemRelevant.size() == 0) continue; // skip groups with no relevant items

		metrics = computePrecisionAndNDCG(rateItemRelevant, rateItemPredicted);

		map<int, float> precisionsAtN = get<0>(metrics);
		map<int, float> recallsAtN = get<1>(metrics);
		map<int, float> NDCGAtN = get<2>(metrics);
		float AP = get<3>(metrics);
		int posRelevant = get<4>(metrics);


		if (meas.RelevantAtPos.find(posRelevant) == meas.RelevantAtPos.end()) {
			meas.RelevantAtPos[posRelevant] = 1;
		}
		else {
			meas.RelevantAtPos[posRelevant] += 1;
		}
		meas.posRelevants.push_back(posRelevant);

		if (posRelevant <= meas.numPos) {
			meas.RelevantAtPosArray[posRelevant-1]++;
		}

		meas.MAP += AP;


		numListsEvaluated++;


//		cout << "precision @N : ";
		for (auto& tmp : precisionsAtN) {
			int N = tmp.first;
			float precAt = tmp.second;

			if (meas.precAt_avg.count(N)==0) {
				numPrecsAt[N] = 0;
				meas.precAt_avg[N] = 0;
			}
			meas.precAt_avg[N] += precAt;
			numPrecsAt[N]++;

			if (meas.precAt_min.count(N)==0 || precAt < meas.precAt_min[N]) {
				meas.precAt_min[N] = precAt;
			}

//			cout << precisionsAtN[N] << " @" << N << " ";
		}
//		cout << endl;

//		cout << "recall @N : ";
		for (auto& tmp : recallsAtN) {
			int N = tmp.first;
			float recAt = tmp.second;

			if (meas.recAt_avg.count(N)==0) {
				numRecsAt[N] = 0;
				meas.recAt_avg[N] = 0;
			}
			meas.recAt_avg[N] += recAt;
			numRecsAt[N]++;

			if (meas.recAt_min.count(N)==0 || recAt < meas.recAt_min[N]) {
				meas.recAt_min[N] = recAt;
			}

//			cout << recallsAtN[N] << " @" << N << " ";
		}
//		cout << endl;

//		cout << "NDCG @N : ";
		for (auto& tmp : NDCGAtN) {
			int N = tmp.first;
			float NDCGAt = tmp.second;

			if (meas.NDCGAt_avg.count(N)==0) {
				numNDCGsAt[N] = 0;
				meas.NDCGAt_avg[N] = 0;
			}
			meas.NDCGAt_avg[N] += NDCGAt;
			numNDCGsAt[N]++;

			if (meas.NDCGAt_min.count(N)==0 || NDCGAt < meas.NDCGAt_min[N]) {
				meas.NDCGAt_min[N] = NDCGAt;
			}

//			cout << NDCGAtN[N] << " @" << N << " ";
		}
//		cout << endl;

	}

	meas.MAP = meas.MAP / evalR.size();

//	cout << "avg precision @N : ";
	for (auto& tmp : meas.precAt_avg) {
		int N = tmp.first;
		float precAt = tmp.second;
		meas.precAt_avg[N] = precAt / numPrecsAt[N];
//		cout << meas.precAt_avg[N] << " @" << N << " ";
	}
//	cout << endl;

//	cout << "avg recall @N : ";
	for (auto& tmp : meas.recAt_avg) {
		int N = tmp.first;
		float recAt = tmp.second;
		meas.recAt_avg[N] = recAt / numRecsAt[N];
//		cout << meas.recAt_avg[N] << " @" << N << " ";
	}
//	cout << endl;

//	cout << "avg NDCG @N : ";
	for (auto& tmp : meas.NDCGAt_avg) {
		int N = tmp.first;
		float NDCGAt = tmp.second;
		meas.NDCGAt_avg[N] = NDCGAt / numNDCGsAt[N];
//		cout << meas.NDCGAt_avg[N] << " @" << N << " ";
	}
//	cout << endl;

}


tuple<map<int, float>, map<int, float>, map<int, float>, float, int> GroupPredictor::computePrecisionAndNDCG(multimap <float, int, std::greater<float>>& rateItemRelevant, multimap <float, int, std::greater<float>>& rateItemPredicted) {
	map<int, float> precisionsAtN;
	map<int, float> recallsAtN;
	map<int, float> NDCGAtN; // normalized discounted cumulative gain @ N

	map<int, float> IDCGAtN; // ideal discounted cumulative gain @ N
	int maxN;
	float maxIDCG;

	// construct itemRateRelevant and compute IDCG@N
	map<int, float> itemRateRelevant;
	int N = 1;
	float IDCG = 0;
	for (auto& rateItemPair : rateItemRelevant) {
		int itemID = rateItemPair.second;
		float rating = rateItemPair.first;
		itemRateRelevant[itemID] = rating;
		if (N == 1) {
			IDCG += rating;
		}
		else {
			IDCG += rating / (log(N)/log(2));
		}
		if (N % 5 == 0) {
			IDCGAtN[N] = IDCG;
		}
		N++;
	}
	maxN = N - 1;
	maxIDCG = IDCG;
//	cout << "maxN = " << maxN << " maxIDCG = " << maxIDCG << endl;


	float AP = 0; // average precision

	int posRelevant = 0; // position of first relevant item

	int numRelevantAtN = 0;
	float DCG = 0;
	N = 1;
	for (auto& rateItemPair : rateItemPredicted) {
		int itemID = rateItemPair.second;
		if (itemRateRelevant.count(itemID)) { // found relevant item
			if (posRelevant == 0) { // first relevant
				posRelevant = N;
			}
			numRelevantAtN++;
			AP += numRelevantAtN / N;
			if (N == 1) {
				DCG += itemRateRelevant[itemID];
			}
			else {
				DCG += itemRateRelevant[itemID] / (log(N)/log(2));
			}
		}
		if (N % 1 == 0) { // was: N % 5 == 0
			precisionsAtN[N] = (float) numRelevantAtN / N;
			recallsAtN[N] = numRelevantAtN;
//			cout << "precisions@" << N << "=" << precisionsAtN[N] << endl;
			if (N >= maxN) {
//				cout << "DCG=" << DCG << " maxIDCG=" << maxIDCG << endl;
				NDCGAtN[N] = DCG / maxIDCG;
			}
			else {
//				cout << "DCG=" << DCG << " IDCGAtN=" << IDCGAtN[N] << endl;
				NDCGAtN[N] = DCG / IDCGAtN[N];
			}
		}
		N++;
	}

	AP = AP / rateItemRelevant.size();


	return make_tuple(precisionsAtN, recallsAtN, NDCGAtN, AP, posRelevant);
}

