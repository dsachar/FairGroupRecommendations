/*
 * createSynGroupRatings.cpp
 *
 *  Created on: Feb 16, 2017
 *      Author: dimitris
 */

#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <boost/program_options.hpp>
#include <boost/math/special_functions/binomial.hpp>

#include "UserRatings.hpp"
#include "GroupRatings.hpp"
#include "Groups.hpp"
#include "UserRatings.hpp"
#include "Binom.hpp"
#include "MatrixFactorization.hpp"
#include "ResidualBehavior.hpp"
#include "GP_Agg_Pred.hpp"
#include "GP_Agg_Prof.hpp"
#include "GP_Residual.hpp"
#include "GP_Triad.hpp"
#include "GP_InfMatch.hpp"
#include "common.hpp"

namespace po = boost::program_options;



using namespace std;


//int globalRandomSeed(123); // -1 means randomness
int globalRandomSeed(-1); // -1 means randomness

class Execute {
public:

	// parameters
	string ratingsFile;
	string baseDir = "scenario";
	string groupRatingsFile = "models/groupRatings";
	string groupsFile = "models/groups";
	string userBehaviorFile = "models/userBehavior.txt";
	string residualBehaviorFile = "models/residualBehavior.txt";


	int splitType = 0; // default split into train/test

	// data
	int numUsers;
	int numItems;
	int numRatings;
	int numGroups;
	int groupSize;
	int numEpochs = 20; // num training epochs
	int numDistinctUsersInGroups;
	int numItemsRatedPerGroup = 10;
	int groupRatingsTrainPrc = 80; // 80%
	int userBehType = BIP;
	int groupBehType = WEI;
	int behaviorType;
	int groupPredType = AVG;
	int numMethodReps = 1;
	int numGroupReps = 1;
	bool roundGroupRatings = false;
	double relevanceThreshFraction = 0.8;
	map<int,map<int,float>> ratings; // user:item:rate
	map<int, float> userBehavior; // user:behavior
	map<int, set<int>> groups; // group:users
	map<int,map<int,float>> groupRatings; // group:item:rate
	map<int, map<int,float>> userResidualBehavior; // user:group:residualBehavior


	void readInputParameters(int argc, char** argv);

	void createGroupsAndGroupRatings();
	void createGroupsAndMultipleGroupRatings();
	void splitGroupRatings();

};

void Execute::readInputParameters(int argc, char** argv) {

	try {
		po::options_description desc("Allowed options");
		desc.add_options()
 			("help", "produce help message")
			("seed", po::value<int>(&globalRandomSeed), "random seed")
			("input,I", po::value<string>(&ratingsFile), "ratings file")
			("output,O", po::value<string>(&baseDir), "output dir")
			("groupRatings,R", po::value<string>(&groupRatingsFile), "group ratings file")
			("numGroups", po::value<int>(&numGroups), "number of groups")
			("groupSize", po::value<int>(&groupSize), "size of groups")
			("trainPrc", po::value<int>(&groupRatingsTrainPrc), "training/test split")
			("splitType", po::value<int>(&splitType), "split type")
			("roundGroupRatings", po::value<bool>(&roundGroupRatings), "round predicted ratings for groups")
			("numDistinctUsersInGroups", po::value<int>(&numDistinctUsersInGroups), "number of distinct users in groups")
			("numItemsRatedPerGroup", po::value<int>(&numItemsRatedPerGroup), "number of items rated per group")
			("userBehType", po::value<int>(&userBehType), "user behavior type (UNF, BIP, RND)")
			("groupBehType", po::value<int>(&groupBehType), "group behavior type (AVG, MIN, BEH, ABS)")
			("behaviorType", po::value<int>(&behaviorType), "behavior type (=group*10 + user)") // 00 is AVERAGE; 10 is MINIMUM; 20 is WEIGHTED-GLOBAL; 21 is LEADER-GLOBAL; 22 is WEIGHTED-LOCAL; 23 is LEADER-LOCAL; 30 is DICTATOR; 40 is MAXIMUM
			("split", "split only")
        ;

	po::positional_options_description p;


        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
        	cout << "Usage: options_description [options]" << endl;
        	cout << desc;
        	cout << "Example:" << endl;
        	cout << argv[0] << " -I datasets/ratings" << endl;
        	exit(0);
        }

        if (vm.count("behaviorType")) {
        	userBehType = behaviorType % 10;
        	groupBehType = behaviorType / 10;
        }


        UserRatings userRatings;
		if (!vm.count("numUsers")) {
			if (!vm.count("input")) {
				cerr << "Missing numUsers or input ratingsFile (-I)." << endl;
//				exit(0);
			}
			userRatings.readRatings(ratingsFile);
			numUsers = userRatings.numUsers;
		}

		if (!vm.count("numGroups") || !vm.count("groupSize") || !vm.count("numDistinctUsersInGroups")) {
			cerr << "Missing group info." << endl;
//			exit(0);
		}

		if (vm.count("split")) {
			splitGroupRatings();
			exit(0);
		}


    }
    catch(std::exception& e)
    {
        cout << e.what() << "\n";
        exit(0);
    }


}




void Execute::createGroupsAndGroupRatings() {
	UserRatings userRatings;
	userRatings.readRatings(ratingsFile);

	Groups groups;
	groups.createSynth(numUsers, numGroups, groupSize, numDistinctUsersInGroups, userBehType, baseDir);


	GroupRatings groupRatings;

	groupRatings.createSynth(groups, userRatings, numItemsRatedPerGroup, groupBehType, groupRatingsTrainPrc, roundGroupRatings);
}


void Execute::createGroupsAndMultipleGroupRatings() {
	UserRatings userRatings;
	userRatings.readRatings(ratingsFile);

	Groups groups;
	groups.createSynth(numUsers, numGroups, groupSize, numDistinctUsersInGroups, userBehType, baseDir);


	GroupRatings groupRatings;

	groupRatings.createMultipleSynth(groups, userRatings, numItemsRatedPerGroup, groupBehType, groupRatingsTrainPrc, roundGroupRatings);
}


void Execute::splitGroupRatings() {

	GroupRatings groupRatings;
	groupRatings.readGroupRatings(groupRatingsFile);
	groupRatings.trainPercentage = groupRatingsTrainPrc;
	groupRatings.splitAndWriteGroupRatings();
}




int main(int argc, char** argv){
	if (globalRandomSeed == -1) {
		globalRandomSeed = time(0);
	}

	Execute exec;
	exec.readInputParameters(argc, argv);

	if (exec.splitType == 0) {
		exec.createGroupsAndGroupRatings();
	}
	else if (exec.splitType == 1) {
		exec.createGroupsAndMultipleGroupRatings();
	}
}

