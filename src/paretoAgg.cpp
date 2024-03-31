/*
 * tester.cpp
 *
 *  Created on: 10 Mar 2017
 *      Author: dimitris
 */

#include <boost/program_options.hpp>
#include <iostream>
#include <random>


#include "common.hpp"
#include "UserRatings.hpp"

#include "ParetoOptimal.hpp"
#include "POEvaluation.hpp"
#include "Evaluator.hpp"
#include "AggMetrics.hpp"


namespace po = boost::program_options;


#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


using namespace std;

//int globalRandomSeed(123); // -1 means randomness
int globalRandomSeed(-1); // -1 means randomness

class Execute {
public:

	// parameters
	string ratingsFile;
	string baseDir = "PAscenario";

	int numUsers = 5;
	int numItems = 100;
//	int relevancePercentage = 20; // 20%
	int numRelevant = 20;
	int numRepetitions = 2;
	int groupType = 0;

	int topK = numRelevant; // relevancePercentage * numItems * 0.01; // topK used by CTK

	// data
	map<int,map<int,float>> ratings; // user:item:rate


	void readInputParameters(int argc, char** argv);

	void run();
};


void Execute::readInputParameters(int argc, char** argv) {

	try {
		po::options_description desc("Allowed options");
		desc.add_options()
 			("help", "produce help message")
			("input,I", po::value<string>(&ratingsFile), "ratings file")
			("output,O", po::value<string>(&baseDir), "output dir")
			("numUsers,n", po::value<int>(&numUsers), "number of users in groups")
			("numItems,m", po::value<int>(&numItems), "number of items to predict ratings/rankings")
			("numRelevant", po::value<int>(&numRelevant), "number of ranked items considered relevant")
			("topK", po::value<int>(&topK), "number of top items used by CTK")
			("numRepetitions", po::value<int>(&numRepetitions), "number of repetitions")
			("groupType", po::value<int>(&groupType), "type of groups; 0 is random, 1 is low similarity, 2 is high similarity")
		;

	po::positional_options_description p;


        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
        	cout << "Usage: options_description [options]" << endl;
        	cout << desc;
        	cout << "Example:" << endl;
        	cout << argv[0] << " -I datasets/ML1M_N" << endl;
        	exit(0);
        }


    }
    catch(std::exception& e)
    {
        cout << e.what() << "\n";
        exit(0);
    }

}

void Execute::run(){

	// create output directory if it doesn't exist
	fs::path dir(baseDir);
	if (!fs::exists(dir) && !fs::is_directory(dir)) {
		fs::create_directory(dir);
	}

	UserRatings ratings;
	ratings.readRatings(ratingsFile);

//	ratings.computeAllPairsSimilarities(); // WARNING very slow


	POEvaluation poe(ratings);
	poe.resultFile = baseDir + "/results.res";
	poe.ur = ratings;
	poe.numUsers = numUsers;
	poe.numItems = numItems;
	poe.numRelevant = numRelevant; //0.01 * relevancePercentage * numItems;
	poe.numRepetitions = numRepetitions;
	poe.groupType = groupType;
	poe.topK = topK;

	poe.doRepetitions(); // for plain pareto agg

//	poe.doRepetitions_NEW(); // for probabilities, payoff, states

//	poe.doRepsFromFile();

}

