// stats.h: part of sequence tagger for Wordnet supersenses, and other tagsets
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

#ifndef STATS_H
#define STATS_H

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <map>
#include <cmath>
#include <vector>
#include <list>

using namespace std;
typedef double Float;

Float KL(map<string,Float> &P,map<string,Float> &Q,Float NP,Float NQ,Float _W_,Float alpha);
Float CHI(map<string,Float> &P,map<string,Float> &Q,Float minf);
Float variance(vector<Float> &X,Float mu);
Float std_err(vector<Float> &X,Float mu);
Float mean(vector<Float> &X);
void Xmax(vector<Float> &X,vector<int> &MAX);
//inline Float standardize(Float x, Float mu, Float sigma);
inline Float z_score(Float x, Float mu, Float sigma){
  return (x-mu)/sigma;
}

template <class T>
Float sum_map(map<string,T> &DATA);

#endif
