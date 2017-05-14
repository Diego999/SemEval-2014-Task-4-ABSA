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

#include "utils.h"

void blip(){
  cerr << "\nBLLIP!";getchar();
}

string my_tolower_sh(string &s,string& sh){
  string ans = "";
  sh = "";
  string::iterator s_i = s.begin();
  char old_char = '#';
  for (; s_i != s.end(); ++s_i){
    char c = *s_i;
    ans += tolower(c);

    char rg_c;
    if (isalpha(c))
      if (isupper(c))
	rg_c = 'X';
      else rg_c = 'x';
    else if (isdigit(c))
      rg_c = 'd';
    else rg_c = c;
    if (rg_c != old_char)
      sh += rg_c;

    old_char = rg_c;
  }
  return ans;
}

string my_tolower(string &s){
  string ans = "";
  string::iterator s_i = s.begin();
  for (; s_i != s.end(); ++s_i){
    char c = *s_i;
    ans += tolower(c);
  }
  return ans;
}

string itos (int arg){
    std::ostringstream buffer;
    buffer << arg;             // send the int to the ostringstream
    return buffer.str();       // capture the string
}

int update_MAP(string s,map<string,int> &SI,vector<string> &IS){
  int ans = -1;
  map<string,int>::iterator SI_i = SI.find(s);
  if (SI_i != SI.end())
    ans = (*SI_i).second;
  else {
    int s_id = SI.size();
    SI.insert(make_pair(s,s_id));
    IS.push_back(s);
    ans = s_id;
  }
  return ans;
}

int rand_int_0_N(int N){
  //  cerr << "\nrand_int_interval(" << N << ")";
  int ans  = -1;
  Float R = (Float) rand()/RAND_MAX;
  ans = int(floor(R*(N+1)));
  //  cerr << "\tans = " << ans;getchar();
  return ans;
}

void spacify(string s,vector<string> &W){
  string w = "";
  int _s_ = s.size(), i = 0;
  while (isspace(s[i]))
    ++i;
  for (; i < _s_; ++i){
    if (s[i] == ' '){
      W.push_back(w);
      w = "";
      while (i < _s_ && isspace(s[i]))
	++i;
    }
    if (i < _s_){
      w += s[i];
    }
  }
  if (w != "")
    W.push_back(w);
//   if (w != ""){
//     int _w_ = w.size();
//     W.push_back(string(w,0,_w_-1));
//     W.push_back(string(w,_w_-1,1));
//   }
}

void de_underscorify(string s,vector<string> &W){
  string w = "";
  int _s_ = s.size(), i = 0;
  while (isspace(s[i]))
    ++i;
  for (; i < _s_; ++i){
    if (s[i] == '_'){
      W.push_back(w);
      w = "";
      while (i < _s_ && isspace(s[i]))
	++i;
    }
    if (i < _s_){
      w += s[i];
    }
  }
  if (w != "")
    W.push_back(w);
//   if (w != ""){
//     int _w_ = w.size();
//     W.push_back(string(w,0,_w_-1));
//     W.push_back(string(w,_w_-1,1));
//   }
}

void parse_fname(string fname,string &pref,string &suff){
  string::iterator f_i = fname.begin();
  while (*f_i != '.' && f_i != fname.end())
    pref += *f_i++;
  ++f_i;
  while (f_i != fname.end())
    suff += *f_i++;
 }

void parse_fname(string fname,string &D,string &F,string &S){
  //  cerr << "\nparse_fname(" << fname << ")";
  D = F = S = "";
  string::reverse_iterator f_i = fname.rbegin();
  while (*f_i != '.' && f_i != fname.rend())
    S = (*f_i++)+S;
  ++f_i;
  while (f_i != fname.rend() && *f_i != '/')
    F = (*f_i++)+F;
  while (f_i != fname.rend())
    D = (*f_i++)+D;
  //  cerr << "\n\tD:" << D << "\n\tF:" << F << "\n\tS:" << S;getchar();
}

