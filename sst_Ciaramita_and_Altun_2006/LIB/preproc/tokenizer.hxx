/**
a three character lookahead tokenizer
**/

#ifndef _NLPYAHOOBNC_TOKENIZER__

#define _NLPYAHOOBNC_TOKENIZER__

using namespace std;

#include <iostream>
#include <iterator>
#include <vector>
#include <set>
#include <fstream>

#define DEBUG 0



#define _TOKEN_LOOKAHEAD_ 3

#define DOUBLEQUOTECHAR '"'
#define OPENSINGLEQUOTECHAR '`'
#define CLOSESINGLEQUOTECHAR '\''

namespace nlpyahoobcn {

/**
 a class tokenizer template
**/
template<typename T>
class tokenizer {

private:

  string bufferToken;
  char buffer[_TOKEN_LOOKAHEAD_+2];
  int bufferIndex;
  int bufferSize;
  bool eos;
  bool bos;


   T inpos;
  ///  only to check the end of the stream iterator
   T endpos;

private:

  void readLookAhead(void);
  const char currentChar(void) const;
  const char lookAhead1(void);
  const char lookAhead2(void); 
  const char lookAhead3(void);

  bool beginSentence(void) const;
  bool endSentence(void) const;

  bool punctuation(const char s) const;
  bool isSpace(const char s) const;
  bool extClosingBracked(const char s) const;
  bool openBracked(const char s) const;

  string internalNextToken(void);

public:

  /// Cosntructor: needs two stream iterators
  tokenizer( T  ppos,  T pendpos);
  
  /// true if there is some token remaining (false otherwise) 
  bool hasMore(void) const;
  
  /// return next token in the sequence and advance the iterator
  string nextToken(void);

 
};

template <typename T>
tokenizer<T>::tokenizer( T  ppos,  T pendpos) : inpos(ppos), endpos(pendpos) {
    bufferIndex=0; bufferSize=0; eos=false; bos=true;
    readLookAhead();
  }

template <typename T>
void tokenizer<T>::readLookAhead(void) {

    while (inpos != endpos && bufferSize<(_TOKEN_LOOKAHEAD_+1)) {
      #if DEBUG
      cerr<<"SET "<<((bufferIndex + bufferSize) % (_TOKEN_LOOKAHEAD_+ 1))<<" to "<<(*inpos)<<endl;
      #endif
      if(isSpace(*inpos)){ 
        buffer[(bufferIndex + bufferSize) % (_TOKEN_LOOKAHEAD_+ 1)]=' ';
        ++inpos; 
        while (inpos != endpos && isSpace(*inpos)) {++inpos; }
        }
      else {
       buffer[(bufferIndex + bufferSize) % (_TOKEN_LOOKAHEAD_+ 1)]=*inpos; 
       ++inpos;
      }
      ++bufferSize;
    }
  
    // fullfill everything with whitspaces
    while (bufferSize< (_TOKEN_LOOKAHEAD_+1)) {buffer[(bufferIndex + bufferSize ) % (_TOKEN_LOOKAHEAD_+ 1)]=' '; ++bufferSize; eos=true;}
   
    }

template <typename T>
const char tokenizer<T>::currentChar(void) const { 
  return buffer[bufferIndex]; 
}
 
template <typename T>
const char tokenizer<T>::lookAhead1(void) {
  return buffer[(bufferIndex+1) % (_TOKEN_LOOKAHEAD_+1)]; 
}

template <typename T>
const char tokenizer<T>::lookAhead2(void) {
  return buffer[ (bufferIndex+2) % (_TOKEN_LOOKAHEAD_+1)]; 
}

template <typename T>
const char tokenizer<T>::lookAhead3(void) {
  return buffer[ (bufferIndex+3) % (_TOKEN_LOOKAHEAD_+1)]; 
}
  
template <typename T>
   bool tokenizer<T>::beginSentence(void) const {return bos;}

