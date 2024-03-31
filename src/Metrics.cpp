/*
 * Metrics.cpp
 *
 *  Created on: Oct 6, 2017
 *      Author: dimitris
 */

#include "Metrics.hpp"

string Metrics::getMetricsString(int step, int stopAt) {
	stringstream sout;

	sout << "KendalTau=" << KendalTau << " ";
	sout << "KendalTauTopK=" << KendalTauTopK << " ";
	sout << "AP=" << AP << " ";
	sout << "NDCG@2=" << NDCG[2] << " ";
	sout << "NDCG@5=" << NDCG[5] << " ";
	sout << "NDCG@10=" << NDCG[10] << " ";
	sout << "NDCG@20=" << NDCG[20] << " ";
	sout << "P@2=" << prec[2] << " ";
	sout << "R@2=" << rec[2] << " ";
	sout << "Fair@5=" << fairness[5] << " ";
	sout << "BRS@5=" << avgBestRankScore[5] << " ";
	sout << "BRS@10=" << avgBestRankScore[10] << " ";
	sout << "BRS@20=" << avgBestRankScore[20] << " ";
	sout << "WRS@5=" << avgWorstRankScore[5] << " ";
	sout << "WRS@10=" << avgWorstRankScore[10] << " ";
	sout << "WRS@20=" << avgWorstRankScore[20] << " ";
	sout << "WRS@50=" << avgWorstRankScore[50] << " ";
	sout << "mNG@5=" << minNumGood[5] << " ";
	sout << "mNG@10=" << minNumGood[10] << " "; // << ":" << maxNumGood[10] << "] ";
	sout << "mNG@20=" << minNumGood[20] << " ";
	sout << "mNG@50=" << minNumGood[50] << " ";
	sout << "sum_utility=" << sum_utility << " ";
	sout << "min_utility=" << min_utility << " ";
	sout << "max_utility=" << max_utility << " ";
	sout << "variance=" << variance << " ";
	sout << "jain_fairness=" << jain_fairness << " ";
	sout << "minmax=" << minmax << " ";


//	sout << "precision: ";
//	for (auto rankValue : prec) {
//		if (stopAt > 0 && rankValue.first > stopAt) break;
//		if (rankValue.first % step != 0) continue;
//		sout << rankValue.second << " ";
//	}
//	sout << endl;
//
//	sout << "recall: ";
//	for (auto rankValue : rec) {
//		if (stopAt > 0 && rankValue.first > stopAt) break;
//		if (rankValue.first % step != 0) continue;
//		sout << rankValue.second << " ";
//	}
//	sout << endl;
//
//	sout << "NDCG: ";
//	for (auto rankValue : NDCG) {
//		if (stopAt > 0 && rankValue.first > stopAt) break;
//		if (rankValue.first % step != 0) continue;
//		sout << rankValue.second << " ";
//	}
//	sout << endl;

	return sout.str();
}

string Metrics::getMetricsResultString() {
	stringstream sout;
	sout << KendalTau << "\t" << KendalTauTopK << "\t" << AP << "\t" << NDCG[1] << "\t" << NDCG[2] << "\t" << NDCG[5] << "\t" << NDCG[10] << "\t" << NDCG[15] << "\t" << NDCG[20] << "\t" << prec[1] << "\t" << prec[2] << "\t" << prec[5] << "\t" << prec[10] << "\t" << prec[15] << "\t" << prec[20] << "\t" << sum_utility << "\t" << min_utility << "\t" << max_utility << "\t" << variance << "\t" << jain_fairness << "\t" << minmax;
	return sout.str();
}


string Metrics::getMetricsResultHeaderString() {
	stringstream sout;
	sout << "KendalTau \t KendalTauTopK \t MAP \t NDCG[1] \t NDCG[2] \t NDCG[5] \t NDCG[10] \t NDCG[15] \t NDCG[20] \t prec[1] \t prec[2] \t prec[5] \t prec[10] \t prec[15] \t prec[20] \t sum_utility \t min_utility \t max_utility \t variance \t jain_fairness \t minmax";
	return sout.str();
}

string Metrics::getMetricsResultFillerString() {
	return "\t \t \t \t \t \t \t \t \t \t \t \t \t \t \t \t \t \t \t \t";
}


