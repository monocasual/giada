#include "../src/utils/utils.h"
#include "catch.hpp"


TEST_CASE("Test gVector template") 
{
  gVector<int> v;
  v.add(0);
  v.add(1);
  v.add(2);
  REQUIRE(v.size == 3);
  
  v.del(0);
  REQUIRE(v.size == 2);
  
  v.swap(0, 1);
  REQUIRE(v.at(0) == 2);
  
  REQUIRE(v.last() == 1);
  
  v.clear();
  REQUIRE(v.size == 0);
  
  /* test copy constructor */
  
  gVector<int> v1;
  v1.add(0);
  v1.add(1);
  v1.add(2);
  gVector<int> v2;
  v2 = v1;
  REQUIRE(v2.size == 3);
  REQUIRE(v2.at(2) == 2); 
}

