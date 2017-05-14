//   Perceptron.h: part of sequence tagger for Wordnet supersenses, and other tagsets
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

#include "Perceptron.h"
#include "../utils/utils.h"
#define DEB 0

///////////////////////////////////////////////////////////////////////
// Features handling:
Float Fs::avg_val(int it,bool use_avg) {
  if (!use_avg)
     return alpha;
  return  (avg_num+(alpha*(it-lst_upd)))/Float(it);
}

void Fs::update(int it,bool doupdate) {
#if DEB
  if (doupdate)
    cerr << "\n\tupdate(" << it << "," << doupdate << ")"
	 << "\n\tupd_val = " << upd_val
	 << "\n\talpha = " << alpha << "\t->\t" << alpha+upd_val
	 << "\n\tlast_upd = " << lst_upd << "\t->\t" << it
	 << "\n\tavg_num = " << avg_num+(alpha*(it-lst_upd));
#endif
  if (upd_val != 0 && doupdate){
    avg_num += alpha*(it-lst_upd);
    lst_upd = it;
    alpha += upd_val;
  }
  upd_val = 0;
}

void Fs::equate(Fs &c) {
 alpha = c.alpha;
 avg_num = c.avg_num;
 lst_upd = c.lst_upd;
 upd_val = c.upd_val;
}

void Fs::rand_alpha(){
  Float r = rand()/Float(RAND_MAX), p = rand()/Float(RAND_MAX);
  if (p > 0.5)
    alpha = r;
  else alpha = r*-1;
}

///////////////////////////////////////////////////////////////////////

void PS_HMM::init(int k_val,vector<string> &LIS,vector<string> &FIS,
        HM &LSI,HM &FSI, bool no_spec, Float wc){
  cerr << "\nPS_HMM::init(" << k_val << ")";
  k = k_val;
  word_weight = wc;
  no_special_end_transition_sym = no_spec;
  if( !no_spec ) {
     init_phi_sf(k,phi_s,LIS,FIS,LSI,FSI,true);         // bigrams begin
     init_phi_sf(k,phi_f,LIS,FIS,LSI,FSI,false);        // bigrams end
  }
  init_phi_b(k,phi_b,LIS,FIS,LSI,FSI);               // std transitions
  init_phi_w(k,phi_w,FIS.size(),active_phi_w);
                                      // sparse matrix for std features
  cerr << "\tdone";
}


//////////////////////////////////////////////////////////////////////
// Virtual functions
Float PS_HMM::get_word_contribution( X &x, int j, int pos ){
  Float word_contribution = 0;
  int _x_ = x.size();
  if( only_active ) 
    for ( int i = 0; i < _x_; ++i ) {
      hash_set<unsigned>::iterator ac_Fs_i = active_phi_w[j].find( x[i] );
      if( ac_Fs_i != active_phi_w[j].end() ) {
	hash_map<unsigned,Fs>::iterator Fs_i = phi_w[j].find( x[i] );
	if ( Fs_i != phi_w[j].end() ) 
	  word_contribution += (*Fs_i).second.avg_val( it, av_mod );
      }
    }
  else 
    for ( int i = 0; i < _x_; ++i ){
      hash_map<unsigned,Fs>::iterator Fs_i = phi_w[j].find( x[i] );
      if ( Fs_i != phi_w[j].end() )
	word_contribution += (*Fs_i).second.avg_val( it, av_mod );
    }
  return word_weight * word_contribution;
}


////////////////////////////////////////////////////////////////////
// HMM Train:

void PS_HMM::train( Z &W, L &T ) {
  int N = W.size();
  set<int> Index;
  for ( int i = 0; i < N; ++i )
    Index.insert( i );
  train( W, T, Index );
}

void rand_reorder_vector( vector<int> &IN, vector<int> &OUT ){
  //  OUT = IN;
  int N = IN.size();
  cerr << "\trand_reorder(|IN|=" << N << ",|OUT|=";
  set<int> visited;
  while( int(OUT.size()) < N ){
    int s = int( floor(( rand() / Float( RAND_MAX )) * N ));
    set<int>::iterator v_i = visited.find( s );
    if ( v_i == visited.end() ){
      visited.insert( s );
      OUT.push_back( IN[s] );
    }
  }
  cerr << OUT.size() <<")";
}

