// Tlight.cpp: part of sequence tagger for Wordnet supersenses, and other tagsets
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

#include "Tlight.h"

void tagger_light::init(string modelname,string tagsetname,string mode){
  cerr << "\ntagger_light.init("+modelname+","+tagsetname+")";
 //@JAB comment out cdir = get_current_dir();
  tagset = tagsetname;
  check_fname(tagset);
  ifstream in(tagset.c_str());
  LABELS.insert("ALL");
  char strbuf[100];
  while (in.peek() != -1){
    string l = "";
    in >> l;
    in.getline(strbuf,100);
    if (l != ""){
      update_hmap(LSI,l,LIS);
      if (l != "0" && mode != "POS")
	LABELS.insert(string(l,2,int(l.size())-2));
    }
  }
  cerr << "\t|Y| = " << LSI.size() << "/" << LIS.size();
  if (modelname != "NULL"){
    model = modelname;
    string unzip = "gunzip "+model+"*";
    system(unzip.c_str());
    check_fname(model+".F");
    check_fname(model+".PS_HMM");
    ofstream out_log;
    ps_hmm.LoadModel(modelname,FIS,FSI,out_log);
    string zip = "gzip "+model+"*";
    system(zip.c_str());
  }
}

void tagger_light::check_consistency(vector<int>& IN,vector<string>& OUT){
  int N = IN.size();
  string prev = "";
  for (int i = 0; i < N; ++i){
    string w_i = LIS[IN[i]];
    if (i > 0)
      prev = LIS[IN[i-1]];
    if (w_i[0] == 'I' && (prev == "" || prev == "0" || (prev[0] == 'I' && prev != w_i)))
      w_i[0] = 'B';
    OUT.push_back(w_i); 
  }
}

void tagger_light::encode_s(vector<vector<string> >& O_str,vector<vector<int> >& O,bool secondorder){
  //  cerr << "\ntagger_light::encode_s()";
  int _O_ = O_str.size();
  for (int i = 0; i < _O_; ++i){
    int n = O_str[i].size();
    vector<int> O_i;
    for (int j = 0; j < n; ++j){
      string a = O_str[i][j];
      O_i.push_back(update_hmap(FSI,a,FIS));
      if (secondorder){
	for (int r = j+1; r < n; ++r){
	  string b = O_str[i][r];
	  if (a < b)
	    O_i.push_back(update_hmap(FSI,a+"-"+b,FIS));
	  else O_i.push_back(update_hmap(FSI,b+"-"+a,FIS));
	}
      }
    }
    O.push_back(O_i);
  }
}

void tagger_light::addpos(vector<int>& G,vector<vector<string> >& D){
  int N = G.size(), M = D.size();
  if (N != M){cerr << "\ntagger_light::addpos -> " << N << " != " << M; abort();  }
  for (int i = 0; i < N; ++i){
    if (i > 0)
      D[i].push_back("pos-1="+LIS[G[i-1]]);
    if (i < N-1)
      D[i].push_back("pos+1="+LIS[G[i+1]]);
    D[i].push_back("pos="+LIS[G[i]]);
  }
}

void tagger_light::load_data_notag(string dname,
				   vector<vector<vector<int> > > &D,
				   vector<string> &ID,
				   bool secondorder){
  cerr << "\ntagger_light::load_data_notag("+dname+")";
  check_fname(dname);
  ifstream in((dname).c_str());
  char strbuf[10000];
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      vector<vector<int> > S;
      ID.push_back(id);
      while (in.peek() == 9){
	vector<string> O;
	vector<int> O_int;
	string f;
	in >> f;
	O.push_back(f);
	while (in.peek() == 32){
	  in >> f;
	  O.push_back(f);
	}
	int _O_ = O.size(), i = 0;
	if (_O_ < 1){
	  cerr << "\ntoo few info in id: " << id; abort();
	}
	for (; i < _O_; ++i){
	  string a = O[i];
	  O_int.push_back(update_hmap(FSI,a,FIS));
	  if (secondorder){
	    for (int j = i+1; j < _O_-1; ++j){
	      string b = O[j];
	      if (a < b)
		O_int.push_back(update_hmap(FSI,a+"-"+b,FIS));
	      else O_int.push_back(update_hmap(FSI,b+"-"+a,FIS));
	    }
	  }
	}
	S.push_back(O_int);
      }
      D.push_back(S);
    }
    in.getline(strbuf,10000);
  }
  cerr << "\t|D| = " << D.size() << "\t|ID| = " << ID.size() << "\t|FIS| = " << FIS.size();
}

