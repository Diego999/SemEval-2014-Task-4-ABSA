//   results.h: part of sequence tagger for Wordnet supersenses, and other tagsets
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

#ifndef RESULTS_H
#define RESULTS_H

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <set>
#include <cmath>
#include "../utils/utils.h"
#include "../stats/stats.h"
#include "../evaluate/evaluate.h"

using namespace std;
typedef double Float;

class inclass_res{
 public:
  vector<vector<Float> > M_nobj, M_nans, M_nfull, M_R, M_P, M_F; 
};

class myres{
  vector<vector<Float> > M; // matrix _CV_ x _T_
  vector<Float> MU, STD;    // mean and std_err on M
  map<string,inclass_res> M_verbose;
 public:
  void init(int x,int y);
  void init(int CV,int T,set<string> &LABELS);
  void update(int i, int j, Float upd_val);
  void update(int cv_i, int t_j, Float upd_val,vector<verbose_res> &vr);
  void stats(int i,int j);
  void print_res(string fname);
  void print_res(string fname,int CV,int T);
};

#endif
