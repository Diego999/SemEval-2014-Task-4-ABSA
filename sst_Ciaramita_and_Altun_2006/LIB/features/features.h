//   features.h: part of sst-light
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

#ifndef FEATURES_H
#define FEATURES_H

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <cassert>
#include <cmath>
#include "../utils/utils.h"
#include "../hash_stuff/hash_stuff.h"
#include "../tagger_light/Tlight.h"

#define USE_MORPH 0  // use morph map from Wordnet
#if USE_MORPH
#include "../wnlemma/wn_morph.h"
#endif

typedef hash_map<std::string,vector<unsigned> > gazetter;
typedef hash_map<std::string,hash_map<std::string,std::string> > morph_cache;

/* class morph_server{ */
/*   ready; */
/*  public: */
/*   morph_cache M; */
/*   morph_server() : ready(false) {} */
/* }; */

class gaz_server{
  bool ready;
  int maxspan;
 public:
  morph_cache M;
  HM LSI;
  vector<string> LIS;
  gazetter G;
  void init(string fname,int n);
  void init(string fname,int n,string mname);
  void update_G(string w,vector<unsigned>& tags);
  void extract_feats(vector<string>& L,vector<string>& P,vector<vector<string> >& O,vector<string>& WM);
  gaz_server() : ready(false), maxspan(0) {}
};

void basic_feats(string targetname,bool is_bi,string mode,string gazfile,bool lowercase);
void basic_feats_R(string targetname,string uname,bool is_bi,string mode,bool secondorder,string gazfile,bool lowercase);
void gazetteerize_wnss_list(string fname);
void extract_feats_R(vector<string> &W,HM& TF, HM& DF,int ndocs, int nwords,vector<vector<pair<string,Float> > >& OR,bool secondorder);
void extract_feats(vector<string> &W,vector<string>& P,vector<vector<string> >& O,bool lowercase,vector<string>& LOW);
void make_MU_SIGMA(Zr& D,vector<Float>& MU,vector<Float>& SIGMA,int _F_); 
void standardize_data(Zr& D,vector<Float>& MU,vector<Float>& SIGMA,int _F_);
void make_MU_SIGMA(vector<vector<vector<pair<string,Float> > > >& D,map<string,Float>& MU,map<string,Float>& SIGMA,set<string>& F_TYPES);
void estimate_theta(string source,string target,string gaz);

#endif
