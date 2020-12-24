#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_FAST_COMPILE
#include <catch2/catch.hpp>

/* There's no main.cpp in the test suite and the following global var is 
unfortunately defined there. Let's fake it. */

class gdMainWindow* G_MainWin;