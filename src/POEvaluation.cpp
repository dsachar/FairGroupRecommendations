/*
 * POEvaluation.cpp
 *
 *  Created on: Oct 6, 2017
 *      Author: dimitris
 */


#include "POEvaluation.hpp"


POEvaluation::POEvaluation(UserRatings& ratings) : ur(ratings) {
}


void POEvaluation::doRepsFromFile() {
	po = ParetoOptimal(ur);


	random_device rd;
	mt19937 g(rd());


	////// read groups
	vector<vector<int>> groups;


	ifstream g_in("datasets/Groups");
	string line;

	int currentGroupID = -1;

	while (getline(g_in, line)) {
	    istringstream iss(line);
	    int groupID, userID;
		if ( !(iss >> groupID) ) {
			break;
		}

		if (groupID != currentGroupID) {
//			cout << "new group with ID " << groupID << endl;
			currentGroupID = groupID;
			vector<int> group;
			groups.push_back(group);
		}

		if ( !(iss >> userID) ) {
			break;
		}
		groups[groups.size()-1].push_back(userID);
//		cout << "userID = " << userID << endl;

		float weight; // ignore this
		if ( !(iss >> weight) ) {
			break;
		}

	}
	g_in.close();


	////// read ground truth, i.e., group ratings
	vector< map<int, float> > truths;

	g_in = ifstream("datasets/GroupRatings");

	currentGroupID = -1;

	while (getline(g_in, line)) {
	    istringstream iss(line);
	    int groupID, itemID;
	    float rating;
		if ( !(iss >> groupID) ) {
			break;
		}

		if (groupID != currentGroupID) {
//			cout << "new group with ID " << groupID << endl;
			currentGroupID = groupID;
			map<int, float> truth;
			truths.push_back(truth);
		}

		if ( !(iss >> itemID) ) {
			break;
		}

		if ( !(iss >> rating) ) {
			break;
		}

		truths[truths.size()-1][itemID] = rating;

	}
	g_in.close();




//	for (map<int, float> truth : truths) {
//		for (auto entry : truth) {
//			cout << entry.first << ":" << entry.second << " ";
//		}
//		cout << endl;
//	}


	numRepetitions = groups.size();


	for (int rep=0; rep<groups.size(); rep++) {
		po.setRandomItems(-1);

		po.users = groups[rep];


		po.retrieveRatings();

//		po.findParetoOptimal();
//		po.findKSkyband(numRelevant);

		po.computeRankScores();


		// create possible rankings
		int numRankings = 100;
		int K; // for the probability of being among top=K
		map<int, float> counts;

		int nonzero = 0;

		K = 1;
		while (nonzero < topK) {
			counts = po.countTopKProbabilities(K);
			nonzero = 0;
			for (auto entry : counts) {
				if (abs(entry.second) > 1e-5 ) {
					nonzero++;
				}
			}
//			cout << "nonzero counts " << nonzero << " for K=" << K << endl;
			K++;
		}
		K--;
		cout << "nonzero counts " << nonzero << " for K=" << K << endl;


		vector< map<int, float> > rankings = po.getRandomRankings(counts, numRankings, topK);
		map<int, float> truth = truths[rep];



		int bestAlter = -1;
		float bestSumKendallTau = -std::numeric_limits<float>::infinity();
		for (int alter=0; alter<rankings.size(); alter++) {

			float sumKendallTau = 0;
			for (int truth=0; truth<rankings.size(); truth++) {
				Evaluator eval = Evaluator(rankings[truth], rankings[alter], topK);
//				cout << "rankings[truth].size()=" << rankings[truth].size() <<  " rankings[alter].size()=" << rankings[alter].size() << endl;
				eval.computeRankingMetrics();

				sumKendallTau += eval.metrics.KendalTau;

			}

			if (sumKendallTau > bestSumKendallTau) {
				bestSumKendallTau = sumKendallTau;
				bestAlter = alter;
			}

		}
//		cout << "best alternative ranking is " << bestAlter << endl;



		/// create rankscores based on rankings
		vector<float> ratings(numRankings, 0); // dummy ratings
		vector<Item> rItems;
		for (int itemID : po.topL) {
			Item rItem = Item(itemID, ratings);
			rItems.push_back(rItem);
		}


		for (int rankingID=0; rankingID<rankings.size(); rankingID++) {
			map<int, float> ranking = rankings[rankingID];

			for (auto entry : ranking){
				Item& rItem = getItemByID(rItems, entry.first);
				rItem.rankScores[rankingID] = entry.second;
//				cout << "rankingID=" << rankingID << " entry.first=" << entry.first << " entry.second=" << entry.second << endl;
			}

		}

//		for (Item rItem : rItems) {
//			cout << rItem.toString() << endl;
//		}

		map<int, float> alt_borda = po.aggregateRanksOfInput(rItems, AVG);

//		cout << "alt_borda: " << endl;
//		for (auto entry : alt_borda){
//			cout << "entry.first=" << entry.first << " entry.second=" << entry.second << endl;
//		}


		counts = po.restrictToTopL(counts);
//		cout << "counts: " << endl;
//		for (auto entry : counts){
//			cout << "entry.first=" << entry.first << " entry.second=" << entry.second << endl;
//		}


		/// run classic methods

		map<int, float> avgRatings = po.aggregateRatings(AVG);
//		map<int, float> avgRatings_R = po.restrictToTopL(avgRatings);

		map<int, float> minRatings = po.aggregateRatings(MIN);
//		minRatings = po.restrictToTopL(minRatings);

		map<int, float> maxRatings = po.aggregateRatings(MAX);
//		maxRatings = po.restrictToTopL(maxRatings);

		map<int, float> prdRatings = po.aggregateRatings(PRD);
//		map<int, float> prdRatings_R = po.restrictToTopL(prdRatings);

		map<int, float> borda = po.aggregateRanks(AVG);
//		map<int, float> borda_R = po.restrictToTopL(borda);

		map<int, float> medrank = po.aggregateRanks(MED);
//		medrank = po.restrictToTopL(medrank);

		map<int, float> kais = po.KAIS_fair(numRelevant);
//		kais = po.restrictToTopL(kais);

		map<int, float> www01 = po.WWW_fair(0.1, 1, 0);
//		www01 = po.restrictToTopL(www01);

		map<int, float> www02 = po.WWW_fair(0.5, 1, 0);
//		www02 = po.restrictToTopL(www02);


		map<int, float> recsys = po.RecSys17_fair(0.7);

		map<int, float> countTopK = po.countTopKOverAllWeights(topK);
//		countTopK = po.restrictToTopL(countTopK);

		SinglePOEvaluation spo(*this);
		methodNames.clear();


		spo.methods.push_back(avgRatings);
		methodNames.push_back("AVG");
//		spo.methods.push_back(avgRatings_R);
//		methodNames.push_back("AVG_R");
		spo.methods.push_back(minRatings);
		methodNames.push_back("MIN");
		spo.methods.push_back(maxRatings);
		methodNames.push_back("MAX");
		spo.methods.push_back(prdRatings);
		methodNames.push_back("PRD");
//		spo.methods.push_back(prdRatings_R);
//		methodNames.push_back("PRD_R");
		spo.methods.push_back(borda);
		methodNames.push_back("BORDA");
//		spo.methods.push_back(borda_R);
//		methodNames.push_back("BORDA_R");
		spo.methods.push_back(medrank);
		methodNames.push_back("MED");
		spo.methods.push_back(kais);
		methodNames.push_back("KAIS");
		spo.methods.push_back(www01);
		methodNames.push_back("WWW01");
		spo.methods.push_back(www02);
		methodNames.push_back("WWW02");

		spo.methods.push_back(recsys);
		methodNames.push_back("RECSYS");

//		spo.methods.push_back(countTopK);
//		methodNames.push_back("CTK");


//		spo.methods.push_back(rankings[bestAlter]);
//		methodNames.push_back("ALTER");

		spo.methods.push_back(alt_borda);
		methodNames.push_back("ALT_BORDA");


		spo.methods.push_back(counts);
		methodNames.push_back("COUNTS");






		spo.criteria.push_back(truth);
		criteriaNames.push_back("R");


		numCriteria = spo.criteria.size();
		numMethods = spo.methods.size();



		for (int i=0; i<numMethods; i++) {
			AggMetrics aggMetrics;
			aggMetrics.numCriteria = numCriteria;
			aggMetrics.numRepetitions = numRepetitions;
			methodAggMetrics.push_back(aggMetrics);
		}


		spo.evaluateMethodsOnCriteria();
		cout << "--- rep " << rep+1 << ": evaluations done" << endl;


	}

	doEvaluation();

}




