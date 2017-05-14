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

#include "examples.h"
#define COMPARE_R 0  // compare normal and R algorithms
#define USE_MORPH_CACHE_HERE 1

void load_words_notag(string dname,vector<vector<string> > &D,vector<string> &ID){
  cerr << "\nload_words_notag("+dname+")";
  //@TODO JAB string cdir = get_current_dir();
  check_fname(dname);
  ifstream in((dname).c_str());
  char strbuf[10000];
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      vector<string> S;
      ID.push_back(id);
      while (in.peek() == 9){
	string w;
	in >> w;
	S.push_back(w);
      }
      D.push_back(S);
    }
    in.getline(strbuf,10000);
  }
  cerr << "\t|D| = " << D.size() << "\t|ID| = " << ID.size();
}


void merge_pos_tag(string posfname,string tagfname,string outfname){
  cerr << "\nmerge_pos_tag("+posfname+","+tagfname+")";
  char strbuf[10000];
  ifstream inpos(posfname.c_str()), intag(tagfname.c_str());
  ofstream out(outfname.c_str());
  while (inpos.peek() != -1){
    string id1 = "", id2 = "";
    inpos >> id1;    intag >> id2;
    if (id1!=id2){cerr << "\nid1="+id1+"!=id2="+id2;abort();};
    if (id1!=""){
      out << id1;
      while (inpos.peek() == 9){
	string w, pos, tag;
	inpos >> w >> pos;
	intag >> tag;
	out << "\t" << w << " " << pos << " " << tag;
      }
      out << endl;
    }
    inpos.getline(strbuf,10000);
    intag.getline(strbuf,10000);
  }
}

void tag_light(string modelname,string targetname,string tagsetname,bool secondorder,string mode){
  cerr << "\ntagger_light::tag("
       << "\nmodelname:"    << modelname
       << "\n\ttargetname:" << targetname
       << "\n\ttagset:"     << tagsetname
       << "\n\tmode:"       << mode+"\t)";
  tagger_light TL;
  TL.init(modelname,tagsetname,mode);
  vector<vector<vector<int> > > D;
  vector<string> ID;
  TL.load_data_notag(targetname,D,ID,secondorder);
  vector<vector<int> > tagdata_tags;
  TL.ps_hmm.guess_sequences(D,tagdata_tags);
  string dir, F, suff;
  parse_fname(tagsetname,dir,F,suff);
  for (int i = 0; i < tagdata_tags.size(); ++i){
    cout << ID[i];
    for (int j = 0; j < tagdata_tags[i].size(); ++j)
      cout << "\t" << TL.LIS[tagdata_tags[i][j]];
    cout << endl;
  }
}

void bitag_light(string modelname_pos,
		 string tagsetname_pos,
		 string modelname,
		 string tagsetname,
		 string targetname,
		 bool secondorder,
		 bool lowercase){
  cerr << "\ntagger_light::bitag("
       << "\n\tmodelname_pos:\t"    << modelname_pos
       << "\n\ttagsetname_pos:\t"   << tagsetname_pos
       << "\n\tmodelname:\t"        << modelname
       << "\n\ttargetname:\t"       << targetname
       << "\n\ttagset:\t"           << tagsetname
       << "\n\tlowercase:\t"        << lowercase
       << "\n\toutput:\t"           << targetname+".tags"+"\t)";
  tagger_light TL_pos,TL;
  vector<vector<vector<int> > > D;
  vector<string> ID;
  vector<vector<int> > tagdata_tags_pos, tagdata_tags;
  vector<vector<string> > WORDS;
  load_words_notag(targetname,WORDS,ID);
  TL_pos.init(modelname_pos,tagsetname_pos,"POS");
  TL.init(modelname,tagsetname,"BIO");
  
  int N = WORDS.size();
  ofstream out((targetname+".tags").c_str());
  cerr << "tagging:"<<endl;
  for (int i = 0; i < N; ++i){
    int n = WORDS[i].size();
    vector<string> P, guess3_i,LOW;
    vector<vector<string> > O_str;
    vector<vector<int> > O, O2;
    extract_feats(WORDS[i],P,O_str,lowercase,LOW);
    TL_pos.encode_s(O_str,O,secondorder);
    vector<int> guess_i, guess2_i;
    TL_pos.ps_hmm.viterbi(O,guess_i);
    TL_pos.addpos(guess_i,O_str);
    TL.encode_s(O_str,O2,secondorder);
    TL.ps_hmm.viterbi(O2,guess2_i);
    TL.check_consistency(guess2_i,guess3_i);
    out << ID[i];
    for (int j = 0; j < n; ++j)
      out << "\t" << WORDS[i][j] << " " << TL_pos.LIS[guess_i[j]] << " " << guess3_i[j];
    out << endl;
    if (i%100==0)cerr << ".";
  }
}

