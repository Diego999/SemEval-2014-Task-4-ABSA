//   evaluate.cpp: part of sequence tagger for Wordnet supersenses, and other tagsets
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

#include "evaluate.h"
// Assumes an encoding of following format
// 0 nonname nametype1: 1:B1 2:I1 nametype2 3:B2 4:I2 ...
// braket types: 0 nonname  1:single entity	2:beginning	3:end	4:cont

inline int class_type(  int label ) { 
    if (!label)  return 0;  
    return  ( ( label - 1 ) / 2 ) + 1; 
}

inline int reverse_class_type(  int label ) { 
    if (!label)  return 0;  
    return  ( ( label - 1 ) * 2 ) + 1; 
}

int braket_type( int n, vector<int> &seq ) { 
     int label = seq[n];
     int N = seq.size();

     if ( !label ) return 0;
     if ( label % 2 == 1 ) {
	// Beginning label
        if ( n + 1 == N ||
	     n + 1 < N && seq[n+1] != label + 1 )  // single word entity
	     return SBrac;
	return BBrac;
     } else {
	// Continuation label
	if( n + 1 == N ||  
	    n + 1 < N && label != seq[n+1]  )	   // next label is different 
           return EBrac;
        return CBrac;
     }
}

inline int getsum( vector<int> &list_val ) {
  int num = 0;
  for ( unsigned int i = 0 ; i < list_val.size(); ++i )
        num += list_val[i];
  return num;
}

Float compute_score( int numcrt, int numans, int numref , string info, int verbose, ofstream &out) {
  Float precision, recall, fscore;

/*
if( numcrt > numref )
cout << "\n\n1 HATA!!\n";
 if( numcrt > numans )
cout << "\n\n2 HATA!!\n";
*/
  recall = ( numref == 0 ) ? 0 : (double)numcrt/numref;
  precision = ( numans == 0 ) ? 0 : (double)numcrt/numans;
  fscore = ( precision + recall == 0 ) ? 0 : 2 * precision * recall /
							( precision + recall );
  if ( verbose )
      out << "    " << info << " with class info: "<< numcrt << " " 
           << recall << " " << precision << " " << fscore << "\n";
  return fscore;
}

