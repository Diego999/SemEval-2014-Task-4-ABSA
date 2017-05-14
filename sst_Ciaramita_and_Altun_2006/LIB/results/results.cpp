//   results.cpp: part of sequence tagger for Wordnet supersenses, and other tagsets
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

#include "results.h"

void init_M(vector<vector<Float> > &M,int CV,int T){
  for (int i = 0; i < CV; ++i){
    vector<Float> tmp(T);
    M.push_back(tmp);
  }
}

void myres::init(int CV,int T){
  init_M(M,CV,T);
}

void myres::init(int CV,int T,set<string> &LABELS){
  init_M(M,CV,T);
  inclass_res ir;
  init_M(ir.M_nobj,CV,T);
  init_M(ir.M_nans,CV,T);
  init_M(ir.M_nfull,CV,T);
  init_M(ir.M_R,CV,T);
  init_M(ir.M_P,CV,T);
  init_M(ir.M_F,CV,T);
  for (set<string>::iterator L_i = LABELS.begin(); L_i != LABELS.end(); ++L_i)
    M_verbose.insert(make_pair(*L_i,ir));
}

void myres::update(int cv_i, int t_j, Float upd_val){
  M[cv_i][t_j] = upd_val;
}

void myres::update(int cv_i, int t_j, Float upd_val,vector<verbose_res> &vr){
  cerr << "\nmyres::update |vr| = " << vr.size();
  M[cv_i][t_j] = upd_val;
  int _vr_ = vr.size();
  for (int i = 0; i < _vr_; ++i){
    map<string,inclass_res>::iterator Mv_i = M_verbose.find(vr[i].L);
    if (Mv_i == M_verbose.end()){
      cerr << "\nM_verbose.find(" << vr[i].L << ") = NULL";abort();
    }
    else {
      (*Mv_i).second.M_nobj[cv_i][t_j] = vr[i].nobjects;
      (*Mv_i).second.M_nans[cv_i][t_j] = vr[i].nanswers;
      (*Mv_i).second.M_nfull[cv_i][t_j] = vr[i].nfullycorrect;
      (*Mv_i).second.M_R[cv_i][t_j] = vr[i].R;
      (*Mv_i).second.M_P[cv_i][t_j] = vr[i].P;
      (*Mv_i).second.M_F[cv_i][t_j] = vr[i].F;
    }
  }
}

void myres::stats(int CV,int T){
  MU.clear();
  STD.clear();
  for (int i = 0; i < T; ++i){
    vector<Float> RES_i;
    for (int j = 0; j < CV; ++j){
      RES_i.push_back(M[j][i]);
    }
    Float mu_i = mean(RES_i);
    Float std_i = std_err(RES_i,mu_i);
    MU.push_back(mu_i);
    STD.push_back(std_i);
  }
}

void myres::print_res(string fname){
  ofstream out(fname.c_str());
  int _M_ = M.size(), _m_ = M[0].size();
  stats(_M_,_m_);
  vector<vector<int> > MAXS;
  for (int i = 0; i < _M_; ++i){
    vector<Float> M_i;
    for (int j = 0; j < _m_; ++j){
      out << M[i][j] << " ";
      M_i.push_back(M[i][j]);
    }
    out << endl;
    vector<int> MAX_i;
    Xmax(M_i,MAX_i);
    MAXS.push_back(MAX_i);
  }
  out << endl;
  for (int i = 0; i < _M_; ++i){
    int _max_ = MAXS[i].size();
    for (int j = 0; j < _max_; ++j)
      out << MAXS[i][j] << " ";
    out << endl;
  }
  out << endl;
  for (int j = 0; j < _m_; ++j)
    out << MU[j] << " " << STD[j] << endl;  
}

void stats_M(vector<vector<Float> > &M,int t,Float &mu,Float &std){
  mu = 0, std = 0;
  vector<Float> scores_s;
  int _cv_ = M.size();
  for (int i = 0; i < _cv_; ++i)
    scores_s.push_back(M[i][t]);
  mu = mean(scores_s);
  std = std_err(scores_s,mu);
}

void myres::print_res(string fname,int CV,int T){
  cerr << "\nprint_res("+fname+")";
  ofstream out(fname.c_str());
  int _M_ = M.size(), _m_ = M[0].size();
  stats(_M_,_m_);
  vector<vector<int> > MAXS;
  for (int i = 0; i < _M_; ++i){
    vector<Float> M_i;
    for (int j = 0; j < _m_; ++j){
      out << M[i][j] << " ";
      M_i.push_back(M[i][j]);
    }
    out << endl;
    vector<int> MAX_i;
    Xmax(M_i,MAX_i);
    MAXS.push_back(MAX_i);
  }
  out << endl;
  for (int i = 0; i < _M_; ++i){
    int _max_ = MAXS[i].size();
    for (int j = 0; j < _max_; ++j)
      out << MAXS[i][j] << " ";
    out << endl;
  }
  out << endl;
  for (int j = 0; j < _m_; ++j)
    out << MU[j] << " " << STD[j] << endl;  

  /// VERBOSE ///
  out << "\nVERBOSE:\n";
  for (int i = 0; i < T; ++i){
    out << "\nT = " << i << "\n----------------\n";
    for (map<string,inclass_res>::iterator Mv_i = M_verbose.begin(); Mv_i != M_verbose.end(); ++Mv_i){
      Float mu_nobj, std_nobj, mu_nans, std_nans, mu_nfull, std_nfull, mu_R, std_R, mu_P, std_P, mu_F, std_F;
      stats_M((*Mv_i).second.M_nobj,i,mu_nobj,std_nobj);
      stats_M((*Mv_i).second.M_nans,i,mu_nans,std_nans);
      stats_M((*Mv_i).second.M_nfull,i,mu_nfull,std_nfull);
      stats_M((*Mv_i).second.M_R,i,mu_R,std_R);
      stats_M((*Mv_i).second.M_P,i,mu_P,std_P);
      stats_M((*Mv_i).second.M_F,i,mu_F,std_F);
      out << "\n" << (*Mv_i).first
	  << "\tnobj = " << mu_nobj << "/" << std_nobj
	  << "\tnans = " << mu_nans << "/" << std_nans
	  << "\tnfull = " << mu_nfull << "/" << std_nfull
	  << "\tR = " << mu_R << "/" << std_R
	  << "\tP = " << mu_P << "/" << std_P
	  << "\tF = " << mu_F << "/" << std_F << endl;
    }
  }
}