void tagger_light::load_data(string dname,vector<vector<vector<int> > > &D,
			     vector<vector<int> > &G,
			     vector<string> &ID,
			     bool secondorder){
  cerr << "\ntagger_light::load_data("+dname+")";
  //  check_fname(cdir+dname);
  //  ifstream in((cdir+dname).c_str());

  check_fname(dname);
  ifstream in((dname).c_str());
  char strbuf[10000];
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      vector<int> L;
      vector<vector<int> > S;
      ID.push_back(id);
      while (in.peek() == 9){
	vector<string> O;
	vector<int> O_int;
	string f;
	in >> f;
	O.push_back(f);
	while (in.peek() == 32){
	  in >> f;
	  O.push_back(f);
	}
	int _O_ = O.size(), i = 0;
	if (_O_ < 2){
	  cerr << "\ntoo few info in id: " << id; abort();
	}
	for (; i < _O_-1; ++i){
	  string a = O[i];
	  O_int.push_back(update_hmap(FSI,a,FIS));
	  if (secondorder){
	    for (int j = i+1; j < _O_-1; ++j){
	      string b = O[j];
	      if (a < b)
		O_int.push_back(update_hmap(FSI,a+"-"+b,FIS));
	      else O_int.push_back(update_hmap(FSI,b+"-"+a,FIS));
	    }
	  }
	}
	S.push_back(O_int);
	HM::iterator LSI_i = LSI.find(O[i]);
	if (LSI_i == LSI.end()){
	  cerr << "\nunknown label = " << O[i] << " |D| = " << D.size(); abort();
	}
	L.push_back(update_hmap(LSI,O[i],LIS));
      }
      D.push_back(S);
      G.push_back(L);
    }
    in.getline(strbuf,10000);
  }
  cerr << "\t|D| = " << D.size() << "\t|G| = " << G.size() << "\t|ID| = " << ID.size() << "\t|FIS| = " << FIS.size();
}

void tagger_light::print_data_R(Zr &D,vector<vector<int> >& Labs){
  cout << "\nprint_data_R()";
  for (int i = 0; i <D.size(); ++i){
    cout << endl << i;
    for (int j = 0; j < D[i].size(); ++j){
      cout << "\n" << LIS[Labs[i][j]] << "\t";
      for (int r = 0; r < D[i][j].size(); ++r)
	cout << FIS[D[i][j][r].first] << ":" << D[i][j][r].second << " ";
    }
  }
}
			       

void tagger_light::load_data_R(string dname,
			       Zr &D,
			       vector<vector<int> > &G,
			       vector<string> &ID,
			       bool secondorder,
			       bool R,
			       bool labeledData){
  cerr << "\ntagger_light::load_data_R("+dname+")";
  //  check_fname(cdir+dname);
  //  ifstream in((cdir+dname).c_str());
  check_fname(dname);
  ifstream in((dname).c_str());
  char strbuf[10000];
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      vector<int> Labels;
      Sr SentenceReal;
      ID.push_back(id);
      while (in.peek() == 9){
	vector<string> O;
	vector<Float> O_val;
	Xr wordFeats;
	string f;
	Float f_val;
	in >> f >> f_val;
	O.push_back(f);
	O_val.push_back(f_val);
	while (in.peek() == 32){
	  in >> f >> f_val;
	  O.push_back(f);
	  O_val.push_back(f_val);
	}
	int _O_ = (labeledData) ? O.size()-1 : O.size(), i = 0;
	if (_O_ < 1){ cerr << "\ntoo few info in id: " << id << "\t|O| = " << _O_; abort();}

	for (; i < _O_; ++i){
	  rf rf_i;
	  rf_i.first = update_hmap(FSI,O[i],FIS);
	  rf_i.second = O_val[i];
	  wordFeats.push_back(rf_i);
	  if (secondorder){
	    for (int j = i+1; j < _O_-1; ++j){
	      rf rf_ij;
	      if (O[i] < O[j])
		rf_ij.first = update_hmap(FSI,O[i]+"-"+O[j],FIS);
	      else
		rf_ij.first = update_hmap(FSI,O[j]+"-"+O[i],FIS);
	      rf_ij.second = O_val[i]*O_val[j];
	      wordFeats.push_back(rf_ij);
	    }
	  }
	}
	SentenceReal.push_back(wordFeats);
	if (labeledData)
	  Labels.push_back(update_hmap(LSI,O[i],LIS));
      }
      D.push_back(SentenceReal);
      G.push_back(Labels);
    }
    in.getline(strbuf,10000);
  }
  cerr << "\t|D| = " << D.size() << "\t|G| = " << G.size() << "\t|ID| = " << ID.size() << "\t|FIS| = " << FIS.size();
}

