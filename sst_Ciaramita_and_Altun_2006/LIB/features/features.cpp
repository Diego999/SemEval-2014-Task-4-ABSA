//   features.cpp: part of sequence tagger for Wordnet supersenses, and other tagsets
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

#include "features.h"

#define USE_R 0            // add specific real-valued feats to standard ones
#define USE_KF 0           // add constant feature for 
#define USE_WORDS 1        // use word/lemma features 
#define LOWERCASE 0        // lowercase all words, generalized lowercase model
#define USE_RELPOS_FEATS 0 // USe/don't relative position features
#define USE_MORPH_CACHE 1  // Use morph_caceh
const int maxspan_gaz = 10; // maximum length of gaz entries to use

void extract_feats(vector<string> &W,vector<string>& P,vector<vector<string> >& O,bool lowercase, vector<string>& LOW){
  int n = W.size(), m = P.size();
  vector<string> SH, SB, SB3, PR, PR3;
  for (int i = 0; i < n; ++i){
    string w = W[i], pos = "";
    if (lowercase)
      w = my_tolower(W[i]);
    if (m > 0)
      pos = P[i];
    string sh = "", lemma = "", sb = "", sb3="", pr = "", pr3 = "";
    lemma = my_tolower_sh(w,sh);
    if (lemma.size() > 2){
      sb = string(lemma,lemma.size()-2,2);
      pr = string(lemma,0,2);
    }
    if (lemma.size() > 3){
      sb3 = string(lemma,lemma.size()-3,3);
      pr3 = string(lemma,0,3);
    }
    if (pos != "")
      P.push_back(pos);
    SB.push_back(sb);
    SB3.push_back(sb3);
    PR.push_back(pr);
    PR3.push_back(pr3);
    LOW.push_back(lemma);
    SH.push_back(sh);
  }
  for (int i = 0; i < W.size(); ++i){
    vector<string> W_i;
#if USE_KF
    W_i.push_back("KF");
#endif
#if USE_RELPOS_FEATS
    if (i==0)
      W_i.push_back("rp=begin");
    else if (i<W.size()-1)
      W_i.push_back("rp=mid");
    else 
      W_i.push_back("rp=end");
#endif
    if (i > 0){
      if (m)
	W_i.push_back("pos-1="+P[i-1]);
#if USE_WORDS
      W_i.push_back("w-1="+LOW[i-1]);
#endif
      W_i.push_back("sh-1="+SH[i-1]);
      W_i.push_back("sb-1="+SB[i-1]);
    }
    if (i < W.size()-1){
      if (m)
	W_i.push_back("pos+1="+P[i+1]);
#if USE_WORDS
      W_i.push_back("w+1="+LOW[i+1]);
#endif
      W_i.push_back("sh+1="+SH[i+1]);
      W_i.push_back("sb+1="+SB[i+1]);
    }
#if USE_WORDS
    W_i.push_back("w="+LOW[i]);
#endif
    if (m)
      W_i.push_back("pos="+P[i]);
    W_i.push_back("sh="+SH[i]);
    W_i.push_back("pr="+PR[i]);
    W_i.push_back("pr3="+PR3[i]);
    W_i.push_back("sb="+SB[i]);
    W_i.push_back("sb3="+SB3[i]);
    O.push_back(W_i);
  }
}

