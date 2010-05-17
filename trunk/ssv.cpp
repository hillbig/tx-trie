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

#include "ssv.hpp"

namespace tx_tool{

  ssv::ssv(const uint _size):
    B(NULL),size(0),oneNum(0),levelL(NULL),levelM(NULL), no_delete(false){
    if (resize(_size) == -1) return;
  }

  ssv::ssv(std::vector<bool>& bv):
    B(NULL),size(0),oneNum(0),levelL(NULL),levelM(NULL), no_delete(false){
    if (resize((uint)bv.size()) == -1) return;
    for (uint i = 0; i < (uint)bv.size(); i++){
      setBit(i,bv[i]);
    }
  }

  int ssv::resize(const uint _size){
    free();
    size = _size;
    blockSize = size/SSV_BLOCK + 1;
    isBuild = false;
    B = new uint [blockSize];
    if (B == NULL){
      return -1;
    }
    memset(B,0,sizeof(uint)*blockSize);

    LBlockSize = size / SSV_LBLOCK + 1;
    MBlockSize = size / SSV_MBLOCK + 1;

    levelL = new uint [LBlockSize];
    levelM = new uchar [MBlockSize];
    return 0;
  }

  void ssv::free(){
    if (!no_delete){
      if (B){
	delete[] B; 
	B=NULL;
      }
    }
    if (levelL){
      delete[] levelL; 
      levelL=NULL;
    }
    if (levelM){
      delete[] levelM;
      levelM=NULL;
    }
  }

  ssv::~ssv(){
    free();
  }

  uint ssv::getBits(const uint pos, uint width) const{
    assert(width <=32);
    assert(pos+width <= size);
    if (width == 0) return 0;

    //---: block version
    const uint endPos = pos+width-1;
    const uint blockPos_1 = pos / SSV_BLOCK;
    const uint blockPos_2 = endPos / SSV_BLOCK;
    const uint offset_1   = pos % SSV_BLOCK;

    if (blockPos_1 == blockPos_2){
      if (width == 32){
	return B[blockPos_1]; //avoid 1U << 32
      }
      else {
	return (B[blockPos_1] >> offset_1) & ((1U << width) - 1);
      }
    }
    else {
      const uint offset_2   = endPos % SSV_BLOCK;
      return  (B[blockPos_1] >> offset_1) + ((B[blockPos_2] & ((1U << (offset_2+1U))-1U)) << (32U-offset_1));
    }		
  }

  void ssv::setBits(const uint pos, const uint width, const uint x){
    assert(width <= 32);
    assert(pos+width <= size);
    if (width == 0) return;

    //---: block version ----
    const uint endPos     = pos+width-1;
    const uint blockPos_1 = pos / SSV_BLOCK;
    const uint blockPos_2 = endPos / SSV_BLOCK;
    const uint offset_1   = pos % SSV_BLOCK;

    if (blockPos_1 == blockPos_2){
      if (width == 32){
	B[blockPos_1] = x;
      }
      else {
	B[blockPos_1] &= ~(((1U << width)-1) << offset_1); // |1111110000000111111|
	B[blockPos_1] |= (x << offset_1);                  //        xxxxxxx
      }
    }
    else {
      const uint offset_2 = endPos % SSV_BLOCK;
      B[blockPos_1] &= ((1U << offset_1)-1U);		
      B[blockPos_1] |= (x << offset_1);
      B[blockPos_2] &= ~((1U << (offset_2+1U))-1U);
      B[blockPos_2] |= (x >> (32-offset_1));
    }
  }


  uint ssv::rank(uint pos, const uint bit) const{
    pos++;
    if (bit == 0) return pos-_rank1(pos);
    else		  return _rank1(pos);
  }

  uint ssv::select(const uint pos, const uint bit) const{
    if (bit) return _select1(pos);
    else     return _select0(pos);
  }

  void ssv::setBit(uint pos, uint x){
    if (!x) B[pos / SSV_BLOCK] &= (~(1U << (pos % SSV_BLOCK)));
    else    B[pos / SSV_BLOCK] |=   (1U << (pos % SSV_BLOCK));
  }

  uint ssv::getAllocate() const{
    uint sum = size;
    if (isBuild){
      sum += LBlockSize * 32 + MBlockSize * 8;
    }
    return sum;
  }

  void ssv::build(){
    oneNum = rankBuild(size);
    isBuild = true;
  }

  uint ssv::rankBuild(const uint t_size){
    uint sum = 0;
    for (uint il = 0; il <= t_size ;il += SSV_LBLOCK){
      levelL[il/SSV_LBLOCK] = sum;
      for (uint im = 0; im < SSV_LBLOCK && il+im <= t_size; im += SSV_MBLOCK){
	levelM[(il+im)/SSV_MBLOCK] = sum - levelL[il/SSV_LBLOCK];
	for (uint is = 0; is < SSV_MBLOCK && (il+im+is) <= t_size; is += SSV_BLOCK){
	  sum += popCount(B[(il+im+is)/SSV_BLOCK]);
	}
      }
    }
    return sum;
  }


  int ssv::write(FILE* outfp){
    if (fwrite(&size,sizeof(uint),1,outfp) != 1){
      return -1;
    }
    if (fwrite(B,sizeof(uint),blockSize,outfp) != blockSize){
      return -1;
    }
    if (fwrite(levelL,sizeof(uint),LBlockSize,outfp) != LBlockSize){
      return -1;
    }
    if (fwrite(levelM,sizeof(uchar),MBlockSize,outfp) != MBlockSize){
      return -1;
    }
    return 0;
  }

