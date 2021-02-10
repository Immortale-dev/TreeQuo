//#define DEBUG
//#define TEST_ONLY_RULE

#include "qtest.hpp"
using namespace std;
#include "forest.hpp"
#include "src/helper.cpp"

SCENARIO_START

#include "src/performance.forest.test.cpp"

SCENARIO_END

int main(){ return 0; };
