/*
 * Evaluator.cpp
 *
 *  Created on: 20 Mar 2017
 *      Author: dimitris
 */


#include "Evaluator.hpp"
#include <boost/math/special_functions/sign.hpp>




Evaluator::Evaluator() {
}

Evaluator::Evaluator(map<int, float> truth, map<int, float> estim, int numRelevant) {
	this->truth = truth;
	this->estim = estim;

	if (numRelevant == -1) {
		numRelevant = truth.size();
	}
	this->numRelevant = numRelevant;



	// flip them
	for (auto itemScore : truth) {
		Rtruth.insert( make_pair(itemScore.second, itemScore.first) );
	}

	int countRelevant = 0;
	for (auto scoreItem : Rtruth) {
		if (++countRelevant > numRelevant) break;
		Rrelev.insert( make_pair(scoreItem.first, scoreItem.second) );
	}


	for (auto itemScore : estim) {
		Restim.insert( make_pair(itemScore.second, itemScore.first) );
	}
}


void Evaluator::computeRankingMetrics() {
	computeKendallTau();
	computeKendallTauTopK(numRelevant);
	computeMetricsAtRank();
}


void Evaluator::computeKendallTau(int maxRank) {
	metrics.KendalTau = getKendallTauTopK(-1);
	return;

	map<int, int> A; // item:rank
	map<int, int> B; // item:rank

	int rank = 1;
	for (auto scoreItem : Rtruth) {
		int itemID = scoreItem.second;
		A.insert( make_pair(itemID, rank++) );
	}

	rank = 1;
	for (auto scoreItem : Restim) {
		int itemID = scoreItem.second;
		B.insert( make_pair(itemID, rank++) );
	}


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

	metrics.KendalTau = (float) numer/denom;
}


void Evaluator::computeKendallTauTopK(int topK) {
	metrics.KendalTauTopK = getKendallTauTopK(topK);
}


float Evaluator::getKendallTauTopK(int topK) {
	map<int, int> A; // item:rank
	map<int, int> B; // item:rank


	set<int> itemsA;
	set<int> itemsB;

	set<int> inter; // intersection of itemsA and itemsB
	set<int> uni; // union of itemsA and itemsB

	int rank = 1;
	for (auto scoreItem : Rtruth) {
		int itemID = scoreItem.second;
		A.insert( make_pair(itemID, rank++) );
		itemsA.insert(itemID);
		uni.insert(itemID);
		if (topK!=-1 && rank > topK) break;
	}

	rank = 1;
	for (auto scoreItem : Restim) {
		int itemID = scoreItem.second;
		B.insert( make_pair(itemID, rank++) );
		itemsB.insert(itemID);
		uni.insert(itemID);
		if (topK!=-1 && rank > topK) break;
	}

	std::set_intersection(itemsA.begin(), itemsA.end(), itemsB.begin(), itemsB.end(), std::inserter(inter, inter.begin()));


	int N = uni.size();

//	cout << "truth.size()=" << truth.size() << " estim.size()=" << estim.size() << endl;
//	cout << "topK=" << topK << " uni.size()=" << uni.size() << " inter.size()=" << inter.size() << endl;

	int numer = 0;
	int denom = N * (N - 1) / 2;


	vector<int> vecA, vecB;
	for (auto& item : uni) {
		if (itemsA.find(item) != itemsA.end()) {
			vecA.push_back(A[item]);
		}
		else {
			vecA.push_back(topK+1);
		}

		if (itemsB.find(item) != itemsB.end()) {
			vecB.push_back(B[item]);
		}
		else {
			vecB.push_back(topK+1);
		}
	}


	for (int i=1; i<N; i++) {
		for (int j=0; j<i; j++) {
			numer += boost::math::sign(vecA[i] - vecA[j]) * boost::math::sign(vecB[i] - vecB[j]);
		}
	}

	return (float) numer/denom;
}



void Evaluator::computeMetricsAtRank() {
	map<int, float> IDCG; // ideal discounted cumulative gain @ N


	// construct relev and compute IDCG@N
	map<int, float> relev; // item:score
	int N = 1;
	float idcg = 0;
	for (auto& rateItemPair : Rrelev) {
		int itemID = rateItemPair.second;
		float rating = rateItemPair.first;
		relev[itemID] = rating;
		if (N == 1) {
			idcg += pow(2,rating) - 1;
		}
		else {
			idcg += (pow(2,rating) - 1) / (log(N+1)/log(2));
		}
		if (N % 1 == 0) { // was: N % 5 == 0
			IDCG[N] = idcg;
		}
		N++;
	}
	int maxN = N - 1;
	float maxIDCG = idcg;


	float AP = 0; // average precision
	int numRelevantAtN = 0;
	float dcg = 0;
	N = 1;

	for (auto& scoreItem : Restim) {
		int itemID = scoreItem.second;
		if (relev.count(itemID)) { // found relevant item
			numRelevantAtN++;
			AP += ( (float) numRelevantAtN ) / N;
			if (N == 1) {
				dcg += pow(2,relev[itemID]) - 1;
			}
			else {
				dcg += (pow(2,relev[itemID]) - 1) / (log(N+1)/log(2));
			}
		}
		if (N % 1 == 0) { // was: N % 5 == 0
			metrics.prec[N] = ( (float) numRelevantAtN ) / N;
			metrics.rec[N] = ( (float) numRelevantAtN ) / Rrelev.size();
			if (N >= maxN) {
				metrics.NDCG[N] = dcg / maxIDCG;
			}
			else {
				metrics.NDCG[N] = dcg / IDCG[N];
			}
		}
		N++;
	}

	AP = AP / Rrelev.size();
	metrics.AP = AP;

}

