/*
 * GroupGenerator.cpp
 *
 *  Created on: Mar 19, 2015
 *      Author: dimitris
 */

#include "Groups.hpp"

#include <cassert>
#include <fstream>
#include <sstream>
#include <random>

#include "Binom.hpp"


#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


Groups::Groups() {
}


void Groups::readFrom(string baseDir, string fileName) {
	if (fileName == "any") { // read any groupFile, ending with .groups
		fs::path dir(baseDir);
		if (fs::exists(dir) && fs::is_directory(dir)) {
		    fs::directory_iterator end_iter;
		    for (fs::directory_iterator dir_itr(dir); dir_itr != end_iter; ++dir_itr) {
		    	if (fs::is_regular_file(dir_itr->status())) {

		    		string extension = fs::extension(dir_itr->path());

		    		if (extension == ".groups") {
			    		string fileName = dir_itr->path().filename().string();
		    			cout << "fileName: " << fileName << endl;
		    			groupsFile = dir_itr->path().string();
		    			break;
		    		}

		    	}
		    }
		}
	}
	else {
		groupsFile = baseDir + "/" + fileName;
	}
	readGroups();
}

void Groups::read(string fileName) {
	groupsFile = fileName;
	readGroups();
}



void Groups::createSynth(int numUsers, int numGroups, int groupSize, int numDistinctUsersInGroups, int behaviorType, string baseDir) { // init synthetic
	this->numGroups = numGroups;
	this->groupSize = groupSize;
	this->numDistinctUsersInGroups = numDistinctUsersInGroups;
	userBehType = behaviorType;
	this->ratingsFile = ratingsFile;

	setGroupsFile(baseDir);
	createGroups(numUsers);
	assignWeights();
	writeGroups();
}


void Groups::setGroupsFile(string baseDir) {

	fs::path dir(baseDir);
	fs::create_directory(dir);
	groupsFile = baseDir + "/G" + to_string(numGroups) + "_g" + to_string(groupSize) + "_d" + to_string(numDistinctUsersInGroups) + "_SD" + to_string(globalRandomSeed) + "_UB" + to_string(userBehType) + ".groups";
	cout << "groupsFile: " << groupsFile << endl;
}




void Groups::createGroups(int numUsers){
	assert (groupSize < numDistinctUsersInGroups);
	uint32_t maxNumGroups = binom(numDistinctUsersInGroups, groupSize);
	assert (numGroups < maxNumGroups);


	random_device rd;
	mt19937 mt_rand(rd());
	if (globalRandomSeed != -1) {
		mt_rand.seed(globalRandomSeed);
	}

	// randomly select numDistinctUsersInGroups users
	vector<int> selectedUsers;
	while (distinctUsers.size() < numDistinctUsersInGroups) {
		uniform_int_distribution<> unif(1, numUsers);  // user IDs start from 1
		int userID = unif(mt_rand);
		distinctUsers.insert(userID);
//		cout << "\rdistinctUsers " << distinctUsers.size();
	}
//	cout << endl;

	for (int userID : distinctUsers) {
		selectedUsers.push_back(userID);
	}

	set<int> groupChoices;
//	cout << "maxNumGroups " << maxNumGroups << endl;
	while (groupChoices.size() < numGroups) {
		uniform_int_distribution<> unif(0, maxNumGroups-1);
		int choice = unif(mt_rand);
//		cout << "choice = " << choice << endl;
		groupChoices.insert(choice);
//		cout << "\rgroupChoices " << groupChoices.size();
	}
//	cout << endl;

	int groupID = 1;
	for (int choice : groupChoices) {
//		cout << "\rchoice " << choice;
		cout.flush();
		set<int> group = decode(choice, groupSize);
		for (int encodedUser : group) {
			groupUserWeights[groupID][selectedUsers[encodedUser]] = 1; // uniform weights
		}
		groupIDs.insert(groupID);
		groupID++;
	}
//	cout << endl;

}


void Groups::assignWeights(){

	random_device rd;
	mt19937 mt_rand(rd());
	if (globalRandomSeed != -1) {
		mt_rand.seed(globalRandomSeed);
	}

	/// Type I: identical user behavior across groups
	if (userBehType == UNF || userBehType == BIP) {

		map<int, float> userBehavior; // user:weight

		int numUsers = 0;
		for (int user : distinctUsers) {
//			cout << "\rnumUsers " << numUsers++;
			uniform_int_distribution<> unif(0, 10);
			userBehavior[user] = (float) unif(mt_rand)/10.0;

			if (userBehType == BIP) { // bipolar
				bernoulli_distribution ber(1.0/groupSize);
//				bernoulli_distribution ber(0.5);
				if (ber(mt_rand)) {
					userBehavior[user] = 10;
				}
				else {
					userBehavior[user] = 0.1;
				}
			}

	//		cout << "userBehavior[" << user << "] " << userBehavior[user] << endl;
		}
//		cout << endl;

		for (auto& groupUserPair : groupUserWeights) {
			int group = groupUserPair.first;
			for (auto& UserWeightPair : groupUserPair.second) {
				int user = UserWeightPair.first;
				groupUserWeights[group][user] = userBehavior[user];
			}
		}

	}


	/// Type II: not identical user behavior across groups
	if (userBehType == RND || userBehType == RBI) {
		for (auto& groupUserPair : groupUserWeights) {
			int group = groupUserPair.first;
			for (auto& UserWeightPair : groupUserPair.second) {
				int user = UserWeightPair.first;

				if (userBehType == RND) {
					uniform_int_distribution<> unif(0, 10);
					groupUserWeights[group][user] = (float) unif(mt_rand)/10.0;
				}
				if (userBehType == RBI) {
					bernoulli_distribution ber(0.5);
					if (ber(mt_rand)) {
						groupUserWeights[group][user] = 10;
					}
					else {
						groupUserWeights[group][user] = 0.1;
					}
				}

			}
		}
	}


}


void Groups::writeGroups(){
	ofstream g_out;
	g_out.open(groupsFile.c_str(), ios::out);

	// numGroups : groupSize : numDistinctUsersInGroups : userBehType
//	g_out << numGroups << " " << groupSize << " " << numDistinctUsersInGroups << " " << userBehType << endl;

	for (auto& groupUserPair : groupUserWeights) {
		int group = groupUserPair.first;
		for (auto& UserWeightPair : groupUserPair.second) {
			int user = UserWeightPair.first;
			float weight = UserWeightPair.second;
			g_out << group << " " << user << " " << weight << endl;

		}
	}
	g_out.close();
}

void Groups::readGroups(){
	ifstream g_in(groupsFile.c_str());
	string line;

	getline(g_in, line);
	istringstream iss(line);
	// numGroups : groupSize : numDistinctUsersInGroups : userBehType
//	iss >> numGroups >> groupSize >> numDistinctUsersInGroups >> userBehType;


	while (getline(g_in, line)) {
	    istringstream iss(line);
	    int groupID, userID;
	    float weight;
		if ( !(iss >> groupID) ) {
			break;
		}
		groupIDs.insert(groupID);
		if ( !(iss >> userID) ) {
			break;
		}
		distinctUsers.insert(userID);
		if ( !(iss >> weight) ) {
			break;
		}
		groupUserWeights[groupID][userID] = weight;
	}
	g_in.close();


}