Float evaluate_sequences(vector<vector<int> > &goldS,
			 vector<vector<int> > &predicted,
			 vector<string> &LIS ,
			 int num_labels,
			 int verbose,
			 ofstream &out){
  int numtag,numctag,numbtag;
  numtag=numctag=numbtag=0;
  int num_class =  ( num_labels / 2 ) + 1;
  vector<int> ccount(num_class,0),pcount(num_class,0),ncorrect(num_class,0),
	      nleft(num_class,0),nright(num_class,0),
	      nbcorrect(num_class,0),nbleft(num_class,0),nbright(num_class,0);
  int match,bmatch;
  match=bmatch=0;
  int num_seq = goldS.size();

  for (int i = 0; i < num_seq ; ++i){
  // For each sequence
       unsigned int N = goldS[i].size();
       if ( N != predicted[i].size() ) {
	  cerr << "Unmatching length of labels for sentence" << i << "\n";
          abort();
       }
       for ( unsigned int n = 0 ; n < N ; ++n, ++numtag )   {
          int clabel = goldS[i][n];
          int cclass = class_type( clabel );
          int cbraket = braket_type( n, goldS[i] );
          if ( cbraket == SBrac || cbraket == BBrac ) {
		ccount[cclass]++;
          }

	  int plabel = predicted[i][n];
          int pclass = class_type( plabel );
          int pbraket = braket_type( n, predicted[i] );
          if ( pbraket == SBrac || pbraket == BBrac ) {
		pcount[pclass]++;
          }

          if ( cbraket == pbraket ) numbtag++;
          if ( clabel == plabel && cbraket == pbraket ) numctag++;
          if ( pbraket == SBrac  &&  cbraket == SBrac ) {
                 nbcorrect[pclass]++; nbleft[pclass]++; nbright[pclass]++;
		 if( pclass == cclass ) { 
		     ncorrect[pclass]++; nleft[pclass]++; nright[pclass]++; 
		 }
	  }
	  else if ( pbraket == SBrac  &&  cbraket == EBrac ) { 
		 nbright[pclass]++;
		 if( pclass == cclass ) { nright[pclass]++; }
	  }
	  else if ( pbraket == SBrac  &&  cbraket == BBrac ) { 
		 nbleft[pclass]++;
		 if( pclass == cclass ) { nleft[pclass]++; }
	  }
	  else if ( pbraket == BBrac  &&  cbraket == SBrac ) { 
		 nbleft[pclass]++;
		 if( pclass == cclass ) { nleft[pclass]++; }
	  }
	  else if ( pbraket == BBrac  &&  cbraket == BBrac ) { 
		 nbleft[pclass]++;   bmatch=1;
		 if( pclass == cclass ) { nleft[pclass]++; match=1;}
	  }
/*
          if ( pbraket != cbraket ) bmatch = 0;
	  if (!(clabel == plabel && cbraket == pbraket ) ) match = 0;
	  if ( pbraket == none || cbraket == none )  match=bmatch=0;
*/
          if ( pbraket != cbraket || pbraket == none )  bmatch = 0;
          if ( cclass != pclass || !bmatch )            match = 0;

	  if ( pbraket == EBrac && cbraket == SBrac )  {
		 nbright[pclass]++;
		 if( pclass == cclass ) { nright[pclass]++; }
      	  }
          if ( pbraket == EBrac && cbraket == EBrac ) {
		if( bmatch )  nbcorrect[pclass]++;
		nbright[pclass]++;
		if( pclass == cclass ) {
		     if ( match && ncorrect[pclass] <= pcount[pclass]) 
			 ncorrect[pclass]++;
		     nright[pclass]++;	
		}
	  }
       }
  }


  Float score;
  int numans = getsum( pcount );
  int numref = getsum( ccount );
  if (verbose) {
    out << "\n[Tagging Performance]\n";
    out << "# of tags: " << numtag << ",\t correct tags: " << numctag
	<< ",\t correct IOBs: " << numbtag << "\n";
    out << "precision with class info: " << (float)numctag/numtag;
    out << ",\t w/o class info: " <<  (float)numbtag/numtag << "\n";
    out << "\n[Object Identification Performance]\n";
    out << "# of OBJECTs: " << numref << "\t ANSWERs: " << numans << ".\n";
    out << "\n# (recall / precision / f-score) of ...\n";
  }
  if ( !numref ) cerr << "\n\nNo names in reference!\n";
  if ( !numans ) cerr << "\n\nNo names in answers!\n";

  Float ret_val = compute_score( getsum(ncorrect), numans, numref , 
				 "FULLY CORRECT answer", verbose, out);
/* There is a bug on the right
  score = compute_score( getsum(nleft), numans, numref , 
			 "correct LEFT boundary", verbose, out );
  score = compute_score( getsum(nright), numans, numref , 
			 "correct RIGHT boundary", verbose, out );
 
*/
  if (num_class > 2 && verbose )
  for( unsigned int cl = 1 ; cl < pcount.size() ; ++cl ) {
    string cl_str = LIS[reverse_class_type(cl)];
    out << "\n[" << cl << "/" << string(cl_str,2,int(cl_str.size()-2)) << "\tIdentification Performance]\n";
    out << "# of OBJECTs: " << ccount[cl] << ",\t ANSWERs: " << pcount[cl];
    out << "\n# (recall / precision / f-score) of ...\n";
    score = compute_score( ncorrect[cl], pcount[cl], ccount[cl] , 
			   "FULLY CORRECT answer", verbose, out );
/*
    score = compute_score( nleft[cl], pcount[cl], ccount[cl] , 
			   "correct LEFT boundary", verbose, out );
    score = compute_score( nright[cl], pcount[cl], ccount[cl] , 
			   "correct RIGHT boundary", verbose, out );
*/


  }
  else if ( !verbose ) 
      for( unsigned int cl = 1 ; cl < pcount.size() ; ++cl ) {
           score = compute_score( ncorrect[cl], pcount[cl], ccount[cl] , 
			          "FULLY CORRECT answer", verbose, out );
	   cerr << "Cl:" << cl << " " << score << " ";
      }
                                                                                

  //assert(ret_val<=1.0);
  if (ret_val>1.0) ret_val=1.0;
  return ret_val;
}

////////////////////////////////////
Float compute_score(int numcrt,int numans,int numref,string info,verbose_res &vr) {
  Float precision, recall, fscore;
  recall = ( numref == 0 ) ? 0 : (double)numcrt/numref;
  precision = ( numans == 0 ) ? 0 : (double)numcrt/numans;
  fscore = ( precision + recall == 0 ) ? 0 : 2 * precision * recall /( precision + recall );
  vr.nfullycorrect = numcrt;
  vr.R = recall;
  vr.P = precision;
  vr.F = fscore;
  return fscore;
}

