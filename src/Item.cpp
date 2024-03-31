/*
 * Item.cpp
 *
 *  Created on: Oct 6, 2017
 *      Author: dimitris
 */


#include "Item.hpp"

string Item::toString() {
	ostringstream sout;
	sout << id << ": " << "( ";
	for (float rating : ratings) {
		sout << rating << " ";
	}
	sout << ")  [ ";
	for (int rankScore : rankScores) {
		sout << rankScore << " ";
	}
	sout << "]";
	return sout.str();
}



Item& getItemByID(vector<Item>& items, int itemID) {
	for (Item& item : items) {
		if (item.id == itemID) {
			return item;
		}
	}
}