void PS_HMM::train( Z &W, L &T, set<int> &Index ){
  int updates = 0, counter = 0, N = Index.size();
  cerr << "\ntrain(|Index| = " << N << ")";
  vector<int> IN,OUT;
  for ( set<int>::iterator I_i = Index.begin(); I_i != Index.end(); ++I_i )
    IN.push_back( *I_i );
  rand_reorder_vector( IN, OUT );
//for(int i=0;i<IN.size();++i) OUT.push_back(IN[i]);
  for ( int i = 0; i < N; ++i ){
    ++it;
    bool updated_s = false;
    cur_ex = OUT[i]; 
    train_on_s( W[OUT[i]], T[OUT[i]], updated_s );
    if ( updated_s )
      ++updates;
    if ( ++counter % 1000 == 0 ) cerr << ".";
  }
  cerr << "\ti = " << it << "\ts-error = " << (Float) updates / N;
}

void PS_HMM::train_cv_check( Z &W, L &T, set<int> &Index, set<int>& visited ){
  int updates = 0, counter = 0, N = Index.size();
  cerr << "\ntrain_cv_check(|Index| = " << N << ")";
  vector<int> IN, OUT;
  for ( set<int>::iterator I_i = Index.begin(); I_i != Index.end(); ++I_i )
    IN.push_back( *I_i );
  rand_reorder_vector( IN, OUT );
  for ( int i = 0; i < N; ++i ){
    ++it;
    bool updated_s = false;
    cur_ex = OUT[i]; 
    train_on_s( W[OUT[i]], T[OUT[i]], updated_s );
    if ( updated_s )
      ++updates;
    visited.insert( OUT[i] );
    if ( ++counter % 1000 == 0 ) cerr << ".";
  }
  cerr << "\ti = " << it << "\t|visited| = " << visited.size() << "\ts-error = " << (Float) updates/N;
}

void PS_HMM::update_boundary_feats( int bU, int bT, int eU, int eT ) {
  if( no_special_end_transition_sym ) {
    phi_b[0][bU].upd_val -= 1;
    phi_b[0][bT].upd_val += 1;
    phi_b[eU][0].upd_val -= 1;
    phi_b[eT][0].upd_val += 1;
  } else {
    phi_s[bU].upd_val -= 1;
    phi_s[bT].upd_val += 1;
    phi_f[eU].upd_val -= 1;
    phi_f[eT].upd_val += 1;
  }
}

void PS_HMM::update_transition_feats( int bT, int eT, int bU, int eU ){
      phi_b[bT][eT].upd_val += 1;
      phi_b[bU][eU].upd_val -= 1;
}


void PS_HMM::finalize_transition_feats( bool s_update) { 
  int bs = phi_b.size();
  for( int i = 0 ; i < bs; ++i ) 
    for( int j = 0 ; j < bs; ++j ) 
      phi_b[i][j].update( it, s_update );
  if ( no_special_end_transition_sym ) 
    for( int i = 0 ; i < bs; ++i ) {
      phi_s[i].equate( phi_b[0][i] ); 
      phi_f[i].equate( phi_b[i][0] ); 
    }
  else
    for( int i = 0 ; i < bs; ++i )  {
      phi_s[i].update( it, s_update );
      phi_f[i].update( it, s_update );
    }
}

void update_feature_val(vector<hash_map<unsigned,Fs> > &phi_w, int l,unsigned feat,
			double val, bool active, vector<hash_set<unsigned> > &active_phi_w ) {
#if DEB
  cerr << "\nupdate_feature_val(" << l << "," << feat << "," << val << "," << active << ")";
#endif
  hash_set<unsigned>::iterator active_F_i = active_phi_w[l].find( feat );
  if( active && active_F_i == active_phi_w[l].end() )    // was not active till now, add as active
    active_phi_w[l].insert( feat );
  hash_map<unsigned,Fs>::iterator F_i = phi_w[l].find( feat );
  if ( F_i == phi_w[l].end() ){
    Fs f;
    phi_w[l].insert( make_pair( feat, f ));
    F_i = phi_w[l].find( feat );
  }
  (*F_i).second.upd_val += val;
}

void PS_HMM::update_word_feats( X &w, int T, int U ){
  int _X_ = w.size();
  for ( int s = 0; s < _X_; ++s ){
    update_feature_val( phi_w, U, w[s], -1, only_active, active_phi_w );
    update_feature_val( phi_w, T, w[s], 1, only_active, active_phi_w );
  }
}