void parse_multifname(string fname,string &D,vector<string> &FN){
  //  cerr << "\nparse_multifname(" << fname << ")";
  D = "";
  string f = "";
  string::reverse_iterator f_i = fname.rbegin();
  bool dlimit = false;
  while (f_i != fname.rend()){
    if (*f_i == '/')
      dlimit = true;
    if (dlimit)
      D = *f_i+D;
    if (!dlimit && *f_i == '#'){
      FN.push_back(f);
      f = "";
    }
    if (!dlimit && *f_i != '#')
      f = *f_i+f;
    f_i++;
  }
  if (f != "")
    FN.push_back(f);
  //  cerr << "\nD:" << D << "\tF={ ";
  //  for (vector<string>::iterator FN_i = FN.begin(); FN_i != FN.end(); ++FN_i)
  //    cerr << *FN_i << " ";
  //  cerr << "}"; getchar();
}

void check_fname(string fname){
  ifstream in(fname.c_str());
  if (!in){
    cerr << "\nCANNOT FIND FILE=\"" << fname << "\", CHECK FILENAME!";
    abort();
  }
}

//@TODO JAB comment out
/**
string get_current_dir(){
  string ans;
  system("pwd > PWD");
  ifstream in("PWD");
  in >> ans;
  ans += "/";
  return ans;
}
**/

///////////////////////////////
/// create_stop_list_tag(fname,tagname) reads in a BI file and produces a list of words 
/// which have been tagged with "targettag" label less than 65% of the time
void update_dist(string entity, map<string,map<string,int> >& ENTITY_DIST,string tag){
  map<string,map<string,int> >::iterator ED_i = ENTITY_DIST.find(entity);
  if (ED_i == ENTITY_DIST.end()){
    map<string,int> tmp;
    tmp.insert(make_pair(tag,1));
    ENTITY_DIST.insert(make_pair(entity,tmp));
  }
  else {
    map<string,int>::iterator ed_i = (*ED_i).second.find(tag);
    if (ed_i == (*ED_i).second.end())
      (*ED_i).second.insert(make_pair(tag,1));
    else (*ed_i).second++;
  }
}

void get_entity_dist(string fname,map<string,map<string,int> >& ENTITY_DIST){
  cerr << "\n\tget_entity_dist("+fname+")";
  char strbuf[1000];
  ifstream in(fname.c_str());
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      vector<string> W, T;
      while (in.peek() == 9){
	string w, pos, tag;
	in >> w >> pos >> tag;
	W.push_back(w);
	T.push_back(tag);
      }
      int i = 0;
      while (i < W.size()){
	string w = "", tag = "0";
	if (T[i] != "0")
	  tag = string(T[i],2,T[i].size()-2);

	if (T[i]=="0")
	  w = W[i++];
	else if (T[i][0]=='B'){
	  w = W[i++];
	  while (i < W.size() && T[i][0]=='I')
	    w += "_"+W[i++];
	  update_dist(w,ENTITY_DIST,tag);
	}
	//	update_dist(w,ENTITY_DIST,tag);
      }
    }
    in.getline(strbuf,1000);
  }
  cerr << "\t|E| = " << ENTITY_DIST.size();
}

void get_entity_dist(string fname){
  cerr << "\nget_entity_dist("+fname+")";
  char strbuf[1000];
  ifstream in(fname.c_str());
  map<string,map<string,int> > ENTITY_DIST;

  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      vector<string> W, T;
      while (in.peek() == 9){
	string w, pos, tag;
	in >> w >> pos >> tag;
	W.push_back(w);
	T.push_back(tag);
      }
      int i = 0;
      while (i < W.size()){
	string w = "", tag = "0";
	if (T[i] != "0")
	  tag = string(T[i],2,T[i].size()-2);

	if (T[i]=="0")
	  w = W[i++];
	else if (T[i][0]=='B'){
	  w = W[i++];
	  while (i < W.size() && T[i][0]=='I')
	    w += "_"+W[i++];
	}
	update_dist(w,ENTITY_DIST,tag);
      }
    }
    in.getline(strbuf,1000);
  }
  for (map<string,map<string,int> >::iterator ED_i = ENTITY_DIST.begin();  ED_i != ENTITY_DIST.end(); ++ED_i){
    cout << (*ED_i).first << "\t" << (*ED_i).second.size();
    for (map<string,int>::iterator ed_i = (*ED_i).second.begin(); ed_i != (*ED_i).second.end(); ++ed_i)
      cout << "\t" << (*ed_i).first << " " << (*ed_i).second; 
    cout << endl;
  }
  cerr << "\tdone";
}

