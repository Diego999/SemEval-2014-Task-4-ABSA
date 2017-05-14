//   utils.cpp: part of sequence tagger for Wordnet supersenses, and other tagsets
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

#ifndef UTILS_NEW_H
#define UTILS_NEW_H

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
#include "../hash_stuff/hash_stuff.h"

using namespace std;
typedef double Float;


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void blip();
string my_tolower(string &s);
string my_tolower_sh(string &s,string& sh);
string itos (int arg);
int update_MAP(string s,map<string,int> &SI,vector<string> &IS);
int rand_int_0_N(int N);
void spacify(string s,vector<string> &W);
void parse_fname(string fname,string &pref,string &suff);
void parse_fname(string fname,string &D,string &F,string &S);
void parse_multifname(string fname,string &D,vector<string> &FN);
void check_fname(string fname);
string get_current_dir();
void de_underscorify(string s,vector<string> &W);
void get_entity_dist(string fname,map<string,map<string,int> >& ENTITY_DIST);

//////////////////////////////////////////////////////////////

void get_entity_dist(string fname);
void conll_extra_training_and_gaz_from_semcor(string fname);
void add_size_to_gaz_feats(string fname);
void split(string filename);
void split_by_doc(string fname);
void split_by_entity(string fname);
void bi_to_pos(string fname);
void pos_to_w(string fname);
void split_and_rank(string fname,string Yfile);
void count_w(string target, string fname,bool lower);
void entity_ratio(string fname1,string fname2,string listname,int seed);
void entity_ratio_rand(string fname1,string fname2,string listname);
void split_triggers(string fname);
void discordant_tags(string fname1,string fname2);
void discordant_tags_tokenwise(string fname1,string fname2);
void sample_sentences(int N,int M,string fname);
void print_tags_feat_file(string fname);
void entity_ratio2(string sname,string tname,string c);

//////////////////////////////////////////////////////////////

template <class KEY_T,class VAL_T> void update_map(map<KEY_T,VAL_T> &X,KEY_T x,VAL_T upd_x){
  typename map<KEY_T,VAL_T>::iterator X_i = X.find(x);
  if (X_i == X.end())
    X.insert(make_pair(x,upd_x));
  else (*X_i).second += upd_x;
}

template <class KEY_T,class VAL_T> void print_rank(ofstream &out,map<KEY_T,VAL_T> X){
  multimap<VAL_T,KEY_T> R;
  typename map<KEY_T,VAL_T>::iterator X_i = X.begin();
  double sum = 0, cum = 0;
  for (; X_i != X.end(); ++X_i){
    R.insert(make_pair((*X_i).second,(*X_i).first));
    sum += Float((*X_i).second);
  }
  typename multimap<VAL_T,KEY_T>::reverse_iterator R_i = R.rbegin();
  for (; R_i != R.rend(); ++R_i){
    double p_i = (double) (*R_i).first/sum;
    cum += p_i;
    out << (*R_i).second << "\t" << (*R_i).first << "\t" << p_i << "\t" << cum << endl;
  }
  out << "\n|R_tp| = " << R.size() << "\t|R_tk| = " << sum;
}

template <class Param>
void init_phi_b(int k, vector<vector<Param> > &phi_b, vector<string> &LIS,
        vector<string> &FIS, HM &LSI, HM &FSI){
  cerr << "\n\tinit_phi_b(" << k << "): ";
  for (int tlx = 0; tlx < k; ++tlx){
    vector<Param> _Fb;
    for (int trx = 0; trx < k; ++trx){
      Param f;
      string id_str = LIS[tlx]+"-"+LIS[trx];
      unsigned f_id = update_hmap(FSI,id_str,FIS);    f_id = 0;
      _Fb.push_back(f);
    }
    phi_b.push_back(_Fb);
  }
  cerr << " |phi_b| = " << phi_b.size() << "*" << phi_b[0].size();
}

template <class Param>
void init_phi_sf(int k, vector<Param> &phi_sf, vector<string> &LIS,
        vector<string> &FIS, HM &LSI, HM &FSI,
        bool is_s){
//  cerr << "\n\tinit_phi_sf(" << is_s << ")";
  for (int r = 0; r < k; ++r){
    string id_str = LIS[r];
    if (is_s)
      id_str = "#-"+id_str;
    else
      id_str = id_str+"-#";
    Param f;
    unsigned f_id = update_hmap(FSI,id_str,FIS);      f_id = 0;
    phi_sf.push_back(f);
  }
 // cerr << " |phi_sf| = " << phi_sf.size();
}

template <class Param>
void init_phi_w(int k, vector<hash_map<unsigned,Param> > &phi_w, int _wf_,vector<hash_set<unsigned> > &active_phi_w){
//  cerr << "\n\tphi_w(" << k << ")";
  for (int i = 0; i < k; ++i){
    hash_map<unsigned,Param> tmp;
    phi_w.push_back(tmp);
    hash_set<unsigned> stmp;
    active_phi_w.push_back(stmp);
//  cerr << "\t|phi_w| = " << phi_w.size();
  }
}

template <class Param>
void init_phi_w_mat(int k, vector<vector<Param> > &phi_w_mat, int _wf_ ){
//  cerr << "\n\tinit_phi(" << k << "," << _wf_ << "): ";
  for (int r = 0; r < k; ++r){
    vector<Param> _Fw;
    for (int i = 0; i < _wf_; ++i){
      Param f;
#if RAND_INIT
      f.rand_alpha();
#endif
      _Fw.push_back(f);
    }
    phi_w_mat.push_back(_Fw);
  }
//  cerr << " |phi_w| = " << phi_w_mat.size() << "*" << phi_w_mat[0].size();
}

#endif
