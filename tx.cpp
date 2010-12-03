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

#include "tx.hpp"
#include "ssv.hpp"
namespace tx_tool{

  uint tx::NOTFOUND = UINT_MAX;

  struct queue_elem{
    queue_elem(size_t _left, size_t _right, int _depth) :left(_left),right(_right),depth(_depth){}
    size_t left;
    size_t right;
    int depth;
  };

  tx::tx():edge(NULL), keyNum(0), no_delete(false) {}

  tx::~tx(){
    if (!no_delete) {
      delete[] edge;
      edge = NULL;
      no_delete = false;
    }
  }

  int tx::build(std::vector<std::string>& wordList, const char* fileName) {
    sort(wordList.begin(),wordList.end());
    const size_t origWordNum = wordList.size();
    wordList.erase(unique(wordList.begin(),wordList.end()),wordList.end());
    int keyNum = (int)wordList.size();
    if (keyNum != origWordNum){
      resultLog << "shrink word list " << origWordNum << " -> " << keyNum << std::endl;
    } else {
      resultLog << "word list " << keyNum << " elements" << std::endl;
    }

    uint totalSize = 0;
    for (size_t i = 0; i < wordList.size(); i++){
      totalSize += (uint)wordList[i].size();
    }
		
    FILE* outfp = fopen(fileName,"wb");
    if (outfp == NULL){
      errorLog << "cannot open " << fileName << std::endl;
      return -1;
    }
	
    std::queue<queue_elem> q;
    if (keyNum != 0){
      q.push(queue_elem(0,keyNum,0));
    }
    if (fwrite(&keyNum,sizeof(int),1,outfp) != 1){
      errorLog << "fwrite error " << std::endl;
      return -1;
    }

    std::vector<bool> vb_loud;
    std::vector<bool> vb_terminal;

    vb_loud.push_back(0); // super root
    vb_loud.push_back(1); 

    uint nodeNum = 0;

    while (!q.empty()){
      queue_elem& elem = q.front();
      const int depth    = elem.depth;
      const size_t left  = elem.left;
      const size_t right = elem.right;
      q.pop();

      nodeNum++;
      size_t newLeft = left;
      if (wordList[left].size() == depth){
	vb_terminal.push_back(1); // this node has terminate
	newLeft++;
	if (newLeft == right){
	  vb_loud.push_back(1);
	  continue;
	}
      } else {
	vb_terminal.push_back(0);
      }
      size_t prev = newLeft;
      char prev_c = wordList[prev][depth];
      for (size_t i = newLeft+1; i < right; i++){
	if (prev_c != wordList[i][depth]){
	  fputc(prev_c,outfp);
	  vb_loud.push_back(0);
	  q.push(queue_elem(prev,i,depth+1));
	  prev = i;
	  prev_c = wordList[prev][depth];
	}
      }
      if (prev != right){
	fputc(prev_c,outfp);
	vb_loud.push_back(0);
	q.push(queue_elem(prev,right,depth+1));
      }
      vb_loud.push_back(1);
    }

    {
      ssv sv(vb_loud);
      sv.build();
      if (sv.write(outfp) == -1){
	errorLog << "fwrite error " << std::endl;
	return -1;
      }
    }
    {
      ssv sv(vb_terminal);
      sv.build();
      if (sv.write(outfp) == -1){
	errorLog << "fwrite error " << std::endl;
	return -1;
      }
    }
	
    if (fwrite(&nodeNum,sizeof(int),1,outfp) != 1){
      errorLog << "fwrite error " << std::endl;
      return -1;
    }

    size_t outfpSize = ftell(outfp);
    resultLog << "outputSize:" << outfpSize << " inputSize:" << totalSize << " ratio:" << (float)outfpSize/totalSize << std::endl;
    if (outfp) fclose(outfp);
    return 0;
  }

  int tx::read(const char* fileName){
    FILE* infp = fopen(fileName,"rb");
    if (infp == NULL){
      errorLog << "cannot open " << fileName << std::endl;
      return -1;
    }

    keyNum = 0;
    if (fread(&keyNum,sizeof(int),1,infp) != 1){
      errorLog << "keyNum read error" << std::endl;
      fclose(infp);
      return -1;
    }

    fseek(infp,0,SEEK_END);
    size_t fileSize = ftell(infp);
    if (fseek(infp,fileSize-(1*sizeof(int)),SEEK_SET) == -1){
      errorLog << "fseek error" << std::endl;
      fclose(infp);
      return -1;
    }

    int nodeNum = -1;
    if (fread(&nodeNum,sizeof(int),1,infp) != 1){
      errorLog << "nodeNum read error" << std::endl;
      fclose(infp);
      return -1;
    }
    resultLog << "keyNum:" << (int)keyNum << " nodeNum:" << nodeNum << std::endl;

    if (fseek(infp,sizeof(int)*1,SEEK_SET) == -1){
      errorLog << "fseek error" << std::endl;
      fclose(infp);
      return -1;
    }

    if (nodeNum > 0){
      edge = new char [nodeNum-1];
      if (fread(edge,sizeof(char),nodeNum-1,infp) != nodeNum-1){
	errorLog << "fseek error" << std::endl;
	fclose(infp);
	return -1;
      }
    }

    loud.read(infp);
    terminal.read(infp);

    if (infp) fclose(infp);
    return 0;
  }