string connlize_tag(string t){
  if (t == "B-noun.person")
    return "B-PER";
  else if (t == "I-noun.person")
    return "I-PER";
  else if (t == "B-noun.group")
    return "B-ORG";
  else if (t == "I-noun.group")
    return "I-ORG";
  else if (t == "B-noun.location")
    return "B-LOC";
  else if (t == "I-noun.location")
    return "I-LOC";
  else if (t == "B-noun.other")
    return "B-MISC";
  else if (t == "I-noun.other")
    return "I-MISC";
}

void conll_extra_training_and_gaz_from_semcor(string fname){
  cerr << "\nconll_extra_training_and_gaz_from_semcor("+fname+")";
  char strbuf[1000];
  ifstream in(fname.c_str());
  map<string,string> GAZ;
  ofstream out("SEMCOR.GAZ");
  int counter = 0;

  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      vector<string> W, T, P;
      while (in.peek() == 9){
	string w, pos, tag;
	in >> w >> pos >> tag;
	W.push_back(w);
	P.push_back(pos);
	T.push_back(tag);
      }
      int i = 0;
      while (i < W.size()){
	vector<string> Wi,Pi,Ti;
	string w = "", pos = P[i], tag = "0";
	if (T[i] != "0")
	  tag = string(T[i],2,T[i].size()-2);

	if (T[i]=="0")
	  w = W[i++];
	else if (T[i][0]=='B'){
	  w = W[i];
	  Wi.push_back(W[i]);
	  Pi.push_back(P[i]);
	  Ti.push_back(T[i]);
	  i++;
	  while (i < W.size() && T[i][0]=='I'){
	    w += "_"+W[i];
	    Wi.push_back(W[i]);
	    Pi.push_back(P[i]);
	    Ti.push_back(T[i]);
	    i++;
	  }
	}

	if (Wi.size() && (pos == "NNP" || pos == "NNPS")){
	  map<string,string>::iterator G_i = GAZ.find(w);
	  if (G_i == GAZ.end()){
	    GAZ.insert(make_pair(w,tag));
	    if (Ti[0]=="B-noun.person" || Ti[0]=="B-noun.group" || Ti[0]=="B-noun.location" || Ti[0]=="B-noun.other"){
	      cout << ++counter;
	      for (int g = 0; g < Wi.size(); ++g)
		cout << "\t" << Wi[g] << " " << Pi[g] << " " << connlize_tag(Ti[g]);
	      cout << endl;
	    }
	  }
	}
      }
    }
    in.getline(strbuf,1000);
  }
  for (map<string,string>::iterator G_i = GAZ.begin(); G_i != GAZ.end(); ++G_i)
    out << (*G_i).first << "\t" << (*G_i).second << endl;
  cerr << "\tdone";
}

void add_size_to_gaz_feats(string fname){
  cerr << "\nadd_size_to_gaz_feats("+fname+")";
  char strbuf[1000];
  ifstream in(fname.c_str());
  while (in.peek() != -1){
    string w;
    in >> w;
    int nu = 1;
    for (string::iterator w_i = w.begin(); w_i != w.end(); ++w_i)
      if (*w_i=='_')
	++nu;
    cout << w;
    string infix = itos(nu);
    while (in.peek() == 9){
      string f;
      in >> f;
      cout << "\t";
      for (string::iterator f_i = f.begin(); f_i != f.end(); ++f_i){
	if (*f_i=='=')
	  cout << infix;
	cout << *f_i;
      }
    }
    cout << endl;
    in.getline(strbuf,1000);
  }
}


/////////////////////////////
void print_line(ofstream& out,string id, vector<string>& W, vector<string>& P, vector<string>& T){
  //  cerr << "\nprint_line()\t|W| = " << W.size() << "\t|P| = " << P.size() << "\t|T| = " << T.size();
  out << id;
  for (int i = 0; i < W.size(); ++i)
    out << "\t" << W[i] << " " << P[i] << " " << T[i];
  out << endl;
}

