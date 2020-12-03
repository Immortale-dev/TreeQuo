#include <iostream>
#include <thread>
#include <queue>


#define DEBUG
//#define TEST_ONLY_RULE

#include "qtest.hpp"

#include "forest.hpp"

using namespace std;
#include "src/helper.cpp"

SCENARIO_START
#include "src/single_thread.forest.test.cpp"
#include "src/multi_thread.forest.test.cpp"
#include "src/performance.forest.test.cpp"
SCENARIO_END


int main(){ return 0; }