void postag_light(string modelname_pos,
		  string tagsetname_pos,
		  string targetname,
		  bool secondorder,
		  bool lowercase){
  cerr << "\ntagger_light::bitag("
       << "\n\tmodelname_pos:\t"    << modelname_pos
       << "\n\ttagsetname_pos:\t"   << tagsetname_pos
       << "\n\ttargetname:\t"       << targetname
       << "\n\tlowercase:\t"        << lowercase
       << "\n\toutput:\t"           << targetname+".pos"+"\t)";
  tagger_light TL_pos;
  vector<vector<vector<int> > > D;
  vector<string> ID;
  vector<vector<int> > tagdata_tags_pos;
  vector<vector<string> > WORDS;
  load_words_notag(targetname,WORDS,ID);
  TL_pos.init(modelname_pos,tagsetname_pos,"POS");
  
  int N = WORDS.size();
  ofstream out((targetname+".pos").c_str());
  cerr << "\ntagging:";
  for (int i = 0; i < N; ++i){
    int n = WORDS[i].size();
    vector<string> P, guess3_i,LOW;
    vector<vector<string> > O_str;
    vector<vector<int> > O;
    extract_feats(WORDS[i],P,O_str,lowercase,LOW);
    TL_pos.encode_s(O_str,O,secondorder);
    vector<int> guess_i;
    TL_pos.ps_hmm.viterbi(O,guess_i);
    out << ID[i];
    for (int j = 0; j < n; ++j)
      out << "\t" << WORDS[i][j] << " " << TL_pos.LIS[guess_i[j]];
    out << endl;
    if (i%100==0)cerr << ".";
  }
}

void train_light(string modelname,string traindata,string tagsetname,bool secondorder, int T,string mode){
  cerr << "\ntrain-light("
       << "\n\tmodelname = "    << modelname
       << "\n\ttraindata = "    << traindata
       << "\n\ttagsetname = "   << tagsetname
       << "\n\tsecondorder = "  << secondorder
       << "\n\tT = "            << T 
       << "\n\tmode = " << mode << " )";
  tagger_light TL;
  TL.init("NULL",tagsetname,mode);
  vector<vector<vector<int> > > D;
  vector<vector<int> > G;
  vector<string> ID;
  TL.load_data(traindata,D,G,ID,secondorder);
  TL.train(D,G,T,modelname);
}

void eval_light(string traindata,string testdata,string tagsetname,bool secondorder, int T, int CV, string mode,bool R, Float ww,string thetafile){
  cerr << "\neval-light("
       << "\n\ttraindata:"   << traindata
       << "\n\ttestdata:"    << testdata
       << "\n\ttagsetname:"  << tagsetname
       << "\n\tsecondorder=" << secondorder
       << "\n\tT = "         << T
       << "\n\tCV = "        << CV 
       << "\n\tmode = "      << mode
       << "\n\tR = "         << R 
       << "\n\twordweight = "  << ww 
       << "\n\ttheta = "  << thetafile
       << " )";
#if COMPARE_R
  tagger_light TL, TL_R;
  TL.init("NULL",tagsetname,mode);
  TL_R.init("NULL",tagsetname,mode);
  vector<vector<int> > G, G2, GR, GR2;
  vector<string> ID, ID2, IDR, IDR2;
  vector<vector<vector<int> > > D, D2;
  TL.load_data(traindata,D,G,ID,secondorder);
  TL.load_data(traindata,D2,G2,ID2,secondorder);

  Zr DR, DR2;
  TL.load_data_R(testdata,DR,GR,IDR,secondorder,R,true);
  TL.load_data_R(testdata,DR2,GR2,IDR2,secondorder,R,true);
    //    TL.print_data_R(D,G);    getchar();
  TL.make_MU_SIGMA(DR);
  TL.standardize_data(DR);
  TL.standardize_data(DR2);
  TL.print_data_R(DR,GR);    getchar();

  PS_HMM ps, psr;
  ps.init(int(TL.LSI.size()),TL.LIS,TL.FIS,TL.LSI,TL.FSI,false, ww);
  psr.init(int(TL_R.LSI.size()),TL_R.LIS,TL_R.FIS,TL_R.LSI,TL_R.FSI,false, ww);
  
  for (int i = 0; i < D.size(); ++i){
    cerr << endl << i << "\ttrain normal:";
    bool upd = false, updr = false;
    ps.train_on_s(D[i],G[i],upd);
    ofstream out;
    ps.SaveModel("temp_"+itos(i),TL.LIS,TL.FIS,TL.LSI,TL.FSI,out);

    cerr << endl << i << "\ttrain R:";
    for (int j = 0; j < DR[i].size(); ++j){
      cerr << endl << j;
      for (int r  = 0; r < DR[i][j].size(); ++r)
	cerr << " " << DR[i][j][r].first << "/" << DR[i][j][r].second;
    }
    getchar();

    psr.train_on_s_R(DR[i],GR[i],updr);
    psr.SaveModel("tempr_"+itos(i),TL.LIS,TL.FIS,TL.LSI,TL.FSI,out);

    getchar();
  }

#else
  tagger_light TL;
  TL.init("NULL",tagsetname,mode);
  vector<vector<int> > G, G2;
  vector<string> ID, ID2;
  if (!R){                                  // Std binary features data
    vector<vector<vector<int> > > D, D2;
    TL.load_data(traindata,D,G,ID,secondorder);
    TL.load_data(testdata,D2,G2,ID2,secondorder);

    string trash, tagsetsuff, trainsuff, testsuff, dirname;

    parse_fname(traindata,dirname,trainsuff,trash);
    parse_fname(testdata,dirname,testsuff,trash);
    parse_fname(tagsetname,dirname,tagsetsuff,trash);

    string description = tagsetsuff+"_"+trainsuff+"_"+testsuff+".results";
    TL.eval(D,D2,G,G2,T,CV,description,mode,ww,thetafile);
  }
  else {
    Zr D, D2;
    TL.load_data_R(traindata,D,G,ID,secondorder,R,true);
    TL.load_data_R(testdata,D2,G2,ID2,secondorder,R,true);
    //    TL.print_data_R(D,G);    getchar();
    //    vector<Float> MU, SIGMA, MU2, SIGMA2; 

    //    make_MU_SIGMA(D,MU,SIGMA,TL.FIS.size());
    //    make_MU_SIGMA(D2,MU2,SIGMA2,TL.FIS.size());
    //    standardize_data(D,MU,SIGMA,TL.FIS.size());
    //    standardize_data(D2,MU2,SIGMA2,TL.FIS.size());

    //    TL.print_data_R(D,G);    getchar();
    //    TL.print_data_R(D2,G2);    getchar();
    TL.eval_R(D,D2,G,G2,T,CV,tagsetname+"_"+itos(T)+"_"+itos(CV),mode,ww,thetafile);
  }
#endif
}