void split_by_entity(string fname){
  cerr << "\nsplit_by_entity("+fname+")";
  srand(time(0));
  ofstream out_trn((fname+"_trn_e").c_str()),out_tst((fname+"_tst_e").c_str()),out_dev((fname+"_dev_e").c_str());
  char strbuf[100000];
  ifstream in(fname.c_str());
  set<string> TRN, TST, DEV;
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      vector<string> W, P, T;
      while (in.peek() == 9){
	string w, pos, tag;
	in >> w >> pos >> tag;
	W.push_back(w);
	P.push_back(pos);
	T.push_back(tag);
      }
      string e = "";
      for (int i = 0; i < W.size(); ++i){
	//	cerr << "\n" << W[i] << "/" << T[i];
	if (T[i][0] == 'B'){
	  string w = W[i];
	  while (i < W.size()-1 && T[++i][0] == 'I')
	    w += "_"+W[i];
	  e = w;
	  break;
	}
      }
      //      cerr << "\ne:" << e;           //getchar();

      if (e != ""){
	set<string>::iterator TRN_i = TRN.find(e), DEV_i = DEV.find(e), TST_i = TST.find(e);
	if (TRN_i != TRN.end())
	  print_line(out_trn,id,W,P,T);
	else if (DEV_i != DEV.end())
	  print_line(out_dev,id,W,P,T);
	else if (TST_i != TST.end())
	  print_line(out_tst,id,W,P,T);
	else {
	  if (rand()/Float(RAND_MAX) < 0.2){
	    if (rand()/Float(RAND_MAX) < 0.5){
	      print_line(out_tst,id,W,P,T);
	      TST.insert(e);
	    }
	    else {
	      DEV.insert(e);
	      print_line(out_dev,id,W,P,T);
	    }
	  }
	  else {
	    print_line(out_trn,id,W,P,T);
	    TRN.insert(e);
	  }
	}
      }
    }
    in.getline(strbuf,100000);

//     string line = strbuf;
//     if (rand()/Float(RAND_MAX) < 0.2){
//       if (rand()/Float(RAND_MAX) < 0.5)
// 	out_tst << line << endl;
//       else
// 	out_dev << line << endl;
//     }
//     else out_trn << line << endl;
  }
}

void split(string fname){
  cerr << "\nsplit("+fname+")";
  srand(time(0));
  ofstream out_trn((fname+"_trn").c_str()),out_tst((fname+"_tst").c_str()),out_dev((fname+"_dev").c_str());
  char strbuf[100000];
  ifstream in(fname.c_str());
  while (in.peek() != -1){
    in.getline(strbuf,100000);
    string line = strbuf;
    if (rand()/Float(RAND_MAX) < 0.2){
      if (rand()/Float(RAND_MAX) < 0.5)
	out_tst << line << endl;
      else
	out_dev << line << endl;
    }
    else out_trn << line << endl;
  }
}

void split_by_doc(string fname){
  cerr << "\nsplit_by_doc("+fname+")";
  srand(time(0));
  ofstream out_trn((fname+"_trn_id").c_str()),out_tst((fname+"_tst_id").c_str()),out_dev((fname+"_dev_id").c_str());
  char strbuf[50000];
  ifstream in(fname.c_str());
  set<string> IDs;
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != "") IDs.insert(id);
    in.getline(strbuf,50000);
  }
  set<string> TRN, TST, DEV;
  cerr << "\t|IDs| = " << IDs.size();
  for (set<string>::iterator ID_i = IDs.begin(); ID_i != IDs.end(); ++ID_i){
    if (rand()/Float(RAND_MAX) < 0.2){
      if (rand()/Float(RAND_MAX) < 0.5)
	TST.insert(*ID_i);
      else
	DEV.insert(*ID_i);
    }
    else TRN.insert(*ID_i);
  }
  cerr << "\t|TRN| = " << TRN.size() << "\t" << TRN.size()/Float(IDs.size())
       << "\t|TST| = " << TST.size() << "\t" << TST.size()/Float(IDs.size())
       << "\t|DEV| = " << DEV.size() << "\t" << DEV.size()/Float(IDs.size());
  in.close();
  in.open(fname.c_str());
  while (in.peek() != -1){
    string id;
    in >> id;
    in.getline(strbuf,50000);
    string line = id+strbuf;
    set<string>::iterator id_i = TRN.find(id);
    if (id_i != TRN.end())
      out_trn << line << endl;
    id_i = TST.find(id);
    if (id_i != TST.end())
      out_tst << line << endl;
    id_i = DEV.find(id);
    if (id_i != DEV.end())
      out_dev << line << endl;
  }
}


