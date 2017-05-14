/**
 * Converts the SST output format into a single-column format
 * This format is compatible with the CoNLL shared task formats
 */

#include <iostream>
#include <string>
#include <vector>

using namespace std;

#define MAX_LINE 1024 * 1024

static void simpleTokenize(const string & input,
			   std::vector<string> & output,
			   const string & separators)
{
  for(int start = input.find_first_not_of(separators);
      start < (int) input.size() && start >= 0;
      start = input.find_first_not_of(separators, start)){
    int end = input.find_first_of(separators, start);
    if(end < 0) end = input.size();
    output.push_back(input.substr(start, end - start));
    start = end;
  }
}

int main()
{
  char line[MAX_LINE];
  while(cin.getline(line, MAX_LINE) != NULL){
    vector<string> tokens;
    simpleTokenize(line, tokens, " \t\n\r");
    for(int i = 1; i < tokens.size(); i ++){
      cout << tokens[i] << endl;
    }
    cout << endl;
  }
}
