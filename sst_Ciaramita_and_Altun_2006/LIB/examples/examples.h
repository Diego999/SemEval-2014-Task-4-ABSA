//   examples.h: part of sst-light
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

#ifndef EXAMPLES_H
#define EXAMPLES_H

#include "../tagger_light/Tlight.h"
#include "../features/features.h"

void tag_light(string modelname,string targetname,string tagsetname,bool secondorder,string mode);
void eval_light(string traindata,string testdata,string tagsetname,bool secondorder,int T,int CV,string mode,bool R,Float ww,string thetafile);
void train_light(string modelname,string traindata,string tagsetname,bool secondorder,int T,string mode);
void collect_postags(string fname);
void add_a_feature(string fname,string featname,string oname);
void collect_tags_dist(string fname,string oname);
void pseudo_train(string fname,string oname);
void merge_pos(string fname,string oname,string posname,string tag);
void bitag_light(string modelname_pos,
		 string tagsetname_pos,
		 string modelname,
		 string tagsetname,
		 string targetname,
		 bool secondorder,
		 bool lowercase);
void postag_light(string modelname_pos,
		  string tagsetname_pos,
		  string targetname,
		  bool secondorder,
		  bool lowercase);
void multitag(string target,bool secondorder,bool lowercase,string gazfile,
	      string model_pos,string tagset_pos,
	      vector<string>& M_bio,vector<string>& T_bio);
void multitag_line(string target,bool secondorder,bool lowercase,string gazfile,
	      string model_pos,string tagset_pos,
		   vector<string>& M_bio,vector<string>& T_bio);
void fragment_data(string fname);
void lowercase_bi_data(string ifname,string ofname);
void uppercase_bi_data(string ifname,string ofname);
void merge_pos_tag(string posfname,string tagfname,string outfname);
void create_nostop_list(string fname);
void fragment_data_nostoplist(string fname,string biname,string lname);
void bi_to_line(string fname);
void map_tags(string dataname,string mapname);
void VRSP_make_B(string fname,int k,bool secondorder);
void VSRP_transform(string fname,string bname);
void ADAPT_olivier_format_data(string fname1,string fname2);
void multitag_tabular_format(string flist,string dirname,bool secondorder,bool lowercase,string gazfile,
			     vector<string>& M_bio,vector<string>& T_bio);
#endif
