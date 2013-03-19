Depending Library:
opencv_core242.lib
opencv_highgui242.lib
opencv_imgproc242.lib
opencv_ml242.lib

Example of how to use this library

	//////////////////////////////////////////////////////////////////////////
	//	For ICE use	
	//////////////////////////////////////////////////////////////////////////

	/*
	////	Train (Server)
	SBDCV mSBD;
	std::vector<long> _vFrameListInput;
	if(ice_parseFrameList("F:/xxx","NASAConnect-HiddenTreasures.txt",_vFrameListInput))
	{
		mSBD.train_extractFeature("F:/xxx","NASAConnect-HiddenTreasures.mpg",_vFrameListInput,true);			
		//the last parameter: when this function is called at the first time, the last parameter should be "true" to initialize pre-stored data.
	}
	mSBD.train_saveFeature("F:/xxx","NASAConnect-HiddenTreasures_feature.txt");	//It is not mandatory.
	mSBD.train_trainSVM();
	mSBD.train_saveSVM("F:/xxx","NASAConnect-HiddenTreasures_svm.xml.gz");	//It is not mandatory. 
	//*/

	/*
	//	Test (Server)
	SBDCV mSBD;
 	std::vector<long> _vFrameListOutput;
	std::string _resultFileName;
 	mSBD.test_loadSVM("F:/xxx","NASAConnect-HiddenTreasures_svm.xml.gz");
  	mSBD.test_run("F:/xxx/anni005.mpg",_vFrameListOutput,true,_resultFileName,true);	
	//the third parameter: whether to save framelist as a txt file
	//the fifth parameter: whether to save transition images
	//*/

	
	//////////////////////////////////////////////////////////////////////////
	//	For Not-ICE use	
	//////////////////////////////////////////////////////////////////////////

	/*
	//	Batch Train and Test 
	SBDCV mSBD;
	std::string _comPath = "F:/xx";
	std::vector<std::string> nameVideoSet;
	nameVideoSet.push_back("Adelante1959_2.mpeg");	//0
	nameVideoSet.push_back("DesertVe1958.mpeg");
	nameVideoSet.push_back("HowaWatc1949.mpeg");
	nameVideoSet.push_back("LivingSt1958.mpeg");
	nameVideoSet.push_back("Sagaofth1957.mpeg");
	nameVideoSet.push_back("TexasFar1952.mpeg");	//5
	nameVideoSet.push_back("Wonderfu1958.mpeg");
	nameVideoSet.push_back("anni005.mpg");
	nameVideoSet.push_back("NASAConnect-HiddenTreasures.mpg");
	nameVideoSet.push_back("NASASF-TheTechnicalKnockout.mpg");
	nameVideoSet.push_back("UGS01.mpg");	//10
	nameVideoSet.push_back("UGS05.mpg");
	nameVideoSet.push_back("UGS09.mpg");
	nameVideoSet.push_back("BOR03.mpg");

	bool _iniFeature;
	int _size = nameVideoSet.size();
	for(int k=0;k<_size;k++)
	{	
		//////////////////////////////////////////////////////////////////////////
		// WAY #1
		_iniFeature = true;
		for(int r=0;r<_size;r++)
		{
			if(r==k)
				continue;
			else
			{				
				std::string _featureFileName = nameVideoSet[r].substr(0,nameVideoSet[r].rfind(".")) + "_feature.txt";				
				if(_iniFeature)
				{
					mSBD.loadFeature(_comPath,_featureFileName,true);
					_iniFeature = false;
				}
				else
					mSBD.loadFeature(_comPath,_featureFileName,false);				
			}
		}		
		std::string _testFileName = nameVideoSet[k].substr(0,nameVideoSet[k].rfind("."));
		mSBD.train_trainSVM();
		mSBD.train_saveSVM(_comPath,_testFileName+"_svm_Result.xml.gz");
		//////////////////////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////////////////
		// WAY #2
		//	std::string _testFileName = nameVideoSet[k].substr(0,nameVideoSet[k].rfind("."));
		//	mSBD.test_loadSVM(_comPath,_testFileName+"_svm_Result.xml.gz");
		//////////////////////////////////////////////////////////////////////////
		std::vector<long> _vFrameListOutput;
		std::string _resultFileName;
		//mSBD.test_run(_comPath+"/"+nameVideoSet[k],_vFrameListOutput,true,_resultFileName,false);	//the last parameter: whether to save transition images
		mSBD.runTestFromFeature(_comPath,nameVideoSet[k],true);
	}
	//*/
	
	/*
	//	Train Method - #1 (From a list file)
	SBDCV mSBD;
	mSBD.setFeatureFromTrainList("F:/xxx","trainList.txt",true);	//the last parameter: whether to initialize or not pre-stored feature data 
	mSBD.train_saveFeature("F:/xxx","Features.txt");		//It is not mandatory.
	mSBD.train_trainSVM();
	mSBD.train_saveSVM("F:/xxx","svm_Result.xml.gz");	//It is not mandatory. 
	//*/

	/*
	//	Test Method	- #1 
	SBDCV mSBD;
	mSBD.test_loadSVM("F:/xxx","svm_Result.xml.gz");
	std::vector<long> _vFrameListOutput;
	std::string _resultFileName;
	mSBD.test_run("F:/xxx/anni005.mpg",_vFrameListOutput,true,_resultFileName,false);	
	//the third parameter: whether to save transition frames into a text file
	//the fifth parameter: whether to save transition images 
	//*/

