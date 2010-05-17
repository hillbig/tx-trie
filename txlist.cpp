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
    cerr << argv[1] << " index" << endl;
    return -1;
  }

  tx_tool::tx t;
  if (t.read(argv[1]) == -1){
    cerr << t.getErrorLog()
	 << "cannot read index " << argv[1] << endl;
    return -1;
  }
  
  tx_tool::uint keyNum = t.getKeyNum();
  for (tx_tool::uint i = 0; i < keyNum; ++i){
    string ret;
    tx_tool::uint len = t.reverseLookup(i, ret);
    cout << ret << endl;
  }

  return 0;
}
