#include <UnitTest++.h>
#include <Data.h>  // Ice classes
#include <convert/convertVatic.h>
#include <util/FileUtils.h>
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
	 
	 std::string CWD = std::string(getCurrentWorkingDirectory().c_str());
	 //cout << "Cwd: " << CWD.c_str() << endl;
	 LabelableList* videoAnn = convert("../data/annotation/vaticVideo.txt");

	 //Labelable firstLine = (videoAnn[0]); // how to reference
	 //assert(firstLine.sub.isVideo);

	 // Verify 1st line values:
	 //0 21 234 46 306 0 1 0 0 "person"
  }
}