//   Chain.cpp: part of sequence tagger for Wordnet supersenses, and other tagsets
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

#include "Chain.h"
#define DEB 0

///////////////////////////////////////////////////////////////////////
//
//  This function should probably go to some data directory
//
// Inner product
Float inner( X &x, X &y ) {
   int val = 0;
   int s1 = x.size(), s2 = y.size();
   if( x[0] == y[0] ) val++;
   if( x[1] == y[1] ) val++;
   for( int i1 = 2, i2 = 2; i1 < s1 && i2 < s2 ;) 
	if( x[i1] == y[i2] ) {
	     val++; i1++; i2++;
	} else if (  x[i1] <  y[i2] ) i1++;
	else i2++;	
   return val;
}

///////////////////////////////////////////////////////////////////////
// Viterbi:

void  Chain::viterbi_tj( vector<vector<Float> > &delta, X &x, int t, int j,
	vector<vector<int> > &psi ) {
  Float wc = get_word_contribution( x, j, t );
  if ( t == 0 ){
    psi[t][j] = -1 ; 
    delta[t][j] =  wc + get_phi_s( j ) ;
    return;
  }
  Float max = -1e100; int argmax = 0;
  for (int i = 0; i < k; ++i){
      Float score_ij = delta[t-1][i] + wc + get_phi_b( i, j );
      if (score_ij > max ){
	max = score_ij;
	argmax = i;
      }
    }
  psi[t][j] = argmax ;
  delta[t][j] = max ;
}

void decode_s(vector<int> &_u,vector<vector<Float> > &delta,vector<vector<int> > &psi,int argmax_t){
  int N = delta.size();
  int best = argmax_t;
  _u.insert( _u.begin(), argmax_t );
  for ( int t = N - 1 ; t > 0 ; --t ){
    _u.insert( _u.begin(), psi[t][best] );
    best = psi[t][best];
  }
#if DEB
  cerr << 

#endif
}

void remove_empty( vector<vector<bool> > &emp, vector<int> &_u ) {
   int N = _u.size();
   for( int i = 0; i < N ; ++i )
      if( emp[i][_u[i]] )
        _u[i] = -1;
   return;
}

void Chain::viterbi( S &W, vector<int> &U ){
  int N = W.size(), argmax_t = 0;
  vector<vector<Float> > delta( N, vector<Float>( k, 0.0 ) );
  vector<vector<int> > psi( N, vector<int>( k, 0 ) );
// ATTENTION!!!
// Incorporate these lines into semisupervised code
//  if( mode == Train ) 
//      avr = false;
  for( int t = 0; t < N; ++t ) 
    for ( int j = 0; j < k; ++j ) 
      viterbi_tj( delta, W[t], t, j, psi );
  Float max_t = -1e100;
  for (int j = 0; j < k; ++j){
	Float last_bit = delta[N-1][j] + get_phi_f( j );
	if ( last_bit > max_t ){
	  max_t = last_bit;
	  argmax_t = j;
	}
  }
  decode_s( U, delta, psi, argmax_t );
}


void Chain::print_viterbi_matrix(S &W,vector<int> &U,vector<string> &FIS,vector<string> &LIS){
  cerr << "\nprint_viterbi_matrix()";
  ofstream out("XY_OUT/V_matrix.tex");
  out << "\\documentclass[10pt]{article}\n\\usepackage[all]{xy}\n\\begin{document}\n\\xymatrix{";
  int N = W.size(), argmax_t = 0;
  vector<vector<Float> > delta( N, vector<Float>( k, 0.0 ) );
  vector<vector<int> > psi( N, vector<int>( k, 0 ) );
  for( int t = 0; t < N; ++t ) 
    for ( int j = 0; j < k; ++j ) 
      viterbi_tj( delta, W[t], t, j, psi );
  Float max_t = -1e100;
  for (int j = 0; j < k; ++j){
    Float last_bit = delta[N-1][j] + get_phi_f( j );
    if ( last_bit > max_t ){
      max_t = last_bit;
      argmax_t = j;
    }
  }
  decode_s( U, delta, psi, argmax_t );


  for (int j = 0; j < k; ++j){
    out << "\n" << LIS[j];
    for(int t = 0; t < N; ++t ){
      out << " & ";
    }
    out << " \\\\";
  }
  out << "\n";
  for(int t = 0; t < N; ++t )
    out << " & w_" << t;// << string(FIS[W[t][0]],3,int(FIS[W[t][0]].size())-3);
  out << "\n}\n\\end{document}";
}