///////////////////////////////


void bi_to_pos(string fname){
  cerr << "\nbi_to_pos("+fname+")";
  ifstream in(fname.c_str());
  char strbuf[1000];
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      cout << id;
      while (in.peek() == 9){
	string w, pos, tag;
	in >> w >> pos >> tag;
	cout << "\t" << w << " " << pos;
      }
      cout << endl;
    }
    in.getline(strbuf,1000);
  }
}

void pos_to_w(string fname){
  cerr << "\npos_to_w("+fname+")";
  ifstream in(fname.c_str());
  char strbuf[1000];
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      cout << id;
      while (in.peek() == 9){
	string w, pos;
	in >> w >> pos;
	cout << "\t" << w;
      }
      cout << endl;
    }
    in.getline(strbuf,1000);
  }
}

void load_yfile(string fname,map<string,multimap<Float,string> >& tags){
  char strbuf[10000];
  ifstream in(fname.c_str());
  multimap<Float,string> tmp;
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != "")
      tags.insert(make_pair(id,tmp));
  }
  cerr << "\t|tags| = " << tags.size();
}

void split_and_rank(string fname,string Yfile){
  cerr << "\nsplit_and_rank("+fname+","+Yfile+")";
  map<string,multimap<Float,string> > TAGS;
  load_yfile(Yfile,TAGS);

  char strbuf[10000];
  ifstream in(fname.c_str());
  while (in.peek() != -1){
    string line = "";
    in >> line;
    if (line != ""){
      vector<vector<string> > INFO;
      while (in.peek() == 9){
	string s;
	in >> s;
	vector<string> info;
	info.push_back(s);
	while(in.peek() == 32){
	  in >> s;
	  info.push_back(s);
	}
	INFO.push_back(info);
      }
      vector<string> tags;
      Float score = 0;
      for (int i = 0; i < INFO.size(); ++i){
	int _i_ = INFO[i].size();
	if (INFO[i][_i_-1][0] == 'B')
	  tags.push_back(string(INFO[i][_i_-1],2,INFO[i][_i_-1].size()-2));
	else if (INFO[i][_i_-1] == "0")
	  score += 1;
	line += "\t";
	for (int j = 0; j < _i_; ++j)
	  line += " "+INFO[i][j];
      }
      if (tags.size()){
	//	cerr << "\n" << tags[0] << "\tline = {"+line+"}" << "\t->\t" << score;	getchar();
	map<string,multimap<Float,string> >::iterator T_i = TAGS.find(tags[0]);
	if (T_i == TAGS.end()){
	  cerr << "\nno tags = " << tags[0];abort();
	}
	(*T_i).second.insert(make_pair(score,line));
      }
    }
    in.getline(strbuf,10000);
  }
  for (map<string,multimap<Float,string> >::iterator T_i = TAGS.begin(); T_i != TAGS.end(); ++T_i){
    ofstream out((fname+"."+(*T_i).first).c_str());
    for(multimap<Float,string>::reverse_iterator L_i = (*T_i).second.rbegin(); L_i != (*T_i).second.rend(); ++L_i)
      out << (*L_i).second << endl;
  }
}

void count_w(string target, string fname,bool lower){
  char strbuf[10000];
  ifstream in(fname.c_str());
  int count_w = 0, N = 0;
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != "")
      while (in.peek() == 9){
	N += 1;
	string w, pos, tag;
	in >> w >> pos >> tag;
	if (lower)
	  w = my_tolower(w);
	if (w == target)
	  ++count_w;
      }
    in.getline(strbuf,10000);
  }
  cerr << "\ncount("+target+") = " << count_w << "\trf = " << count_w/Float(N) << " (" << N << ")";
}

