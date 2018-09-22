// GpuOwl Mersenne primality tester; Copyright (C) 2017-2018 Mihai Preda.

#pragma once

#include "common.h"

#include <vector>
#include <string>

class Kset {
  vector<u32> Ks;
  u32 B1;
  vector<u32>::iterator hint;
  
public:
  Kset(const string &name);

  u32 getB1() { return B1; }
  
  u32 getFirstAfter(u32 k);
};