void finalize_feature_val(vector<hash_map<unsigned,Fs> > &phi_w,int l,unsigned feat,
			  int it, bool active, vector<hash_set<unsigned> > &act_phi_w, bool &s_upd){
  hash_set<unsigned>::iterator active_F_i = act_phi_w[l].find( feat );
  if( !active || active_F_i != act_phi_w[l].end() ) {
    hash_map<unsigned,Fs>::iterator F_i = phi_w[l].find( feat );
    assert ( F_i != phi_w[l].end() );
    (*F_i).second.update( it, s_upd );
  }
}

void PS_HMM::finalize_update_word_feats( X &W, int T, int U, bool &su ){
  int _X_ = W.size();
  for ( int s = 0; s < _X_; ++s ){
    finalize_feature_val( phi_w, U, W[s], it, only_active, active_phi_w, su );
    finalize_feature_val( phi_w, T, W[s], it, only_active, active_phi_w, su );
  }
}

void PS_HMM::update( S &W, vector<int> &T, bool &s_update, vector<int> &U ){
  int N = U.size();
  update_boundary_feats( U[0], T[0], U[N-1], T[N-1] );
  if ( U[0] != T[0] )  {
      s_update = true;
      cur_pos = 0;
      update_word_feats( W[0], T[0], U[0] );
  }
  for ( int i = 1; i < N; ++i )
    if ( U[i] != T[i] ) {
      s_update = true;
      cur_pos = i;
      update_word_feats( W[i], T[i], U[i] );
      update_transition_feats( T[i-1], T[i], U[i-1], U[i] );
    }
  if ( s_update ){
    if( U[0] != T[0] )
       finalize_update_word_feats( W[0], T[0], U[0], s_update );
    for ( int i = 1; i < N; ++i ) 
      if( U[i] != T[i] )
        finalize_update_word_feats( W[i], T[i], U[i], s_update );
    finalize_transition_feats( s_update ); 
  }
}

void PS_HMM::train_on_s( S &W, vector<int> &T, bool &s_update ) {
  vector<int> _u;
  av_mod = false;
  viterbi( W, _u );
#if DEB
  cerr << "pred : " ; for(int i=0;i<_u.size();i++) cerr << _u[i] << "/" << T[i] << " ";getchar();
#endif
  av_mod = true;
  update( W, T, s_update, _u );
  cur_ex = -1;
}

///////////////////////////////////////////////////////////////////////
// additional

void print_XIS_XSI(string fname,vector<string> &XIS,HM &XSI){
  map<unsigned,string> MIS;
  for (HM::iterator XSI_i = XSI.begin(); XSI_i != XSI.end(); ++XSI_i)
    MIS.insert(make_pair((*XSI_i).second,(*XSI_i).first));
  if (int(MIS.size()) != int(XSI.size())){
    cerr << "\n|I| and |S| have different sizes: " << "\t|MIS| = " << MIS.size() << "\t|XSI| = " << XSI.size();
    abort();
  }
  ofstream out(fname.c_str());
  for (map<unsigned,string>::iterator MIS_i = MIS.begin(); MIS_i != MIS.end(); ++MIS_i)
    out << (*MIS_i).first << "\t" << (*MIS_i).second << endl;
}

void PS_HMM::SaveModel(string modelname,
		       vector<string> &LIS,
		       vector<string> &FIS,
		       HM &LSI,
		       HM &FSI,
		       ofstream &out_log){
  cerr << "\nSaveModel(" << modelname << ")";
  out_log << "\nSaveModel(" << modelname << ")";
  print_XIS_XSI("./MODELS/"+modelname+".F",FIS,FSI);
  //  print_XIS_XSI("./MODELS/"+modelname+".L",LIS,LSI);

  ofstream out(("./MODELS/"+modelname +".PS_HMM").c_str());
  out << k << endl << it << endl << phi_s.size() << endl << word_weight 
      << "\tphi_s\talpha\tavg_num\tlst_upd\tupd_val\n";
  for (int i = 0; i < int(phi_s.size()); ++i)
    out << phi_s[i].alpha << " " << phi_s[i].avg_num << " " << phi_s[i].lst_upd 
	<< " " << phi_s[i].upd_val << endl;
  out << phi_f.size() << "\tphi_f\n";
  for (int i = 0; i < int(phi_f.size()); ++i)
    out << phi_f[i].alpha << " " << phi_f[i].avg_num << " " << phi_f[i].lst_upd 
	<< " " << phi_f[i].upd_val << endl;
  out << phi_b.size() << "\t" << "\tphi_b\n";
  for (int i = 0; i < int(phi_b.size()); ++i){
    int _i_ = phi_b[i].size();
    out << _i_;
    for (int j = 0; j < _i_; ++j)
      out << "\t" << phi_b[i][j].alpha << " " << phi_b[i][j].avg_num 
	  << " " << phi_b[i][j].lst_upd << " " << phi_b[i][j].upd_val;
    out << endl;
  }
  out << phi_w.size() << "\tphi_w\n";
  for (int i = 0; i < int(phi_w.size()); ++i){
    out << i;
    for (hash_map<unsigned,Fs>::iterator w_i = phi_w[i].begin(); w_i != phi_w[i].end(); ++w_i)
      out << "\t" << (*w_i).first << " " << (*w_i).second.alpha << " " << (*w_i).second.avg_num 
	  << " " << (*w_i).second.lst_upd << " " << (*w_i).second.upd_val;
    out << endl;
  }
  out << "ENDOFMODEL\n";
}