int count_ws(set<string>& targets, string fname,bool lower,int& N){
  char strbuf[10000];
  ifstream in(fname.c_str());
  int count_w = 0;
  N = 0;
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != "")
      while (in.peek() == 9){
	N += 1;
	string w, pos, tag;
	in >> w >> pos >> tag;
	if (lower)
	  w = my_tolower(w);
	set<string>::iterator t_i = targets.find(w);
	if (t_i != targets.end())
	  ++count_w;
      }
    in.getline(strbuf,10000);
  }
  return count_w;
}

void load_list(string fname,set<string>& tags){
  char strbuf[10000];
  ifstream in(fname.c_str());
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != "")
      tags.insert(id);
  }
  cerr << "\t|list| = " << tags.size();
}

void entity_ratio(string fname1,string fname2,string listname,int seed){
  cerr << "\nentity_ration("+fname1+","+fname2+","+listname+")";
  set<string> trig_list, trig_list_filtered;
  load_list(listname,trig_list);
  srand(seed);
  int N = 50, M = trig_list.size();
  Float t = (N+1)/Float(M);
  while (trig_list_filtered.size() < N){
    for (set<string>::iterator tl_i = trig_list.begin(); tl_i != trig_list.end(); ++tl_i){
      Float r = rand()/Float(RAND_MAX);
      //      cerr << endl << *tl_i << "\tr=" << r << "\tt=" << t;
      if (r < t && trig_list_filtered.size() < N){
	trig_list_filtered.insert(*tl_i);
	//	cerr << "\tins!";getchar();
      }
    }
  }
  if (seed != 0){
    trig_list = trig_list_filtered;
    //    for (set<string>::iterator t_i = trig_list.begin(); t_i != trig_list.end(); ++t_i)
    //      cerr << "\n" << *t_i;
    //    cerr << endl;
  }
  cerr << "\t|ttrig_list| = " << trig_list.size();
  int DEN1 = 0, DEN2 = 0;
  bool lower = true;
  int count1 = count_ws(trig_list,fname1,lower,DEN1);
  int count2 = count_ws(trig_list,fname2,lower,DEN2);
  cerr << "\nestimate-1 = " << count1 << "/" << DEN1 << " = " << count1/Float(DEN1);
  cerr << "\nestimate-2 = " << count2 << "/" << DEN2 << " = " << count2/Float(DEN2);
  cerr << "\ndiff  = " << count1/Float(DEN1) << " - " << count2/Float(DEN2) << " = " << count1/Float(DEN1)-count2/Float(DEN2) 
       << " ratio = "  << (count1/Float(DEN1))/Float(count2/Float(DEN2))
       << " est = " <<  (count2/Float(DEN2)-count1/Float(DEN1))/Float(count1/Float(DEN1));
  cerr << endl;
}

void entity_ratio_rand(string fname1,string fname2,string listname){
  cerr << "\nentity_ratio_rand("+fname1+","+fname2+","+listname+")";
  Float avg = 0;
  int T = 50;
  ofstream out("entity_ratio_vec");
  for (int i = 0; i < T; ++i){
    set<string> trig_list, trig_list_filtered;
    load_list(listname,trig_list);
    int N = 50, M = trig_list.size();
    Float t = (N+1)/Float(M);
    while (trig_list_filtered.size() < N)
      for (set<string>::iterator tl_i = trig_list.begin(); tl_i != trig_list.end(); ++tl_i){
	Float r = rand()/Float(RAND_MAX);
	if (r < t && trig_list_filtered.size() < N)
	  trig_list_filtered.insert(*tl_i);
      }
    trig_list = trig_list_filtered;
    cerr << "\t|ttrig_list| = " << trig_list.size();
    int DEN1 = 0, DEN2 = 0;
    bool lower = true;
    int count1 = count_ws(trig_list,fname1,lower,DEN1);
    int count2 = count_ws(trig_list,fname2,lower,DEN2);
    cerr << "\nestimate-1 = " << count1 << "/" << DEN1 << " = " << count1/Float(DEN1);
    cerr << "\nestimate-2 = " << count2 << "/" << DEN2 << " = " << count2/Float(DEN2);
    cerr << "\ndiff  = " << count1/Float(DEN1) << " - " << count2/Float(DEN2) << " = " << count1/Float(DEN1)-count2/Float(DEN2) 
	 << " ratio = "  << (count1/Float(DEN1))/Float(count2/Float(DEN2))
	 << " est = " <<  (count2/Float(DEN2)-count1/Float(DEN1))/Float(count1/Float(DEN1));
    cerr << endl;
    out << (count2/Float(DEN2)-count1/Float(DEN1))/Float(count1/Float(DEN1)) << endl;
    avg += (count2/Float(DEN2)-count1/Float(DEN1))/Float(count1/Float(DEN1));
  }
  cerr << "\navg = " << avg/Float(T);
}

