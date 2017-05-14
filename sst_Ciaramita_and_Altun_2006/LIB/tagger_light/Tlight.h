//  Tlight.h: part of sst-light
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

#ifndef TLIGHT_H
#define TLIGHT_H

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
#include "../results/results.h"
#include "../Perceptron/Perceptron.h"
#include "../utils/utils.h"
#include "../hash_stuff/hash_stuff.h"
#include "../Chain/Chain.h"
#include "../stats/stats.h"

using namespace std;
typedef double Float;

class wpt{
  string w, pos, tag;
  wpt() : w(""), pos(""), tag("") {}
};

class bi_sentence{
 public:
  string id;
  vector<wpt> info;
  bi_sentence() : id("") {}
};

class tagger_light{
 private:
  bool initialized;
 public:
  set<string> LABELS;
  
  HM LSI, FSI;             // labels string to uns int  
  vector<string> LIS, FIS; // labels int to string 
  
  //string cdir;
  string tagset, model;
  PS_HMM ps_hmm;
  
  void init(string modelname,string tagsetname,string mode);
  void load_data(string dname,vector<vector<vector<int> > > &D,
		 vector<vector<int> > &G,vector<string> &ID,bool secondorder);
  void load_data_notag(string dname,vector<vector<vector<int> > > &D,vector<string> &ID,bool secondorder);
  void train(vector<vector<vector<int> > > &D,vector<vector<int> > &G,int T,string modelname);
  void eval(vector<vector<vector<int> > > &D,vector<vector<vector<int> > > &D2,
	    vector<vector<int> > &G,vector<vector<int> > &G2,int T,int CV,string descr,string mode,Float ww,string thetafile);
  void tag_online(vector<vector<string> > &words, vector<string> &tags,bool secondorder);
  void encode_s(vector<vector<string> >& O_str,vector<vector<int> >& O,bool secondorder);
  void addpos(vector<int>& G,vector<vector<string> >& D);
  void check_consistency(vector<int>& IN,vector<string>& OUT);
  void load_data_R(string dname,Zr &D,vector<vector<int> > &G,vector<string> &ID,
		   bool secondorder,bool R,bool labeledData);
  void print_data_R(Zr &D,vector<vector<int> >& Labs);
  void eval_R(Zr &D,Zr& D2,vector<vector<int> > &G,vector<vector<int> > &G2,int T,int CV,string descr,string mode,Float ww,string thetafile);
  void tagSemSequence(vector<vector<string> > & O_str_features, vector <string> & results, const bool secondorder, 
		      const bool BItag,const bool addFeature);
  void decode_tags(vector<int>  & O_int_tags, vector<string>   & O_str_tags);
  tagger_light() : initialized(0), tagset("NULL"), model("NULL") {}
   // tagger_light() : initialized(0), cdir("NULL"), tagset("NULL"), model("NULL") {}
};

void tagSemSequence(vector<vector<string> > & O_str_features, vector <string> & results, const bool secondorder, const bool BItag,const bool addFeature);
void decode_tags(vector<int>  & O_int_tags, vector<string>   & O_str_tags);





#endif