void load_XIS(string fname,vector<string> &XIS,HM &XSI){
  ifstream in(fname.c_str());
  char strbuf[1000];
  while (in.peek() != -1){
    string x = "";
    unsigned x_i;
    in >> x_i >> x;
    if (x != ""){
      if (int(XIS.size()) != x_i){
	cerr << "\nload_XIS(" << fname << ")\t" << x << "\t" << x_i << "\t|XIS| = " << XIS.size();
	abort();
      }
      XIS.push_back(x);
      XSI[x] = x_i;
    }
    else in.getline(strbuf,1000);
  }
}

void PS_HMM::LoadModel(string modelname,vector<string> &FIS,HM &FSI,ofstream &out_log){
  cerr << "\nLoadModel(" << modelname << ")";
  out_log << "\nLoadModel(" << modelname << ")";
  //  load_XIS("./MODELS/"+modelname+".L",LIS,LSI);
  //  load_XIS(sstdir+"/MODELS/"+modelname+".F",FIS,FSI);
  load_XIS(modelname+".F",FIS,FSI);
  cerr << ".";
  out_log << "\t|FEATURES| = " << FIS.size();

  ifstream in((modelname+".PS_HMM").c_str());
  char strbuf[1000];
  int _phis_, _phif_, _phib_, _phiw_;
  in >> k >> it >> _phis_ >> word_weight;  
  in.getline(strbuf,1000);
  for (int i = 0; i < _phis_; ++i){
    Fs fs;
    in >> fs.alpha >> fs.avg_num >> fs.lst_upd >> fs.upd_val;
    phi_s.push_back(fs);
  }
  in >> _phif_;
  in.getline(strbuf,1000);
  cerr << ".";
  for (int i = 0; i < _phif_; ++i){
    Fs fs;
    in >> fs.alpha >> fs.avg_num >> fs.lst_upd >> fs.upd_val;
    phi_f.push_back(fs);
  }
  in >> _phib_;
  in.getline(strbuf,1000);
  cerr << ".";
  for (int i = 0; i < _phif_; ++i){
    int _i_;
    in >> _i_;
    vector<Fs> Fi;
    for (int j = 0; j < _i_; ++j){
      Fs fs;
      in >> fs.alpha >> fs.avg_num >> fs.lst_upd >> fs.upd_val;
      Fi.push_back(fs);
    }
    phi_b.push_back(Fi);
  }
  in >> _phiw_;
  in.getline(strbuf,1000);
  cerr << ".";
  for (int i = 0; i < _phiw_; ++i){
    int _i_;
    in >> _i_;
    hash_map<unsigned,Fs> Fi;
    while (in.peek() == 9){
      Fs fs;
      int fid;
      in >> fid >> fs.alpha >> fs.avg_num >> fs.lst_upd >> fs.upd_val;
      Fi.insert(make_pair(fid,fs));
    }
    phi_w.push_back(Fi);
  }
  string check;
  in >> check;
  if (check != "ENDOFMODEL"){
    cerr << "\n\tcheck error:" << check;
    abort();
  }
  else cerr << "\tOK";
}

////////////////////////////////////////////////////////////////////
// HMM Train_R