void get_words_in_c(multimap<int,string>& RANK, string fname,string c){
  char strbuf[10000];
  ifstream in(fname.c_str());
  map<string,int> WC;
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != "")
      while (in.peek() == 9){
	string w, pos, tag;
	in >> w >> pos >> tag;
	if (c == tag)
	  update_map(WC,w,1);
      }
    in.getline(strbuf,10000);
  }
  for (map<string,int>::iterator WC_i = WC.begin(); WC_i != WC.end(); ++WC_i)
    RANK.insert(make_pair((*WC_i).second,(*WC_i).first));
}

void intersecting_words(set<string>& IN, string fname,set<string>& OUT,bool lower ){
  char strbuf[10000];
  ifstream in(fname.c_str());
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != "")
      while (in.peek() == 9){
	string w, pos, tag;
	in >> w >> pos >> tag;
	if (lower)
	  w = my_tolower(w);
	set<string>::iterator IN_i = IN.find(w);
	if (IN_i != IN.end())
	  OUT.insert(w);
      }
    in.getline(strbuf,10000);
  }
}

void entity_ratio2(string sname,string tname,string c){
  cerr << "\nentity_ratio2("+sname+","+tname+","+c+")";
  multimap<int,string> source_wc;
  int M = 50, DENs = 0, DENt = 0;
  get_words_in_c(source_wc,sname,c);
  ofstream out(("eratio2."+c+".log").c_str());
  out << "source = " << sname << "\ntarget = " << tname << "\nlabel = " << c;
  out << "\n|source_wc| = " << source_wc.size() << "\ntop-" << M << ":\n";
  multimap<int,string>::reverse_iterator w_i = source_wc.rbegin();
  set<string> target, target_cup;
  for (int i = 0; i < M; ++i){
    out << (*w_i).second << "\t" << (*w_i).first << endl;
    target.insert((*w_i).second);
    w_i++;
  }
  out << "\n|target| = " << target.size();
  bool lower = false;
  intersecting_words(target,tname,target_cup,lower);
  cerr << "\n|target_cup| = " << target_cup.size();
  int counts = count_ws(target_cup,sname,lower,DENs);
  int countt = count_ws(target_cup,tname,lower,DENt);
  cerr << "\nestimate-s = " << counts << "/" << DENs << " = " << counts/Float(DENs);
  cerr << "\nestimate-t = " << countt << "/" << DENt << " = " << countt/Float(DENt);

  cerr << "\ndiff  = " << counts/Float(DENs) << " - " << countt/Float(DENt) << " = " << counts/Float(DENs)-countt/Float(DENt) 
       << " ratio = "  << (counts/Float(DENs))/Float(countt/Float(DENt))
       << " est = " <<  (countt/Float(DENt)-counts/Float(DENs))/Float(counts/Float(DENs));
  cerr << endl;

  out << endl;
}

void split_triggers(string fname){
  cerr << "\nsplit_triggers("+fname+")";
  char strbuf[10000];
  ifstream in(fname.c_str());
  ofstream out_org("DATA/ADAPT/trig.org");
  ofstream out_per("DATA/ADAPT/trig.per");
  ofstream out_loc("DATA/ADAPT/trig.loc");
  ofstream out_misc("DATA/ADAPT/trig.misc");
  while (in.peek() != -1){
    string w, f;
    in >> w >> f;
    if (f == "TRIG_ORG_1")
      out_org << w << endl;
    else if (f == "TRIG_PER_1")
      out_per << w << endl;
    if (f == "TRIG_LOC_1")
      out_loc << w << endl;
    if (f == "TRIG_MISC_1")
      out_misc << w << endl;
    in.getline(strbuf,10000);
  }
}