void basic_feats(string targetname,bool is_bi,string mode,string gazfile,bool lowercase){
  cerr << "\nbasic_feats(tagged=" << is_bi << "," << mode << ",low=" << lowercase << ")";
  check_fname(targetname);
  ifstream in((targetname).c_str());
// #if USE_KF
//   ofstream out((targetname+"_basefeats_k").c_str());
// #else
//   ofstream out((targetname+"_basefeats").c_str());
// #endif
  gaz_server GZ;
  if (mode=="BIO"){
#if USE_MORPH_CACHE
    system("gunzip DATA/MORPH/MORPH_CACHE.gz");
    GZ.init(gazfile,maxspan_gaz,"DATA/MORPH/MORPH_CACHE");
    system("gzip DATA/MORPH/MORPH_CACHE");
#else
    GZ.init(gazfile,maxspan_gaz);
#endif
  }

  char strbuf[10000];
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      vector<string> W, P, L,LOW, WLEMMA;
      while (in.peek() == 9){
	string w = "", pos = "", tag = "";
	if (mode == "POS"){
	  in >> w;
	  W.push_back(w);
	}
	else {
	  in >> w >> pos;
	  W.push_back(w);
	  P.push_back(pos);
	}
	if (is_bi){
	  in >> tag;
	  L.push_back(tag);
	}
      }
      vector<vector<string> > O, O_gaz;
      extract_feats(W,P,O,lowercase,LOW);
      if (mode=="BIO")
	GZ.extract_feats(LOW,P,O_gaz,WLEMMA);

      cout << id;
      for (int i = 0; i < O.size(); ++i){
	cout << "\t" << O[i][0];
	for (int j = 1; j < O[i].size(); ++j)
	  cout << " " << O[i][j];
	if (mode=="BIO")
	  for (int j = 0; j < O_gaz[i].size(); ++j)
	    cout << " " << O_gaz[i][j];
	if (is_bi)
	  cout << " " << L[i];
      }
      cout << endl;
    }
    in.getline(strbuf,10000);
  }
}

/** Standardize: substitute standard score (z-score) to feature values**/
void make_MU_SIGMA(Zr& D,vector<Float>& MU,vector<Float>& SIGMA,int _F_){
  cerr << "\n\tmake_MU_SIGMA()";
  MU.resize(_F_);
  SIGMA.resize(_F_);
  vector<vector<Float> > FEATS(_F_);
  for (int i = 0; i < D.size(); ++i)
    for (int j = 0; j < D[i].size(); ++j)
      for (int r = 0; r < D[i][j].size(); ++r)
	FEATS[D[i][j][r].first].push_back(D[i][j][r].second);
  for (int i = 0; i < FEATS.size(); ++i){
    MU[i] = mean(FEATS[i]);
    if (FEATS[i].size()==1)
      SIGMA[i] == 0;
    else
      SIGMA[i] = std_err(FEATS[i],MU[i]);
    //    cerr << "\nmu=" << MU[i] << "\t" << SIGMA[i] << "\t" << FEATS[i].size();getchar();
  }
  cerr << "\t|MU| = " << MU.size() << "\t|SIGMA| = " << SIGMA.size();
}

void make_MU_SIGMA(vector<vector<vector<pair<string,Float> > > >& D,map<string,Float>& MU,map<string,Float>& SIGMA,set<string>& F_TYPES){
  cerr << "\n\tmake_MU_SIGMA_str()";
  map<string,vector<Float> > FEATS;
  vector<Float> tmp;
  for (set<string>::iterator FT_i = F_TYPES.begin(); FT_i != F_TYPES.end(); ++FT_i)
    FEATS.insert(make_pair(*FT_i,tmp));

  for (int i = 0; i < D.size(); ++i)
    for (int j = 0; j < D[i].size(); ++j)
      for (int r = 0; r < D[i][j].size(); ++r){
	map<string,vector<Float> >::iterator F_i = FEATS.find(D[i][j][r].first);
	if (F_i == FEATS.end()){cerr << "\nno feat("+D[i][j][r].first+")";abort();}
	(*F_i).second.push_back(D[i][j][r].second);
      }
  for (map<string,vector<Float> >::iterator F_i = FEATS.begin(); F_i != FEATS.end(); ++F_i){
    Float mu_i = mean((*F_i).second), sigma_i = 0;
    if (int((*F_i).second.size()) > 1)
      sigma_i = std_err((*F_i).second,mu_i);
    //    cerr << "\n" << (*F_i).first << "\t" << mu_i << "\t" << sigma;
    MU.insert(make_pair((*F_i).first,mu_i));
    SIGMA.insert(make_pair((*F_i).first,sigma_i));
  }
  cerr << "\t|MU| = " << MU.size() << "\t|SIGMA| = " << SIGMA.size();
}

