/*
 * ExpMeasurement.cpp
 *
 *  Created on: Apr 15, 2015
 *      Author: dimitris
 */


#include "ExpMeasurement.hpp"

#include <sstream>

string ExpMeasurement::getMetricString() {
	stringstream sout;

	sout << "RMSE (total) \t RMSE (group-max) \t Kendal-Tau (avg) \t Kendal-Tau (group-min) \t MAP";

	return sout.str();
}

string ExpMeasurement::getMeasurementString() {
	stringstream sout;

	sout << rmse_total << "\t" << rmse_group_max << "\t" << ktau_avg << "\t" << ktau_group_min << "\t" << MAP;

	return sout.str();
}

string ExpMeasurement::getMetricTabsString() {
	return "\t\t\t\t\t";
}

string ExpMeasurement::getMeasurementRelAtNString() {
	stringstream sout;

//	for (auto posRelevant : posRelevants) {
//		sout << posRelevant << " ";
//	}

//	for (auto& x : RelevantAtPos) {
//		sout << x.second << "\t";
//	}

	for (int val : RelevantAtPosArray) {
		sout << val << "\t";
	}

	return sout.str();
}

string ExpMeasurement::getMetricAtNString() {
	stringstream sout;

	int atN[] = { 1, 2, 3, 4, 5, 6 }; // default {5, 10, 20}

	for (int N : atN) {
		sout << "prec@" << N << " (avg) \t";
	}
//	for (int N : atN) {
//		sout << "prec@" << N << " (min) \t";
//	}
	for (int N : atN) {
		sout << "rec@" << N << " (avg) \t";
	}
//	for (int N : atN) {
//		sout << "rec@" << N << " (min) \t";
//	}
	for (int N : atN) {
		sout << "NDCG@" << N << " (avg) \t";
	}
//	for (int N : atN) {
//		sout << "NDCG@" << N << " (min) \t";
//	}

	return sout.str();
}


string ExpMeasurement::getMeasurementAtNString() {
	stringstream sout;

	int atN[] = { 1, 2, 3, 4, 5, 6 }; // default {5, 10, 20}


	// precision @
	for (int N : atN) {
		if (precAt_avg.count(N)) {
			sout << precAt_avg[N] << "\t";
		}
		else {
			sout << "---" << "\t";
		}
	}

//	for (int N : atN) {
//		if (precAt_min.count(N)) {
//			sout << precAt_min[N] << "\t";
//		}
//		else {
//			sout << "---" << "\t";
//		}
//	}


	// recall @
	for (int N : atN) {
		if (recAt_avg.count(N)) {
			sout << recAt_avg[N] << "\t";
		}
		else {
			sout << "---" << "\t";
		}
	}

//	for (int N : atN) {
//		if (recAt_min.count(N)) {
//			sout << recAt_min[N] << "\t";
//		}
//		else {
//			sout << "---" << "\t";
//		}
//	}

	// NDCG @
	for (int N : atN) {
		if (NDCGAt_avg.count(N)) {
			sout << NDCGAt_avg[N] << "\t";
		}
		else {
			sout << "---" << "\t";
		}
	}

//	for (int N : atN) {
//		if (NDCGAt_min.count(N)) {
//			sout << NDCGAt_min[N] << "\t";
//		}
//		else {
//			sout << "---" << "\t";
//		}
//	}

	return sout.str();
}

string ExpMeasurement::getMetricAtNTabsString() {
	return "\t\t\t\t\t\t\t\t\t\t\t\t\t";
}