  // BUG: Buffer end of sentence!?
template <typename T>
  bool tokenizer<T>::endSentence(void ) const { return eos;}



/// basic checks
template <typename T>
  bool tokenizer<T>::punctuation(const char s) const {
    return (s==',' || s==';' || s==':' || s=='#' || s=='$' || s=='&' || s == '!' || s=='?' || s=='[' || s==']' || s=='(' || s==')' || s=='{' || s=='}');
  }

template <typename T>
   bool tokenizer<T>::isSpace(const char s) const { return s==' ' || s== '\t' || s== '\n';  }
    
template <typename T>
   bool tokenizer<T>::extClosingBracked(const char s) const  { 
    return (s==')' || s==']' || s=='>' || s== '}');
  }

template <typename T>
   bool tokenizer<T>::openBracked(const char s) const { 
   return (s=='(' || s=='[' || s=='<' || s== '{');
  }



template <typename T>
   bool  tokenizer<T>::hasMore(void) const {
    return !(eos && bufferToken.empty() && currentChar()==' ');
  }

template <typename T>
    string  tokenizer<T>::nextToken(void) {
     if(bufferToken.empty()) { 
       return internalNextToken();
     }
     else {
       string s=bufferToken;
       bufferToken.clear();
       return s;
     }
   }

template <typename T>
   string tokenizer<T>::internalNextToken(void) {
    string token="";
    bufferToken="";
    int comsumed=0;
    bool eot = false;
    do {
      #if DEBUG
        cerr<<"B WINDOW "<<"|"<< currentChar()<<"|"<<lookAhead1()<<"|"<<lookAhead2()<<"|"<<lookAhead3()<<"|"<<endl;
      #endif
    // Check s=``= `` =g
    if(currentChar()== OPENSINGLEQUOTECHAR && lookAhead1() ==OPENSINGLEQUOTECHAR) { 
      token = "``"; comsumed=2; 
    }


    // Check s=''= '' =g
    if(currentChar()== CLOSESINGLEQUOTECHAR && lookAhead1() == CLOSESINGLEQUOTECHAR) { 
      token = "''"; comsumed=2; 
    }

    // Check s=^"=`` =g *added espace)
    else if(currentChar()==DOUBLEQUOTECHAR && ( beginSentence() || bufferToken.empty())) { 
      // advance
      token= "``"; comsumed=1; 
      #if DEBUG
         cerr<<"Sentence/token  starting quote rule"<<endl; 
      #endif
    }


    // s=\([ ([{<])"=``=g ups braket" => <braket> <``> (speace is treated previously)
    else if(openBracked(currentChar()) && lookAhead1()==DOUBLEQUOTECHAR) {
      bufferToken = bufferToken +currentChar();
      token="``"; 
      comsumed=2; 
    }

    // s=[,;:#$&]= & =g and  s=[!?]= & =g
    else if(punctuation(currentChar())) {
      token=currentChar(); comsumed=1; 
    }


    // s=\.\.\.= ... =g
    else if(currentChar()=='.' && lookAhead1()=='.' && lookAhead2()=='.') {
      token="..."; comsumed=3;
    }
   

    // BUG This is possible  infinite lookAhead!
    // # Assume sentence tokenization has been done first, so split FINAL periods  only. 
    // s=\([^.]\)\([.]\)\([])}>"']*\)[ 	]*$=\1 \2\3 =g
    // last dot should be split? ... but
    //if(currentChar()!='.' && lookAhead1()=='.' && (( extClosingBracked(lookAhead2()) && extClosingBracked(lookAhead3())) || eos))
    
    else if(currentChar()=='.' && (( extClosingBracked(lookAhead1()) && extClosingBracked(lookAhead2()) && extClosingBracked(lookAhead3())) || eos))
    {
      token="."; comsumed=1;
      #if DEBUG
       cerr<<"rule BUG"<<endl;
      #endif
    }
    
      // s=--= -- =g
    else if(currentChar()=='-' && lookAhead1()=='-') {
        token="--"; comsumed=2;
    }

    // ?? should we apply to the token build??
     // remaining ambigous quotes are closing quotes
    else if(currentChar()==DOUBLEQUOTECHAR) {
        token="''"; comsumed=1;
       #if DEBUG
        cerr<<"starting quote rule"<<endl;
       #endif 
    }
  
     // # possessive or close-single-quote
     // s=\([^']\)' =\1 ' =g
    else if(currentChar()!='\'' && lookAhead1()=='\'' && lookAhead2()==' ') {
         // next quote should be also a token
          bufferToken = bufferToken +currentChar();token="'"; comsumed=2;
     }
           
    else if(currentChar()=='\'') {
    // # as in it's, I'm, we'd
      if( (lookAhead1()=='s' ||  lookAhead1()=='S' ||
         lookAhead1()=='m' ||  lookAhead1()=='M' ||
	   lookAhead1()=='d' ||  lookAhead1()=='D') && lookAhead2()== ' ' ) {
         token = token + currentChar() ; 
         token = token + lookAhead1();
         comsumed=2;
	 #if DEBUG
          cerr<<"CCHAR "<<currentChar()<<endl;
          cerr<<"LH1 "<<lookAhead1()<<endl;
          cerr<<"aplied rule 's token:"<<token<<endl;
	 #endif
      }
      else if(  (lookAhead1()=='l' &&  lookAhead2()=='l') ||
          (lookAhead1()=='L' &&  lookAhead2()=='L') ||
          (lookAhead1()=='r' &&  lookAhead2()=='e') ||
          (lookAhead1()=='R' &&  lookAhead2()=='E') ||
          (lookAhead1()=='v' &&  lookAhead2()=='e') || 
          (lookAhead1()=='V' &&  lookAhead2()=='E') && lookAhead3()== ' ') {
         token = token +currentChar() ; 
         token = token + lookAhead1();
         token = token + lookAhead2();
         comsumed=3;
     }
     }
     else if(  (currentChar()=='n' ||  currentChar()=='N') &&
          (lookAhead1()=='\'') &&
          (lookAhead2()=='t' ||  lookAhead2()=='T')) {
         token = token + currentChar() ; 
         token = token + lookAhead1();
         token = token + lookAhead2();
         
         comsumed=3;
     }
    
    

     // move window 
    #if DEBUG
     cerr<<"TOKEN:"<<token<<endl;
     cerr<<"CUR CHAR:"<<currentChar()<<endl;
    #endif

     if(token.empty())
       {
       if(currentChar()!=' ') {
	  #if DEBUG 
	   cerr<<"add char"<<endl;
	  #endif
	 bufferToken= bufferToken+ currentChar(); comsumed=1;
       }
       else { 
         eot=true; 
          #if DEBUG 
	   cerr<<"rule applied"<<endl;
	  #endif
       }
       }
   
      #if DEBUG  
        cerr<<"BufferToken:"<<bufferToken<<endl;
      #endif

      bufferSize=  bufferSize - comsumed;
      bufferIndex = (bufferIndex + comsumed) % (_TOKEN_LOOKAHEAD_+1);
       #if DEBUG 
      cerr<<"Size "<<bufferSize<<" index "<<bufferIndex<<" token "<<token<<" bufferToken "<<bufferToken<<endl;
      #endif

      while (bufferSize>0 && currentChar()==' ') {
	--bufferSize;
        bufferIndex = (bufferIndex + 1) % (_TOKEN_LOOKAHEAD_+1); 
        eot=true; //maybe we can avoid that imposgin token is not empty
      }

     readLookAhead();  

      #if DEBUG 
     cerr<<"Size "<<bufferSize<<" index "<<bufferIndex<<" token "<<token<<" bufferToken "<<bufferToken<<endl;
     cerr<<"EWINDOW "<<"|"<< currentChar()<<"|"<<lookAhead1()<<"|"<<lookAhead2()<<"|"<<lookAhead3()<<"|"<<endl;
     #endif

    } while(token.empty() &&  !eot);

     
    bos=false;
  
  // Choose
     string s;
     if(bufferToken.empty()) {s=token;}
     else {  s=bufferToken; bufferToken=token;}
     return s;
  }
};

/**

// TEST MAIN
int main() {
  istreambuf_iterator<char> tend;

  tokenizer<istreambuf_iterator<char> > t(cin,tend);

  while(t.hasMore()) {
    cout<<t.nextToken()<<endl;
   
  }
  return 1;
}
**/

#endif
