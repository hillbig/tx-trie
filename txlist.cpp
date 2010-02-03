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