Metrics Metrics::operator+(const Metrics& other) {
	Metrics res;
	res.KendalTau = KendalTau + other.KendalTau;
	res.KendalTauTopK = KendalTauTopK + other.KendalTauTopK;
	res.AP = AP + other.AP;

	res.sum_utility = sum_utility + other.sum_utility;
	res.min_utility = min_utility + other.min_utility;
	res.max_utility = max_utility + other.max_utility;
	res.variance = variance + other.variance;
	res.jain_fairness = jain_fairness + other.jain_fairness;
	res.minmax = minmax + other.minmax;

	assert(prec.size() == other.prec.size());
	for (auto rankValue : prec) {
		res.prec[rankValue.first] = rankValue.second + other.prec.at(rankValue.first);
	}

	assert(rec.size() == other.rec.size());
	for (auto rankValue : rec) {
		res.rec[rankValue.first] = rankValue.second + other.rec.at(rankValue.first);
	}

	assert(NDCG.size() == other.NDCG.size());
	for (auto rankValue : NDCG) {
		res.NDCG[rankValue.first] = rankValue.second + other.NDCG.at(rankValue.first);
	}

	assert(fairness.size() == other.fairness.size());
	for (auto rankValue : fairness) {
		res.fairness[rankValue.first] = rankValue.second + other.fairness.at(rankValue.first);
	}

	assert(avgBestRankScore.size() == other.avgBestRankScore.size());
	for (auto rankValue : avgBestRankScore) {
		res.avgBestRankScore[rankValue.first] = rankValue.second + other.avgBestRankScore.at(rankValue.first);
	}

	assert(avgWorstRankScore.size() == other.avgWorstRankScore.size());
	for (auto rankValue : avgWorstRankScore) {
		res.avgWorstRankScore[rankValue.first] = rankValue.second + other.avgWorstRankScore.at(rankValue.first);
	}

	assert(minNumGood.size() == other.minNumGood.size());
	for (auto rankValue : minNumGood) {
		res.minNumGood[rankValue.first] = rankValue.second + other.minNumGood.at(rankValue.first);
	}

	return res;
}


Metrics operator* (float x, const Metrics& that) {
	Metrics res;
	res.KendalTau = x * that.KendalTau;
	res.KendalTauTopK = x * that.KendalTauTopK;
	res.AP = x * that.AP;

	res.sum_utility = x * that.sum_utility;
	res.min_utility = x * that.min_utility;
	res.max_utility = x * that.max_utility;
	res.variance = x * that.variance;
	res.jain_fairness = x * that.jain_fairness;
	res.minmax = x * that.minmax;


	for (auto rankValue : that.prec) {
		res.prec[rankValue.first] = x * rankValue.second;
	}

	for (auto rankValue : that.rec) {
		res.rec[rankValue.first] = x * rankValue.second;
	}

	for (auto rankValue : that.NDCG) {
		res.NDCG[rankValue.first] = x * rankValue.second;
	}

	for (auto rankValue : that.fairness) {
		res.fairness[rankValue.first] = x * rankValue.second;
	}

	for (auto rankValue : that.avgBestRankScore) {
		res.avgBestRankScore[rankValue.first] = x * rankValue.second;
	}

	for (auto rankValue : that.avgWorstRankScore) {
		res.avgWorstRankScore[rankValue.first] = x * rankValue.second;
	}

	for (auto rankValue : that.minNumGood) {
		res.minNumGood[rankValue.first] = x * rankValue.second;
	}

	return res;
}