  int tx::setArray(void* ptr, size_t readSize){
    keyNum = *(uint*)(ptr);
    printf("keyNum:%d\n", keyNum);
    int nodeNum = *(uint*)((uchar*)ptr+readSize-sizeof(uint));
    printf("nodeNum:%d\n", nodeNum);
    edge = (char*)ptr + sizeof(uint);
    size_t readNum = loud.set_array((void*)((uchar*)ptr + sizeof(uint) + nodeNum - 1));
    size_t readNum2 = terminal.set_array((void*)((uchar*)ptr + sizeof(uint) + nodeNum - 1 + readNum));
    if (sizeof(uint) + nodeNum + - 1 + readNum + readNum2 + sizeof(uint) != readSize){
      errorLog << "setArray error" << std::endl;
      return -1;
    }
    no_delete = true;
    return 0;
  }
  
  uint tx::prefixSearch(const char* str, const size_t len, size_t& retLen) const {
    uint curPos = 2;
    uint retId = NOTFOUND;
    if (terminal.getSize() <= 2) return retId;

    for (size_t i = 0 ; ; i++){
      const uint nodeId = loud.rank(curPos-1,1)-1;
      if (terminal.getBit(nodeId)){
	retLen = i;
	retId = terminal.rank(nodeId,1)-1;
      }
      if (i == len) break;
      uint nextPos = getChild(curPos,str[i]);
      if (nextPos == UINT_MAX){
	break;
      }
      curPos = nextPos;
    }
    return retId;
  }

  uint tx::expandSearch(const char* str, const size_t len, std::vector<std::string>& ret, const uint limit) const {
    ret.clear();
    if (limit == 0) return 0;
    if (terminal.getSize() <= 2) return 0;

    bool prefix = false;
    uint curPos = 2;
    for (size_t i = 0; i < len; i++){
      uint nextPos = getChild(curPos,str[i]);      
      const uint nodeId = loud.rank(curPos-1,1)-1;
      if (terminal.getBit(nodeId) && ret.size() < limit){
	ret.push_back(std::string(str,str+i));
      }
      
      if (nextPos == UINT_MAX){
	prefix = true;
	break;
      }
      curPos = nextPos;
    }
    
    if (!prefix){
      std::string curStr(str, len);
      std::vector<std::pair<size_t, std::pair<std::string, uint> > > ret_p;
      enumerateAll(curPos,curStr,ret_p); 
      sort(ret_p.begin(),ret_p.end());
      for (size_t i = 0; i < ret_p.size() && ret.size() < limit; i++){
	ret.push_back(ret_p[i].second.first);
      }
    } 
    return (uint)ret.size();
  }

  uint tx::commonPrefixSearch(const char* str, const size_t len, std::vector<std::string>& ret, std::vector<uint>& retID, const uint limit) const{
    ret.clear();
    retID.clear();
    if (limit == 0) return 0;
    if (terminal.getSize() <= 2) return 0;

    uint curPos = 2;

    for (size_t i = 0; ; i++){
      const uint nodeId = loud.rank(curPos-1,1)-1;
      if (terminal.getBit(nodeId)){
	ret.push_back(std::string(str, str+i));
	retID.push_back(terminal.rank(nodeId,1)-1);
	if (ret.size() == limit) break;
      }
      if (i == len) break;
      
      uint nextPos = getChild(curPos,str[i]);      
      if (nextPos == UINT_MAX){
	break;
      }
      curPos = nextPos;
    }    
    return (uint)ret.size();
  }

