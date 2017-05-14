//   Perceptron.cpp: part of sequence tagger for Wordnet supersenses, and other tagsets
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

#ifndef PERCEPTRON_SPARSE_H
#define PERCEPTRON_SPARSE_H

#include "../Chain/Chain.h"
#include "../hash_stuff/hash_stuff.h"

/////////////////////////////////////////////


class Fs{
 public:
  Float alpha;                            // Weight associated with F
  Float avg_num;                          // Numerator of the average F-value
  int lst_upd;                            // Iteration number of last update
  Float upd_val;

  Float avg_val( int it, bool use_avg );
  void update( int it, bool update );
  void equate( Fs &c );
  void rand_alpha();

  Fs(): alpha(0),avg_num(0),lst_upd(0),upd_val(0) {}
};

///////////////////////////////////////////////

class PS_HMM : public Chain {
 public:
  vector<Fs> phi_s, phi_f;
  vector<vector<Fs> > phi_b;
  //  vector<map<int,Fs> > phi_w;
  vector<hash_map<unsigned,Fs> > phi_w;

  int it;       	 // size of state space/total number of iterations
  bool av_mod;
  bool only_active;      // when true, only use active features
  vector<hash_set<unsigned> > active_phi_w;
  Float word_weight;     // the ratio of word_contribution/label-label contr
  int cur_ex, cur_pos;   // temporary variables useful for carrying around

  // virtual functions
  Float get_phi_s( int j ) { return phi_s[j].avg_val( it, av_mod ); }
  Float get_phi_f( int j ) { return phi_f[j].avg_val( it, av_mod ); }
  Float get_phi_b( int i, int j ) { return phi_b[i][j].avg_val( it, av_mod ); }
  Float get_word_contribution( X &x, int j, int pos );

  // rest
  void init( int k_val, vector<string> &LIS, vector<string> &FIS,
	     HM &LSI, HM &FSI, bool no_spec, Float wc );
  void train( Z &W, L &T );
  void train( Z &W, L &T, set<int> &Index );
  void train_cv_check( Z &W, L &T, set<int> &Index, set<int>& visited );
  void train_on_s( S &W, vector<int> &T, bool &s_update );
  virtual void update_boundary_feats( int bU, int bT, int eU, int eT );
  virtual void update_transition_feats( int bT, int eT, int bU, int eU );
  void finalize_transition_feats( bool s_update );
  virtual void update_word_feats( X &w, int T, int U );
  virtual void finalize_update_word_feats( X &W, int T, int U, bool &s_update );
  void update( S &W, vector<int> &T, bool &s_update, vector<int> &U );

  // additional
  void SaveModel(string modelname,vector<string> &LIS,vector<string> &FIS,HM &LSI,HM &FSI,ofstream &out_log);
  void LoadModel(string modelname,vector<string> &FIS,HM &FSI,ofstream &out_log);
  void stack_decode(S &W,vector<int> &U,bool average,vector<string> &LIS,vector<string> &FIS,int mode);
  Float LM_score(int i,int j,vector<int> &seq,vector<string> &FIS,vector<string> &LIS,bool average,int N);

  // Real-valued feature HMM
  void train_R( Zr &W, L &T );
  void train_R( Zr &W, L &T, set<int> &Index );
  void train_on_s_R( Sr &W, vector<int> &T, bool &s_update );
  Float get_word_contribution_R(Xr &x,int j,int pos);
  void update_R(Sr &W,vector<int> &T,bool &s_update,vector<int> &U);
  void update_word_feats_R( Xr &w, int T, int U );
  void finalize_update_word_feats_R( Xr &W, int T, int U, bool &su );

  PS_HMM(): Chain() { it = 0; only_active = false; av_mod = true; cur_ex = -1;
                      word_weight = 1.0; }
 virtual ~PS_HMM() {} 
};

void update_feature_val(vector<hash_map<unsigned,Fs> > &phi_w, int l, int feat,
			double val, bool active, vector<set<int> > &active_phi_w );
void finalize_feature_val(vector<hash_map<unsigned,Fs> > &phi_w, int l, int feat,
			  int it, bool active, vector<set<int> > &act_phi_w, bool &s_upd);

#endif