void Chain::viterbi_matrix_sequences(Z &EVAL_D,L &EVAL_L,vector<string> &FIS,vector<string> &LIS){
  int _E_ = EVAL_D.size();
  for ( int i = 0; i < _E_; ++i ){
    vector<int> GUESS_i;                 
    print_viterbi_matrix(EVAL_D[i],GUESS_i,FIS,LIS);
    EVAL_L.push_back( GUESS_i );
  }
}

///////////////////////////////////////////////////////////////////////
// Backward Viterbi

void  Chain::backward_viterbi_tj( vector<vector<Float> > &delta,
	vector<Float> &word_cont, int t, int j, vector<vector<int> > &psi ){
  Float max = -1e100; int argmax = 0;
  for ( int i = 0; i < k; ++i ){
      Float score_ij = delta[t+1][i] + word_cont[i] + get_phi_b( j, i );
      if ( score_ij > max ){
	max = score_ij;
	argmax = i;
      }
  }
  psi[t][j] = argmax;
  delta[t][j] = max;
}

void Chain::backward_viterbi( S &W ){
// Computes the backward viterbi table, doesn't finish computing the 
// final transition and getting the best sequence!
  int N = W.size();
  vector<vector<Float> > delta( N, vector<Float>( k, 0 ) );
  vector<vector<int> > psi( N, vector<int>( k, 0 ) );
// ATTENTION!!!
// Incorporate these lines into semisupervised code
//  if( mode == Train ) 
//      average = false;
  for ( int j = 0; j < k; ++j ) {
    psi[N-1][j] = -1;
    delta[N-1][j] = get_phi_f( j );
  }
  for( int t = N - 2 ; t >= 0; --t ){
    vector<Float> word_cont;
    for ( int j = 0; j < k; ++j )
	word_cont.push_back( get_word_contribution( W[t+1], j, t+1 ));
    for ( int j = 0; j < k; ++j )
      backward_viterbi_tj( delta, word_cont, t, j, psi );
  }
}

///////////////////////////////////////////////////////////////////////
// Forward-Backward Probabilities

Float get_largest_exponent(vector<Float> &v1, vector<Float> &v2,
			vector<Float> &v3, Float v4, vector<Float> &b, int k) {
    Float max_exp = -1e100;  
    // Get the largest exponent multipling two tables
    for ( int j = 0; j < k; ++j ) {
	Float tmp = v1[j] + v2[j] + v3[j] + log( b[j] );
	if( tmp > max_exp ) max_exp = tmp;
    }
    return max_exp + v4;
}

inline Float get_total( vector<Float> &b1, vector<Float> &e1, vector<Float> &e2,
			vector<Float>  &e3, Float e4, int k, Float &max_emp ){
  max_emp = get_largest_exponent( e1, e2, e3, e4, b1, k ); 
  Float total = 0.0;
  for( int i = 0; i < k; ++i )
    total += b1[i] * exp( e1[i] + e2[i] + e3[i] + e4 - max_emp );
  if(!finite(total))
    for( int i = 0; i < k; ++i ) cerr << b1[i] << " " << e1[i] << " " << e2[i]
				      << " " << e3[i] << " " << e4 << " " << max_emp << endl;
  assert(finite(total));
  return total;
}

void forward_table( vector<vector<Float> > &obser_mat, vector<Float> &phi_s,
	vector<vector<Float> > &col_phi_b, int k, int N,
	vector<vector<Float> > &delta, vector<vector<Float> > &scale ) {
  // Get scaled forward table, scaling factors in for_scale

  // Initial position
  for ( int j = 0; j < k; ++j ) {
	delta[0][j] = 1.0;
	scale[0][j] = obser_mat[0][j] + phi_s[j] ;
  }
  // Rest of sequence
  vector<Float> vec0( k, 0.0 );
  for( int t = 1; t < N; ++t )
    for ( int j = 0; j < k; ++j )
      delta[t][j] = get_total( delta[t-1], scale[t-1], col_phi_b[j], vec0,
			  obser_mat[t][j], k, scale[t][j]);
}