void collect_postags(string fname){
  cerr << "\ncollect_postags("+fname+")";
  ifstream in(fname.c_str());
  set<string> postags;
  char strbuf[1000];
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      while (in.peek() == 9){
	string w, pos, tag;
	in >> w >> pos >> tag;
	postags.insert(pos);
      }
    }
    in.getline(strbuf,1000);
  }
  cerr << "\t |POS| = " << postags.size() << endl;
  for (set<string>::iterator p_i = postags.begin(); p_i != postags.end(); ++p_i)
    cout << *p_i << endl;
}

void collect_tags_dist(string fname,string oname){
  cerr << "\ncollect_tags_dist("+fname+","+oname+")";
  ifstream in(fname.c_str());
  map<string,int> tags;
  char strbuf[1000];
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      while (in.peek() == 9){
	string w, pos, tag;
	in >> w >> pos >> tag;
	//	if (tag[0] == 'B')
	update_map(tags,tag,1);
      }
    }
    in.getline(strbuf,1000);
  }
  ofstream out(oname.c_str());
  print_rank(out,tags);
  cerr << "\t |TAGS| = " << tags.size();
}

void add_a_feature(string fname,string featname,string oname){
  cerr << "\ncollect_postags("+fname+","+featname+","+oname+")";
  ifstream in(fname.c_str()), inf(featname.c_str());
  ofstream out(oname.c_str());
  char strbuf[1000];
  while (in.peek() != -1){
    string id = "", idf = "";
    in >> id;
    inf >> idf;
    if (id != idf){
      cerr << "\nerr: id = " << id << " != " << idf;abort();}
    if (id != ""){
      out << id;
      while (in.peek() == 9){
	string f, add_f;
	inf >> add_f;
	in >> f;
	out << "\t" << add_f << " " << f;
	while (in.peek() == 32){
	  in >> f;
	  out << " " << f;
	}
      }
      out << endl;
    }
    in.getline(strbuf,1000);
    inf.getline(strbuf,1000);
  }
}

void pseudo_train(string fname,string oname){
  cerr << "\npseudo_train("+fname+","+oname+")";
  char strbuf[1000];
  ifstream in(fname.c_str());
  ofstream out(oname.c_str());
  int count = 0;
  while (in.peek() != -1){
    vector<string> W;
    string w = "";
    in >> w;
    W.push_back(w);
    while (in.peek() != 10 && in.peek() != 9){
      string w_cont = "";
      in >> w_cont;
      if (w_cont != "")
	W.push_back(w_cont);
    }
    in.getline(strbuf,1000);
    if (W.size()){
      out << ++count << "\t" << W[0];
    for (int i = 1; i < W.size(); ++i)
      out << " " << W[i];
    out << endl;
    }
  }
}

void merge_pos(string fname,string oname,string posname,string tag){
  cerr << "\nmerge_pos("+fname+","+oname+","+posname+")";
  char strbuf[1000];
  ifstream in(fname.c_str());
  ifstream inpos(posname.c_str());
  ofstream out(oname.c_str());
  while (in.peek() != -1){
    string id;
    in >> id;
    inpos >> id;
    out << id;
    int count = 0;
    while (in.peek() == 9){
      string w = "", pos = "";
      in >> w;
      inpos >> pos;
      out << "\t" << w << " " << pos;
      if (count == 0)
	out << " B-" << tag;
      else 
	out << " I-" << tag;
      ++count;
    }
    out << endl;
    in.getline(strbuf,1000);
    inpos.getline(strbuf,1000);
  }
}