void tagger_light::train(vector<vector<vector<int> > > &D,vector<vector<int> > &G,int T,string modelname){
  Float ww = 1.0;
  bool no_special_symbol = false;
  ps_hmm.init(int(LSI.size()),LIS,FIS,LSI,FSI, no_special_symbol, ww);
  for (int t = 1; t <= T; ++t)
    ps_hmm.train(D,G);
  ofstream out_dummy;
  ps_hmm.SaveModel(modelname,LIS,FIS,LSI,FSI,out_dummy);
}

//////////////////////////////////// EVAL

void tagger_light::eval(vector<vector<vector<int> > > &D,
			vector<vector<vector<int> > > &D2,
			vector<vector<int> > &G,
			vector<vector<int> > &G2,
			int T,
			int CV,
			string descr,
			string mode,
			Float ww,
			string thetafile){
  bool no_special_symbol = false;
  myres res;
  res.init(CV,T,LABELS);
  vector<vector<Float> > CV_T_ALL;
  vector<vector<vector<Float> > > CV_POS_RES;
  for (int cv = 0; cv < CV; ++cv){
    cerr << "\ncv = " << cv;
    vector<Float> T_ALL;
    vector<vector<Float> > POS_RES;
    PS_HMM ps_hmm_cv;
    ps_hmm_cv.init(int(LSI.size()),LIS,FIS,LSI,FSI,no_special_symbol, ww);
    for (int t = 0; t < T; ++t){
      ps_hmm_cv.train(D,G);
      vector<vector<int> > tst_guess;
      ps_hmm_cv.guess_sequences(D2,tst_guess);
      Float tst_f = 0;      
      if (mode == "POS"){
	vector<Float> itemized_res;
	tst_f = evaluate_pos(G2,tst_guess,LIS,LIS.size(),itemized_res);
	POS_RES.push_back(itemized_res);
	T_ALL.push_back(tst_f);
      }
      else {
	vector<verbose_res> tst_vr;
	tst_f = evaluate_sequences(G2,tst_guess,LIS,LIS.size(),tst_vr);
	res.update(cv,t,tst_f,tst_vr);
      }
      cerr << "\ttst = " << tst_f;
    }
    CV_T_ALL.push_back(T_ALL);
    CV_POS_RES.push_back(POS_RES);
  }
  if (mode != "POS")
    res.print_res(descr+".eval",CV,T);
  //    res.print_res("tmp.eval",CV,T);
  else {
    ofstream out((descr+".eval_pos").c_str());
    //    ofstream out("tmp.eval_pos");
    
    for (int j = 0; j < T; ++j){    
      vector<Float> all_j;
      for (int i = 0; i < CV; ++i)
	all_j.push_back(CV_T_ALL[i][j]);
      Float mu_all_j = mean(all_j);
      Float err_all_j = std_err(all_j,mu_all_j);
      out << mu_all_j << "\t" << err_all_j << endl;
    }

    out << endl << endl;
    for (int r = 0; r < LIS.size(); ++r)
      out << LIS[r] << " ";
    out << endl;

    for (int j = 0; j < T; ++j){
      for (int r = 0; r < LIS.size(); ++r){
	vector<Float> pos_tj;
	for (int i = 0; i < CV; ++i)
	  pos_tj.push_back(CV_POS_RES[i][j][r]);
	Float mu_tj = mean(pos_tj);
	out << mu_tj << " ";
      }
      out << endl;
    } 
  }
}

