#include <iomanip>
#include <vector>
#include <util/FileUtils.h>
#include <opencv2/opencv.hpp>
//#pragma comment(lib,"opencv_core245.lib")
//#pragma comment(lib,"opencv_highgui245.lib")
//#pragma comment(lib,"opencv_imgproc245.lib")

namespace cvac
{  
  class MediaConverter
  {
  public:
    MediaConverter(){};
    ~MediaConverter(){};	
  public:
    virtual bool convert(const string& _srcAbsPath,
                         const string& _desAbsDir,
                         const string& _desFilename,
                         vector<string>& _resFilename) = 0;
  };


  class MediaConverter_openCV_i2i : public MediaConverter
  {
  public:
    MediaConverter_openCV_i2i(){};
    ~MediaConverter_openCV_i2i(){};
  public:
    bool convert(const string& _srcAbsPath,
                 const string& _desAbsDir,
                 const string& _desFilename,
                 vector<string>& _resFilename);
  };

  class MediaConverter_openCV_v2i : public MediaConverter
  {
  public:
    MediaConverter_openCV_v2i(int _perFrame = -1){  PerFrame = _perFrame;   };
    ~MediaConverter_openCV_v2i();

  private:
    cv::VideoCapture mVideoFile;	
    int PerFrame;

  public:
    bool convert(const string& _srcAbsPath,
                 const string& _desAbsDir,
                 const string& _desFilename,
                 vector<string>& _resFilename);
  };
}