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

using namespace std;

int main(int argc, char* argv[]){
  if (argc != 2){
    fprintf(stderr,"%s index\n",argv[0]);
    return -1;
  }

  string indexName(argv[1]);
  tx_tool::tx t1;
  if (t1.read((indexName+".1").c_str()) == -1){
    fprintf(stderr, "%s", t1.getErrorLog().c_str());
    fprintf(stderr,"cannot read index %s\n", (indexName+".1").c_str());
    return -1;
  }
  fprintf(stderr,"%s", t1.getResultLog().c_str());

  tx_tool::tx t2;
  if (t2.read((indexName+".2").c_str()) == -1){
    fprintf(stderr, "%s", t2.getErrorLog().c_str());
    fprintf(stderr,"cannot read index %s\n", (indexName+".2").c_str());
    return -1;
  }
  fprintf(stderr,"%s", t2.getResultLog().c_str());

  std::vector<std::vector<tx_tool::uint> > IDmap;
  {
    FILE* infp = fopen((indexName+".12").c_str(), "rb");
    if (infp == NULL){
      fprintf(stderr, "cannot open %s\n", (indexName + ".12").c_str());
      return -1;
    }
    const size_t keyNum1 = t1.getKeyNum();
    for (size_t i = 0; i < keyNum1; i++){
      tx_tool::uint num = 0;
      if (fread(&num, sizeof(tx_tool::uint), 1, infp) != 1){
	fprintf(stderr, "read error\n");
	fclose(infp);
	return -1;
      }
      std::vector<tx_tool::uint> vals;
      for (tx_tool::uint j = 0; j < num; j++){
	tx_tool::uint val = 0;
	if (fread(&val, sizeof(tx_tool::uint), 1, infp) != 1){
	  fprintf(stderr, "read error\n");
	  fclose(infp);
	  return -1;
	}
	vals.push_back(val);
      }
      IDmap.push_back(vals);
    }
  }

  string query;
  for (;;){
    putchar('>');
    getline(cin,query);
    if (query.size() == 0) break;
		
    // commonPrefixSearch
    for (size_t i = 0; i < query.size(); i++){
      vector<string> ret;
      vector<tx_tool::uint> retID;
      const tx_tool::uint retNum = t1.commonPrefixSearch(query.c_str()+i,query.size()-i, ret, retID, 10);
      for (tx_tool::uint i = 0; i < retNum; i++){
	printf("%s\t",ret[i].c_str());
	for (size_t j = 0; j < IDmap[retID[i]].size(); j++){
	  string ret;
	  size_t retLen = t2.reverseLookup(IDmap[retID[i]][j], ret);
	  if (retLen == 0) continue;
	  printf("%s ", ret.c_str());
	}
	printf("\n");
      }
    }

  }
  return 0;
}
