Depending Library:
opencv_core242.lib
opencv_highgui242.lib
opencv_imgproc242.lib

Example of how to use this library

	VideoValidatorForOpenCV mValidation;
	
	std::string _returnMsg;
	std::string _path = "F:/CVAC/data";
	std::string _fileName = "NASAKSN-NorthernLights.mpg";
	bool _flagSuccess = mValidation.runTest(_path,_fileName,_returnMsg);
	
	//_flagSucess
	// true = it means that a system may work for video-processing as desired .
	// false = it means that the system man NOT work for video-processing as desired. 
	
	//_returnMsg: this string contains details about the success or the failure. 