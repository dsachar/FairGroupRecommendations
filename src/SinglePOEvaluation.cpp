/*
 * SinglePOEvaluation.cpp
 *
 *  Created on: Oct 6, 2017
 *      Author: dimitris
 */


#include "SinglePOEvaluation.hpp"


SinglePOEvaluation::SinglePOEvaluation(POEvaluation &poe) : poe(poe) {
}


void SinglePOEvaluation::evaluateMethodsOnCriteria() {

	Evaluator eval;

	for (int i=0; i<poe.numMethods; i++) { // evaluate each method
//		cout << "eval method " << poe.methodNames[i] << endl;
		map<int, float> estim = methods[i];
		for (int crit=0; crit<poe.numCriteria; crit++) { // on each criterion
//			cout << "on criterion " << poe.criteriaNames[crit] << endl;
			map<int, float> truth = criteria[crit];

			eval = Evaluator(truth, estim, poe.numRelevant);
			eval.computeRankingMetrics();
			eval.computeFairness(poe.po.items, poe.po.users.size(), poe.numItems);
			eval.computeSatisfactionFunctions(poe.po.items, poe.po.users.size(), poe.numItems);
//			cout << "evaluation done" << endl;
			poe.methodAggMetrics[i].allMetrics.push_back(eval.metrics);
		}

	}

}
