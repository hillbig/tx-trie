#ifndef __TX_HPP__
#define __TX_HPP__

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <sstream>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ssv.hpp"

namespace tx_tool{

#define TX_LIMIT_DEFAULT (0xffffffff)

  class tx{

  public:
    tx();
    ~tx();
    int build(std::vector<std::string>& wordList, const char* fileName) ;
    int read(const char* fileName);

    uint prefixSearch(const char* str, const size_t len, size_t& retLen) const;
    uint expandSearch(const char* str, const size_t len, std::vector<std::string>& ret, const uint limit = 0) const;
    uint commonPrefixSearch(const char* str, const size_t len, std::vector<std::string>& ret, std::vector<uint>& retID,  const uint limit = TX_LIMIT_DEFAULT) const;
    uint commonPrefixSearch(const char* str, const size_t len, std::vector<uint>& retLen, std::vector<uint>& retID,  const uint limit = TX_LIMIT_DEFAULT) const;
    uint predictiveSearch(const char* str, const size_t len, std::vector<std::string>& ret, std::vector<uint>& retID, const uint limit = TX_LIMIT_DEFAULT) const;
    uint predictiveSearch(const char* str, const size_t len, std::vector<uint>& retLen, std::vector<uint>& retID,  const uint limit = TX_LIMIT_DEFAULT) const;

    uint reverseLookup(const uint id, std::string& ret) const;

    std::string getResultLog() const;
    std::string getErrorLog() const;

    int setArray(void* ptr, size_t readSize);
    uint getKeyNum() const;
    
    static uint NOTFOUND;
    
  private:
    uint getChild(const uint pos, const char c) const;
    uint getParent(const uint pos, char& c) const;
    void enumerateAll(const uint pos, const std::string str, std::vector<std::pair<size_t, std::pair<std::string, uint> > >& ret) const; 
    ssv loud;
    ssv terminal;
    char* edge;

    size_t keyNum;
    std::ostringstream resultLog;
    std::ostringstream errorLog;

    bool no_delete;
  };

}

#endif // __TX_HPP__