//////////////////////////////////// EVAL_R
// #define PRINT_AGGR_TAGDIST 0
// #if PRINT_AGGR_TAGDIST
//       Float den_dist = 0;
//       map<string,int> tagdist;
//       for (int i = 0; i < tst_guess.size(); ++i)
// 	for (int j = 0; j < tst_guess[i].size(); ++j){
// 	  update_map(tagdist,LIS[tst_guess[i][j]],1);
// 	  den_dist += 1;
// 	}
//       cerr << "\n\tTAGDIST (den=" << den_dist << ") = ";
//       for(map<string,int>::iterator td_i = tagdist.begin(); td_i != tagdist.end(); ++td_i)
// 	cerr << "\n\t" << (*td_i).first << " " << (*td_i).second << "/" << (*td_i).second/den_dist << " ";
//       cerr << endl;
// #endif

#define BOOTSTRAP 0

void tagger_light::eval_R(Zr &D,Zr& D2,
			  vector<vector<int> > &G,
			  vector<vector<int> > &G2,
			  int T,
			  int CV,
			  string descr,
			  string mode,
			  Float ww,
			  string thetafile){
  bool no_special_symbol = false;
  myres res;
  res.init(CV,T,LABELS);
  vector<vector<Float> > CV_T_ALL;
  vector<vector<vector<Float> > > CV_POS_RES;

  vector<Float> THETA;
  ifstream in_theta(thetafile.c_str());
  char strbuf[100];
  if (in_theta.is_open()){
    THETA.resize(LSI.size());
    while (in_theta.good()){
      string y_str = "";
      Float theta_y = 0;
      in_theta >> y_str >> theta_y;
      if (y_str != ""){
	HM::iterator LSI_i = LSI.find(y_str);
	THETA[(*LSI_i).second] = theta_y;
      }
      in_theta.getline(strbuf,100);
    }
  }
  cerr << "\nTHETA = {";
  for (int  i = 0; i < THETA.size(); ++i)
    cerr << " " << LIS[i] << ":" << THETA[i];
  cerr << " }";

#if BOOTSTRAP
  int _DEV_ = D2.size();
  vector<Zr> D2_vec(CV);
  vector<vector<vector<int> > > G2_vec(CV);
  for (int i = 0; i < CV; ++i)
    for (int j = 0; j < _DEV_; ++j){
      int s = int(floor((rand()/Float(RAND_MAX))*_DEV_));
      D2_vec[i].push_back(D2[s]);
      G2_vec[i].push_back(G2[s]);
    }
  PS_HMM ps_hmm_cv;
  ps_hmm_cv.init(int(LSI.size()),LIS,FIS,LSI,FSI,no_special_symbol, ww);
  for (int t = 0; t < T; ++t){
    cerr << "\nt = " << t;
    ps_hmm_cv.train_R(D,G);
    Float avg_t = 0;
    for (int cv = 0; cv < CV; ++cv){
      vector<vector<int> > tst_guess;
      if (THETA.size())
	ps_hmm_cv.guess_sequences_R(D2_vec[cv],tst_guess,THETA);
      else
	ps_hmm_cv.guess_sequences_R(D2_vec[cv],tst_guess);
      Float tst_f = 0;      
      vector<verbose_res> tst_vr;
      tst_f = evaluate_sequences(G2_vec[cv],tst_guess,LIS,LIS.size(),tst_vr);
      res.update(cv,t,tst_f,tst_vr);
      cerr << "\ttst[" << cv << "] = " << tst_f;
      avg_t += tst_f;
    }
    cerr << "\tavg[" << t << "] = " << avg_t/Float(CV);
  }
  
#else
  for (int cv = 0; cv < CV; ++cv){
    cerr << "\ncv = " << cv;
    vector<Float> T_ALL;
    vector<vector<Float> > POS_RES;
    PS_HMM ps_hmm_cv;
    ps_hmm_cv.init(int(LSI.size()),LIS,FIS,LSI,FSI,no_special_symbol, ww);
    for (int t = 0; t < T; ++t){
      ps_hmm_cv.train_R(D,G);
      vector<vector<int> > tst_guess;
      if (THETA.size())
	ps_hmm_cv.guess_sequences_R(D2,tst_guess,THETA);
      else
	ps_hmm_cv.guess_sequences_R(D2,tst_guess);
      Float tst_f = 0;      
      if (mode == "POS"){
	vector<Float> itemized_res;
	tst_f = evaluate_pos(G2,tst_guess,LIS,LIS.size(),itemized_res);
	POS_RES.push_back(itemized_res);
	T_ALL.push_back(tst_f);
      }
      else {
	vector<verbose_res> tst_vr;
	tst_f = evaluate_sequences(G2,tst_guess,LIS,LIS.size(),tst_vr);
	res.update(cv,t,tst_f,tst_vr);
      }
      cerr << "\ttst = " << tst_f;
    }
    CV_T_ALL.push_back(T_ALL);
    CV_POS_RES.push_back(POS_RES);
  }
#endif
  if (mode != "POS")
#if BOOTSTRAP
    res.print_res(descr+".eval_boot",CV,T);
#else
    res.print_res(descr+".eval",CV,T);
#endif
  else {
    ofstream out((descr+".eval_pos").c_str());
    
    for (int j = 0; j < T; ++j){    
      vector<Float> all_j;
      for (int i = 0; i < CV; ++i)
	all_j.push_back(CV_T_ALL[i][j]);
      Float mu_all_j = mean(all_j);
      Float err_all_j = std_err(all_j,mu_all_j);
      out << mu_all_j << "\t" << err_all_j << endl;
    }

    out << endl << endl;
    for (int r = 0; r < LIS.size(); ++r)
      out << LIS[r] << " ";
    out << endl;

    for (int j = 0; j < T; ++j){
      for (int r = 0; r < LIS.size(); ++r){
	vector<Float> pos_tj;
	for (int i = 0; i < CV; ++i)
	  pos_tj.push_back(CV_POS_RES[i][j][r]);
	Float mu_tj = mean(pos_tj);
	out << mu_tj << " ";
      }
      out << endl;
    }
  }
}