////////////////////////////////////////////////
// STDIN->STDOUT

#include "../preproc/sensplit.hxx"

using namespace nlpyahoobcn;//

void multitag(string target,bool secondorder,bool lowercase,string gazfile,
	      string model_pos,string tagset_pos,
	      vector<string>& M_bio,vector<string>& T_bio){
  cerr << "\ntagger_light::multitag("
       << "\n\ttarget:\t"           << target
       << "\n\tsecondorder="        << secondorder
       << "\n\tmodel_pos:\t"        << model_pos
       << "\n\ttagset_pos:\t"       << tagset_pos
       << "\n\tgazfile:\t"          << gazfile
       << "\n\t|M_bio|/|T_bio| = "  << M_bio.size() << "/" << T_bio.size()
       << "\t)";
  istream_iterator<char> tend;
  istream * sstinput;
  
  if(target.compare("stdin")!=0) {
    check_fname(target);
    sstinput =  new ifstream(target.c_str());
  }
  else {
    sstinput  = &cin;
  }
  sentenceSequence ss(*sstinput);

  // create a PoS tagger
  tagger_light TL_pos;
  TL_pos.init(model_pos,tagset_pos,"POS");
  
  gaz_server GZ;
#if USE_MORPH_CACHE_HERE
  system("gunzip DATA/MORPH/MORPH_CACHE.gz");
  GZ.init(gazfile,10,"DATA/MORPH/MORPH_CACHE");
  system("gzip DATA/MORPH/MORPH_CACHE");
#else
  GZ.init(gazfile,10);
#endif
  
  // create a Semantic taggers
  const int ntaggers = M_bio.size();
  vector<tagger_light> semTaggers(ntaggers);
  for(int i=0;i<ntaggers;++i) { 
    semTaggers[i].init(M_bio[i],T_bio[i],"BIO");
  }
  string oldid="";

  while(!ss.EOD()) { 
    string sentence=ss.next();
    string docid = ss.getDocId();

    istringstream buff(sentence);
    buff.unsetf(ios::skipws);
    tokenizer<istream_iterator<char> > t((istream_iterator<char>) buff,tend);
    vector<string> vtoken;
    
    // call the tokenizer
    while(t.hasMore()) {
      vtoken.push_back(t.nextToken());
    }   
    // vtoken is avector of tokens
    vector<string> P,LOW;
    vector<vector<string> > O_str_features;
    vector<int> pos_guess;
    // create a place to  store results
    const int extraTags=2;
    vector< vector <string> > results(ntaggers+extraTags);

    // extracts the features and put them into O_str_features
    // P is empty so no POS is used yet 
    extract_feats(vtoken,P,O_str_features,lowercase,LOW);
    TL_pos.tagSemSequence(O_str_features, results[0], secondorder, false, true);
    GZ.extract_feats(LOW,results[0],O_str_features,results[1]);

    for(int i=0;i<ntaggers;++i) 
      semTaggers[i].tagSemSequence(O_str_features,  results[i+extraTags], secondorder, true, false);

    // print out every thing
    if(oldid.compare(docid)!=0) { 
      oldid=docid; 
      cout<<"%%#DOC "<<docid<<endl;
    }
    for(int w=0;w<vtoken.size();++w) {
      cout<<vtoken[w];
      for(int i=0;i<ntaggers+extraTags;++i) {
	cout<<"\t"<<results[i][w];
      }
      cout<<endl;
    }
    cout<<endl;
  }   
}

