#include <string>
#include <fstream>
#include <vector>
#include "tx.hpp"

int main(int argc, char* argv[]){
  if (argc != 3){
    fprintf(stderr,"%s wordlist index\n",argv[0]);
    return -1;
  }

  std::vector<std::string> wordList;
  {
    std::ifstream ifs(argv[1]);
    if (!ifs){
      fprintf(stderr,"cannot open %s\n",argv[1]);
      return -1;
    }

    std::string word;
    while (getline(ifs, word)){
      // handle CR-LF
      if (word.size() > 0 && word[word.size()-1] == '\r'){
	word = word.substr(0, word.size()-1);
      }
      if (word == "") continue;
      wordList.push_back(word);
    }
  }

  tx_tool::tx t;
  if (t.build(wordList,argv[2]) == -1){
    fprintf(stderr, "%s", t.getErrorLog().c_str());
    fprintf(stderr, "cannot build index for %s\n",argv[1]);
    return -1;
  }
  fprintf(stderr, "%s", t.getResultLog().c_str());
  
  return 0;
}
