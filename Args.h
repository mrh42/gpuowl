// Copyright Mihai Preda.

#pragma once

#include "common.h"

#include <string>
#include <set>

class Args {
public:
  static std::string mergeArgs(int argc, char **argv);

  enum {CARRY_AUTO = 0, CARRY_SHORT, CARRY_LONG};

  void parse(string line);
  void setDefaults();
  bool uses(const std::string& key) const { return flags.count(key); }
  
  string user;
  string cpu;
  string dump;
  string dir;
  string resultsFile = "results.txt";
  string masterDir;
  string uid;
  std::set<std::string> flags;
  
  int device = 0;
  
  bool timeKernels = false;
  bool enableTF = false;
  bool cudaYield = false;
  bool cleanup = false;
  bool noSpin = false;
  
  u32 proofPow = 0;
  
  int carry = CARRY_AUTO;
  u32 blockSize = 400;
  u32 logStep = 200000;
  int fftSize = 0;

  u64 B1 = 1000000;
  u64 B2 = 0;
  u32 B2_B1_ratio = 30;

  u32 prpExp = 0;
  u32 pm1Exp = 0;

  u32 maxBuffers = 0;
  size_t maxAlloc = 0;

  u32 iters = 0;

  void printHelp();
};