Float evaluate_sequences(vector<vector<int> > &goldS,vector<vector<int> > &predicted,vector<string> &LIS ,int num_labels,vector<verbose_res> &VR){
  int numtag,numctag,numbtag;
  numtag=numctag=numbtag=0;
  int num_class =  ( num_labels / 2 ) + 1;
  vector<int> ccount(num_class,0),pcount(num_class,0),ncorrect(num_class,0),
    nleft(num_class,0),nright(num_class,0),
    nbcorrect(num_class,0),nbleft(num_class,0),nbright(num_class,0);
  int match,bmatch;
  match=bmatch=0;
  int num_seq = goldS.size();

  for (int i = 0; i < num_seq ; ++i){                                 // For each sequence
    unsigned int N = goldS[i].size();
    if ( N != predicted[i].size() ) {
      cerr << "Unmatching length of labels for sentence" << i << "\n"; abort();
    }
    for ( unsigned int n = 0 ; n < N ; ++n, ++numtag )   {
      int clabel = goldS[i][n];
      int cclass = class_type( clabel );
      int cbraket = braket_type( n, goldS[i] );
      if ( cbraket == SBrac || cbraket == BBrac ) {
	ccount[cclass]++;
      }

      int plabel = predicted[i][n];
      int pclass = class_type( plabel );
      int pbraket = braket_type( n, predicted[i] );
      if ( pbraket == SBrac || pbraket == BBrac ) {
	pcount[pclass]++;
      }
      
      if ( cbraket == pbraket ) numbtag++;
      if ( clabel == plabel && cbraket == pbraket ) numctag++;
      if ( pbraket == SBrac  &&  cbraket == SBrac ) {
	nbcorrect[pclass]++; nbleft[pclass]++; nbright[pclass]++;
	if( pclass == cclass ) { 
	  ncorrect[pclass]++; nleft[pclass]++; nright[pclass]++; 
	}
      }
      else if ( pbraket == SBrac  &&  cbraket == EBrac ) { 
	nbright[pclass]++;
	if( pclass == cclass ) { nright[pclass]++; }
      }
      else if ( pbraket == SBrac  &&  cbraket == BBrac ) { 
	nbleft[pclass]++;
	if( pclass == cclass ) { nleft[pclass]++; }
      }
      else if ( pbraket == BBrac  &&  cbraket == SBrac ) { 
	nbleft[pclass]++;
	if( pclass == cclass ) { nleft[pclass]++; }
      }
      else if ( pbraket == BBrac  &&  cbraket == BBrac ) { 
	nbleft[pclass]++;   bmatch=1;
	if( pclass == cclass ) { nleft[pclass]++; match=1;}
      }
      if ( pbraket != cbraket || pbraket == none )  bmatch = 0;
      if ( cclass != pclass || !bmatch )            match = 0;
      
      if ( pbraket == EBrac && cbraket == SBrac )  {
	nbright[pclass]++;
	if( pclass == cclass ) { nright[pclass]++; }
      }
      if ( pbraket == EBrac && cbraket == EBrac ) {
	if( bmatch )  nbcorrect[pclass]++;
	nbright[pclass]++;
	if( pclass == cclass ) {
	  if ( match && ncorrect[pclass] <= pcount[pclass]) 
	    ncorrect[pclass]++;
	  nright[pclass]++;	
	}
      }
    }
  }

  Float score;
  int numans = getsum(pcount), numref = getsum(ccount);
  verbose_res vr("ALL",int(numref),int(numans));
  if ( !numref ) cerr << "\n\nNo names in reference!\n";
  if ( !numans ) cerr << "\n\nNo names in answers!\n";
  Float ret_val = compute_score(getsum(ncorrect),numans,numref,"FULLY CORRECT answer",vr);
  VR.push_back(vr);

  if (num_class > 2)
    for( unsigned int cl = 1 ; cl < pcount.size() ; ++cl ) {
      string cl_str = LIS[reverse_class_type(cl)];
      verbose_res vr_cl(string(cl_str,2,int(cl_str.size()-2)),ccount[cl],pcount[cl]);
      score = compute_score(ncorrect[cl],pcount[cl],ccount[cl],"FULLY CORRECT answer",vr_cl);
      VR.push_back(vr_cl);
    }
  if (ret_val>1.0) ret_val=1.0;
  return ret_val;
}

Float evaluate_pos(vector<vector<int> > &goldS,
		   vector<vector<int> > &predicted,
		   vector<string> &LIS ,
		   int num_labels,
		   vector<Float>& itemized){
  int num_seq = goldS.size();
  vector<int> hits_pos;
  Float hits = 0, den = 0;
  for (int r = 0; r < num_labels; ++r){
    itemized.push_back(0);
    hits_pos.push_back(0);
  }
  for (int i = 0; i < num_seq ; ++i){                                 // For each sequence
    unsigned int N = goldS[i].size();
    if ( N != predicted[i].size() ) {
      cerr << "Unmatching length of labels for sentence" << i << "\n"; abort();
    }
    for (int j = 0; j < N; ++j){
      if (goldS[i][j] == predicted[i][j]){
	hits += 1;
	hits_pos[goldS[i][j]] += 1;
      }
      itemized[goldS[i][j]] += 1;
      den += 1;
    }
  }
  for (int r = 0; r < num_labels; ++r)
    itemized[r] = hits_pos[r]/itemized[r];
  return hits/den;
}
