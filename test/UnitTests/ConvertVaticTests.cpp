#include <UnitTest++.h>
#include <Data.h>  // Ice classes
#include <convert/convertVatic.h>
#include <fstream>
#include <iostream>

using namespace cvac;
using namespace UnitTest;
using namespace std;
/////////////////////////

SUITE(ConvertVaticTests)
{
  TEST(loadAnnotation)
  {
     cout << "## Convert Vatic Test Suite ##" << endl;
	 convert("test");

	 //0 21 234 46 306 0 1 0 0 "person"
  }
}