void expand_basic_feats(string targetname,bool is_bi,string mode){
  cerr << "\nexpand_basic_feats(tagged=" << is_bi << "," << mode << ")";
  check_fname(targetname);
  ifstream in((targetname).c_str());
#if CONST_FEAT
  ofstream out((targetname+"_basefeats_k").c_str());
#else
  ofstream out((targetname+"_basefeats").c_str());
#endif
  char strbuf[10000];
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      out << id;
      vector<string> W, P;
      vector<string> L, W, POS, LOW, SH, SB, SB3, PR, PR3;
      while (in.peek() == 9){
	string w, pos, tag, sh = "", lemma, sb = "", sb3="", pr = "", pr3 = "";
	if (mode == "POS")
	  in >> w;
	else
	  in >> w >> pos;
	lemma = my_tolower_sh(w,sh);
	if (lemma.size() > 2){
	  sb = string(lemma,lemma.size()-2,2);
	  pr = string(lemma,0,2);
	}
	if (lemma.size() > 3){
	  sb3 = string(lemma,lemma.size()-3,3);
	  pr3 = string(lemma,0,3);
	}
	W.push_back(w);
	if (mode != "POS")
	  POS.push_back(pos);
	SB.push_back(sb);
	SB3.push_back(sb3);
	PR.push_back(pr);
	PR3.push_back(pr3);
	LOW.push_back(lemma);
	SH.push_back(sh);
	if (is_bi){
	  in >> tag;
	  L.push_back(tag);
	}
	//	cerr << endl << w << "\t" << pos << "\t" << lemma << "\t" << sh << "\t" << sb << "\t" << tag;getchar();
      }
      in.getline(strbuf,10000);
      for (int i = 0; i < W.size(); ++i){
	out << "\t";
#if CONST_FEAT
	out << "KF ";
#endif
	if (i==0)
	  out << "rp=begin ";
	else if (i<W.size()-1)
	  out << "rp=mid ";
	else 
	  out << "rp=end ";
	
	if (i > 0){
	  if (mode != "POS")
	    out << "pos-1="+POS[i-1]+" ";
	  out << "w-1="+LOW[i-1]+" "
	      << "sh-1="+SH[i-1]+" "
	      << "sb-1="+SB[i-1]+" ";
	}
	if (i < W.size()-1){
	  if (mode != "POS")
	    out << "pos+1="+POS[i+1]+" ";
	  out << "w+1="+LOW[i+1]+" "
	      << "sh+1="+SH[i+1]+" "
	      << "sb+1="+SB[i+1]+" ";
	}
	out << "w="+LOW[i]+" "
	    << "sh="+SH[i]+" "
	    << "pr="+PR[i]+" "
	    << "pr3="+PR3[i]+" "
	    << "sb="+SB[i]+" "
	    << "sb3="+SB3[i];

	if (is_bi){
	  if (mode != "POS")
	    out << " pos="+POS[i];
	  out << " " << L[i];
	}
	else if (mode != "POS")
	  out << " pos="+POS[i];
      }
      out << endl;
    }
  }
}