void standardize_data(Zr& D,vector<Float>& MU,vector<Float>& SIGMA,int _F_){
  cerr << "\n\tstandardize_data()";
  for (int i = 0; i < D.size(); ++i)
    for (int j = 0; j < D[i].size(); ++j)
      for (int r = 0; r < D[i][j].size(); ++r){
	if (D[i][j][r].first < _F_){
	  //	  cerr << endl << i << "/" << j << "/" << r << "\t" << MU[D[i][j][r].first] << "/" << SIGMA[D[i][j][r].first];getchar();
	  if (MU[D[i][j][r].first] == 1 && SIGMA[D[i][j][r].first] == 0)
	    D[i][j][r].second = 1;             // binary feature
	  else
	    D[i][j][r].second = z_score(D[i][j][r].second,MU[D[i][j][r].first],SIGMA[D[i][j][r].first]);
	}
	else 
	  D[i][j][r].second = 0;
	//	cerr << "\t->" << D[i][j][r].second;getchar();
      }
}

void get_TF(string fname,HM& TF,int& N){
  cerr << "\n\tget_TF()";
  char strbuf[1000];
  N = 0;
  ifstream in(fname.c_str());
  while (in.peek() != -1){
    string w = "";
    in >> w;
    if (w != ""){
      update_hmap_count(TF,w);
      N += 1;
    }
  }
  cerr << "\tN = " << N << "\t|TF| = " << TF.size();
}

void get_DF(string fname,HM& DF,int& N){
  cerr << "\n\tget_DF()";
  char strbuf[1000];
  N = 0;
  ifstream in(fname.c_str());
  while (in.peek() != -1){
    set<string> s;
    string w = "";
    in >> w;
    //    cerr << "\n1\t" << w << in.peek();getchar();
    if (w != ""){
      s.insert(w);
      while (in.peek() == 32){
	in >> w;
	s.insert(w);
	//	cerr << "\n2\t" << w << in.peek();getchar();
      }
      for (set<string>::iterator s_i = s.begin(); s_i != s.end(); ++s_i)
	update_hmap_count(DF,*s_i);
      N += 1;
    }
    in.getline(strbuf,10000);
  }
  cerr << "\tN = " << N << "\t|DF| = " << DF.size();
}

void global_MU_SIGMA(string fname,HM& TF,HM& DF,int nwords,int ndocs,bool secondorder,map<string,Float>& MU,map<string,Float>& SIGMA){
  cerr << "\n\tglobal_MU_SIGMA("+fname+")";
  char strbuf[100];
  ifstream in(fname.c_str());
  vector<vector<vector<pair<string,Float> > > > D;
  set<string> F_TYPES;
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      vector<string> W;
      while (in.peek() == 32){
	string w;
	in >> w;
	W.push_back(w);
      }
      vector<vector<pair<string,Float> > > OR;
      extract_feats_R(W,TF,DF,ndocs,nwords,OR,secondorder);
      D.push_back(OR);
      for (int i = 0; i < OR.size(); ++i)
	for (int j = 0; j < OR[i].size(); ++j)
	  F_TYPES.insert(OR[i][j].first);
    }
    in.getline(strbuf,100);
  }
  cerr << "\t|F_TYPES| = " << F_TYPES.size();
  make_MU_SIGMA(D,MU,SIGMA,F_TYPES);
}

Float normalized_score(string f,Float val,map<string,Float>& MU,map<string,Float>& SIGMA){
  Float ans = 0;
  map<string,Float>::iterator MU_i = MU.find(f), S_i = SIGMA.find(f);
  if (MU_i == MU.end() || S_i == SIGMA.end()){cerr << "\nno feat("+f+")";abort();}
  if ((*MU_i).second == 1 && (*S_i).second == 0)
    ans = 1;
  else ans = z_score(val,(*MU_i).second,(*S_i).second);
  return ans;
}

