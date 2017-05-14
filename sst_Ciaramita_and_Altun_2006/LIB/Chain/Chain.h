//   Chain.h: part of sequence tagger for Wordnet supersenses, and other tagsets
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

#ifndef CHAIN_H
#define CHAIN_H

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include <cassert>
#include <cmath>
#include <set>
#include "../utils/utils.h"

using namespace std;
typedef double Float;
typedef vector<vector<int> > L;  // T[1:n]^N
typedef vector<int>  X;		 // w_i
typedef vector<X> S;             // w[1:n]
typedef vector<S> Z;             // W[1:n]^N
typedef pair<unsigned,Float> rf; // real-valued feature
typedef vector<rf> Xr;
typedef vector<Xr> Sr;
typedef vector<Sr> Zr; 

#define Train 0
#define Decode 1


class Chain{
 public:
  int k;       				// size of state space
  bool no_special_end_transition_sym;	// seperate parameters for end transition

  // Virtual functions
  virtual Float get_phi_s( int j ) = 0; 
  virtual Float get_phi_f( int j ) = 0; 
  virtual Float get_phi_b( int i, int j ) = 0; 
  virtual Float get_word_contribution( X &x, int j, int pos ) = 0; 
  virtual Float get_word_contribution_R( Xr &x, int j, int pos ) = 0; 

  // The rest
  void  viterbi_tj( vector<vector<Float> > &delta, X &x, int t, int j,
        vector<vector<int> > &psi );
  void viterbi(S &W, vector<int> &U ) ;
  void print_viterbi_matrix(S &W, vector<int> &U,vector<string> &FIS,vector<string> &LIS);
  Float partial_viterbi( int t, int tp, vector<int> &GUESS_i,
	vector<vector<Float> > &delta );
  void backward_viterbi_tj( vector<vector<Float> > &delta, vector<double> &x,
	int t, int j, vector<vector<int> > &psi );
  void backward_viterbi( S &W );
  void viterbi_matrix_sequences(Z &EVAL_D,L &EVAL_L,vector<string> &FIS,vector<string> &LIS);
  void guess_sequences( Z &EVAL_D, L &EVAL_L );
  void guess_sequences( Z &EVAL_D, L &EVAL_L, set<int> &Index );
  void guess_sequences_check( Z &EVAL_D, L &EVAL_L, vector<string> &LIS,
	vector<string> &FIS, set<int> &Index, set<int> &visited,
	vector<vector<int> > &gold, string fname );

  // Real-valued features models
  void viterbi_R( Sr &W, vector<int> &U );
  void viterbi_R( Sr &W, vector<int> &U,vector<Float>& THETA );
  void viterbi_tj_R(vector<vector<Float> > &delta,Xr &x,int t,int j,vector<vector<int> > &psi);
  void viterbi_tj_R(vector<vector<Float> > &delta,Xr &x,int t,int j,vector<vector<int> > &psi,Float PRIOR);
  void guess_sequences_R( Zr &EVAL_D, L &EVAL_L );
  void guess_sequences_R( Zr &EVAL_D, L &EVAL_L,vector<Float>& THETA );

  Chain(): k(0), no_special_end_transition_sym(false) {}
};

Float inner( X &x, X &y );	// Move this to data files

void forward_table( vector<vector<Float> > &obser_mat, vector<Float> &phi_s,
        vector<vector<Float> > &col_phi_b, int k, int N,
        vector<vector<Float> > &delta, vector<vector<Float> > &scale );
void backward_table( vector<vector<Float> > &obser_mat, vector<Float> &phi_f,
        vector<vector<Float> > &phi_b, int k, int N,
        vector<vector<Float> > &delta, vector<vector<Float> > &scale );


#endif