void POEvaluation::doRepetitions_NEW() {
	po = ParetoOptimal(ur);

	random_device rd;
	mt19937 g(rd());




	for (int rep=0; rep<numRepetitions; rep++) {
		po.setRandomItems(numItems);

		po.createGroupNEW(numUsers, groupType); // greedily create group

		po.retrieveRatings();

		po.findParetoOptimal();
		po.findKSkyband(numRelevant);

		po.computeRankScores();


		// create possible rankings
		int numRankings = 200;
		int K; // for the probability of being among top=K
		map<int, float> counts;

		int nonzero = 0;

		K = 3;
		while (nonzero < topK) {
			counts = po.countTopKProbabilities(K);
			nonzero = 0;
			for (auto entry : counts) {
				if (abs(entry.second) > 1e-5 ) {
					nonzero++;
				}
			}
//			cout << "nonzero counts " << nonzero << " for K=" << K << endl;
			K++;
		}
		K--;
		cout << "nonzero counts " << nonzero << " for K=" << K << endl;


		int numTruths = 50;

		vector< map<int, float> > rankings = po.getRandomRankings(counts, numRankings + numTruths, topK); // + numTruths for the ground truths

		vector< map<int, float> > truths(numTruths);
		for (int i=0; i<numTruths; i++) {
			truths[i] = rankings[rankings.size()-1];
			rankings.pop_back();
		}






		int bestAlter = -1;
		float bestSumMetric = -std::numeric_limits<float>::infinity();
		for (int alter=0; alter<rankings.size(); alter++) {

			float sumMetric = 0;
			for (int truth=0; truth<rankings.size(); truth++) {
				Evaluator eval = Evaluator(rankings[truth], rankings[alter], topK);
//				cout << "rankings[truth].size()=" << rankings[truth].size() <<  " rankings[alter].size()=" << rankings[alter].size() << endl;
				eval.computeRankingMetrics();

//				sumMetric += eval.metrics.KendalTau;
				sumMetric += eval.metrics.NDCG[5];

			}

			if (sumMetric > bestSumMetric) {
				bestSumMetric = sumMetric;
				bestAlter = alter;
			}

		}
//		cout << "best alternative ranking is " << bestAlter << endl;



		/// create rankscores based on rankings
		vector<float> ratings(numRankings, 0); // dummy ratings
		vector<Item> rItems;
		for (int itemID : po.topL) {
			Item rItem = Item(itemID, ratings);
			rItems.push_back(rItem);
		}


		for (int rankingID=0; rankingID<rankings.size(); rankingID++) {
			map<int, float> ranking = rankings[rankingID];

			for (auto entry : ranking){
				Item& rItem = getItemByID(rItems, entry.first);
				rItem.rankScores[rankingID] = entry.second;
//				cout << "rankingID=" << rankingID << " entry.first=" << entry.first << " entry.second=" << entry.second << endl;
			}

		}

//		for (Item rItem : rItems) {
//			cout << rItem.toString() << endl;
//		}

		map<int, float> alt_borda = po.aggregateRanksOfInput(rItems, AVG);

		map<int, float> alt_comp = po.aggregateRanksOfInput(rItems, CMP);


		map<int, float> alt_borda_kem = po.localKemenize(alt_borda, rItems);


//		cout << "alt_borda: " << endl;
//		for (auto entry : alt_borda){
//			cout << "entry.first=" << entry.first << " entry.second=" << entry.second << endl;
//		}


		counts = po.restrictToTopL(counts);
//		cout << "counts: " << endl;
//		for (auto entry : counts){
//			cout << "entry.first=" << entry.first << " entry.second=" << entry.second << endl;
//		}


		map<int, float> counts_kem = po.localKemenize(counts, rItems);


		/// run classic methods

		map<int, float> avgRatings = po.aggregateRatings(AVG);
		map<int, float> avgRatings_R = po.restrictToTopL(avgRatings);

		map<int, float> minRatings = po.aggregateRatings(MIN);
		map<int, float> minRatings_R = po.restrictToTopL(minRatings);

		map<int, float> maxRatings = po.aggregateRatings(MAX);
		map<int, float> maxRatings_R = po.restrictToTopL(maxRatings);

		map<int, float> prdRatings = po.aggregateRatings(PRD);
		map<int, float> prdRatings_R = po.restrictToTopL(prdRatings);

		map<int, float> borda = po.aggregateRanks(AVG);
		map<int, float> borda_R = po.restrictToTopL(borda);

		map<int, float> medrank = po.aggregateRanks(MED);
		map<int, float> medrank_R = po.restrictToTopL(medrank);

		map<int, float> kais = po.KAIS_fair(numRelevant);
		map<int, float> kais_R = po.restrictToTopL(kais);

		map<int, float> www01 = po.WWW_fair(0.05, 1, 0);
		map<int, float> www01_R = po.restrictToTopL(www01);

		map<int, float> www02 = po.WWW_fair(0.05, 1, 1);
		map<int, float> www02_R = po.restrictToTopL(www02);


		map<int, float> recsys = po.RecSys17_fair(0.7);
		map<int, float> recsys_R = po.restrictToTopL(recsys);


		map<int, float> countTopK = po.countTopKOverAllWeights(topK);
		map<int, float> countTopK_R = po.restrictToTopL(countTopK);

		SinglePOEvaluation spo(*this);
		methodNames.clear();


//		spo.methods.push_back(avgRatings);
//		methodNames.push_back("AVG");
		spo.methods.push_back(avgRatings_R);
		methodNames.push_back("AVG_R");
//		spo.methods.push_back(minRatings);
//		methodNames.push_back("MIN");
		spo.methods.push_back(minRatings_R);
		methodNames.push_back("MIN_R");
		spo.methods.push_back(maxRatings);
//		methodNames.push_back("MAX");
//		spo.methods.push_back(maxRatings_R);
		methodNames.push_back("MAX_R");
		spo.methods.push_back(prdRatings);
//		methodNames.push_back("PRD");
//		spo.methods.push_back(prdRatings_R);
		methodNames.push_back("PRD_R");
//		spo.methods.push_back(borda);
//		methodNames.push_back("BORDA");
		spo.methods.push_back(borda_R);
		methodNames.push_back("BORDA_R");
//		spo.methods.push_back(medrank);
//		methodNames.push_back("MED");
		spo.methods.push_back(medrank_R);
		methodNames.push_back("MED_R");
//		spo.methods.push_back(kais);
//		methodNames.push_back("KAIS");
		spo.methods.push_back(kais_R);
		methodNames.push_back("KAIS_R"); // GR-FAIR
//		spo.methods.push_back(www01);
//		methodNames.push_back("WWW01");
		spo.methods.push_back(www01_R);
		methodNames.push_back("WWW01_R");
//		spo.methods.push_back(www02);
//		methodNames.push_back("WWW02");
		spo.methods.push_back(www02_R);
		methodNames.push_back("WWW02_R");

//		spo.methods.push_back(recsys);
//		methodNames.push_back("RECSYS");
		spo.methods.push_back(recsys_R);
		methodNames.push_back("RECSYS_R");

//		spo.methods.push_back(countTopK);
//		methodNames.push_back("CTK");


//		spo.methods.push_back(rankings[bestAlter]);
//		methodNames.push_back("ALTER");
//
//		spo.methods.push_back(alt_borda);
//		methodNames.push_back("ALT_BORDA");
//
//		spo.methods.push_back(alt_borda_kem);
//		methodNames.push_back("ALT_BORDA_KEM");
//
//		spo.methods.push_back(alt_comp);
//		methodNames.push_back("ALT_COMP");

		spo.methods.push_back(counts);
		methodNames.push_back("COUNTS");

//		spo.methods.push_back(counts_kem);
//		methodNames.push_back("COUNTS_KEM");


		/// insert all rankings
//		int counter = 0;
//		for (auto ranking : rankings) {
//			spo.criteria.push_back(ranking);
//			criteriaNames.push_back("R" + to_string(counter));
//			counter++;
//		}


		/// insert random ranking
//		int randomRanking = rand() % rankings.size();
//		spo.criteria.push_back(rankings[randomRanking]);



//		spo.criteria.push_back(truth);
//		criteriaNames.push_back("R");

		for (int i=0; i<numTruths; i++) {
			spo.criteria.push_back(truths[i]);
			criteriaNames.push_back("R"+ to_string(i));
		}


		numCriteria = spo.criteria.size();
		numMethods = spo.methods.size();





		for (int i=0; i<numMethods; i++) {
			AggMetrics aggMetrics;
			aggMetrics.numCriteria = numCriteria;
			aggMetrics.numRepetitions = numRepetitions;
			methodAggMetrics.push_back(aggMetrics);
		}


		spo.evaluateMethodsOnCriteria();
		cout << "--- rep " << rep+1 << ": evaluations done" << endl;


	}

	doEvaluation();

}