////////////////////////////////////

void tagger_light::tag_online(vector<vector<string> > &words, vector<string> &tags,bool secondorder){
  if (!initialized){
    cerr << "\ntagger not initialized: error";abort();
  }
  vector<vector<int> > encoded_words;
  int _w_ = words.size();
  for (int i = 0; i < _w_; ++i){
    vector<int> wi;
    int _wi_ = words[i].size();
    for (int j = 0; j < _wi_; ++j){
      string a = words[i][j];
      wi.push_back(update_hmap(FSI,a,FIS));
      if (secondorder){
	for (int r = j+1; r < _wi_; ++r){
	  string b = words[i][r];
	  if (a < b)
	    wi.push_back(update_hmap(FSI,a+"-"+b,FIS));
	  else wi.push_back(update_hmap(FSI,b+"-"+a,FIS));
	}
      }
    }
    encoded_words.push_back(wi);
  }
  vector<int> U;
  ps_hmm.viterbi(encoded_words,U);
  for (int i = 0; i < _w_; ++i)
    tags.push_back(LIS[i]);
}


/// decode a sequence of integer features into the strings
void tagger_light::decode_tags(vector<int> &  O_int_tags, vector<string>   & O_str_tags) {
  for(int i=0;i<O_int_tags.size();++i) {
    O_str_tags.push_back(LIS[O_int_tags[i]]);
  }
}


/**
 tags a sequence of words representated by O_str_features and sets the resulting tags in result.
 if secondorder is set to true second order methos is tried.
 if BItag is true teh B/I sequences are check
 if addFeature is true the resulting tags are added as a new feature in the festure vector
**/ 
void tagger_light::tagSemSequence(vector<vector<string> > & O_str_features, vector <string> & result, const bool secondorder, const bool BItag,const bool addFeature) { 
      vector<vector<int> > O_int_features;
      vector <int> buffRes;
      encode_s(O_str_features,O_int_features,secondorder);
      ps_hmm.viterbi(O_int_features,buffRes);
      if(addFeature)  addpos(buffRes,O_str_features);
      if(BItag) check_consistency(buffRes,result); 
      else decode_tags(buffRes,result);
}

