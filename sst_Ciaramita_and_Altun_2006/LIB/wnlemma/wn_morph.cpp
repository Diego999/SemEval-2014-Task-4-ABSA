// wn_morph.cpp Massimiliano Ciaramita (C) 2004
// Morphologically simplifies nouns, verbs, adjectives and adverbs using
// several Wordnet library functions. Needs the compiled libWN.a from (if installed)
// Wordnet.

// bool wn_morph(string wx,string &_wd,int _pos)
// returns false if wx is not in Wordnet, otherwise it returns true and _wd contains 
// the morphologically simplified form of wx. The integer pos must ve one of these:
// 1 - nouns (common proper)
// 2 - verbs
// 3 - adjectives
// 4 - adverbs
// If wx consists of several words the components must be separated with an hyphen.

#include "wn_morph.h"
#include <map>


string wn_base(const string & wx,int _pos) {
  static bool not_initialized = true;
  if (not_initialized) {
    not_initialized = false;
    cerr<<"WNINIT:"<<wninit()<<endl;
    
}

  char *w = const_cast<char*>(wx.c_str());
  char* possible_base = morphstr(w, _pos);   // Can Wordnet simplify this word?
  if(possible_base==NULL) {return "NULL";}
  else {return possible_base;}
}



///////////////////////////////////////////////////////////
bool wn_morph(string wx,string &_wd,int _pos){
  bool ans = false;
  string _w = "";
  string::iterator w_i = wx.begin();
  for (; w_i != wx.end(); ++w_i)
    _w += *w_i;

  _wd = "";
  static bool not_initialized = true;
  if (not_initialized) {
    not_initialized = false;
    wninit();
  }
  
  char *w = const_cast<char*>(_w.c_str());
  char* possible_base = morphstr(w, _pos);   // Can Wordnet simplify this word?
  // cerr << "\npossible_base = " << possible_base;//getchar();
  char *w2 = const_cast<char*>(possible_base);
 
  if(possible_base == NULL)  { w2=w;
  cerr<<"\npossible_base:"<<possible_base<<endl;    
  }
  else cerr<<"\nNULL\n";

  IndexPtr I2 = getindex(w2,_pos);
  if (I2 != NULL){
    Index In2 = *I2;
    _wd = In2.wd;
    ans = true;
    free_index(I2); 
  }
  else {
    IndexPtr I = getindex(w,_pos);
    if(I != NULL){
      Index In = *I;
      _wd = In.wd;
      ans = true;
      free_index(I);
    }
  }
  return ans;
}

void test_wnmorph(){
  while(1){
    string w, pos, _w;
    cerr << "\nw:\n";
    cin >> w;
    cerr << "\npos:\n";
    cin >> pos;
    string wm = wn_morph(w,_w,atoi(pos.c_str())) ? _w : "NULL";
    cerr << "\nw:" << w << " -> " << wm; getchar();
  }
}

///////////////////////////////////////////////////

string wnmy_tolower(const string &s){
  string ans = "";
  string::const_iterator s_i = s.begin();
  for (; s_i != s.end(); ++s_i){
    char c = *s_i;
    ans += tolower(c);
  }
  return ans;
}

void proc_files(){
  string dir = ".", fname = "wikifiles.txt";
 // string dir = "../", fname = "../wikifiles.test";
  cerr << "\nload_files("+fname+")";
  map<string,string> M;
  ifstream in(fname.c_str());
  char strbuf[10000];
  while (in.peek() != -1){
    string f = "";
    in >> f;
    cerr << "\n" << f;
    if (f != ""){
      ifstream in2((dir+"/"+f).c_str());
      for (int i = 0; i < 3; ++i)
	in2.getline(strbuf,10000);

      while (in2.peek() != -1){
	string w = "", pos;
	in2 >> w >> pos;
        cerr << "\nw = " << w<< "_"<<pos;
	if (w != "" && isalnum(w[0]) && isalnum(w[w.size()-1])){
	  string lemma; 
          if (pos=="NN" || pos=="NNS"){
	    lemma = wn_base(w,1);	    
	  }
	  else if (pos[0] == 'V'){
	    lemma = wn_base(w,2);	    
	  }
	  else if (pos[0] == 'J'){
	    lemma = wn_base(w,3);	    
	  }
	  else if (pos[0] == 'R'){
	    lemma = wn_base(w,4);	    
	  }
          else {lemma="NULL";}
          cerr<<"LEMA "<<lemma<<endl;    
	  if (lemma != "NULL"){
	    M.insert(make_pair(wnmy_tolower(w)+"-"+pos[0],lemma));
	      cerr << "\n\t" << w << "\t" << lemma; //getchar();
	  }
	}
	in2.getline(strbuf,10000);
      }
    }
    cerr << "\t|M| = " << M.size();
    in.getline(strbuf,10000);
  }
  ofstream out("M_CACHE");
  for(map<string,string>::iterator M_i = M.begin(); M_i != M.end(); ++M_i)
    out << (*M_i).first << "\t" << (*M_i).second << endl;
}

/**
int main (int argc, char** argv){
 // test_wnmorph();
  proc_files();
  cerr<<"ENDED"<<flush;
}
**/
//
 
string getLemma(const string & w,const string & pos ) {
  if (w != "" && isalnum(w[0]) && w.size()>1 && isalnum(w[w.size()-1]) && !(pos[0] == 'N' & pos[2]=='P')){
    string lemma;
    if (pos[0]== 'N'){
      lemma = wn_base(w,1);
    }
    else if (pos[0] == 'V'){
        lemma = wn_base(w,2);
      }
    else if (pos[0] == 'J'){
      lemma = wn_base(w,3);
    }
    else if (pos[0] == 'R'){
      lemma = wn_base(w,4);
    }
    else {lemma="NULL";}

    if(lemma == "NULL") return wnmy_tolower(w);
    else return lemma;
  }
  return w;
} 
