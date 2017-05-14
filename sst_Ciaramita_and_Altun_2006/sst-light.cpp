//   sst-light.cpp: HMM sequence tagger based on regularized perceptron
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

#include <cstdio>
#include <iostream>
#include <string>
#include <fstream>
#include "LIB/examples/examples.h"
#include "LIB/features/features.h"

using namespace std;
typedef double Float;

///////////////////////////////////////////////

int main (int argc, char** argv){
  //  split_by_doc("DATA/SEM_07.BI");  getchar();
  //  lowercase_bi_data(argv[1],argv[2]);getchar();
  //  collect_tags_dist(argv[1],argv[2]);getchar();
  //  fragment_data("../DATA/WIKI/wiki_data_1.BI_basefeats");getchar();
  //  merge_pos_tag(argv[1],argv[2],argv[3]);exit(EXIT_SUCCESS);
  //  create_nostop_list(argv[1]);getchar();
  //  fragment_data_nostoplist(argv[1],argv[2],argv[3]);getchar();
  //  get_entity_dist(argv[1]);exit(EXIT_SUCCESS);
  //  conll_extra_training_and_gaz_from_semcor(argv[1]);exit(EXIT_SUCCESS);
  //  bi_to_line(argv[1]);exit(EXIT_SUCCESS);
  //  map_tags(argv[1],argv[2]);exit(EXIT_SUCCESS);
  //  VRSP_make_B(argv[1],atoi(argv[2]),atoi(argv[3]));exit(EXIT_SUCCESS);
  //  ADAPT_olivier_format_data(argv[1],argv[2]);exit(EXIT_SUCCESS);
  //  print_tags_feat_file(argv[1]);exit(EXIT_SUCCESS);
  //  gazetteerize_wnss_list(argv[1]);exit(EXIT_SUCCESS);
  //  add_size_to_gaz_feats(argv[1]);exit(EXIT_SUCCESS);
  //  collect_postags(argv[1]);getchar();
  //  count_w(argv[1],argv[2],atoi(argv[3]));exit(EXIT_SUCCESS);
  //  entity_ratio(argv[1],argv[2],argv[3],atoi(argv[4]));exit(EXIT_SUCCESS);
  //  entity_ratio_rand(argv[1],argv[2],argv[3]);exit(EXIT_SUCCESS);
  //  entity_ratio2(argv[1],argv[2],argv[3]);exit(EXIT_SUCCESS);
  //  split_triggers(argv[1]);exit(EXIT_SUCCESS);
  //  discordant_tags(argv[1],argv[2]);exit(EXIT_SUCCESS);
  //  sample_sentences(atoi(argv[1]),atoi(argv[2]),argv[3]);exit(EXIT_SUCCESS);

  string usage7 = "Usage:\n\t7)\tsst tag modelname target_data tagsetname secondorder mode";
  string usage8 = "\n\t8)\tsst postag modelname tagsetname targetname secondorder lowercase";
  string usage9 = "\n\t9)\tsst bitag modelname_pos tagsetname_pos modelname_bio tagsetname_bio target_data secondorder lowercase";
  string usage6 = "\n\t6)\tsst train modelname traindata tagsetname secondorder number_of_epochs mode";
  string usage5 = "\n\t5)\tsst eval traindata testdata tagsetname secondorder number_of_epochs crossval_trials mode real wordweight thetafile";
  string usage3 = "\n\t3)\tsst basic-feats filename istaggedfile mode gazfile lowercase";
  string usage4 = "\n\t4)\tsst split filename";
  string usage1 = "\n\t1)\tsst multitag rowdata 2nd-order lowercase gazfile model_pos tagset_pos model_bio_1 tagset_bio_1 .. model_bio_N tagset_bio_N";
  //  string usage9 = "\n\t9)\tsst basic-feats-R filename unlab-fname istaggedfile mode secondorder gazfile lowercase";
  //  string usage10= "\n\t10)\tsst fragment featurefile bio_file nostoplist";
  //  string usage11= "\n\t11)\tsst estimate-diff source target gazetteers";
  string usage12= "\n\t12)\tsst fragment-data feature-file bi-file nostop-list";
  string usage10= "\n\t10)\tsst bi-to-pos file.bi";
  string usage11= "\n\t11)\tsst pos-to-w file.pos";
  string usage2= "\n\t2)\tsst multitag-line rowdata 2nd-order lowercase gazfile model_pos tagset_pos model_bio_1 tagset_bio_1 .. model_bio_N tagset_bio_N";
  string usage16= "\n\t16)\tsst split-by-entity filename-bi";
  string usage17= "\n\t17)\tsst split-rank filename-bi filename-tags";
  string usage18= "\n\t18)\tsst multitag-tab flist dirname 2nd-order lowercase gazfile model_bio_1 tagset_bio_1 .. model_bio_N tagset_bio_N\n";
  string usagereadme = "For more information see the README file\n\n";
  
  if (argc==7 && string(argv[1])=="tag")
    tag_light(argv[2],argv[3],argv[4],atoi(argv[5]),argv[6]);
  else if (argc==3 && string(argv[1])=="split-by-entity")
    split_by_entity(argv[2]);
  else if (argc==3 && string(argv[1])=="bi-to-pos")
    bi_to_pos(argv[2]);
  else if (argc==3 && string(argv[1])=="pos-to-w")
    pos_to_w(argv[2]);
  else if (argc==5 && string(argv[1])=="fragment-data")
    fragment_data_nostoplist(argv[2],argv[3],argv[4]);
  else if (argc==9 && string(argv[1])=="bitag")
    bitag_light(argv[2],argv[3],argv[4],argv[5],argv[6],atoi(argv[7]),atoi(argv[8]));
  else if (argc==7 && string(argv[1])=="postag")
    postag_light(argv[2],argv[3],argv[4],atoi(argv[5]),atoi(argv[6]));
  else if (argc==8 && string(argv[1])=="train")
    train_light(argv[2],argv[3],argv[4],atoi(argv[5]),atoi(argv[6]),argv[7]);
  else if (argc==12 && string(argv[1])=="eval")
    eval_light(argv[2],argv[3],argv[4],atoi(argv[5]),atoi(argv[6]),atoi(argv[7]),argv[8],atoi(argv[9]),atof(argv[10]),argv[11]);
  else if (argc==7 && string(argv[1])=="basic-feats")
    basic_feats(argv[2],atoi(argv[3]),argv[4],argv[5],atoi(argv[6]));
  else if (argc==9 && string(argv[1])=="basic-feats-R")
    basic_feats_R(argv[2],argv[3],atoi(argv[4]),argv[5],atoi(argv[6]),argv[7],atoi(argv[8]));
  else if (argc==3 && string(argv[1])=="split")
    split(argv[2]);
  else if (argc==5 && string(argv[1])=="fragment")
    fragment_data_nostoplist(argv[1],argv[2],argv[3]);
  else if (argc==5 && string(argv[1])=="estimate-diff")
    estimate_theta(argv[2],argv[3],argv[4]);
  else if (argc==4 && string(argv[1])=="split-rank")
    split_and_rank(argv[2],argv[3]);
  else if (argc >= 8 && string(argv[1],0,8)=="multitag"){
    vector<string> M, T;
    if (string(argv[1])=="multitag" || string(argv[1])=="multitag-line"){
      if (argc%2 != 0){
	cerr << "Error in the number of arguments"<<endl; abort();
      }
      int ntaggers = (argc-8)/2;
      for (int i = 8; i < argc; i=i+2){
	M.push_back(argv[i]);
	T.push_back(argv[i+1]);
      }
      if (string(argv[1])=="multitag")
	multitag(argv[2],atoi(argv[3]),atoi(argv[4]),argv[5],argv[6],argv[7],M,T);
      else if (string(argv[1])=="multitag-line")
	multitag_line(argv[2],atoi(argv[3]),atoi(argv[4]),argv[5],argv[6],argv[7],M,T);
    }
    else if (string(argv[1])=="multitag-tab"){
      int ntaggers = (argc-7)/2;
      for (int i = 7; i < argc; i=i+2){
	M.push_back(argv[i]);
	T.push_back(argv[i+1]);
      }
      multitag_tabular_format(argv[2],argv[3],atoi(argv[4]),atoi(argv[5]),argv[6],M,T);
    }
  }
  else{ 
    cerr << usage1+usage2+usage3+usage4+usage5+usage6+usage7+usage8+usage9+usage10+usage11+"\n"+usagereadme;
//+usage10+usage11+usage12+usage13+usage14+usage15+usage16+usage17+usage18+usagereadme;
    abort();
  }
  cerr << endl;
}

///////////////////////////////////////////////
