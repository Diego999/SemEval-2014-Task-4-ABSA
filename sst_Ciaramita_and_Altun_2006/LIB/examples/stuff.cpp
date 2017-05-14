//@TODO JAB COMMMENT OUT
/**
void basic_feats_R(string targetname,string uname,bool is_bi,string mode,bool secondorder){
  cerr << "\nbasic_feats_R("+targetname+","+uname << ",tagged=" << is_bi << "," << mode << ")";
  check_fname(targetname);
  ifstream in((targetname).c_str());
  char strbuf[10000];
  HM TF, DF;
  int nwords = 0, ndocs = 0;
  get_TF(uname,TF,nwords);
  get_DF(uname,DF,ndocs);
  while (in.peek() != -1){
    string id = "";
    in >> id;
    if (id != ""){
      cout << id;
      vector<string> W, POS, P;
      while (in.peek() == 9){
	string w, pos;
	in >> w >> pos;
	W.push_back(w);
	POS.push_back(pos);
      }
      vector<vector<string> > O;
      extract_feats(W,P,O);
      vector<vector<pair<string,Float> > > OR;
#if USE_R
      extract_feats_R(W,TF,DF,ndocs,nwords,OR,secondorder);
#endif
      for (int i = 0; i < O.size(); ++i){
	cout << "\t" << O[i][0] << " 1";
	for (int j = 1; j < O[i].size(); ++j)
	  cout << " " << O[i][j] << " 1";
	if (OR.size())
	  for (int r = 0; r < OR[i].size(); ++r)
	    cout << " " << OR[i][r].first << " " << OR[i][r].second;
	cout << " " << POS[i] << " 1";
      }
      cout << endl;
    }
    in.getline(strbuf,10000);
  }
}
**/
