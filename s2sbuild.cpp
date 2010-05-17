/* 
 *  Copyright (c) 2007-2010 Daisuke Okanohara
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 * 
 *   1. Redistributions of source code must retain the above Copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above Copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 */

#include <string>
#include <fstream>
#include <vector>
#include "tx.hpp"

int createIndex(std::vector<std::string>& wordList, const std::string& indexName){
  tx_tool::tx t;
  if (t.build(wordList, indexName.c_str()) == -1){
    fprintf(stderr, "%s", t.getErrorLog().c_str());
    fprintf(stderr, "cannot build index for %s\n", indexName.c_str());
    return -1;
  }
  return 0;
}

int main(int argc, char* argv[]){
  if (argc != 3){
    fprintf(stderr,"%s word2wordlist index\n",argv[0]);
    return -1;
  }

  std::vector<std::string> wordList1;
  std::vector<std::string> wordList2;
  std::vector<std::pair<std::string, std::string> > wordListPair;
  {
    std::ifstream ifs(argv[1]);
    if (!ifs){
      fprintf(stderr,"cannot open %s\n",argv[1]);
      return -1;
    }

    // Each line contains 2 keyword. 
    // Assume that these keywords are separated by tab.

    std::string line;
    size_t lineno = 0;
    while (getline(ifs, line)){
      const size_t p = line.find('\t');
      if (p == std::string::npos){
	fprintf(stderr, "WARNING: cannot find tab in line in %s (lineno=%u)\n", line.c_str(), lineno);
	continue;
      }
      wordList1.push_back(line.substr(0, p));
      wordList2.push_back(line.substr(p+1));
      wordListPair.push_back(make_pair(line.substr(0, p), line.substr(p+1)));
      lineno++;
    }
  }

  std::string indexName(argv[2]);

  if (createIndex(wordList1, indexName + ".1") == -1){
    return -1;
  }

  if (createIndex(wordList2, indexName + ".2") == -1){
    return -1;
  }

  tx_tool::tx t1;
  tx_tool::tx t2;

  if (t1.read((indexName + ".1").c_str()) == -1){
    return -1;
  }

  if (t2.read((indexName + ".2").c_str()) == -1){
    return -1;
  }

  
  std::vector< std::vector<tx_tool::uint> > IDmap(wordListPair.size());
  for (size_t i = 0; i < wordListPair.size(); i++){
    size_t retLen1 = 0;
    size_t retLen2 = 0;
    const std::string str1 = wordListPair[i].first;
    const std::string str2 = wordListPair[i].second;
    tx_tool::uint mappedID = t2.prefixSearch(str2.c_str(), str2.size(), retLen2);
    IDmap[t1.prefixSearch(str1.c_str(), str1.size(), retLen1)].push_back(mappedID);
  }

  FILE* outfp = fopen((indexName + ".12").c_str(), "wb");
  if (outfp == NULL){
    fprintf(stderr, "cannot open %s\n", (indexName + ".12").c_str());
    return -1;
  }
  for (size_t i = 0; i < IDmap.size(); i++){
    tx_tool::uint num = IDmap[i].size();
    if (fwrite(&num, sizeof(tx_tool::uint), 1, outfp) != 1){
      fprintf(stderr, "fwrite error\n");
      return -1;
    }
    for (tx_tool::uint j = 0; j < num; j++){
      tx_tool::uint val = IDmap[i][j];
      if (fwrite(&val, sizeof(tx_tool::uint), 1, outfp) != 1){
	fprintf(stderr, "fwrite error\n");
	return -1;
      }
    }
  }

  fclose(outfp);
  return 0;
}
