// stats.cpp: part of sequence tagger for Wordnet supersenses, and other tagsets
//
// Copyright (C) 2006 Massimiliano Ciaramita (massi@alumni.brown.edu)
// & Yasemin Altun (altun@tti-c.org)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "stats.h"

template <class SUM_TYPE>
Float sum_map(map<string,SUM_TYPE> &DATA){
  Float sum = 0;
  typename map<string,SUM_TYPE>::iterator D_i = DATA.begin();
  for (; D_i != DATA.end(); ++D_i)
    sum += Float((*D_i).second);
  return sum;
}

// KL(P|Q) = sum_x P(x) log P(x)/Q(x)
Float KL(map<string,Float> &P,
	 map<string,Float> &Q,
	 Float NP,
	 Float NQ,
	 Float _W_,
	 Float alpha){
  Float ans = 0;
  if (NP == 0)
    NP = sum_map(P);
  if (NQ == 0)
    NQ = sum_map(Q);
  Float denP = Float(NP)+(_W_*alpha);
  Float denQ = Float(NQ)+(_W_*alpha);
  Float p_unk = alpha/denP;
  Float q_unk = alpha/denQ;
  Float nsampledtypes = P.size();

  map<string,Float>::iterator P_i = P.begin();
  for (; P_i != P.end(); ++P_i){
    string x = (*P_i).first;
    Float p_x = ((*P_i).second+alpha)/denP;
    Float q_x = q_unk;
    map<string,Float>::iterator Q_i = Q.find(x);
    if (Q_i != Q.end())
      q_x = ((*Q_i).second+alpha)/denQ;
    if (p_x != q_x)
      ans += p_x*log2(p_x/q_x);
  }

  map<string,Float>::iterator Q_i = Q.begin();
  for (; Q_i != Q.end(); ++Q_i){
    string x  = (*Q_i).first;
    map<string,Float>::iterator P2_i = P.find(x);
    if (P2_i == P.end()){
      nsampledtypes += 1;
      Float q_x = ((*Q_i).second+alpha)/denQ; 
      Float p_x = p_unk;
      ans += p_x*log2(p_x/q_x);
    }
  }
  ans += (_W_-nsampledtypes)*(p_unk*(log2(p_unk/q_unk)));
  if (ans < 0)
    ans = 0;
  return ans;
}

////////////////////////////////////////////////////////////////////////
// CHI(P,Q) = chi-square value between P and Q

void make_O(map<string,Float> &P,
	    map<string,Float> &Q,
	    vector<vector<Float> > &O){
  //  cerr << "\n\t\tmake_O(" << P_str << "," << Q_str << ")";
  vector<string> w_index;
  Float smooth_val = 0.0;
  map<string,Float>::iterator P_i = P.begin();
  for (; P_i != P.end(); ++P_i){
    string p = (*P_i).first;
    Float cp = (Float) (*P_i).second+smooth_val, cq = smooth_val;
    map<string,Float>::iterator Q_i = Q.find(p);
    if (Q_i != Q.end())
      cq = (*Q_i).second;
    vector<Float> row;
    row.push_back(cp);
    row.push_back(cq);
    O.push_back(row);
    w_index.push_back(p);
  }
  map<string,Float>::iterator Q_i = Q.begin();
  for (; Q_i != Q.end(); ++Q_i){
    string q = (*Q_i).first;
    Float cq = (Float) (*Q_i).second+smooth_val, cp = smooth_val;
    map<string,Float>::iterator P_i = P.find(q);
    if (P_i == P.end()){
      vector<Float> row;
      row.push_back(cp);
      row.push_back(cq);
      O.push_back(row);
      w_index.push_back(q);
    }
  }
#define DEB_O 0
#if DEB_O
  cerr << "\nW\n";
  int _O_ = O.size();
  for (int i = 0; i < _O_; ++i)
    cerr << w_index[i] << "\t" << O[i][0] << "\t" << O[i][1] << endl;
  //  getchar();
#endif
}