void multitag_line(string target,bool secondorder,bool lowercase,string gazfile,
	      string model_pos,string tagset_pos,
	      vector<string>& M_bio,vector<string>& T_bio){
  cerr << "\ntagger_light::multitag_line("
       << "\n\ttarget:\t"           << target
       << "\n\tsecondorder="        << secondorder
       << "\n\tmodel_pos:\t"        << model_pos
       << "\n\ttagset_pos:\t"       << tagset_pos
       << "\n\tgazfile:\t"          << gazfile
       << "\n\t|M_bio|/|T_bio| = "  << M_bio.size() << "/" << T_bio.size()
       << "\t)";
  istream_iterator<char> tend;
  istream * sstinput;
  
  if(target.compare("stdin")!=0) {
    check_fname(target);
    sstinput =  new ifstream(target.c_str());
  }
  else {
    sstinput  = &cin;
  }
  sentenceSequence ss(*sstinput);

  // create a PoS tagger
  tagger_light TL_pos;
  TL_pos.init(model_pos,tagset_pos,"POS");
  
  gaz_server GZ;
#if USE_MORPH_CACHE_HERE
  system("gunzip DATA/MORPH/MORPH_CACHE.gz");
  GZ.init(gazfile,10,"DATA/MORPH/MORPH_CACHE");
  system("gzip DATA/MORPH/MORPH_CACHE");
#else
  GZ.init(gazfile,10);
#endif
  
  // create a Semantic taggers
  const int ntaggers = M_bio.size();
  vector<tagger_light> semTaggers(ntaggers);
  for(int i=0;i<ntaggers;++i) { 
    semTaggers[i].init(M_bio[i],T_bio[i],"BIO");
  }
  string oldid="";
 
  const int MAXBUFFSIZE=1000000;
  char temp[MAXBUFFSIZE];
  while(!sstinput->getline(temp, MAXBUFFSIZE).eof()) {
    if (temp[0] != '\0'){
      istringstream buff(temp);
      buff.unsetf(ios::skipws);
      tokenizer<istream_iterator<char> > t((istream_iterator<char>) buff,tend);
      vector<string> vtoken;
    
      // call the tokenizer
      while(t.hasMore()) {
	vtoken.push_back(t.nextToken());
      }   
      // vtoken is avector of tokens
      vector<string> P,LOW;
      vector<vector<string> > O_str_features;
      vector<int> pos_guess;
      // create a place to  store results
      const int extraTags=2;
      vector< vector <string> > results(ntaggers+extraTags);
      
      // extracts the features and put them into O_str_features
      // P is empty so no POS is used yet 
      extract_feats(vtoken,P,O_str_features,lowercase,LOW);
      TL_pos.tagSemSequence(O_str_features, results[0], secondorder, false, true);
      GZ.extract_feats(LOW,results[0],O_str_features,results[1]);
      
      for(int i=0;i<ntaggers;++i) 
	semTaggers[i].tagSemSequence(O_str_features,  results[i+extraTags], secondorder, true, false);
      
      for(int w=0;w<vtoken.size();++w) {
	if (w > 0)
	  cout << " ";
	cout<<vtoken[w];
	for(int i=0;i<ntaggers+extraTags;++i) {
	  cout<<" "<<results[i][w];
	}
	//	cout<<endl;
      }
    }
    cout<<endl;
  }   
}

void load_filenames(string flist,vector<string>& files){
  cerr << "\nload_filenames("+flist+")";
  char strbuf[1000];
  ifstream in(flist.c_str());
  while (in.peek() != -1){
    string f;
    in >> f;
    in.getline(strbuf,1000);
    files.push_back(f);
  }
  cerr << "\t|files| = " << files.size();
}

void multitag_tabular_format(string flist,string dirname,bool secondorder,bool lowercase,string gazfile,
			     vector<string>& M_bio,vector<string>& T_bio){
  cerr << "\ntagger_light::multitag_tab("
       << "\n\tdirname:\t"           << dirname
       << "\n\tflist:\t"           << flist
       << "\n\tsecondorder="        << secondorder
       << "\n\tlowercase=\t"        << lowercase
       << "\n\tgazfile:\t"          << gazfile
       << "\n\t|M_bio|/|T_bio| = "  << M_bio.size() << "/" << T_bio.size()
       << "\t)";
  gaz_server GZ;
#if USE_MORPH_CACHE_HERE
  system("gunzip DATA/MORPH/MORPH_CACHE.gz");
  GZ.init(gazfile,10,"DATA/MORPH/MORPH_CACHE");
  system("gzip DATA/MORPH/MORPH_CACHE");
#else
  GZ.init(gazfile,10);
#endif
  vector<string> files;                                        // load filenames
  load_filenames(flist,files);
  
  const int ntaggers = M_bio.size();                           // create the semantic taggers
  vector<tagger_light> semTaggers(ntaggers);
  for(int i=0;i<ntaggers;++i) 
    semTaggers[i].init(M_bio[i],T_bio[i],"BIO");
  char strbuf[100000];
  for (int i = 0; i < files.size(); ++i){
    ofstream out;
    if (lowercase)    out.open((dirname+files[i]+".tagged_low").c_str());
    else              out.open((dirname+files[i]+".tagged_upp").c_str());
    ifstream in((dirname+files[i]).c_str());
    if (!in){
      cerr << "\nnofilename:" << dirname+files[i];
    }
    else {
      cerr << "\nprocessing file:" << dirname+files[i];
      while(in.peek() != -1){
	vector<string> vtoken, P, LOW, WM, REST;
	string look = ""; in >> look;
	while(look!= "" && string(look,0,3) != "%%#"){
	  string pos;	  in >> pos;
	  vtoken.push_back(look);
	  P.push_back(pos);
	  in.getline(strbuf,100000);
	  string rest = strbuf;
	  REST.push_back(rest);
	  in >> look;
	}
	in.getline(strbuf,100000);
	string rest_line = strbuf;
	if (vtoken.size()){
	  vector<vector<string> > O_str_features;
	  vector< vector <string> > results(ntaggers);  // create a place to  store results
	  extract_feats(vtoken,P,O_str_features,lowercase,LOW);
	  GZ.extract_feats(LOW,P,O_str_features,WM);
	  for(int i=0;i<ntaggers;++i) 
	    semTaggers[i].tagSemSequence(O_str_features,results[i], secondorder, true, false);
	  for (int i = 0; i < vtoken.size(); ++i){
	    out << vtoken[i] << "\t" << P[i] << REST[i];
	    for (int j = 0; j < ntaggers; ++j)
	      out << "\t" << results[j][i];
	    out << endl;
	  }
	  out << look << rest_line << endl;
	}
	else out << look << rest_line << endl;
      }
    }
  }
}   

