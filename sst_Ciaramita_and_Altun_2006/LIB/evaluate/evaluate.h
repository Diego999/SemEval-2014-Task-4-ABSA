//   evaluate.h: part of sequence tagger for Wordnet supersenses, and other tagsets
//
//   Copyright 2006 Massimiliano Ciaramita (massi@alumni.brown.edu), Yasemin Altun (altun@tti-c.org)
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#ifndef EVALUATE_H
#define EVALUATE_H

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <map>
#include <cmath>
#include <vector>
#include <list>
#include <cassert>

using namespace std;
typedef double Float;

// braket types: 0 nonname  1:single entity     2:beginning     3:end   4:cont
#define none 0
#define SBrac 1
#define BBrac 2
#define EBrac 3
#define CBrac 4

class verbose_res{
 public:
  string L;
  int nobjects, nanswers, nfullycorrect;
  Float R, P, F;
  verbose_res() : L(""),nobjects(0), nanswers(0), nfullycorrect(0), R(0), P(0), F(0) {}
    verbose_res(string _L,int _no,int _na) : L(_L), nobjects(_no), nanswers(_na), nfullycorrect(0), R(0), P(0), F(0) {}
};

class ET { 
   public:
     vector<int> label;
     vector<vector<double> > score;
     int position;
     double tot_score;
     ET() : label(0), score(0) { position = 0;  tot_score = 0.0;}
     void empty_it() { position = 0; label.resize(0); score.resize(0);}
     void update( int l, vector<double> s ) {
	label.push_back(l);
	score.push_back(s);
        return;
     }
     void update_score( double s ) { tot_score += s; }
     void add_nonnames( vector<double> s ) {
 	score.push_back(s);
	label.push_back(0);
	tot_score += s[0];
        return;
     }
     void take_copy( ET &orig , double tot_val ) {
	 position = orig.position;
         int N = orig.label.size();
         label.resize(N);
	 score.resize(N);
	 for( int i = 0; i < N ; ++i ) {
	    label[i] = orig.label[i];
            int dim = orig.score[i].size();
	    score[i].resize(dim);
	    for( int j = 0; j < dim ; ++j ) 
		score[i][j] = orig.score[i][j];
	 }
	tot_score = tot_val;
        return;
     }
};

Float evaluate_sequences(vector<vector<int> > &goldS,
			 vector<vector<int> > &predicted,
			 vector<string> &LIS,
                         int num_labels, 
			 int verbose, 
			 ofstream &out);

Float evaluate_sequences(vector<vector<int> > &goldS,
			 vector<vector<int> > &predicted,
			 vector<string> &LIS,
                         int num_labels,
			 vector<verbose_res> &VR);

Float evaluate_pos(vector<vector<int> > &goldS,
		   vector<vector<int> > &predicted,
		   vector<string> &LIS ,
		   int num_labels,
		   vector<Float>& itemized);
#endif