void make_O_limit(map<string,Float> &P,
		  map<string,Float> &Q,
		  vector<vector<Float> > &O,
		  Float minf){
#define DEB_OL 0
#if DEB_OL
  cerr << "\n\t\tmake_O_limit(" << minf << ")";
  vector<string> w_index;
#endif
  map<string,Float>::iterator P_i = P.begin();
  for (; P_i != P.end(); ++P_i){
    string p = (*P_i).first;
    Float cp = (*P_i).second, cq = 0.0;
    map<string,Float>::iterator Q_i = Q.find(p);
    if (Q_i != Q.end())
      cq = (*Q_i).second;
    if (cp >= minf && cq >= minf){
      vector<Float> row;
      row.push_back(cp);
      row.push_back(cq);
      O.push_back(row);
#if DEB_OL
      w_index.push_back(p);
#endif
    }
  }

#if DEB_OL
  cerr << "\nO_limited\n";
  int _O_ = O.size();
  for (int i = 0; i < _O_; ++i)
    cerr << w_index[i] << "\t" << O[i][0] << "\t" << O[i][1] << endl;
  getchar();
#endif
}

Float chi_from_table(vector<vector<Float> > &O,
		     vector<vector<Float> > &E,
		     int &v,
		     Float &N,
		     Float &ans_G2,
		     bool deb
		     ){
  if (deb){
    cerr << "\nchi_from_table:" << "\nO:\n";
    getchar();
  }
  N = 0;
  Float ans = 0;
  ans_G2 = 0;
  vector<Float> Ei, Ej;
  int _i_ = O.size(), _j_ = O[0].size();
  v = (_i_-1)*(_j_-1);

  for (int i = 0; i < _i_; ++i){
    Float sum_j = 0;
    vector<Float> tmp;
    for (int j = 0; j < _j_; ++j){
      tmp.push_back(0);
      sum_j += O[i][j];
      N += O[i][j];
    }
    Ei.push_back(sum_j);
    E.push_back(tmp);
  }

  for (int j = 0; j < _j_; ++j){
    Float sum_i = 0;
    for (int i = 0; i < _i_; ++i)
      sum_i += O[i][j];
    Ej.push_back(sum_i);
  }

  for (int i = 0; i < _i_; ++i){
    for (int j = 0; j < _j_; ++j){
      E[i][j] = (Ei[i]/N)*(Ej[j]/N)*N;
      if (deb){
	cerr << "\n" << O[i][0] << "\t" << O[i][1] << "\t" << Ei[i] << "\t" << Ej[j]
	     << "\nO_ij = " << O[i][j] << "\tE_ij = " << E[i][j];
	//	getchar();
      }
      
      //      if (O[i][j] != E[i][j]){
      Float upd = pow(O[i][j]-E[i][j],2)/E[i][j];
      ans += upd;
      if (O[i][j] > 0)
	ans_G2 += O[i][j]*log(O[i][j]/E[i][j]);
      //      }
    }
  }
  
  ans_G2 *= 2;
  if (deb){
    cerr << "\nv = " << v << "\nN = " << N << "\nE:\n";
    for (int i = 0; i < _i_; ++i){
      for (int j = 0; j < _j_; ++j)
	cerr << E[i][j] << "\t";
      cerr << endl;
    }
    cerr << "\nchi = " << ans << "\tG2 = " << ans_G2;//getchar();
  }
  return ans;
}

Float CHI(map<string,Float> &P,map<string,Float> &Q,Float minf){
  vector<vector<Float> > O, E;
  int v;
  Float _N, G2;
  if (minf == 0)
    make_O(P,Q,O);
  else 
    make_O_limit(P,Q,O,minf);
  Float ans = chi_from_table(O,E,v,_N,G2,false);
  return ans;
}

//////////////////////////////////////////////////////////////////

Float variance(vector<Float> &X,Float mu){
  Float ans = 0;
  int _X_ = X.size();
  for (int i = 0; i < _X_; ++i)
    ans += (X[i]-mu)*(X[i]-mu);
  ans = ans/Float(_X_-1);
  return ans;
}

Float std_err(vector<Float> &X,Float mu){
  return sqrt(variance(X,mu));
}

Float mean(vector<Float> &X){
  Float ans = 0;
  int _X_ = X.size();
  for (int i = 0; i < _X_; ++i)
    ans += X[i];
  ans = ans/Float(_X_);
  return ans;
}

void Xmax(vector<Float> &X,vector<int> &MAX){
  int max_i = 0, _X_ = X.size();
  Float max_val = 0;
  for (int i = 0; i < _X_; ++i)
    if (X[i] >= max_val){
      max_i = i;
      max_val = X[i];
    }
  for (int i = 0; i < _X_; ++i)
    if (X[i] == max_val)
      MAX.push_back(i);
  //  cerr << "\n|max| = " << MAX.size();getchar();
}