void backward_table( vector<vector<Float> > &obser_mat, vector<Float> &phi_f,
	vector<vector<Float> > &phi_b, int k, int N,
	vector<vector<Float> > &delta, vector<vector<Float> > &scale){
  // Get scaled backward table, scaling factors in back_scale

  // End position
  for ( int j = 0; j < k; ++j ) {
     delta[N-1][j] = 1.0; 
     scale[N-1][j] = phi_f[j];
  }
  // Rest of sequence
  for( int t = N-2; t >= 0; --t )
    for ( int j = 0; j < k; ++j )
      delta[t][j] = get_total( delta[t+1], scale[t+1], phi_b[j], 
			obser_mat[t+1], 0.0, k, scale[t][j]);
}

///////////////////////////////////////////////////////////////////////
// Decode:
void Chain::guess_sequences( Z &EVAL_D, L &EVAL_L ){
  int _E_ = EVAL_D.size();
  int tenth = int(_E_/Float(10));
  if (tenth < 1)
    tenth = 1;
  for ( int i = 0; i < _E_; ++i ){
    vector<int> GUESS_i;                 
    viterbi( EVAL_D[i], GUESS_i );
    EVAL_L.push_back( GUESS_i );
    if (i%tenth==0)cerr << ".";
  }
}

void Chain::guess_sequences( Z &EVAL_D, L &EVAL_L, set<int> &Index ){
  int _E_ = EVAL_D.size();
  for ( int i = 0; i < _E_; ++i ){
    set<int>::iterator I_i = Index.find(i);
    if ( I_i != Index.end() ){
      vector<int> GUESS_i;              
      viterbi( EVAL_D[i], GUESS_i );
      EVAL_L.push_back( GUESS_i );
    }
  }
}

//Is this func necessary?
void Chain::guess_sequences_check(Z &EVAL_D, L &EVAL_L,
	vector<string> &LIS, vector<string> &FIS, set<int> &Index,
	set<int> &visited, vector<vector<int> > &gold, string fname){
  int _E_ = EVAL_D.size(), vis = 0;
  ofstream out( fname.c_str() );
  for ( int i = 0; i < _E_; ++i ){
    set<int>::iterator I_i = Index.find( i );
    if ( I_i != Index.end() ){
      vector<int> GUESS_i;     
      viterbi( EVAL_D[i], GUESS_i );
      EVAL_L.push_back( GUESS_i );
      set<int>::iterator v_i = visited.find( i );
      if ( v_i != visited.end() )
	++vis;
      int M = EVAL_D[i].size();
      for ( int u = 0; u < M; ++u )
	out << FIS[EVAL_D[i][u][0]] << "\t" << FIS[EVAL_D[i][u][1]] << "\t" 
	    << LIS[GUESS_i[u]] << "\t" << LIS[gold[i][u]] << endl; 
    }
  }
  out << "\n|vis| = " << vis << "\tgetchar()";
  getchar();
}

///////////////////////////////////// REAL-VALUED FEATS

void Chain::viterbi_R( Sr &W, vector<int> &U ){
  int N = W.size(), argmax_t = 0;
  vector<vector<Float> > delta( N, vector<Float>( k, 0.0 ) );
  vector<vector<int> > psi( N, vector<int>( k, 0 ) );

  for( int t = 0; t < N; ++t ) 
    for ( int j = 0; j < k; ++j ) 
      viterbi_tj_R( delta, W[t], t, j, psi );

  Float max_t = -1e100;
  for (int j = 0; j < k; ++j){
	Float last_bit = delta[N-1][j] + get_phi_f( j );
	if ( last_bit > max_t ){
	  max_t = last_bit;
	  argmax_t = j;
	}
  }
  decode_s( U, delta, psi, argmax_t );
}

void Chain::viterbi_tj_R(vector<vector<Float> > &delta,Xr &x,int t,int j,vector<vector<int> > &psi){
  Float wc = get_word_contribution_R(x,j,t);
  if (t == 0){
    psi[t][j] = -1; 
    delta[t][j] = wc + get_phi_s(j);
    return;
  }
  Float max = -1e100; int argmax = 0;
  for (int i = 0; i < k; ++i){
      Float score_ij = delta[t-1][i] + wc + get_phi_b( i, j );
      if (score_ij > max ){
	max = score_ij;
	argmax = i;
      }
    }
  psi[t][j] = argmax ;
  delta[t][j] = max ;
}