int main(int argc, char** argv){

	static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 required");

	Execute exec;
	exec.readInputParameters(argc, argv);

	exec.run();

	exit(1);
	//////////////////////////////////////////////////////////////

	UserRatings ratings;
	ratings.readRatings(exec.ratingsFile);


	random_device rd;
	mt19937 mt_rand(rd());
	if (globalRandomSeed != -1) {
		mt_rand.seed(globalRandomSeed);
	}

	int userA, userB;

	uniform_int_distribution<> unif(1, ratings.numUsers);

	userA = unif(mt_rand);
	userB = unif(mt_rand);

	float selfsim;
	int numCommonItems;
	tie(selfsim, numCommonItems) = ratings.computeCosSimilarity(userA, userA);

	cout << "user " << userA << " has self sim:" << selfsim << endl;

//	tie(selfsim, numCommonItems) = ratings.computeCosSimilarity(userA, userB);
//	cout << "numCommonItems: " << numCommonItems << endl;



	ParetoOptimal po(ratings);

	po.setRandomItems(100);
//	po.setRandomItems(-1);


	int numUsers = 5;
	set<int> users;
//	users = ratings.findSimilar(userA, numUsers, po.itemIDs);
//	users = ratings.findSimilar(userA, numUsers);
//	users = ratings.findDissimilar(userA, numUsers);
	users = ratings.selectRandomUsers(numUsers);

	vector<int> group(users.begin(), users.end());
	po.setUsers(group);


	po.retrieveRatings();



	po.findParetoOptimal();


//	cout << "AVG: " << po.getTopItem(AVG) << endl;
//	cout << "MIN: " << po.getTopItem(MIN) << endl;
//	cout << "MAX: " << po.getTopItem(MAX) << endl;
//	cout << "PRD: " << po.getTopItem(PRD) << endl;
//	cout << "WEI: " << po.getTopItem(WEI, vector<float>(numUsers,0.5)) << endl;

//	po.countTopItemAppearancesOverAllWeights();


	int numRelevant = 20;


	po.findKSkyband(numRelevant);

	po.getTopKItems(AVG, numRelevant);
	po.getTopKItems(MIN, numRelevant);
	po.getTopKItems(MAX, numRelevant);
	po.getTopKItems(PRD, numRelevant);

	for (int i=0; i<numUsers; i++) {
		po.getTopKItemsForUserIndx(numRelevant, i);
	}

	po.countTopKAppearancesOverAllWeights(numRelevant);



	map<int, float> avgRatings = po.aggregateRatings(AVG);
	map<int, float> minRatings = po.aggregateRatings(MIN);
	map<int, float> maxRatings = po.aggregateRatings(MAX);
	map<int, float> prdRatings = po.aggregateRatings(PRD);
	map<int, float> countTopK = po.countTopKOverAllWeights(numRelevant);
	map<int, float> member0 = po.memberRatings(0);
	map<int, float> member1 = po.memberRatings(1);
	map<int, float> member2 = po.memberRatings(2);
	map<int, float> member3 = po.memberRatings(3);
	map<int, float> member4 = po.memberRatings(4);



	Evaluator eval;



	cout << "**** AVG performance" << endl;
	AggMetrics AVGMetrics;
	AVGMetrics.numCriteria = 8;
	AVGMetrics.numRepetitions = 1;

	eval = Evaluator(maxRatings, avgRatings, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	AVGMetrics.allMetrics.push_back(eval.metrics);

	eval = Evaluator(minRatings, avgRatings, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	AVGMetrics.allMetrics.push_back(eval.metrics);

	eval = Evaluator(prdRatings, avgRatings, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	AVGMetrics.allMetrics.push_back(eval.metrics);

	eval = Evaluator(avgRatings, avgRatings, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	AVGMetrics.allMetrics.push_back(eval.metrics);

	eval = Evaluator(member0, avgRatings, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	AVGMetrics.allMetrics.push_back(eval.metrics);

	eval = Evaluator(member1, avgRatings, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	AVGMetrics.allMetrics.push_back(eval.metrics);

	eval = Evaluator(member2, avgRatings, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	AVGMetrics.allMetrics.push_back(eval.metrics);

	eval = Evaluator(member3, avgRatings, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	AVGMetrics.allMetrics.push_back(eval.metrics);

	AVGMetrics.averageOverRepetititions();

//	cout << "Agg Metrics" << endl;
//	for (int i=0; i<AVGMetrics.numCriteria; i++) {
//		cout << AVGMetrics.avgOverRepsMetrics[i].getMetricsString(10);
//	}

	AVGMetrics.computeMarginals();
	cout << "-- AVG" << endl;
	cout << AVGMetrics.avgMetrics.getMetricsString(10);

	cout << "-- WORST" << endl;
	cout << AVGMetrics.worstMetrics.getMetricsString(10);

	cout << "-- BEST" << endl;
	cout << AVGMetrics.bestMetrics.getMetricsString(10);



	cout << endl << "**** countTopK performance" << endl;
	AggMetrics CTKMetrics;
	CTKMetrics.numCriteria = 8;
	CTKMetrics.numRepetitions = 1;

	eval = Evaluator(maxRatings, countTopK, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	CTKMetrics.allMetrics.push_back(eval.metrics);

	eval = Evaluator(minRatings, countTopK, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	CTKMetrics.allMetrics.push_back(eval.metrics);

	eval = Evaluator(prdRatings, countTopK, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	CTKMetrics.allMetrics.push_back(eval.metrics);

	eval = Evaluator(avgRatings, countTopK, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	CTKMetrics.allMetrics.push_back(eval.metrics);

	eval = Evaluator(member0, countTopK, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	CTKMetrics.allMetrics.push_back(eval.metrics);

	eval = Evaluator(member1, countTopK, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	CTKMetrics.allMetrics.push_back(eval.metrics);

	eval = Evaluator(member2, countTopK, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	CTKMetrics.allMetrics.push_back(eval.metrics);

	eval = Evaluator(member3, countTopK, numRelevant);
	eval.computeRankingMetrics();
//	cout << eval.metrics.getMetricsString(10);
	CTKMetrics.allMetrics.push_back(eval.metrics);


	CTKMetrics.averageOverRepetititions();

//	cout << "Agg Metrics" << endl;
//	for (int i=0; i<CTKMetrics.numCriteria; i++) {
//		cout << CTKMetrics.avgOverRepsMetrics[i].getMetricsString(10);
//	}

	CTKMetrics.computeMarginals();
	cout << "-- AVG" << endl;
	cout << CTKMetrics.avgMetrics.getMetricsString(10);

	cout << "-- WORST" << endl;
	cout << CTKMetrics.worstMetrics.getMetricsString(10);

	cout << "-- BEST" << endl;
	cout << CTKMetrics.bestMetrics.getMetricsString(10);




	POEvaluation poe(ratings);
	poe.ur = ratings;
	poe.numUsers = numUsers;
	poe.numRelevant = numRelevant;
	poe.numRepetitions = 2;
	poe.groupType = 1;

	poe.doRepetitions();


//	po.computeRankScores();
//	for (Item item : po.items) {
//		cout << item.toString();
//	}
//
//	map<int, float> borda = po.aggregateRanks(AVG);
//	map<int, float> medrank = po.aggregateRanks(MED);
//
//	cout << po.getRankingString(borda, "BORDA");
//	cout << po.getRankingString(medrank, "MED-RANK");
//	cout << po.getRankingString(countTopK, "CTK");

}

