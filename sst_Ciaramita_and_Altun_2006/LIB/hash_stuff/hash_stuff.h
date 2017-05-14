// hash_stuff.h: part of sequence tagger for Wordnet supersenses, and other tagsets
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


#ifndef HASH_STUFF_H
#define HASH_STUFF_H

#include <string>
#include <vector>
#include <ext/hash_map>
#include <ext/hash_set>
#include <cstring>

using namespace __gnu_cxx;
typedef hash_map<std::string,unsigned> HM;
typedef hash_set<std::string> HS;

///////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__
// Used in hash_set, hash_map
namespace std
{
 template <>
 struct equal_to<char const*> :
   public binary_function<char const*, char const*, bool>
 {
   bool operator()(first_argument_type __x, first_argument_type __y) const {
     return ::strcmp(__x, __y) == 0;
   }
 };
}

#if __GNUC__ >= 3
namespace __gnu_cxx
#else
namespace std
#endif
{
 template<> struct hash<std::string>
 {
   size_t operator()(const std::string& x) const
   {
     return hash<const char*>()(x.c_str());
   }
 };
}
#endif

inline void update_hmap_count(HM& h,std::string w){
  HM::iterator hm_i = h.find(w);
  if (hm_i == h.end())
    h[w] = 1;
  else
    ++hm_i->second;
}

inline unsigned update_hmap(HM& h,std::string w){
  unsigned ans = h.size();
  HM::iterator hm_i = h.find(w);
  if (hm_i == h.end())
    h[w] = ans;
  else
    ans = hm_i->second;
  return ans;
}

inline unsigned update_hmap(HM& h,std::string w,std::vector<std::string>& V_STRINGS){
  unsigned ans = h.size();
  HM::iterator hm_i = h.find(w);
  if (hm_i == h.end()){
    h[w] = ans;
    V_STRINGS.push_back(w);
  }
  else
    ans = hm_i->second;
  return ans;
}

#endif