////////////////////////////////////////////////
// DATA UTILITIES

void fragment_data(string fname){
  cerr << "\nfragment_data("+fname+")";
  ifstream in(fname.c_str());
  char strbuf[10000];
  ofstream out((fname+"_frag").c_str());
  while (in.peek() != -1){
    string id = "";
    in >> id;
    //    cerr << id;//getchar();
    if (id != ""){
      vector<vector<string> > sequence;
      vector<string> pos;
      while (in.peek() == 9){
	vector<string> feats;
	string f = "";
	in >> f;
	feats.push_back(f);
	while (in.peek() == 32){
	  in >> f;
	  feats.push_back(f);
	  if (string(f,0,4)=="pos="){
	    string p = "";
	    p += f[4];
	    pos.push_back(p);
	  }
	}
	sequence.push_back(feats);
      }
      out << id;
      string prev = "0", next = "";
      for (int i = 0; i < sequence.size(); ++i){
	int d_i = sequence[i].size();
	string tag_i = sequence[i][d_i-1];
	if (i > 0)
	  prev = sequence[i-1][sequence[i-1].size()-1];
	if (i < sequence.size()-1)
	  next = sequence[i+1][sequence[i+1].size()-1];
	else next = "0";

	//	cerr << "\n" << id << "\t" << tag_i << "/" << pos[i] << "\t" << prev << "/" << next;getchar();

	if (tag_i!="0" || ((next!="0" || prev!="0") && pos[i]!="N" && pos[i]!="J" && pos[i]!="R" && pos[i]!="V")){
	  //	if (tag_i!="0" || ((next!="0" || prev!="0") && pos[i]!="N")){
	  out << "\t" << sequence[i][0];
	  for (int j = 1; j < d_i; ++j)
	    out << " " << sequence[i][j];
	}
      }
      out << endl;
    }
    in.getline(strbuf,10000);
  }
  cerr << "\tdone";
}

void uppercase_bi_data(string ifname,string ofname){
  cerr << "\nuppercase_bi_data("+ifname+","+ofname+")";
  ofstream out(ofname.c_str());
  char strbuf[100];
  ifstream in(ifname.c_str());
  while (in.peek() != -1){
    string id;
    in >> id;
    out << id;
    while (in.peek() == 9){
      string w, pos, tag;
      in >>w >> pos >> tag;
      string new_w = w;
      new_w[0] = toupper(new_w[0]);
      out << "\t" << new_w << " " << pos << " " << tag;
    }
    out << endl;
    in.getline(strbuf,100);
  }
  cerr << "\tdone";
}


void lowercase_bi_data(string ifname,string ofname){
  cerr << "\nlowercase_bi_data("+ifname+","+ofname+")";
  ofstream out(ofname.c_str());
  char strbuf[100];
  ifstream in(ifname.c_str());
  while (in.peek() != -1){
    string id;
    in >> id;
    out << id;
    while (in.peek() == 9){
      string w, pos, tag;
      in >> w >> pos >> tag;
      out << "\t" << my_tolower(w) << " " << pos << " " << tag;
    }
    out << endl;
    in.getline(strbuf,100);
  }
  cerr << "\tdone";
}


///
/// reader object
///

// class reader {


// public:

//   reader(const string &  filename) {}
//   inline bool hasNext(void) {}
//   inline massiSentence next(void){}
  
// };
//
//  reader r("myfile");
//   while(r.hasNext(){ massiSentece s = next(); ... } 
//


/// create_nostop_list(string fname) reads in a BI file and produces a list of words 
/// which have been tagged with the NULL (0) label more than 65% of the time
void create_nostop_list(string fname){
  cerr << "\ncreate_nostop_list("+fname+")";
  char strbuf[1000];
  ifstream in(fname.c_str());
  HM DEN, ZERO_TAGS;
  while (in.peek() != -1){
    string id = "";
    in >> id;
    while (in.peek() == 9){
      string w, pos, tag;
      in >> w >> pos >> tag;
      update_hmap_count(DEN,w);
      if (tag == "0"){
	update_hmap_count(ZERO_TAGS,w);
      }
    }
    in.getline(strbuf,1000);
  }
  cerr << "\t|DEN| = " << DEN.size() << "\t|ZERO_TAGS| = " << ZERO_TAGS.size();
  int admitted = 0;
  for (HM::iterator Z_i = ZERO_TAGS.begin(); Z_i != ZERO_TAGS.end(); ++Z_i){
    HM::iterator D_i = DEN.find(Z_i->first);
    Float ratio = (Float) Z_i->second/D_i->second;
    if (ratio > .66){
      ++admitted;
      cout << Z_i->first << "\t" << Z_i->second << "\t" << D_i->second << endl;
    }
  }
  cerr << "\tadmitted = " << admitted;
}