void POEvaluation::doRepetitions() {
	po = ParetoOptimal(ur);
//	ParetoOptimal po(ur);


    random_device rd;
    mt19937 g(rd());

    uniform_real_distribution<> dis(1, 10);

	int numRandomGroups = 10;



	for (int rep=0; rep<numRepetitions; rep++) {
		po.setRandomItems(numItems);

//		po.sampleAllPairsSimilarities(); // WARNING run only once

//		po.createGroupOLD(numUsers, groupType);
//		po.createGroup(numUsers, groupType); // randomly create group
		po.createGroupNEW(numUsers, groupType); // greedily create group

		po.retrieveRatings();

		po.findParetoOptimal();
		po.findKSkyband(numRelevant);

		po.computeRankScores();

		/// run methods

		map<int, float> avgRatings = po.aggregateRatings(AVG);
		avgRatings = po.restrictToN(avgRatings, topK);

		map<int, float> minRatings = po.aggregateRatings(MIN);
		minRatings = po.restrictToN(minRatings, topK);

		map<int, float> maxRatings = po.aggregateRatings(MAX);
		maxRatings = po.restrictToN(maxRatings, topK);

		map<int, float> prdRatings = po.aggregateRatings(PRD);
		prdRatings = po.restrictToN(prdRatings, topK);

		map<int, float> penRatings = po.aggregateRatings(PEN);
		penRatings = po.restrictToN(penRatings, topK);


		map<int, float> countTopK = po.countTopKOverAllWeights(topK);
		countTopK = po.restrictToN(countTopK, topK);

		map<int, float> countTopK1 = po.countTopKOverAllWeights(topK, 1);
		countTopK1 = po.restrictToN(countTopK1, topK);

		map<int, float> countTopK2 = po.countTopKOverAllWeights(topK, 2);
		countTopK2 = po.restrictToN(countTopK2, topK);

		map<int, float> countTopK3 = po.countTopKOverAllWeights(topK, 3);
		countTopK3 = po.restrictToN(countTopK3, topK);

		map<int, float> countTopX = po.countsForTopKMostProbableItems(topK);
		countTopX = po.restrictToN(countTopX, topK);


		map<int, float> poAvgRatings = po.aggregateRatings(AVG, vector<float>(), true);

		map<int, float> borda = po.aggregateRanks(AVG);
		borda = po.restrictToN(borda, topK);

		map<int, float> medrank = po.aggregateRanks(MED);
		medrank = po.restrictToN(medrank, topK);

		map<int, float> kais = po.KAIS_fair(numRelevant);
		kais = po.restrictToN(kais, topK);

		map<int, float> www01 = po.WWW_fair(0.1, 1, 0);
		www01 = po.restrictToN(www01, topK);

		map<int, float> www02 = po.WWW_fair(0.5, 1, 0);
		www02 = po.restrictToN(www02, topK);

		map<int, float> recsys = po.RecSys17_fair(0.7);
		recsys = po.restrictToN(recsys, topK);


		map<int, float> counts = po.countTopKProbabilities(topK);
		vector< map<int, float> > rankings = po.getRandomRankings(counts, 10, topK);



//		printRecommendations("BORDA", borda);
//		printRecommendations("CTK", countTopK);
//		printRecommendations("KAIS", kais);
//		printRecommendations("WWW01", www01);
//		printRecommendations("WWW02", www02);


		SinglePOEvaluation spo(*this);

		methodNames.clear();
		spo.methods.push_back(avgRatings);
		methodNames.push_back("AVG");
		spo.methods.push_back(minRatings);
		methodNames.push_back("MIN");
		spo.methods.push_back(maxRatings);
		methodNames.push_back("MAX");
		spo.methods.push_back(prdRatings);
		methodNames.push_back("PRD");
		spo.methods.push_back(borda);
		methodNames.push_back("BORDA");
		spo.methods.push_back(medrank);
		methodNames.push_back("MED");
		spo.methods.push_back(kais);
		methodNames.push_back("KAIS");
		spo.methods.push_back(www01);
		methodNames.push_back("WWW01");
		spo.methods.push_back(www02);
		methodNames.push_back("WWW02");
		spo.methods.push_back(recsys);
		methodNames.push_back("RECSYS");
		spo.methods.push_back(countTopK);
		methodNames.push_back("CTK");
//		spo.methods.push_back(countTopK1);
//		methodNames.push_back("CTK1");
//		spo.methods.push_back(countTopK2);
//		methodNames.push_back("CTK2");
//		spo.methods.push_back(countTopK3);
//		methodNames.push_back("CTK3");
		spo.methods.push_back(countTopX);
		methodNames.push_back("CTX");
		spo.methods.push_back(penRatings);
		methodNames.push_back("PEN");
//		spo.methods.push_back(poAvgRatings);
//		methodNames.push_back("PO-AVG");


		criteriaNames.clear();
//		spo.criteria.push_back(avgRatings);
//		criteriaNames.push_back("AVG");
//		spo.criteria.push_back(minRatings);
//		criteriaNames.push_back("MIN");
//		spo.criteria.push_back(maxRatings);
//		criteriaNames.push_back("MAX");
//		spo.criteria.push_back(prdRatings);
//		criteriaNames.push_back("PRD");
//		spo.criteria.push_back(countTopK);
//		criteriaNames.push_back("CTK");
//		spo.criteria.push_back(borda);
//		criteriaNames.push_back("BORDA");
//		spo.criteria.push_back(medrank);
//		criteriaNames.push_back("MED");
//		spo.criteria.push_back(poAvgRatings);
//		criteriaNames.push_back("PO-AVG");

		for (int i=0; i<numUsers; i++) {
			map<int, float> member = po.memberRatings(i);
			member = po.restrictToN(member, topK);

			spo.criteria.push_back(member);
			criteriaNames.push_back("USR"+to_string(i+1));
		}


//		for (int i=0; i<numRandomGroups; i++){
//			vector<float> weights = po.getRandomWeights();
//
//			map<int, float> rnd = po.aggregateRatings(WEI, weights);
//			rnd = po.restrictToN(rnd, topK);
//
//			spo.criteria.push_back(rnd);
//			criteriaNames.push_back("RND"+to_string(i));
//		}



		numCriteria = spo.criteria.size();
		numMethods = spo.methods.size();



		for (int i=0; i<numMethods; i++) {
			AggMetrics aggMetrics;
			aggMetrics.numCriteria = numCriteria;
			aggMetrics.numRepetitions = numRepetitions;
			methodAggMetrics.push_back(aggMetrics);
		}


		spo.evaluateMethodsOnCriteria();
		cout << "--- rep " << rep+1 << ": evaluations done" << endl;
	}

	doEvaluation();


}


