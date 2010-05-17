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
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "tx.hpp"

using namespace std;

int main(int argc, char* argv[]){
  if (argc != 2){
    fprintf(stderr,"%s index\n",argv[0]);
    return -1;
  }

  void* buf = NULL;
  size_t bufSize = 0;
  {
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0){
      fprintf(stderr, "cannot open %s\n", argv[1]);
      return -1;
    }
    struct stat st;
    if (fstat(fd, &st) == -1){
      fprintf(stderr, "stat error\n");
      return -1;
    }
    bufSize = st.st_size;
    buf = mmap(0, bufSize, PROT_READ, MAP_SHARED, fd, 0);
    if (buf == MAP_FAILED){
      fprintf(stderr, "mmap error\n");
      return -1;
    }
    close(fd);
  }

  tx_tool::tx t;
  if (t.setArray(buf, bufSize) == -1){
    fprintf(stderr, "%s", t.getErrorLog().c_str());
    fprintf(stderr,"cannot read index %s\n",argv[1]);
    return -1;
  }
  fprintf(stderr,"%s", t.getResultLog().c_str());

  string query;
  for (;;){
    putchar('>');
    getline(cin,query);
    if (query.size() == 0) break;
		
    // prefixSearch
    {
      size_t retLen = 0;
      const tx_tool::uint id = t.prefixSearch(query.c_str(),query.size(),retLen);
      printf("prefixSearch ");
      if (id == tx_tool::tx::NOTFOUND){
	printf("not found\n");
      } else {
	printf("id:%u len:%u\n", id, retLen);
      }
    }

    // expandSearch
    {
      vector<string> ret;
      const tx_tool::uint retNum = t.expandSearch(query.c_str(),query.size(), ret, 10);
      printf("expansionSearch %u\n", retNum);
      for (vector<string>::const_iterator it = ret.begin(); it != ret.end(); it++){
	printf("%s\n",it->c_str());
      }
    }

    // commonPrefixSearch
    {
      vector<string> ret;
      vector<tx_tool::uint> retID;
      const tx_tool::uint retNum = t.commonPrefixSearch(query.c_str(),query.size(), ret, retID, 10);
      printf("commonPrefixSearch %u\n", retNum);
      for (size_t i = 0; i < ret.size(); i++){
	printf("%s (id=%d)\n",ret[i].c_str(), retID[i]);
      }
    }

    // predictiveSearch
    {
      vector<string> ret;
      vector<tx_tool::uint> retID;
      const tx_tool::uint retNum = t.predictiveSearch(query.c_str(),query.size(), ret, retID, 10);
      printf("predictiveSearch %u\n", retNum);
      for (size_t i = 0; i < ret.size(); i++){
	printf("%s (id=%d)\n",ret[i].c_str(), retID[i]);
      }
    }
  }
  munmap(buf, bufSize);
  return 0;
}
