//////////////////////////////////////////////////////////////////////////
//
// Sentence Splitter
//
// Copyright Massimiliano Ciaramita and Jordi Atserias
//
/////////////////////////////////////////////////////////////////////////7

#ifndef _NLPYAHOOBNC_SENSPLIT_

#define _NLPYAHOOBNC_SENSPLIT_

using namespace std;
#include <iostream>
#include <iterator>
#include <vector>
#include <set>
#include <fstream>
#include <iterator> 
#include <sstream>

#include "tokenizer.hxx"

///DEBUGING?
#define DEBUG 0

namespace nlpyahoobcn {






  /**
  A class for recognizing and splitting senteces
  It behaves as an iterator
  **/
class sentenceSequence {

private:
  static const int max_abbr = 0;
  static const int min_abbr = 100;

  // to store docids (if any)
  string docid;
  string ndocid;

  string  pWord;
  string  cWord;
  string sentence;
  string posWord;


  istreambuf_iterator<char>  inpos;
  // end-of-stream iterator
  istreambuf_iterator<char> endpos;

 set<string>  ABBR;
  void load_ABBR(char * exceptionFile);


  // Some minor functions

inline bool basicsym(const char c) const  {
  return (c=='.' || c=='?' || c=='!');
}


//? range?
// Next word is Capital letter word space Capital
//  and previos word previous-last-letter is not (Capital [.?!] space Capital)
bool next_char_begins(string s, char nChar) const {
  return (isupper(nChar) && !isupper(s[s.size()-2]));
}




//?? 
inline bool is_abbreviation(string s){
    set<string>::iterator A_i = ABBR.find(s);
    return (A_i != ABBR.end());

}


inline bool wordSep(const char c) const {
  return (c == ' ' ||  c =='\t' || c=='\n');
}


inline bool endOfSentence(string pWord, string cWord) {
  #if DEBUG 
  cerr<<"READ "<<pWord<<"  ["<<cWord<<"]"<<endl;
 #endif 
 

  if (basicsym(pWord[pWord.size()-1]) && next_char_begins(pWord,cWord[0]) && !is_abbreviation(pWord)) return true;
  //Ups need 2 look ahead  to avoid breaking  . "<EOF>
  if (cWord[0] =='"' && basicsym(pWord[pWord.size()-1]) && !basicsym(cWord[0])) return true;
  return false;
}






  

public: 

  sentenceSequence(istream & pin) : inpos(pin) { 
      load_ABBR("exception_list");
      readWord();
  }

  string next(void) {
    docid=ndocid;
    priv_EOS();
    return sentence;
  }
 
  bool EOD(void) const  { return  inpos == endpos;}

   void readBasicWord(void) {
    cWord.clear();
    while(inpos != endpos &&  ! wordSep(*inpos))  { 
	 cWord=cWord+*inpos; ++inpos; 
       }
  }

  string getDocId(void) { return docid;}

private:

  // some internal functions

 bool  readWord(void) {
   bool seof = ORGreadWord();
   string oWord=cWord;
   //ORGreadWord();
   //posWord=cWord; 
   cWord=oWord;
   return seof;
 }



 bool  ORGreadWord(void) {
    bool seof=false;
    readBasicWord(); 
    while(cWord.compare("%%#DOC")==0 || cWord.compare("%%#LINE")==0 || cWord.compare("%%#SEN")==0) {
      ///@TODO set the docid
      if(cWord.compare("%%#DOC")==0) { 
        ndocid="";
	while(inpos != endpos &&  *inpos !='\n') {
	  ndocid=ndocid+(*inpos);
	  ++inpos;
	}
      }
      else
	while(inpos != endpos &&  *inpos !='\n') ++inpos;
        
           while(inpos != endpos &&  wordSep(*inpos))++inpos; 
          seof=true; 
          readBasicWord();
    }
    while(inpos != endpos &&  wordSep(*inpos))++inpos;
    return seof;
  }



  
  void priv_EOS(void) {

     
     // check first word
      pWord=cWord;
      sentence= pWord; // +" "+posWord; 

      bool seof = readWord();
      seof= seof || ((inpos == endpos) || endOfSentence(pWord,cWord));
     
     // check eof.
      while(!seof){
       
	sentence=sentence+" "+cWord; //+" "+posWord;
       pWord=cWord;

       seof = readWord();

       seof= seof || ((inpos == endpos) || endOfSentence(pWord,cWord));
       
      
     };
     
 
     while(inpos != endpos &&  (*inpos == ' ' ||  *inpos =='\n'))++inpos;
       
     // Reached EOF (pWord is not empty cWord could be)
     if(inpos == endpos && !cWord.empty()) { 
       sentence= sentence+" "+cWord; // +" "+posWord;
     }
     
  }
};



inline void sentenceSequence::load_ABBR(char * exceptionFile){
  ifstream in(exceptionFile);
  set<string> tmp;
  
  char strbuf[1000];
  while (in.peek() != -1){
    string w;
    in >> w;
    in.getline(strbuf,1000);
    ABBR.insert(w);
  }
#if DEBUG
  cerr << "\n|ABBR| = " << ABBR.size() << "\tmax_abbr = " << max_abbr << "\tmin_abbr = " << min_abbr;
#endif
}

}

#endif