  uint tx::commonPrefixSearch(const char* str, const size_t len, std::vector<uint>& retLen, std::vector<uint>& retID, const uint limit) const{
    retLen.clear();
    retID.clear();
    if (limit == 0) return 0;
    if (terminal.getSize() <= 2) return 0;

    uint curPos = 2;
    for (size_t i = 0; ; i++){
      const uint nodeId = loud.rank(curPos-1,1)-1;
      if (terminal.getBit(nodeId)){
	retLen.push_back(i);
	retID.push_back(terminal.rank(nodeId,1)-1);
	if (retLen.size() == limit) break;
      }
      if (i == len) break;
      
      uint nextPos = getChild(curPos,str[i]);      
      if (nextPos == UINT_MAX){
	break;
      }
      curPos = nextPos;
    }
    
    return (uint)retLen.size();
  }

  
  uint tx::predictiveSearch(const char* str, const size_t len, std::vector<std::string>& ret, std::vector<uint>& retID, const uint limit) const{
    ret.clear();
    retID.clear();
    if (limit == 0) return 0;
    if (terminal.getSize() <= 2) return 0;
    
    bool prefix = false;
    uint curPos = 2;
    for (size_t i = 0; i < len; i++){
      uint nextPos = getChild(curPos,str[i]);      
      if (nextPos == UINT_MAX){
	prefix = true;
	break;
      }
      curPos = nextPos;
    }
    
    if (!prefix){
      std::string curStr(str, len);
      std::vector<std::pair<size_t, std::pair<std::string, uint> > > ret_p;
      enumerateAll(curPos, curStr, ret_p); 
      sort(ret_p.begin(),ret_p.end());
      for (size_t i = 0; i < ret_p.size() && i < limit; i++){
	ret.push_back(ret_p[i].second.first);
	retID.push_back(ret_p[i].second.second);
      }
    } 
    return (uint)ret.size();
  }

  uint tx::predictiveSearch(const char* str, const size_t len, std::vector<uint>& retLen, std::vector<uint>& retID, const uint limit) const{
    retLen.clear();
    retID.clear();
    if (limit == 0) return 0;
    if (terminal.getSize() <= 2) return 0;
    
    bool prefix = false;
    uint curPos = 2;
    for (size_t i = 0; i < len; i++){
      uint nextPos = getChild(curPos,str[i]);      
      if (nextPos == UINT_MAX){
	prefix = true;
	break;
      }
      curPos = nextPos;
    }
    
    if (!prefix){
      std::string curStr(str, len);
      std::vector<std::pair<size_t, std::pair<std::string, uint> > > ret_p;
      enumerateAll(curPos, curStr, ret_p); 
      sort(ret_p.begin(),ret_p.end());
      for (size_t i = 0; i < ret_p.size() && i < limit; i++){
	retLen.push_back(ret_p[i].second.first.size());
	retID.push_back(ret_p[i].second.second);
      }
    } 
    return (uint)retLen.size();
  }



  void tx::enumerateAll(const uint pos, const std::string str, std::vector<std::pair<size_t, std::pair<std::string, uint> > >& ret) const{
    const uint nodeId = loud.rank(pos-1,1)-1;
    if (terminal.getBit(nodeId)){
      std::pair<std::string, uint> tmp(str,  terminal.rank(nodeId,1)-1);
      ret.push_back(std::make_pair<size_t, std::pair<std::string, uint> >(str.size(), tmp));
    }

    uint curPos = pos;
    uint edgePos = loud.rank(pos,0)-2;
    while (loud.getBit(curPos) == 0){
      const uint nextPos = loud.select(loud.rank(curPos,0),1)+1;
      enumerateAll(nextPos,str + edge[edgePos],ret);
      curPos++;
      edgePos++;
    }
  }

  uint tx::getChild(const uint pos, const char c) const{
    uint curPos = pos;
    uint edgePos = loud.rank(pos,0)-2;
    for (;;){
      if (loud.getBit(curPos) == 1) {
	curPos = UINT_MAX;
	return curPos;
      }
      if (edge[edgePos] == c){
	uint nextPos = loud.select(loud.rank(curPos,0),1)+1;
	return nextPos;
      }
      curPos++;
      edgePos++;
    }
  }

  uint tx::getParent(const uint pos, char& c) const{
    c = edge[loud.rank(pos,0)-2];
    return loud.select(loud.rank(pos-1, 1), 0);
  }


  uint tx::reverseLookup(const uint id, std::string& ret) const {
    ret.clear();
    if (id >= keyNum) return 0;
    if (terminal.getSize() <= 2) return 0;

    const uint nodeId = terminal.select(id + 1, 1);
    char unused_c = 0;
    uint curPos = getParent(loud.select(nodeId+1,1)+1, unused_c);
    while (curPos >= 2){
      char c = 0;
      curPos = getParent(curPos, c);
      ret += c;
    }
    reverse(ret.begin(), ret.end());
    return ret.size();
  }

  std::string tx::getResultLog() const {
    return resultLog.str();
  }

  std::string tx::getErrorLog() const{
    return errorLog.str();
  }

  uint tx::getKeyNum() const {
    return keyNum;
  }


} // namespace tx_tool
