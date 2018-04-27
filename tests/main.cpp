#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_FAST_COMPILE
#include <catch.hpp>

/* There's no main.cpp in the test suite and the following global vars are 
unfortunately defined there. Let's fake them. */

class gdMainWindow* G_MainWin;
bool G_quit;