// lname = nostop list
// fname = feature vector representation
// biname = BIO representation
void fragment_data_nostoplist(string fname,string biname,string lname){
  cerr << "\nfragment_data_nostoplist("+fname+","+lname+")";
  HS NOLIST;
  char strbuf[10000];
  ifstream in2(lname.c_str());
  while (in2.peek() != -1){
    string w = "";    in2 >> w;    in2.getline(strbuf,10000);
    if (w != "")      NOLIST.insert(w);
  }  cerr << "\t|NOLIST| = " << NOLIST.size();

  ifstream in(fname.c_str());
  ifstream inbi(biname.c_str());
  //  ofstream out((fname+"_frag_nolist").c_str());
  while (in.peek() != -1){
    string id = "", idbi = "";
    in >> id; inbi >> idbi;
    //    cerr << endl << id << "\t";
    if (id != ""){
      vector<vector<string> > sequence;
      vector<string> words, poses, tags;
      while (in.peek() == 9){
	string w, pos, tag;
	inbi >> w >> pos >> tag;
	words.push_back(w);
	poses.push_back(pos);
	tags.push_back(tag);
	//	cerr << w << " ";
 	vector<string> feats;
 	string f = "";
 	in >> f; 	feats.push_back(f);
 	while (in.peek() == 32){
 	  in >> f; 	  feats.push_back(f);
 	}
 	sequence.push_back(feats);
      }

      int start = 0;
      while (start < words.size()){
	//	cerr << "\nstart = " << start;
	int lx = -1, rx = -1;
	for (int i = start; i < words.size(); ++i)        	//look forward
	  if (tags[i][0]=='B'){
	    lx = i;
	    rx = i;
	  }
	//	cerr << "\nlx/rx = " << lx << "/" << rx << endl;
	if (lx == -1)
	  start = words.size();
	else {
	  --lx;
	  ++rx;
	  for (; lx >= 0; --lx){
	    //	    cerr << lx << "/" << words[lx];
	    HS::iterator N_i = NOLIST.find(words[lx]);
	    if (N_i != NOLIST.end() ||tags[lx]!="0"){}
	    else {
	      ++lx;
	      break;
	    }
	  }
	  lx = max(lx,int(0));
	  //	  cerr << "\n\tlx = " << lx << endl;
	  for (; rx < words.size(); ++rx){
	    //	    cerr << rx << "/" << words[rx] << " ";
	    HS::iterator N_i = NOLIST.find(words[rx]);
	    if (N_i != NOLIST.end() || tags[rx]!="0"){}
	    else {
	      --rx;
	      break;
	    }
	  }
	  rx = min(rx,int(words.size()-1));
	  //	  cerr << "\n\trx = " << rx;
	  cout << id;
	  for (int r = lx; r <= rx && r < words.size(); ++r){
	    //	    cout << "\t" << words[r] << " " << poses[r] << " " << tags[r];
	    cout << "\t" << sequence[r][0];
	    for (int g = 0; g < sequence[r].size(); ++g)
	      cout << " " << sequence[r][g];
	    //	    cout << " " << tags[r];
	  }
	  cout << endl;

	  start = rx+1;
	  //	  cerr << endl << "\nstart = " << start;	  getchar();
	}
      }
    }
    in.getline(strbuf,10000);
    inbi.getline(strbuf,10000);
  }
  cerr << "\tdone";
}

void bi_to_line(string fname){
  cerr << "\nbi_to_line("+fname+")";
  char strbuf[1000];
  ifstream in(fname.c_str());
  while (in.peek() != -1){
    string id = "";
    in >> id;
    vector<string> W;
    while (in.peek() == 9){
      string w, pos, tag;
      in >> w >> pos >> tag;
      W.push_back(w);
    }
    in.getline(strbuf,1000);
    if (W.size()){
      cout << W[0];
      for (int i = 1; i < W.size(); ++i)
	cout << " " << W[i];
      cout << endl;
    }
  }
  cerr << "\tdone";
}


void load_tmap(string fname,map<string,string>& tmap){
  cerr << "\n\tload_tmap("+fname+")";
  char strbuf[100];
  ifstream in(fname.c_str());
  while (in.peek() != -1){
    string m1 = "", m2, trash;
    in >> m1 >> trash >> m2;
    in.getline(strbuf,100);
    if (m1!="")
      tmap.insert(make_pair(m1,m2));
  }
  cerr << "\t|tmap| = " << tmap.size();
}

void map_tags(string dataname,string mapname){
  cerr << "\nmap_tags("+dataname+","+mapname+")";
  map<string,string> tmap;
  load_tmap(mapname,tmap);
  char strbuf[1000];
  ifstream in(dataname.c_str());
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      cout << id;
      while (in.peek() == 9){
	string w, pos, tag;
	in >> w >> pos >> tag;
	string newtag = "0";
	if (tag != "0"){
	  map<string,string>::iterator tm_i = tmap.find(string(tag,2,tag.size()-2));
	  if (tm_i != tmap.end()){
	    newtag = "-"+(*tm_i).second; 
	    if (tag[0]=='B')
	      newtag = "B"+newtag;
	    else newtag = "I"+newtag;
	  }
	}
	cout << "\t" << w << " " << pos << " " << newtag;
      }
      cout << endl;//getchar();
    }
  }
  cerr << "\tdone";
}


