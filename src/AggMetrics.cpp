/*
 * AggMetrics.cpp
 *
 *  Created on: Oct 6, 2017
 *      Author: dimitris
 */


#include "AggMetrics.hpp"


AggMetrics::AggMetrics() {
}


void AggMetrics::averageOverRepetititions() {


	/// hack to compute st. dev. of KendallTau
	double s = 0;
	double ss = 0;

	for (int rep=0; rep<numRepetitions; rep++) {

		s += allMetrics[rep*numCriteria].KendalTauTopK;
		ss += allMetrics[rep*numCriteria].KendalTauTopK * allMetrics[rep*numCriteria].KendalTauTopK;

		for (int crit=0; crit<numCriteria; crit++) {
			if (rep == 0) {
				avgOverRepsMetrics.push_back( 1.0/numRepetitions * allMetrics[rep*numCriteria + crit] );
			}
			else {
				avgOverRepsMetrics[crit] = avgOverRepsMetrics[crit] + 1.0/numRepetitions * allMetrics[rep*numCriteria + crit];
			}
		}
	}

	float st_dev = sqrt(  ss /numRepetitions - s*s/(numRepetitions*numRepetitions)  );
	cout << "KendallTau mean=" << s/numRepetitions << " stdev=" << st_dev << endl;

}


void AggMetrics::computeMarginals() {
//	for (int crit=0; crit<numCriteria; crit++) {
//		if (crit == 0) {
//			avgMetrics = avgOverRepsMetrics[crit];
//			worstMetrics = avgOverRepsMetrics[crit];
//			bestMetrics = avgOverRepsMetrics[crit];
//		}
//		else {
//			avgMetrics = avgMetrics + avgOverRepsMetrics[crit];
//			worstMetrics = worst(worstMetrics, avgOverRepsMetrics[crit]);
//			bestMetrics = best(bestMetrics, avgOverRepsMetrics[crit]);
//		}
//	}
//
//	avgMetrics = 1.0/numCriteria * avgMetrics;

	//// new

	aggregateOverCriteria();

	for (int rep=0; rep<numRepetitions; rep++) {
		if (rep == 0) {
			avgMetrics = avgOverCriteriaMetrics[rep];
			worstMetrics = worstOverCriteriaMetrics[rep];
			bestMetrics = bestOverCriteriaMetrics[rep];
		}
		else {
			avgMetrics = avgMetrics + avgOverCriteriaMetrics[rep];
			worstMetrics = worstMetrics + worstOverCriteriaMetrics[rep];
			bestMetrics = bestMetrics + bestOverCriteriaMetrics[rep];
		}
	}

	avgMetrics = 1.0/numRepetitions * avgMetrics;
	worstMetrics = 1.0/numRepetitions * worstMetrics;
	bestMetrics = 1.0/numRepetitions * bestMetrics;


}


void AggMetrics::aggregateOverCriteria() {
	avgOverCriteriaMetrics.resize(numRepetitions);
	worstOverCriteriaMetrics.resize(numRepetitions);
	bestOverCriteriaMetrics.resize(numRepetitions);

	for (int rep=0; rep<numRepetitions; rep++) {
		for (int crit=0; crit<numCriteria; crit++) {
			if (crit == 0) {
				avgOverCriteriaMetrics[rep] = allMetrics[rep*numCriteria + crit];
				worstOverCriteriaMetrics[rep] = allMetrics[rep*numCriteria + crit];
				bestOverCriteriaMetrics[rep] = allMetrics[rep*numCriteria + crit];
			}
			else {
				avgOverCriteriaMetrics[rep] = avgOverCriteriaMetrics[rep] + allMetrics[rep*numCriteria + crit];
				worstOverCriteriaMetrics[rep] = worst(worstOverCriteriaMetrics[rep], allMetrics[rep*numCriteria + crit]);
				bestOverCriteriaMetrics[rep] = best(bestOverCriteriaMetrics[rep], allMetrics[rep*numCriteria + crit]);
			}

		}
		avgOverCriteriaMetrics[rep] = 1.0/numCriteria * avgOverCriteriaMetrics[rep];

	}

}


string AggMetrics::getMarginalsString(int step, int stopAt) {
	stringstream sout;
	sout << "AVG:: ";
	sout << avgMetrics.getMetricsString(step, stopAt) << endl;
	sout << "WORST:: ";
	sout << worstMetrics.getMetricsString(step, stopAt) << endl;
	sout << "BEST:: ";
	sout << bestMetrics.getMetricsString(step, stopAt) << endl;

	return sout.str();
}

