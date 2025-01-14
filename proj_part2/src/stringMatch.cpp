/*
 * stringMatch.cpp
 *
 *  Created on: May 28, 2016
 *      Author: epassos
 */

#include "stringMatch.h"


using namespace std;


/**
 * @brief utilitary funcion that returns the minimum of three ints
 * used in approximate string matching
 */
inline int min(int a, int b, int c){
	return a < b ? a < c ? a : c :  b < c ?  b : c;
}

vector<int> KMPcomputePrefix(string P) {

	vector<int> pi(P.length());
	int m, k, q;

	m = P.size();
	k = -1;
	pi[0] = -1;

	for (q = 1; q < m; q++) {						//iterate the whole string
		while (k > -1 && tolower(P[k + 1]) != tolower(P[q]))
			k = pi[k];					//gets the length of the current prefix
		if (tolower(P[k + 1]) == tolower(P[q]))		//if it finds a bigger prefix, update
			k++;
		pi[q] = k;
	}

	return pi;
}


int numStringMatching(string pattern,string text) {

	vector<int> pi(pattern.length());
	pi = KMPcomputePrefix(pattern);


	int n, m, q, ret = 0;

	n = text.length();
	m = pattern.length();
	q = -1;

	for (int i = 0; i < n; i++) {
		while (q > -1 && tolower(pattern[q+1])  != tolower(text[i])){
			q = pi[q];
		}

		if (tolower(pattern[q+1]) == tolower(text[i])){
			q++;
		}

		if (q == m-1) {
			q = pi[q];
			ret++;
		}
	}

	return ret;

}


int editDistance(string  pattern, string text){

	int **distance;
	unsigned int m = pattern.size();
	unsigned int n = text.size();

	distance = (int**) malloc (m * sizeof(int*) + 1);
	for (unsigned int i = 0; i <= m; i++){
		distance[i] = (int*) malloc (n * sizeof(int) + 1);
	}

	//initializing
	for(unsigned int i = 0; i <= m; i++){
		distance[i][0] = i;
	}

	for(unsigned int j = 0; j <= n; j++){
		distance[0][j] = j;
	}

	for(unsigned int i = 1; i <= m; i++){
		for(unsigned int j = 1; j <= n; j++){
			if(tolower(pattern[i-1]) == tolower(text[j-1]))
				distance[i][j] = distance[i-1][j-1];
			else
				distance[i][j] = 1 + min(distance[i-1][j-1],distance[i-1][j],distance[i][j-1]);
		}
	}

	return distance[pattern.size()][ text.size()];
}

