/*
 * parseInput.cpp
 *
 *  Created on: Apr 20, 2015
 *      Author: dsachar
 */

#include <fstream>
#include <sstream>
#include <iostream>
#include <set>
#include <map>
#include <tuple>
#include <vector>
#include <boost/program_options.hpp>


using namespace std;
namespace po = boost::program_options;


int main(int argc, char** argv){


	string inputFile;
	string outputFile;

	int maxNumUsers = INT_MAX;
	int maxNumItems = INT_MAX;
	int maxNumRatings = INT_MAX;


	set<string> userNames;
	set<string> itemNames;

	vector<tuple<string, string, float>> inputRatings;  // userName:itemName:rating



	try {
			po::options_description desc("Allowed options");
			desc.add_options()
				("input,I", po::value<string>(&inputFile), "input file")
				("output,O", po::value<string>(&outputFile), "output file")
				("maxNumUsers,n", po::value<int>(&maxNumUsers), "max number of users")
				("maxNumItems,m", po::value<int>(&maxNumItems), "max number of items")
				("maxNumRatings,r", po::value<int>(&maxNumRatings), "max number of ratings")
	        ;

			po::positional_options_description p;

	        po::variables_map vm;
	        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
	        po::notify(vm);


			if (!vm.count("input") || !vm.count("output")) {
				cerr << "missing input/output files" << endl;
				exit(0);
			}

	}
    catch(std::exception& e)
    {
        cout << e.what() << "\n";
        exit(0);
    }




	ifstream rin(inputFile.c_str());

	string line;
	while (getline(rin, line)) {
	    istringstream iss(line);
	    string user, item;
	    float rate;
		if ( !(iss >> user) ) {
			break;
		}
		userNames.insert(user);
		if ( !(iss >> item) ) {
			break;
		}
		itemNames.insert(item);
		if ( !(iss >> rate) ) {
			break;
		}
		inputRatings.push_back(make_tuple(user, item, rate));
	}
	rin.close();

	map<string, int> userNameToID; // userName:userID
	map<string, int> itemNameToID; // itemName:itemID

	int userID = 1;
	for (string userName : userNames) {
		userNameToID[userName] = userID++;
	}

	int itemID = 1;
	for (string itemName : itemNames) {
		itemNameToID[itemName] = itemID++;
	}

	cout << "numUsers=" << userNames.size() << " numItems=" << itemNames.size() << endl;

	ofstream rout;
	rout.open(outputFile.c_str(), ios::out);

	for (auto& ratingTuple : inputRatings) {
		string userName = get<0>(ratingTuple);
		if (userNameToID[userName] > maxNumUsers) continue;
		string itemName = get<1>(ratingTuple);
		if (itemNameToID[itemName] > maxNumItems) continue;
		float rating = get<2>(ratingTuple);
		rout << userNameToID[userName] <<  " " << itemNameToID[itemName] <<  " " << rating << endl;
	}

	rout.close();


}

