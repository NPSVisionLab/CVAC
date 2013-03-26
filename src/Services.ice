#ifndef _SERVICES_ICE
#define _SERVICES_ICE

#include <Ice/Identity.ice>
#include "Data.ice"

module cvac {


  /** The client needs to implement these callback interfaces
   */
  interface CallbackHandler {
    /** called soon after starting processing;
     * returns -1 if runtime cannot be estimated
     */
    void estimatedTotalRuntime( int seconds );

    /** this method may or may not be called
     */
    void estimatedRuntimeLeft( int seconds );

    /** mainly for debugging, string messages can be sent to the client;
     * levels are:
     * 1: fatal error
     * 2: warning
     * 3: information
     * 4: debug
     * 5, 6, 7: incrementally more verbose debugging messages
     */
    void message( int level, string messageString );
  };



  interface DetectorCallbackHandler extends CallbackHandler {
    /** all results are returned through this callback,
     * even complete results
     */
    void foundNewResults( ResultSetV2 results );
  };

 /**
   * The data required along with a detector algorithm that is
   * required to create a functioning detector.  This data is
   * detector algorithm specific.  The provider architecture
   * permits a trainer to just return a reference without 
   * transmitting large or sensitive data over the network.
   */
  enum DetectorDataType { BYTES, FILE, PROVIDER };
  interface DetectorDataProvider;
  struct DetectorData {
    DetectorDataType type = BYTES;
    ByteSeq data;
    FilePath file;
    DetectorDataProvider provider;
  };
  interface DetectorDataProvider {
    DetectorData getDetectorData();
  };

  /** A service that runs a CVAlgorithm
   */
  interface CVAlgorithmService {
    bool isInitialized();
    void destroy();

    string getName();
    string getDescription();
    void setVerbosity( int verbosity );
  };


  interface DetectorProperties {
    bool isSlidingWindow();
    /** Returns the size of the "native" detector, that is,
     *  the size of objects that can be detected without scaling
     */
    Size getNativeWindowSize();
    /** stepX and stepY are respective to the nativeWindowSize and scaled along with the window
     */
    void getSlidingParameters( out Size startSize, out Size stopSize, out float scaleFactor,
                   out float stepX, out float stepY );
    /** To use default values for one parameter, use the value of -1;
     *  stopSize is inclusive.  If the specified sizes do not have the same aspect ratio
     *  as the nativeWindowSize, the maximum range will be assumed.
     *  stepX and stepY are respective to the nativeWindowSize and scaled along with the window.
     */
    void setSlidingParameters( Size startSize, Size stopSize, float scaleFactor,
                   float stepX, float stepY );

    bool canSetSensitivity();
    void getSensitivity( out double falseAlarmRate, out double recall );
    /** To use current values for one parameter, use the value of -1
     */
    void setSensitivity( double falseAlarmRate, double recall );

    bool canPostProcessNeighbors();
    int getMinNeighbors();
    /** -1 for default
     */
    void setMinNeighbors( int num );
  };

  /**
   * Interface to configure trainer properties for building the next
   * detector.
   */
  interface TrainerProperties {
    /**
     * Set the training window size
     */
    void setWindowSize(Size wsize);
    bool canSetWindowSize();
    Size getWindowSize();

    /**
     *  Set the sensitivity for the detector to be trained. Pass -1
     *  if you want to hold that parameter to the current value.
     */
    void setSensitivity(double falseAlarmRate, double recall);
    bool canSetSensitivity();
    void getSensitivity(out double falseAlarmRate, out double recall);
  };

  /** An object detection service
   */
  interface Detector extends CVAlgorithmService {
    /** To create a Detector from an algorithm, it needs to be
     *  parameterized for the actual object to detect.
     * @param verbosity The level of output verbosity (Detector specific)
     * @param data Optional parameter to supply this Detector with DetectorData to use. Some
     *   Detectors have build-in DetectorData and won't need this (could be NULL)
     */
    void initialize(int verbosity, DetectorData data);

    /** Get a copy of the DetectorData used by this Detector. The
     *  returned DetectorData could be saved
     *  locally, or modified and passed back to the Detector via initialize()
     *  @param how requests a certain type: ByteSeq, file, or provider via proxy.  
     *   This might or might not be heeded.
     *  @see initialize()
     */
    //idempotent DetectorData createCopyOfDetectorData( DetectorDataType how );


    /** This call will block until after completedProcessing was called on
     *  the CallbackHandler (which needs to be of type DetectorCallbackHandler)
     */
    void process( Ice::Identity client, RunSet run );

    DetectorProperties* getDetectorProperties();
  };

  interface TrainerCallbackHandler extends CallbackHandler {
    /** This is the main output of training.
     */
    void createdDetector( DetectorData detData );
  };

  /** A trainer service that produces an object detection service
   */
  interface DetectorTrainer extends CVAlgorithmService {
    void initialize( int verbosity );
    /** This call will block until after completedProcessing was called on
     *  the CallbackHandler (which needs to be of type TrainerCallbackHandler)
     */
    void process( Ice::Identity client, RunSet run );
    TrainerProperties *getTrainerProperties();
  };

};

#endif