void extract_feats_R(vector<string> &W,HM& TF, HM& DF,int ndocs, int nwords,vector<vector<pair<string,Float> > >& OR,bool secondorder){
  int n = W.size();
  vector<Float> DFs, TFs, RP;
  for (int i = 0; i < W.size(); ++i){
    HM::iterator TF_i = TF.find(W[i]);if (TF_i == TF.end()){cerr << "\nnotf("+W[i]+")";abort();}
    HM::iterator DF_i = DF.find(W[i]);if (DF_i == DF.end()){cerr << "\nnodf("+W[i]+")";abort();}
    Float idf_i = log(Float(ndocs)/Float(DF_i->second));
    Float trf_i = TF_i->second/Float(nwords);
    Float rp_i = i/Float(W.size());
    DFs.push_back(idf_i);
    TFs.push_back(trf_i);
    RP.push_back(rp_i);
  }
  for (int i = 0; i < W.size(); ++i){
    vector<pair<string,Float> > O;
    O.push_back(make_pair("IDF",DFs[i]));
    O.push_back(make_pair("TRF",TFs[i]));
    O.push_back(make_pair("L",W[i].size()));
    //    O.push_back(make_pair("RPS",RP[i]));
    if (i > 0){
      O.push_back(make_pair("IDF-1",DFs[i-1]));
      O.push_back(make_pair("TRF-1",TFs[i-1]));
      O.push_back(make_pair("L-1",W[i-1].size()));
    }
    if (i < W.size()-1){
      O.push_back(make_pair("IDF+1",DFs[i+1]));
      O.push_back(make_pair("TRF+1",TFs[i+1]));
      O.push_back(make_pair("L+1",W[i+1].size()));
    }
    if (secondorder){
      vector<pair<string,Float> > Oplus;
      for (int i = 0; i < O.size(); ++i)
	for (int j = i+1; j < O.size(); ++j){
	  string a = O[i].first, b = O[j].first;
	  if (a < b)
	    Oplus.push_back(make_pair(a+"-"+b,O[i].second*O[j].second));
	  else
	    Oplus.push_back(make_pair(b+"-"+a,O[i].second*O[j].second));
	}
      for (int i = 0; i < Oplus.size(); ++i)
	O.push_back(Oplus[i]);
    }
    OR.push_back(O);
  }
}

void basic_feats_R(string targetname,string uname,bool is_bi,string mode,bool secondorder,string gazfile,bool lowercase){
  cerr << "\nbasic_feats_R("+targetname+","+uname << ",tagged=" << is_bi << "," << mode << "," << gazfile << ",low=" << lowercase << ")";
  check_fname(targetname);
  HM TF, DF;
  map<string,Float> MU, SIGMA;
  int nwords = 0, ndocs = 0;
#if USE_R
  get_TF(uname,TF,nwords);
  get_DF(uname,DF,ndocs);
  global_MU_SIGMA(uname,TF,DF,nwords,ndocs,secondorder,MU,SIGMA);
#endif
  gaz_server GZ;
  if (mode=="BIO"){
#if USE_MORPH_CACHE
    system("gunzip DATA/MORPH/MORPH_CACHE.gz");
    GZ.init(gazfile,maxspan_gaz,"DATA/MORPH/MORPH_CACHE");
    system("gzip DATA/MORPH/MORPH_CACHE");
#else
    GZ.init(gazfile,maxspan_gaz);
#endif
  }

  ifstream in((targetname).c_str());
  char strbuf[10000];
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      cout << id;
      vector<string> W, POS, TAG, LOW, WLEMMA;
      while (in.peek() == 9){
	string w = "", pos = "", tag = "";
	in >> w;
	if (mode=="BIO") {
	  in >> pos;
	  POS.push_back(pos);
	}
	W.push_back(w);
	if (is_bi){
	  in >> tag;
	  TAG.push_back(tag);
	}
      }
      vector<vector<string> > O,O_gaz;
      extract_feats(W,POS,O,lowercase,LOW);
      if (mode=="BIO")
	GZ.extract_feats(LOW,POS,O_gaz,WLEMMA);
      vector<vector<pair<string,Float> > > OR;
#if USE_R
      extract_feats_R(W,TF,DF,ndocs,nwords,OR,secondorder);
#endif
      for (int i = 0; i < O.size(); ++i){
	cout << "\t" << O[i][0] << " 1";
	for (int j = 1; j < O[i].size(); ++j)
	  cout << " " << O[i][j] << " 1";
	if (mode=="BIO")
	  for (int j = 0; j < O_gaz[i].size(); ++j)
	    cout << " " << O_gaz[i][j] << " 1";
#if USE_R
	for (int r = 0; r < OR[i].size(); ++r)
	  cout << " " << OR[i][r].first << " " << normalized_score(OR[i][r].first,OR[i][r].second,MU,SIGMA);
#endif
	if (is_bi)
	  cout << " " << TAG[i] << " 1";
      }
      cout << endl;
    }
    in.getline(strbuf,10000);
  }
}

