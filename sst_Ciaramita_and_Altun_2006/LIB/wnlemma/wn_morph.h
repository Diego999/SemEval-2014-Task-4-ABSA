#include <cstdio>
#include <iostream>
#include <fstream>
#include <wn.h>

using namespace std;

///////////////////////////////////////////////////////////
bool wn_morph(string wx,string &_wd,int _pos);
string wn_base(const string & wx,int _pos);
string wnmy_tolower(const string & w);
string getLemma(const string & w,const string & pos );