void get_entity_dist_tokenwise(string fname,map<string,map<string,int> >& D,bool useshort){
  cerr << "\n\tget_entity_dist_tokenwise("+fname+")";
  char strbuf[1000];
  ifstream in(fname.c_str());
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      while (in.peek() == 9){
	string w, pos, tag;
	in >> w >> pos >> tag;
	if (useshort && tag != "0")
	  tag = string(tag,2,tag.size()-2);
	map<string,map<string,int> >::iterator D_i = D.find(w);
	if (D_i == D.end()){
	  map<string,int> tmp;
	  tmp.insert(make_pair(tag,1));
	  D.insert(make_pair(w,tmp));
	}
	else {
	  map<string,int>::iterator d_i = (*D_i).second.find(tag);
	  if (d_i == (*D_i).second.end())
	    (*D_i).second.insert(make_pair(tag,1));
	  else (*d_i).second += 1;
	}
      }
    }
    in.getline(strbuf,1000);
  }
  cerr << "\t|D| = " << D.size();
}

void discordant_tags(string fname1,string fname2){
  cerr << "\ndiscordant_tags("+fname1+","+fname2+")";
  map<string,map<string,int> > E1, E2;
  get_entity_dist(fname1,E1);
  get_entity_dist(fname2,E2);
  int unk = 0, den  = 0;
  for (map<string,map<string,int> >::iterator E2_i = E2.begin(); E2_i != E2.end(); ++E2_i){
    string w = (*E2_i).first, wshort = "";
    int fw = 0;
    for (map<string,int>::iterator F_i = (*E2_i).second.begin(); F_i != (*E2_i).second.end(); ++F_i)
      fw += (*F_i).second;
    cerr << "\nw:" << w << "\t" << fw;
    den += fw;
    map<string,map<string,int> >::iterator E1_i = E1.find(w);
    if (E1_i != E1.end()){
      multimap<int,string> R1, R2;
      for (map<string,int>::iterator F_i = (*E1_i).second.begin(); F_i != (*E1_i).second.end(); ++F_i)
	R1.insert(make_pair((*F_i).second,(*F_i).first));
      for (map<string,int>::iterator F_i = (*E2_i).second.begin(); F_i != (*E2_i).second.end(); ++F_i)
	R2.insert(make_pair((*F_i).second,(*F_i).first));
      multimap<int,string>::reverse_iterator R1_i = R1.rbegin(), R2_i = R2.rbegin();
      cerr << "\t" << (*R1_i).second << "/" << (*R1_i).first << "\t" << (*R2_i).second << "/" << (*R2_i).first;
      if ((*R1_i).second != (*R2_i).second){
	cout << "\n" << (*R1_i).second << "/" << (*R1_i).first << "\t" << (*R2_i).second << "/" << (*R2_i).first;
      }
    }
    else {
      unk += fw;
    }
  }
  cerr << "\nUNK = " << unk << "/" << den << " = " << unk/Float(den);
  cerr << "\tdone\n";
}

void discordant_tags_tokenwise(string fname1,string fname2){
  cerr << "\ndiscordant_tags_tokenwise("+fname1+","+fname2+")";
  map<string,map<string,int> > E1, E2;
  get_entity_dist_tokenwise(fname1,E1,false);
  get_entity_dist_tokenwise(fname2,E2,false);


}

void sample_sentences(int N,int M,string fname){
  cerr << "\nsample_sentences(" << N << "," << M << "," << fname << ")";
  ifstream in(fname.c_str());
  int MAXSIZE = 100000;
  char strbuf[MAXSIZE];
  Float t = N+1/Float(M);
  cerr << "\tt = " << t;
  int counter = 0;
  while (in.peek() != -1){
    in.getline(strbuf,MAXSIZE);
    string line = strbuf;
    Float r = rand()/Float(RAND_MAX);
    if (r < t && counter < N){
      cout << line << endl;
      counter++;
    }
  }
}

void print_tags_feat_file(string fname){
  cerr << "\nprint_tags_feat_file("+fname+")";
  char strbuf[10000];
  ifstream in(fname.c_str());
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      while (in.peek() == 9){
	string w, pos, tag;
	in >> w >> pos >> tag;
	cout << tag << endl;
      }
    }
    in.getline(strbuf,10000);
  }
  cerr << "\tdone\n";
}
