/*
 * main.cpp
 *
 *  Created on: Mar 18, 2015
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

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

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

	// setting
	ExpSetting exp;

	// data
	int numUsers;
	int numItems;
	int numRatings;
	int numGroups;
	int groupSize;
	int numEpochs = 20; // num training epochs
	int groupPredType = AVG;
	int numMethodReps = 1;
	double relevanceThreshFraction = 0.8;
	map<int,map<int,float>> ratings; // user:item:rate
	map<int, float> userBehavior; // user:behavior
	map<int, set<int>> groups; // group:users
	map<int,map<int,float>> groupRatings; // group:item:rate
	map<int, map<int,float>> userResidualBehavior; // user:group:residualBehavior

	// methods
	vector<GroupPredictor *> methods;

	// results file
	string resultsFile = "new_results";

	void readInputParameters(int argc, char** argv);

	void runMethods();


};

void Execute::readInputParameters(int argc, char** argv) {

	try {
		po::options_description desc("Allowed options");
		desc.add_options()
 			("help", "produce help message")
			("seed", po::value<int>(&globalRandomSeed), "random seed")
			("input,I", po::value<string>(&ratingsFile), "ratings file")
			("output,O", po::value<string>(&baseDir), "output dir")
			("groups,G", po::value<string>(&groupsFile), "groups file")
			("groupRatings,R", po::value<string>(&groupRatingsFile), "group ratings file")
			("residual", po::value<string>(&residualBehaviorFile), "residual behavior output file")
			("numUsers,n", po::value<int>(&numUsers), "number of users")
			("numItems,m", po::value<int>(&numItems), "number of items")
			("relevanceThreshFraction", po::value<double>(&relevanceThreshFraction), "relevance threshold as as fraction of maximum rating")
			("resultsFile", po::value<string>(&resultsFile), "results output file")
			("numMethodReps", po::value<int>(&numMethodReps), "number of method repetitions")
			("numEpochs", po::value<int>(&numEpochs), "number of training epochs")
			("groupPredType", po::value<int>(&groupPredType), "group prediction type (AVG, MIN, BEH)")
			("AGG-PRED", "predict using AGG-PRED")
			("AGG-PRED-AVG", "predict using AGG-PRED-AVG")
			("AGG-PRED-MIN", "predict using AGG-PRED-MIN")
			("AGG-PRED-BEH", "predict using AGG-PRED-BEH")
			("AGG-PRED-MAX", "predict using AGG-PRED-MAX")
			("AGG-PRED-MUL", "predict using AGG-PRED-MUL")
			("AGG-PROF", "predict using AGG-PROF")
			("AGG-PROF-AVG", "predict using AGG-PROF-AVG")
			("AGG-PROF-MIN", "predict using AGG-PROF-MIN")
			("AGG-PROF-BEH", "predict using AGG-PROF-BEH")
			("AGG-PROF-MAX", "predict using AGG-PROF-MAX")
			("AGG-PROF-MUL", "predict using AGG-PROF-MUL")
			("RESIDUAL", "predict using RESIDUAL")
			("TRIAD", "predict using TRIAD")
			("INF-MATCH", "predict using INF-MATCH")
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




		if (!vm.count("input")) {
        	cerr << "Missing input ratingsFile (-I)." << endl;
        	exit(0);
		}
		UserRatings userRatings;
		userRatings.readRatings(ratingsFile);
		numUsers = userRatings.numUsers;


//        if (vm.count("AGG-PRED") || vm.count("AGG-PROF") || vm.count("RESIDUAL") || vm.count("TRIAD") || vm.count("INF-MATCH")) {
//
//        	if (!vm.count("groups")) {
//            	cerr << "Missing input file groups (-G)." << endl;
//            	exit(0);
//        	}
//
//        	if (!vm.count("groupRatings")) {
//            	cerr << "Missing input file groupRatings (-O)." << endl;
//            	exit(0);
//        	}
//        }


		if (vm.count("AGG-PRED")) {
        	GP_Agg_Pred *pred = new GP_Agg_Pred(exp, groupPredType);
        	exp.addMethod(pred->methodName());
        	methods.push_back(pred);
        }

        if (vm.count("AGG-PRED-AVG")) {
        	GP_Agg_Pred *pred = new GP_Agg_Pred(exp, AVG);
        	exp.addMethod(pred->methodName());
        	methods.push_back(pred);
        }

        if (vm.count("AGG-PRED-MIN")) {
        	GP_Agg_Pred *pred = new GP_Agg_Pred(exp, MIN);
        	exp.addMethod(pred->methodName());
        	methods.push_back(pred);
        }

        if (vm.count("AGG-PRED-BEH")) {
        	GP_Agg_Pred *pred = new GP_Agg_Pred(exp, WEI);
        	exp.addMethod(pred->methodName());
        	methods.push_back(pred);
        }

        if (vm.count("AGG-PRED-MAX")) {
        	GP_Agg_Pred *pred = new GP_Agg_Pred(exp, MAX);
        	exp.addMethod(pred->methodName());
        	methods.push_back(pred);
        }

        if (vm.count("AGG-PRED-MUL")) {
        	GP_Agg_Pred *pred = new GP_Agg_Pred(exp, PRD);
        	exp.addMethod(pred->methodName());
        	methods.push_back(pred);
        }


        if (vm.count("AGG-PROF")) {
        	GP_Agg_Prof *pred = new GP_Agg_Prof(exp, groupPredType);
        	exp.addMethod(pred->methodName());
        	methods.push_back(pred);
        }

        if (vm.count("AGG-PROF-AVG")) {
        	GP_Agg_Prof *pred = new GP_Agg_Prof(exp, AVG);
        	exp.addMethod(pred->methodName());
        	methods.push_back(pred);
        }

        if (vm.count("AGG-PROF-MIN")) {
        	GP_Agg_Prof *pred = new GP_Agg_Prof(exp, MIN);
        	exp.addMethod(pred->methodName());
        	methods.push_back(pred);
        }

        if (vm.count("AGG-PROF-BEH")) {
        	GP_Agg_Prof *pred = new GP_Agg_Prof(exp, WEI);
        	exp.addMethod(pred->methodName());
        	methods.push_back(pred);
        }

        if (vm.count("AGG-PROF-MAX")) {
        	GP_Agg_Prof *pred = new GP_Agg_Prof(exp, MAX);
        	exp.addMethod(pred->methodName());
        	methods.push_back(pred);
        }

        if (vm.count("AGG-PROF-MUL")) {
        	GP_Agg_Prof *pred = new GP_Agg_Prof(exp, PRD);
        	exp.addMethod(pred->methodName());
        	methods.push_back(pred);
        }

        if (vm.count("RESIDUAL")) {
        	GP_Residual *pred = new GP_Residual(exp);
        	exp.addMethod(pred->methodName());
        	methods.push_back(pred);
        }

        if (vm.count("TRIAD")) {
        	GP_Triad *pred = new GP_Triad(exp);
        	exp.addMethod(pred->methodName());
        	methods.push_back(pred);
        }

        if (vm.count("INF-MATCH")) {
        	GP_InfMatch *pred = new GP_InfMatch(exp);
        	exp.addMethod(pred->methodName());
        	methods.push_back(pred);
        }

        if (methods.size() > 0) {
        	runMethods();
        }

//        if (vm.count("createResidual")) {
//
//    		if (!vm.count("input")) {
//            	cerr << "Missing input ratingsFile (-I)." << endl;
//            	exit(0);
//    		}
//
//        	if (!vm.count("groups")) {
//            	cerr << "Missing input file groups (-G)." << endl;
//            	exit(0);
//        	}
//
//        	if (!vm.count("groupRatings")) {
//            	cerr << "Missing input file groupRatings (-O)." << endl;
//            	exit(0);
//        	}
//
//        	if (!vm.count("residual")) {
//            	cerr << "Missing output file residual (-R)." << endl;
//            	exit(0);
//        	}
//
//
//    		UserRatings userRatings;
//    		userRatings.readRatings(ratingsFile);
//
//        	Groups groups;
//        	groups.init();
//        	groups.readGroups(groupsFile);
//
//        	GroupRatings groupRatings;
//        	groupRatings.readGroupRatings(groupRatingsFile);
//
//        	ResidualBehavior residualBehavior(userRatings, groupRatings, groups);
//        	residualBehavior.calculateResidualBehavior();
//        	residualBehavior.writeResidualBehavior(residualBehaviorFile);
//
//        }



    }
    catch(std::exception& e)
    {
        cout << e.what() << "\n";
        exit(0);
    }


}







void Execute::runMethods(){
	if (methods.size() == 0) return;

	UserRatings userRatings;
	userRatings.readRatings(ratingsFile);
	exp.maxRating = userRatings.maxRating;
	exp.relevanceThreshFraction = relevanceThreshFraction;
	exp.inputFile = ratingsFile;
	exp.numEpochs = numEpochs;


	exp.setBaseDir(baseDir);



	vector<string> groupsFiles;
	vector<string> groupRatingsFiles;

	fs::path dir(baseDir);
	if (fs::exists(dir) && fs::is_directory(dir)) { // base directory exists
	    fs::directory_iterator end_iter;
	    for (fs::directory_iterator dir_itr(dir); dir_itr != end_iter; ++dir_itr) {
	    	if (fs::is_regular_file(dir_itr->status())) {

	    		string extension = fs::extension(dir_itr->path());
//	    		cout << "extension: " << extension << endl;

	    		if (extension == ".groups") {
	    			string groupsFile = dir_itr->path().string();
	    			groupsFiles.push_back(groupsFile);
	    		}

	    		if (extension == ".train") {
		    		string groupRatingsFile = dir_itr->path().parent_path().string() + "/" + dir_itr->path().stem().string();
	    			groupRatingsFiles.push_back(groupRatingsFile);
	    		}

	    	}
	    }
	}
	else { // base directory does not exist
		return;
	}


	for (string groupsFile : groupsFiles) { // for each group file
		string groupsFileNoExt = fs::path(groupsFile).stem().string();
		cout << "groupsFile: " << groupsFile << endl;
		for (string groupRatingsFile : groupRatingsFiles) { // for each ratings file for this group
			if (groupRatingsFile.find(groupsFileNoExt) == string::npos) {
				continue;
			}
			cout << "groupRatingsFile: " << groupRatingsFile << endl;


			Groups groups;
			groups.read(groupsFile);
			GroupRatings groupRatings;
			groupRatings.readGroupRatings(groupRatingsFile + ".train");

			exp.setInputParameters(groups, groupRatings);
			exp.writeSetting();
			exp.writeHeader();


			// run methods
			for (int mrep=0; mrep<numMethodReps; mrep++) {
				cout << "method repetition " << mrep << endl;
				int repetition = mrep;
				for (auto& pred : methods) {
					pred->readRatings(groupRatingsFile, ratingsFile);
					pred->resetPredictions();
					pred->predictRatings(ratingsFile, groupsFile, groupRatingsFile);
					ExpMeasurement meas = pred->evaluate();
					exp.addMeasurement(repetition, pred->methodName(), meas);
					cout << pred->methodName() << "::: " << meas.getMeasurementString() << endl;
					cout << pred->methodName() << "::: " << meas.getMeasurementRelAtNString() << endl;
//					cout << pred->methodName() << "::: " << meas.getMeasurementAtNString() << endl;
				}
				exp.writeRepetition(repetition);
			}

		}
	}

	/*

	for (int grep=0; grep<numGroupReps; grep++) {
		cout << "group repetition " << grep << endl;
		createGroups();
		createGroupRatings();

		Groups groups;
		groups.readGroups(groupsFile);
		GroupRatings groupRatings;
		groupRatings.readGroupRatings(groupRatingsFile + ".train");

		if (grep==0) {
			exp.setInputParameters(groups, groupRatings);
			exp.setResultsFile(resultsFile);
			exp.writeSetting();
			exp.writeHeader();
		}
		// run methods
		for (int mrep=0; mrep<numMethodReps; mrep++) {
			cout << "method repetition " << mrep << endl;
			int repetition = grep*numMethodReps + mrep;
			for (auto& pred : methods) {
				pred->readRatings(groupRatingsFile, ratingsFile);
				pred->resetPredictions();
				pred->predictRatings(ratingsFile, groupsFile, groupRatingsFile);
				ExpMeasurement meas = pred->evaluate();
				exp.addMeasurement(repetition, pred->methodName(), meas);
				cout << pred->methodName() << "::: " << meas.getMeasurementString() << endl;
				cout << pred->methodName() << "::: " << meas.getMeasurementAtNString() << endl;
			}
			exp.writeRepetition(repetition);
		}
	}
	*/

}



int main(int argc, char** argv){
//	if (globalRandomSeed == -1) {
//		globalRandomSeed = time(0);
//	}

	Execute exec;
	exec.readInputParameters(argc, argv);
}