Metrics worst (const Metrics& one, const Metrics& other) {
	Metrics res;
	res.KendalTau = min(one.KendalTau, other.KendalTau);
	res.KendalTauTopK = min(one.KendalTauTopK, other.KendalTauTopK);
	res.AP = min(one.AP, other.AP);

	res.sum_utility = min(one.sum_utility, other.sum_utility);
	res.min_utility = min(one.min_utility, other.min_utility);
	res.max_utility = min(one.max_utility, other.max_utility);
	res.variance = min(one.variance, other.variance);
	res.jain_fairness = min(one.jain_fairness, other.jain_fairness);
	res.minmax = min(one.minmax, other.minmax);

	assert(one.prec.size() == other.prec.size());
	for (auto rankValue : one.prec) {
		int key = rankValue.first;
		res.prec[key] = min(one.prec.at(key), other.prec.at(key));
	}

	assert(one.rec.size() == other.rec.size());
	for (auto rankValue : one.rec) {
		int key = rankValue.first;
		res.rec[key] = min(one.rec.at(key), other.rec.at(key));
	}

	assert(one.NDCG.size() == other.NDCG.size());
	for (auto rankValue : one.NDCG) {
		int key = rankValue.first;
		res.NDCG[key] = min(one.NDCG.at(key), other.NDCG.at(key));
	}

	assert(one.fairness.size() == other.fairness.size());
	for (auto rankValue : one.fairness) {
		int key = rankValue.first;
		res.fairness[key] = min(one.fairness.at(key), other.fairness.at(key));
	}

	assert(one.avgBestRankScore.size() == other.avgBestRankScore.size());
	for (auto rankValue : one.avgBestRankScore) {
		int key = rankValue.first;
		res.avgBestRankScore[key] = min(one.avgBestRankScore.at(key), other.avgBestRankScore.at(key));
	}

	assert(one.avgWorstRankScore.size() == other.avgWorstRankScore.size());
	for (auto rankValue : one.avgWorstRankScore) {
		int key = rankValue.first;
		res.avgWorstRankScore[key] = min(one.avgWorstRankScore.at(key), other.avgWorstRankScore.at(key));
	}

	assert(one.minNumGood.size() == other.minNumGood.size());
	for (auto rankValue : one.minNumGood) {
		int key = rankValue.first;
		res.minNumGood[key] = min(one.minNumGood.at(key), other.minNumGood.at(key));
	}

	return res;
}

Metrics best (const Metrics& one, const Metrics& other) {
	Metrics res;
	res.KendalTau = max(one.KendalTau, other.KendalTau);
	res.KendalTauTopK = max(one.KendalTauTopK, other.KendalTauTopK);
	res.AP = max(one.AP, other.AP);

	res.sum_utility = max(one.sum_utility, other.sum_utility);
	res.min_utility = max(one.min_utility, other.min_utility);
	res.max_utility = max(one.max_utility, other.max_utility);
	res.variance = max(one.variance, other.variance);
	res.jain_fairness = max(one.jain_fairness, other.jain_fairness);
	res.minmax = max(one.minmax, other.minmax);

	assert(one.prec.size() == other.prec.size());
	for (auto rankValue : one.prec) {
		int key = rankValue.first;
		res.prec[key] = max(one.prec.at(key), other.prec.at(key));
	}

	assert(one.rec.size() == other.rec.size());
	for (auto rankValue : one.rec) {
		int key = rankValue.first;
		res.rec[key] = max(one.rec.at(key), other.rec.at(key));
	}

	assert(one.NDCG.size() == other.NDCG.size());
	for (auto rankValue : one.NDCG) {
		int key = rankValue.first;
		res.NDCG[key] = max(one.NDCG.at(key), other.NDCG.at(key));
	}

	assert(one.fairness.size() == other.fairness.size());
	for (auto rankValue : one.fairness) {
		int key = rankValue.first;
		res.fairness[key] = max(one.fairness.at(key), other.fairness.at(key));
	}

	assert(one.avgBestRankScore.size() == other.avgBestRankScore.size());
	for (auto rankValue : one.avgBestRankScore) {
		int key = rankValue.first;
		res.avgBestRankScore[key] = max(one.avgBestRankScore.at(key), other.avgBestRankScore.at(key));
	}

	assert(one.avgWorstRankScore.size() == other.avgWorstRankScore.size());
	for (auto rankValue : one.avgWorstRankScore) {
		int key = rankValue.first;
		res.avgWorstRankScore[key] = max(one.avgWorstRankScore.at(key), other.avgWorstRankScore.at(key));
	}

	assert(one.minNumGood.size() == other.minNumGood.size());
	for (auto rankValue : one.minNumGood) {
		int key = rankValue.first;
		res.minNumGood[key] = max(one.minNumGood.at(key), other.minNumGood.at(key));
	}

	return res;
}

