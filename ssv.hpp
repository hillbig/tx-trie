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

#ifndef __SSV_HPP__
#define __SSV_HPP__

#include <memory.h>
#include <vector>
#include <cassert>
#include <stdio.h>

namespace tx_tool{

  typedef unsigned int uint; // 32bit
  typedef unsigned short ushort; // 16bit
  typedef unsigned char uchar; // 8bit


#define SSV_BLOCK_SHIFT  (5)
#define SSV_BLOCK        (1U << SSV_BLOCK_SHIFT)

#define SSV_LBLOCK_SHIFT (8)
#define SSV_LBLOCK		 (1U << SSV_LBLOCK_SHIFT)
#define SSV_MBLOCK_SHIFT (5)
#define SSV_MBLOCK       (1U << SSV_MBLOCK_SHIFT)

#define logLL (16)
#define LL (1U << logLL)
#define logLLL (5)
#define LLL (1U << logLLL)
#define logL (logLL-1-5)
#define L (1U << logL)

  class ssv{

  public:
    ssv(const uint _size = 0);
    ssv(std::vector<bool>& bv);

    int resize(const uint _size);

    void free();

    ~ssv();

    inline uint getBit(const uint pos) const {
      return (B[pos / SSV_BLOCK] >> (pos % SSV_BLOCK)) & 1;
    }

    uint getBits(const uint pos, uint width) const;
    void setBits(const uint pos, const uint width, const uint x);
    uint rank(uint pos, const uint bit) const;
    uint select(uint pos, const uint bit) const;

    void setBit(uint pos, uint x);
    uint getAllocate() const;

    void build();

    uint rankBuild(const uint t_size); // return oneNum
    void selectBuild(const uint t_size);

    int write(FILE* outfp);
    int read(FILE* infp);

    uint getBlock(const uint blockPos) const;
    void setBlock(const uint blockPos, const uint x);
    uint getSize() const;
    uint getBlockSize() const;

    size_t set_array(void* ptr);

  private:
    uint popCount(uint r) const;
    uint _rank1(const uint pos) const;
    uint _select1(uint x) const;
    uint _select0(uint x) const;
    uint* B;
    uint size;
    uint oneNum;
    uint blockSize;	// (size+SSV_BLOCK-1)/SSV_BLOCKSZIE

    uint LBlockSize;
    uint MBlockSize;

    // for rank
    uint* levelL;
    uchar* levelM;

    bool isBuild;
    bool no_delete;
  };

}

#endif