//////////////////////////
void gazetteerize_wnss_list(string fname){
  cerr << "\ngazetteerize_wnss_list("+fname+")";
  ifstream in(fname.c_str());
  char strbuf[100];
  bool nnpmarks = false;
  while (in.peek() != -1){
    set<string> tags, Ntags;
    string w, id, tag, trash;
    in >> w >> id >> tag;
    vector<string> W;
    de_underscorify(w,W);

    tags.insert(tag);
    Ntags.insert(tag);
    if (tag[0]=='n')
      nnpmarks = true;
    if (nnpmarks)
      in >> trash;
    while (in.peek()==32){
      string newtag;
      in >> trash >> newtag;
      if (nnpmarks)
	in >> trash;
      if (tags.size() < 2)
	Ntags.insert(newtag);
      tags.insert(newtag);
    }
    in.getline(strbuf,100);
    set<string>::iterator t_i = Ntags.begin();
    string ntags = *t_i++;
    for (; t_i != Ntags.end(); ++t_i)
      ntags += "-"+*t_i;
    if (w.size() > 2 && !(W.size() == 2 && (W[0]=="the" || W[0]=="a")))
      cout << w << "\tFWNSS"+itos(W.size())+"=" << tag << "\tBIFWNSS"+itos(W.size())+"=" << ntags << "\tNWNSS"+itos(W.size())+"_"+string(tag,0,3)+"=" << tags.size() << endl;//getchar();
  }
  cerr << "\tdone";
}

/////////////////// GAZ  ////////////

void load_GAZ_files(vector<string>& GAZ,string fname){
  char strbuf[100];
  ifstream in(fname.c_str());
  while (in.peek() != -1){
    string f = "";
    in >> f;
    in.getline(strbuf,100);
    if (f.size())
      GAZ.push_back(f);
  }
  cerr << "\t|G("+fname+")| = " << GAZ.size();
}

void gaz_server::update_G(string w,vector<unsigned>& tags){
  gazetter::iterator G_i = G.find(w);
  if (G_i == G.end())
    G[w] = tags;
  else 
    for (int i = 0; i < tags.size(); ++i)
      (*G_i).second.push_back(tags[i]);
}

void gaz_server::init(string fname,int n){
  if (!ready){
    cerr << "\n\tgaz_server::init("+fname+"," << n << ")";
    maxspan = n;
    vector<string> fnames;
    load_GAZ_files(fnames,fname);
    for (int i = 0; i < fnames.size(); ++i){
      check_fname(fnames[i]+".gz");
      system(("gunzip "+fnames[i]+".gz").c_str());
      ifstream in(fnames[i].c_str());
      char strbuf[1000];
      while (in.peek() != -1){
	string w;
	in >> w;
	w = my_tolower(w);
	vector<unsigned> tags;
	while (in.peek() == 9){
	  string tag;
	  in >> tag;
	  tags.push_back(update_hmap(LSI,tag,LIS));
	}
	in.getline(strbuf,1000);
	if (tags.size())
	  update_G(w,tags);
      }
      system(("gzip "+fnames[i]).c_str());
    }
    cerr << "\t|G| = " << G.size();
    ready = true;
  }
}

void gaz_server::init(string fname,int n,string mname){
  cerr << "\ngaz:init("+fname+"," << n << "," << mname+")";
  init(fname,n);
  char strbuf[1000];
  ifstream in(mname.c_str());
  while (in.peek() != -1){
    string pos = "", w, wm;
    in >> pos >> w >> wm;
    if (pos.size() > 1)
      pos = string(pos,0,2);
    string wl = my_tolower(w);
    if (wl != wm){
      morph_cache::iterator M_i = M.find(pos);
      if (M_i == M.end()){
	hash_map<std::string,std::string> tmp;
	tmp[wl] = wm;
	M[pos] = tmp;
      }
      else {
	hash_map<std::string,std::string>::iterator w_i = (*M_i).second.find(wl);
	if (w_i == (*M_i).second.end())
	  (*M_i).second[wl] = wm;
      }
    }
  }
  cerr << "\t|M| = " << M.size();
}