void Evaluator::computeFairness(vector<Item> items, int numUsers, int numItems) {

	vector<bool> isFairToUser;
	for (int i=0; i<numUsers; i++) {
		isFairToUser.push_back(false);
	}


	vector<int> bestRankScore;
	vector<int> worstRankScore;


	vector<int> numGood;
	for (int i=0; i<numUsers; i++) {
		numGood.push_back(0);
	}

	int N = 1;
	for (auto& scoreItem : Restim) {
//		cout << "position " << N << endl;
		int itemID = scoreItem.second;

//		cout << "itemID=" << itemID << " score=" << scoreItem.first << endl;


		Item item = getItemByID(items, itemID);

		for (int i=0; i<numUsers; i++) {
//			cout << "user " << i << " rankScore=" << item.rankScores[i] << endl;

			int rankScore =  item.rankScores[i];

			if (N == 1) { // first item
				bestRankScore.push_back(rankScore);
				worstRankScore.push_back(rankScore);
			}
			else {
				if (rankScore > bestRankScore[i]) {
					bestRankScore[i] = rankScore;
				}
				if (rankScore < worstRankScore[i]) {
					worstRankScore[i] = rankScore;
				}
			}

			if ( rankScore > numItems - numRelevant) {
				isFairToUser[i] = true;  // if at least one item for user i ranks among the top numRelevant items, then it's fair to user i
				numGood[i] ++;
			}
		}

		float avgBestRankScore = 0.0;
		float avgWorstRankScore = 0.0;
		int fairnessCount = 0;
		for (int i=0; i<numUsers; i++) {
			if ( isFairToUser[i] ) {
				fairnessCount++;
			}
			avgBestRankScore += bestRankScore[i];
			avgWorstRankScore += worstRankScore[i];
		}
		avgBestRankScore /= numUsers;
		avgWorstRankScore /= numUsers;
//		cout << "fairness=" << fairnessCount << endl;
//		cout << "avgBestRankScore=" << avgBestRankScore << endl;
//		cout << "avgWorstRankScore=" << avgWorstRankScore << endl;

		int minNumGood = N; // across users
		int maxNumGood = 0; // across users
		for (int i=0; i<numUsers; i++) {
			if (numGood[i] < minNumGood) {
				minNumGood = numGood[i];
			}
			if (numGood[i] > maxNumGood) {
				maxNumGood = numGood[i];
			}
		}

		if (N % 1 == 0) { // was: N % 5 == 0
			metrics.fairness[N] = fairnessCount;
			metrics.avgBestRankScore[N] = avgBestRankScore;
			metrics.avgWorstRankScore[N] = avgWorstRankScore;
			metrics.minNumGood[N] = minNumGood;
//			metrics.maxNumGood[N] = maxNumGood;
		}
		N++;

	}

}


void Evaluator::computeSatisfactionFunctions(vector<Item> items, int numUsers, int numItems) {

	int N = 1;

	vector<float> utilities(numUsers, 0.0); // utility of a member is the sum of ratings for all relevant items

	for (auto& scoreItem : Restim) {
//		cout << "position " << N << endl;
		int itemID = scoreItem.second;

		Item item = getItemByID(items, itemID);

		for (int i=0; i<numUsers; i++) {

			float rating =  item.ratings[i];

			utilities[i] += rating;

		}


		if (N == numRelevant) break;
		N++;
	}

	// from RecSys'17 Fairness-Aware Group Recommendation with Pareto-Efficiency
	float sum_utility = 0; // sum of utilities over all members; social welfare
	float min_utility = numeric_limits<float>::infinity(); // min utility among members; least misery fairness
	float max_utility = -numeric_limits<float>::infinity();
	float sum_sq_utility = 0; // sum of squared utilities

	for (int i=0; i<numUsers; i++) {
		sum_utility += utilities[i];
		sum_sq_utility += utilities[i] * utilities[i];
		if (utilities[i] > max_utility) {
			max_utility = utilities[i];
		}
		if (utilities[i] < min_utility) {
			min_utility = utilities[i];
		}
	}

	float variance = 1 - ( sum_sq_utility/numUsers - (sum_utility/numUsers)*(sum_utility/numUsers) );

	float jain_fairness = (sum_utility * sum_utility) / ( numUsers * sum_sq_utility );

	float minmax = min_utility / max_utility;

	metrics.sum_utility = sum_utility;
	metrics.min_utility = min_utility;
	metrics.max_utility = max_utility;
	metrics.variance = variance;
	metrics.jain_fairness = jain_fairness;
	metrics.minmax = minmax;

}