void PS_HMM::train_R( Zr &W, L &T ) {
  int N = W.size();
  set<int> Index;
  for ( int i = 0; i < N; ++i )
    Index.insert( i );
  train_R( W, T, Index );
}

void PS_HMM::train_R( Zr &W, L &T, set<int> &Index ){
  int updates = 0, counter = 0, N = Index.size();
  cerr << "\ntrain_R(|Index| = " << N << ")";
  vector<int> IN,OUT;
  for ( set<int>::iterator I_i = Index.begin(); I_i != Index.end(); ++I_i )
    IN.push_back( *I_i );
  rand_reorder_vector( IN, OUT );

  for ( int i = 0; i < N; ++i ){
    ++it;
    bool updated_s = false;
    cur_ex = OUT[i]; 
    train_on_s_R( W[OUT[i]], T[OUT[i]], updated_s );
    if ( updated_s )
      ++updates;
    if ( ++counter % 1000 == 0 ) cerr << ".";
  }
  cerr << "\ti = " << it << "\ts-error = " << (Float) updates / N;
}

void PS_HMM::train_on_s_R( Sr &W, vector<int> &T, bool &s_update ) {
  vector<int> _u;
  av_mod = false;
  viterbi_R( W, _u );
#if DEB
  cerr << "pred : " ; for(int i=0;i<_u.size();i++) cerr << _u[i] << "/" << T[i] << " ";getchar();
#endif
  av_mod = true;
  update_R( W, T, s_update, _u );
  cur_ex = -1;
}

Float PS_HMM::get_word_contribution_R(Xr &x,int j,int pos){
  Float word_contribution = 0;
  int _x_ = x.size();
  if(only_active) 
    for (int i = 0; i < _x_; ++i){
      hash_set<unsigned>::iterator ac_Fs_i = active_phi_w[j].find(x[i].first);
      if(ac_Fs_i != active_phi_w[j].end()) {
	hash_map<unsigned,Fs>::iterator Fs_i = phi_w[j].find(x[i].first);
	if (Fs_i != phi_w[j].end()) 
	  word_contribution += (*Fs_i).second.avg_val(it,av_mod)*x[i].second;
      }
    }
  else 
    for (int i = 0; i < _x_; ++i){
      hash_map<unsigned,Fs>::iterator Fs_i = phi_w[j].find(x[i].first);
      if (Fs_i != phi_w[j].end())
	word_contribution += (*Fs_i).second.avg_val(it,av_mod)*x[i].second;
    }
  return word_weight * word_contribution;
}

void PS_HMM::update_R(Sr &W,vector<int> &T,bool &s_update,vector<int> &U){
  int N = U.size();
  update_boundary_feats( U[0], T[0], U[N-1], T[N-1] );
  if ( U[0] != T[0] )  {
      s_update = true;
      cur_pos = 0;
      update_word_feats_R( W[0], T[0], U[0] );
  }
  for ( int i = 1; i < N; ++i )
    if ( U[i] != T[i] ) {
      s_update = true;
      cur_pos = i;
      update_word_feats_R( W[i], T[i], U[i] );
      update_transition_feats( T[i-1], T[i], U[i-1], U[i] );
    }
  if ( s_update ){
    if( U[0] != T[0] )
       finalize_update_word_feats_R( W[0], T[0], U[0], s_update );
    for ( int i = 1; i < N; ++i ) 
      if( U[i] != T[i] )
        finalize_update_word_feats_R( W[i], T[i], U[i], s_update );
    finalize_transition_feats( s_update ); 
  }
}

void PS_HMM::update_word_feats_R( Xr &w, int T, int U ){
#if DEB
  cerr << "\nupdate_wf_R(" << T << "," << U << ")";
#endif
  int _X_ = w.size();
  for ( int s = 0; s < _X_; ++s ){
#if DEB
    cerr << "\nfeature:" << w[s].first;getchar();
#endif
    update_feature_val( phi_w, U, w[s].first,-w[s].second, only_active, active_phi_w );
    update_feature_val( phi_w, T, w[s].first, w[s].second, only_active, active_phi_w );
  }
}

void PS_HMM::finalize_update_word_feats_R( Xr &W, int T, int U, bool &su ){
  int _X_ = W.size();
  for ( int s = 0; s < _X_; ++s ){
    finalize_feature_val( phi_w, U, W[s].first, it, only_active, active_phi_w, su );
    finalize_feature_val( phi_w, T, W[s].first, it, only_active, active_phi_w, su );
  }
}
