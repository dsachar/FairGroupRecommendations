/*
 * common.hpp
 *
 *  Created on: Apr 1, 2015
 *      Author: dimitris
 */

#ifndef SRC_COMMON_HPP_
#define SRC_COMMON_HPP_

#include <vector>
#include <map>
#include <cassert>
#include <tuple>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;



enum GroupAggType : int {AVG=0, MIN=1, WEI=2, ABS=3, MAX=4, PRD=5, MED=6, CMP=7, PEN=8}; // average, minimum, weighted (according to UserBeh), random user (king) has largest weight 1 others 0
enum UserBehType : int {UNF=0, BIP=1, RND=2, RBI=3}; // common and uniform, common and bipolar (0.1 or 1), random and uniform, random and bipolar

static const char * GroupAggTypeNames[] = {"AVG", "MIN", "WEI", "ABS", "MAX", "PRD"};
static const char * UserBehTypeNames[] = {"UNF", "BIP", "RND", "RBI"};

extern int globalRandomSeed;

#define FACTORS 10

typedef vector<double> Colvec;
typedef tuple<int, int, float> MatrixEntry;


inline double innerProduct(Colvec A, Colvec B) {
	double val = 0;
//	cout << "A.size()=" << A.size() << " B.size()=" << B.size() << endl;
	assert(A.size() == B.size());
	for (int i=0; i<A.size(); i++) {
		val += A[i] * B[i];
	}
	return val;
}


inline double innerProductDEBUG(Colvec A, Colvec B) {
	double val = 0;
	cout << "A.size()=" << A.size() << " B.size()=" << B.size() << endl;
	assert(A.size() == B.size());
	for (int i=0; i<A.size(); i++) {
		double prod = A[i] * B[i];
		cout << "(" << A[i] << "*" << B[i] << "=" << prod << ") ";
		val += prod;
	}
	cout << endl;
	return val;
}


inline double norm(map<int, float> vec) {
	double theNorm = 0;
	for (auto idValPair : vec) {
		float val = idValPair.second;
		theNorm += val * val;
	}
	return theNorm;
}

inline double norm(Colvec vec) {
	double theNorm = 0;
	for (auto val : vec) {
		theNorm += val * val;
	}
	return theNorm;
}

inline double norm(map<int, Colvec> matrix) {
	double theNorm = 0;
	for (auto& pair : matrix) {
		Colvec vec = pair.second;
		theNorm += norm(vec);
	}
	return theNorm;
}


inline string vecToStr (vector<float> values) {
	ostringstream os;
	for (int i=0; i<values.size(); i++) {
		os << values[i] << " ";
	}
	return os.str();
}

inline string vecToStr (vector<int> values) {
	ostringstream os;
	for (int i=0; i<values.size(); i++) {
		os << values[i] << " ";
	}
	return os.str();
}



#endif /* SRC_COMMON_HPP_ */