void POEvaluation::doEvaluation() {

	// compute evaluation metrics
	for (int i=0; i<numMethods; i++) {
		methodAggMetrics[i].averageOverRepetititions();
		methodAggMetrics[i].computeMarginals();
	}

	// print summary
	for (int i=0; i<numMethods; i++) {
		cout << "---- " << methodNames[i] << endl;
		cout << methodAggMetrics[i].getMarginalsString();
		cout << "-----------------------------------" << endl;
	}

	// write to output file
	ofstream rout;
	rout.open(resultFile, ios::app);

	rout << endl;
	rout << getSettingString();


	//// hack to print KendallTau
//	rout << "Kendall Taus" << endl;
//	for (int i=0; i<numMethods; i++) {
//		rout << methodNames[i] << endl;
//
//		for (int rep=0; rep<numRepetitions; rep++){
//			rout <<  methodAggMetrics[i].allMetrics[rep*numCriteria].KendalTauTopK << " ";
//		}
//		rout << endl;
//
//	}



	rout << "criteria \t";
	for (int crit=0; crit<numCriteria; crit++){
		rout << criteriaNames[crit] << Metrics::getMetricsResultFillerString() << "\t";
	}
	rout << endl;

	rout << "methods \t";
	for (int crit=0; crit<numCriteria; crit++){
		rout << Metrics::getMetricsResultHeaderString() << "\t";
	}
	rout << endl;

	for (int i=0; i<numMethods; i++) {
		rout << methodNames[i] << "\t";

		for (int crit=0; crit<numCriteria; crit++){
			rout << methodAggMetrics[i].avgOverRepsMetrics[crit].getMetricsResultString() << "\t";
		}
		rout << endl;
	}

	rout << "summary \t";
	rout << "average" << Metrics::getMetricsResultFillerString() << "\t";
	rout << "worst" << Metrics::getMetricsResultFillerString() << "\t";
	rout << "best" << Metrics::getMetricsResultFillerString() << "\t";
	rout << endl;

	rout << "methods \t";
	for (int i=0; i<3; i++){
		rout << Metrics::getMetricsResultHeaderString() << "\t";
	}
	rout << endl;

	for (int i=0; i<numMethods; i++) {
		rout << methodNames[i] << "\t";

		rout << methodAggMetrics[i].avgMetrics.getMetricsResultString() << "\t";
		rout << methodAggMetrics[i].worstMetrics.getMetricsResultString() << "\t";
		rout << methodAggMetrics[i].bestMetrics.getMetricsResultString() << "\t";

		rout << endl;
	}


	rout.close();
}


string POEvaluation::getSettingString() {
	ostringstream sout;
	sout << "numUsers \t numItems \t numRelevant \t groupType \t numRepetitions \t topK" << endl;
	sout << numUsers << " \t " << numItems << " \t " << numRelevant << " \t " << groupType << " \t " << numRepetitions << "\t" << topK << endl;
	return sout.str();
}


void POEvaluation::printRecommendations(string methodName, map<int, float> scores) {

	cout << "Method " << methodName << endl;

	multimap<float, int, std::greater<float>> fscores; // score:item in decreasing score
	for (auto itemScore : scores) {
		fscores.insert( make_pair(itemScore.second, itemScore.first) );
	}

	for (auto& scoreItem : fscores) {
		int itemID = scoreItem.second;
		Item item = getItemByID(po.items, itemID);
		cout << item.toString() << "*" << scoreItem.first << "*" <<  " ";
		cout << endl;
	}
	cout << endl;

}