/// Find all instances of length up to maxspan which terminate at i
string get_wm(morph_cache& M,string wl,string pos){
  string ans =  wl;
  morph_cache::iterator M_i = M.find(pos);
  if (M_i != M.end()){
    hash_map<std::string,std::string>::iterator w_i = (*M_i).second.find(wl);
    if (w_i != (*M_i).second.end())
      ans = (*w_i).second;
  }
  return ans;
}

void gaz_server::extract_feats(vector<string>& L,vector<string>& P,vector<vector<string> >& O,vector<string>& WM){
  int _L_ = L.size();
  if (!O.size())
    O.resize(_L_);
  for (int i = 0; i < _L_; ++i){
    if (M.size())
      WM.push_back(get_wm(M,L[i],string(P[i],0,2)));
    for (int j = 0; j < maxspan; ++j)
      if (i-j >= 0){
	int lx = i-j;
	string w = L[lx], wm = "", pos = P[lx];
	if (M.size())
	  wm = WM[lx];
	++lx;
	while (lx <= i){
	  w += "_"+L[lx];
	  wm += "_"+WM[lx];
	  ++lx;
	}
	if (w.size()-j+1 > 2){
	  gazetter::iterator G_i = G.find(w);
	  if (G_i == G.end())
	    G_i = G.find(wm);
	  if (G_i != G.end()){
	    for (int r = 0; r < (*G_i).second.size(); ++r){
	      int left = i-j;
	      if (string(LIS[(*G_i).second[r]],0,4)=="TRIG")
		if (left-1 >= 0)
		  O[left-1].push_back("RX-"+LIS[(*G_i).second[r]]);
	      O[left++].push_back("B-"+LIS[(*G_i).second[r]]);
	      while (left <= i)
		O[left++].push_back("I-"+LIS[(*G_i).second[r]]);
	      if (string(LIS[(*G_i).second[r]],0,4)=="TRIG" && left < _L_)
		O[left].push_back("LX-"+LIS[(*G_i).second[r]]);
	    }
	  }
	}
      }
  }

//   for (int i = 0; i < _L_; ++i){
//     cerr << endl << L[i] << "/" << P[i];
//     for (int j = 0; j < O[i].size(); ++j)
//       cerr << "\n\t" << O[i][j];
//     getchar(); 
//   }
}

/// ESTIMATE THETA

void estimate_theta(string source,string target,string gaz){
  cerr << "\nestimate_theta("+source+","+target+","+gaz+")";
  map<string,map<string,int> > WT_src;
  int N_src = 0, N_tgt = 0;
  map<string,int> L_src, L_tgt;
  char strbuf[100];
  ifstream in(source.c_str());
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      while (in.peek() == 9){
	string w, pos, tag;
	in >> w >> pos >> tag;
	N_src++;
	update_map(L_src,tag,1);
	if (tag != "0"){
	  map<string,map<string,int> >::iterator WT_i = WT_src.find(w);
	  if (WT_i == WT_src.end()){
	    map<string,int> tmp;
	    WT_src.insert(make_pair(w,tmp));
	    WT_i = WT_src.find(w);
	  }
	  update_map((*WT_i).second,tag,1);
	}
      }
    }
    in.getline(strbuf,100);
  }

  ifstream in_tgt(target.c_str());
  while (in_tgt.peek() != -1){
    string id = "";
    in_tgt >> id;
    if (id != ""){
      while (in_tgt.peek() == 9){
	string w, pos, tag;
	in_tgt >> w >> pos >> tag;
	map<string,map<string,int> >::iterator WT_i = WT_src.find(w);
	if (WT_i != WT_src.end()){
	  for (map<string,int>::iterator L_i = (*WT_i).second.begin(); L_i != (*WT_i).second.end(); ++L_i)
	    update_map(L_tgt,(*L_i).first,1);
	}
	N_tgt++;
      }
    }
    in_tgt.getline(strbuf,100);
  }

   ofstream out("tag_dist_compare");
   out << source << "\t" << N_src << endl;
   print_rank(out,L_src);
   out << endl << target << "\t" << N_tgt << endl;
   print_rank(out,L_tgt);
}

//void aggretate_gaz