void Chain::guess_sequences_R( Zr &EVAL_D, L &EVAL_L ){
  int _E_ = EVAL_D.size();
  int tenth = int(_E_/Float(10));
  if (tenth < 1)
    tenth = 1;
  for ( int i = 0; i < _E_; ++i ){
    vector<int> GUESS_i;                 
    viterbi_R( EVAL_D[i], GUESS_i );
    EVAL_L.push_back( GUESS_i );
    if (i%tenth==0)cerr << ".";
  }
}

////// PRIORS on guess

void Chain::guess_sequences_R( Zr &EVAL_D, L &EVAL_L,vector<Float>& THETA ){
  int _E_ = EVAL_D.size();
  int tenth = int(_E_/Float(10));
  if (tenth < 1)
    tenth = 1;
  for ( int i = 0; i < _E_; ++i ){
    vector<int> GUESS_i;                 
    viterbi_R( EVAL_D[i], GUESS_i, THETA );
    EVAL_L.push_back( GUESS_i );
    if (i%tenth==0)cerr << ".";
  }
}

#define DEB_MARGINS 0
#if DEB_MARGINS
ofstream out_mat("margins.log");
#endif

void Chain::viterbi_R( Sr &W, vector<int> &U, vector<Float>& THETA ){
  int N = W.size(), argmax_t = 0;
  vector<vector<Float> > delta( N, vector<Float>( k, 0.0 ) );
  vector<vector<int> > psi( N, vector<int>( k, 0 ) );
  Float PRIOR = 19;
#if DEB_MARGINS
  for( int t = 0; t < N; ++t ) {
    multimap<Float,int> scores;
    for ( int j = 0; j < k; ++j ) {
      viterbi_tj_R( delta, W[t], t, j, psi, PRIOR*THETA[j] );
      scores.insert(make_pair(delta[t][j],j));
    }
    multimap<Float,int>::reverse_iterator s_i = scores.rbegin();
    Float s1 = (*s_i).first;
    int y1 = (*s_i).second;
    ++s_i;
    Float s2 = (*s_i).first;
    int y2 = (*s_i).second;
    out_mat << s1 << "\t" << s2 << endl;
    //    cerr << "\n(" << s1 << "," << y1 << ") - (" << s2 << "," << y2 << ") = " << s1-s2;getchar();
  }
#else
  for( int t = 0; t < N; ++t )
    for ( int j = 0; j < k; ++j )
      viterbi_tj_R( delta, W[t], t, j, psi, PRIOR*THETA[j] );
#endif

  Float max_t = -1e100;
  for (int j = 0; j < k; ++j){
	Float last_bit = delta[N-1][j] + get_phi_f( j ) + PRIOR*THETA[j];
	if ( last_bit > max_t ){
	  max_t = last_bit;
	  argmax_t = j;
	}
  }
  decode_s( U, delta, psi, argmax_t );
}

void Chain::viterbi_tj_R(vector<vector<Float> > &delta,Xr &x,int t,int j,vector<vector<int> > &psi,Float PRIOR){
  Float wc = get_word_contribution_R(x,j,t) + PRIOR;
  if (t == 0){
    psi[t][j] = -1; 
    delta[t][j] = wc + get_phi_s(j);
    //    cerr << "\nviterbi[" << t << "," << j << "] = " << wc << "+" << get_phi_s(j) << "+" << PRIOR;getchar();
    return;
  }
  Float max = -1e100; int argmax = 0;
  for (int i = 0; i < k; ++i){
    Float score_ij = delta[t-1][i] + wc + get_phi_b( i, j );
      //      cerr << "\nviterbi[" << t << "," << j << "] = " << delta[t-1][i] << "+" << wc << "+" << get_phi_b(i,j) << "+" << PRIOR;getchar();
      if (score_ij > max ){
	max = score_ij;
	argmax = i;
      }
    }
  psi[t][j] = argmax ;
  delta[t][j] = max ;
}