  int ssv::read(FILE* infp){
    if (fread(&size,sizeof(uint),1,infp) != 1){
      return -1;
    }
    if (resize(size) == -1){
      return -1;
    }

    if (fread(B,sizeof(uint),blockSize,infp) != blockSize){
      return -1;
    }
    isBuild = true;

    if (fread(levelL,sizeof(uint),LBlockSize,infp) != LBlockSize){
      return -1; 
    }
    if (fread(levelM,sizeof(uchar),MBlockSize,infp) != MBlockSize){
      return -1;
    }
    return 0;
  }

  size_t ssv::set_array(void* ptr){
    size = *(uint*)(ptr);

    blockSize = size/SSV_BLOCK + 1;
    isBuild = false;
    LBlockSize = size / SSV_LBLOCK + 1;
    MBlockSize = size / SSV_MBLOCK + 1;
  
    B = ((uint*)ptr + 1);
    levelL = ((uint*)ptr + 1 + blockSize);
    levelM = ((uchar*)ptr + sizeof(uint)*(1 + blockSize + LBlockSize));

    no_delete = true;
    return sizeof(uint)*(1 + blockSize + LBlockSize) + MBlockSize;
  }
  
  
  uint ssv::getBlock(const uint blockPos) const {
    return B[blockPos]; 
  }

  void ssv::setBlock(const uint blockPos, const uint x) {
    B[blockPos] = x; 
  }

  uint ssv::getSize() const {
    return size;
  }

  uint ssv::getBlockSize() const {
    return blockSize;
  }

  uint ssv::popCount(uint r) const{
    r = ((r & 0xAAAAAAAA) >> 1) + (r & 0x55555555);
    r = ((r & 0xCCCCCCCC) >> 2) + (r & 0x33333333);
    r = ((r >> 4) + r) & 0x0F0F0F0F;
    r = (r>>8) + r;
    return ((r>>16) + r) & 0x3F;
  }

  uint ssv::_rank1(const uint pos) const{
    if (pos > size){
      fprintf(stderr,"_rank1 range error pos:%d size:%d\n",pos,size);
      return (uint)-1;
    }
    //printf("pos:%d size:%d lr:%d lp:%d mr:%d \n",pos,size,levelL[pos >> SSV_LBLOCK_SHIFT],pos >> SSV_LBLOCK_SHIFT,levelM[pos >> SSV_MBLOCK_SHIFT]);
    return levelL[pos >> SSV_LBLOCK_SHIFT] + 
      levelM[pos >> SSV_MBLOCK_SHIFT] + 
      popCount((B[pos >> SSV_BLOCK_SHIFT] & ((1U << (pos  % SSV_BLOCK))-1)));
  }

  uint ssv::_select0(uint x) const{
    uint left  = 0;
    uint right = LBlockSize;
    while (left < right){
      uint mid = (left+right)/2;
      if (SSV_LBLOCK * mid - levelL[mid] < x) left  = mid+1;
      else                 right = mid;
    }
    uint posL = (left != 0) ? left-1: 0;

    // sequential search over levelB
    uint posM = posL * (SSV_LBLOCK/SSV_MBLOCK);
    x -= (SSV_LBLOCK*posL - levelL[posL]);

    uint mstep = SSV_LBLOCK/SSV_MBLOCK;
    while ((posM+1 < MBlockSize) && (((posM+1)%mstep) != 0) && (SSV_MBLOCK*((posM+1)%mstep)-levelM[posM+1] < x)){
      posM++;
    }

    x -= (SSV_MBLOCK*(posM % mstep) - levelM[posM]);

    uint ret    = posM * SSV_MBLOCK;
    uint posB   = posM * (SSV_MBLOCK/SSV_BLOCK);
    uint rank32 = 32- popCount(B[posB]);
    if (rank32 < x){
      posB++;
      ret += SSV_BLOCK;
      x -= rank32;
    }

    uint curB   = B[posB];
    uint rank16 = 16-popCount(curB & ((1U<<16)-1));
    if (rank16 < x){
      curB >>= 16;
      ret += 16;
      x -= rank16;
    }

    uint rank8  = 8-popCount(curB & ((1U << 8)-1));
    if (rank8 < x){
      curB >>= 8;
      ret += 8;
      x -= rank8;
    }

    while (x > 0){
      if ((~curB) & 1){
	x--;
      }
      curB >>= 1;
      ret++;
    }
    return ret-1;
  }

  uint ssv::_select1(uint x) const{
    uint left  = 0;
    uint right = LBlockSize;
    while (left < right){
      uint mid = (left+right)/2;
      if (levelL[mid] < x) left  = mid+1;
      else                 right = mid;
    }
    uint posL = (left != 0) ? left-1: 0;

    // sequential search over levelB
    uint posM = posL * (SSV_LBLOCK/SSV_MBLOCK);
    x -= levelL[posL];
    while ((posM+1 < MBlockSize) && (((posM+1)%(SSV_LBLOCK/SSV_MBLOCK)) != 0) && (levelM[posM+1] < x)){
      posM++;
    }

    x -= levelM[posM];

    uint ret    = posM * SSV_MBLOCK;
    uint posB   = posM * (SSV_MBLOCK/SSV_BLOCK);
    uint rank32 = popCount(B[posB]);
    if (rank32 < x){
      posB++;
      ret += SSV_BLOCK;
      x -= rank32;
    }

    uint curB   = B[posB];
    uint rank16 = popCount(curB & ((1U<<16)-1));
    if (rank16 < x){
      curB >>= 16;
      ret += 16;
      x -= rank16;
    }

    uint rank8  = popCount(curB & ((1U << 8)-1));
    if (rank8 < x){
      curB >>= 8;
      ret += 8;
      x -= rank8;
    }

    while (x > 0){
      if (curB & 1){
	x--;
      }
      curB >>= 1;
      ret++;
    }
    return ret-1;
  }

}