////////////////////////////////////////////////////////////////
// VSRP (Very Sparse Random Projection dimensionality reduction)
////////////////////////////////////////////////////////////////

void VRSP_make_B(string fname,int k,bool secondorder){
  cerr << "\nVSRP_make_B("+fname+"," << k << ")";
  string dirname, filename, sufname;
  parse_fname(fname,dirname,filename,sufname);
  tagger_light TL;
  Zr D;
  vector<vector<int> > G;
  vector<string> ID;
  TL.load_data_R(fname,D,G,ID,secondorder,false,true);  
  int d = TL.FIS.size();
  Float s = sqrt(Float(d)), sqrt_s = sqrt(s), one_on_s = 1/s, one_on_2s = 1/(2*s);
  cerr << "\n\td = " << d << "\ts = " << s << "\t1/s = " << one_on_s << "\t1/2s = " << one_on_2s;
  ofstream outb((filename+".B"+itos(k)).c_str());
  outb << d << "\t" << k << "\t" << s << endl;
  int zeros = 0;
  for (int j = 0; j < k; ++j){
    outb << j;
    for (int i = 0; i < d; ++i){
      Float R = (Float) rand()/RAND_MAX;
      if (R < one_on_2s)
	outb << "\t" << TL.FIS[i] << " 1";
      else if (R < 2*one_on_2s)
	outb << "\t" << TL.FIS[i] << " -1";
      else ++zeros;
    }
    outb << endl;
  }
  cerr << "\tzeros = " << zeros << "/" << k*d << " = " << zeros/Float(k*d);
}

void VSRP_transform(string fname,string bname,bool islabeled){
  cerr << "\nVSRP("+fname+","+bname+")";
  ifstream in(fname.c_str());
  char strbuf[1000];
  int d, k;
  Float s;
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      while (in.peek() == 9){
	vector<pair<string,Float> > O;
	pair<string,Float> o;
	in >> o.first >> o.second;
	O.push_back(o);
	while (in.peek() == 9){
	  in >> o.first >> o.second;
	  O.push_back(o);
	}
      }
    }
    in.getline(strbuf,1000);
  }

}

/////////////////////
void ADAPT_reprint_int(vector<string>& FIS,HM& FSI,string fname,string oname,vector<string>& Tags,string tname){
  ofstream out(oname.c_str());
  ifstream in(fname.c_str());
  char strbuf[1000];
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      int count = 0;
      while (in.peek() == 9){
	vector<pair<string,Float> > O;
	pair<string,Float> o;
	in >> o.first >> o.second;
	O.push_back(o);
	while (in.peek() == 32){
	  pair<string,Float> o2;
	  in >> o2.first >> o2.second;
	  O.push_back(o2);
	}
	if (count++ > 0)
	  out << "\t";
	out << update_hmap(FSI,O[0].first,FIS) << " " << O[0].second;
	for (int i = 1; i < O.size()-1; ++i)
	  out << " " << update_hmap(FSI,O[i].first,FIS) << " " << O[i].second;
	Tags.push_back(O[O.size()-1].first);
      }
      out << endl;
    }
    in.getline(strbuf,1000);
  }
  ofstream outt(tname.c_str());
  for (int i = 0; i < Tags.size(); ++i)
    outt << Tags[i] << endl;
}

void ADAPT_olivier_format_data(string fname1,string fname2){
  cerr << "\nADAPT_olivier_format_data("+fname1+","+fname2+")";
  HM FSI;
  vector<string> FIS, T1, T2;
  ADAPT_reprint_int(FIS,FSI,fname1,fname1+"_int",T1,fname1+"_tags");
  cerr << "\t|FIS| = " << FIS.size();
  ADAPT_reprint_int(FIS,FSI,fname2,fname2+"_int",T2,fname2+"_tags");
  cerr << "\t|FIS| = " << FIS.size();
  ofstream out("FMAP");
  for (int i = 0; i < FIS.size(); ++i)
    out << FIS[i] << endl;
}

/////////////

void remove_pref_feats_R(string fname,string pref){
  cerr << "\nremove_posfeats)"+fname+")";
  char strbuf[100];
  ifstream in(fname.c_str());
  int _pref_ = pref.size();
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      cout << id;
      while (in.peek() == 9){
	vector<pair<string,Float> > O;
	pair<string,Float> p1;
	in >> p1.first >> p1.second;
	if (p1.first.size() <= _pref_ || string(p1.first,0,_pref_) != pref)
	  O.push_back(p1);
	while (in.peek() == 32){
	  pair<string,Float> pi;
	  in >> pi.first >> pi.second;
	  if (pi.first.size() <= _pref_ || string(pi.first,0,_pref_) != pref)
	    O.push_back(pi);
	}
	cout << "\t" << O[0].first << " " << O[0].second;
	for (int i = 1; i < O.size(); ++i)
	  cout << " " << O[i].first << " " << O[i].second;
      }
      cout << endl;
    }
  }
}
