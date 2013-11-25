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

    /** Called if the client requested cancellation of the
     *  process function.
     */
    void cancelled();
  };

  interface DetectorCallbackHandler extends CallbackHandler {
    /** all results are returned through this callback,
     * even complete results
     */
    void foundNewResults( ResultSet results );
  };

  interface TrainerCallbackHandler extends CallbackHandler {
    /** This is the main output of training.
     */
    void createdDetector( FilePath detectorData );
  };

  /** A service that runs a CVAlgorithm
   */
  interface CVAlgorithmService {
    idempotent string getName();
    idempotent string getDescription();
  };

  dictionary<string, string> Properties;
  
  /** Structure to obtain and set properties of detector services
   */
  struct DetectorProperties {
    /** The level of output verbosity from server to client; use -1 for default.
    */
    int verbosity;
    
    bool isSlidingWindow;
    /** Returns the size of the "native" detector, that is,
     *  the size of objects that can be detected without scaling
     */
    Size nativeWindowSize;
    /** stepX and stepY are respective to the nativeWindowSize and scaled along with the window
     *  To use default values for one parameter, use the value of -1;
     *  stopSize is inclusive.  If the specified sizes do not have the same aspect ratio
     *  as the nativeWindowSize, the maximum range will be assumed.
     *  stepX and stepY are respective to the nativeWindowSize and scaled along with the window.
     */
    Size slideStartSize;
    Size slideStopSize;
    float slideScaleFactor;
    float slideStepX;
    float slideStepY;

    bool canSetSensitivity;
    /** To use current values for one parameter, use the value of -1
     */
    double falseAlarmRate;
    double recall;

    bool canPostProcessNeighbors;
    /** -1 for default
     */
    int minNeighbors;

    /** If applicable, process frames in a video at the specified number of
     *  frames per second.  This fps is a suggestion to the algorithm which might
     *  or might not observe it.  It is also understood as an approximate value since
     *  convertion is probably not feasible for any number of fps.  Special values:
     *  -1: use the specific trainer/detector's default setting
     *  -2: process at the native video frame rate
     *  -3.n: process every n-th frame in the video
     */
    double videoFPS;
    
    /** Props are name-value pairs that can
    *  specify detector-specific properties, if any.
    */
    Properties props;
  };

  /**
   * Data structure to configure trainer properties for building the next
   * detector.
   */
  struct TrainerProperties {
    /** The level of output verbosity from server to client; use -1 for default.
    */
    int verbosity;

    /**
     * Set the training window size
     */
    bool canSetWindowSize;
    Size windowSize;

    bool canSetSensitivity;
    /**
     *  Set the sensitivity for the detector to be trained. Pass -1
     *  if you want to hold that parameter to the current value.
     */
    double falseAlarmRate;
    double recall;

    /** If applicable, process frames in a video at the specified number of
     *  frames per second.  This fps is a suggestion to the algorithm which might
     *  or might not observe it.  It is also understood as an approximate value since
     *  convertion is probably not feasible for any number of fps.  Special values:
     *  -1: use the specific trainer/detector's default setting
     *  -2: process at the native video frame rate
     *  -3.n: process every n-th frame in the video
     */
    double videoFPS;
    
    /** Props are name-value pairs that can
    *  specify detector-specific properties, if any.
    */
    Properties props;
  };

  /** An object detection service
   */
  interface Detector extends CVAlgorithmService {
    /** 
     * @param run  The images, videos to detect objects in.
     * @param detectorData Optional parameter to supply this Detector
     *   with DetectorData to use. Some
     *   Detectors have build-in DetectorData and won't need this (could be NULL), others
     *   need to be parameterized for the actual object to detect.
     * @param props Optionally configure the detector.
     *
     * This call will block until after completedProcessing was called on
     *  the CallbackHandler (which needs to be of type DetectorCallbackHandler)
     */
    void process( Ice::Identity client, RunSet run, FilePath detectorData, DetectorProperties props );

    /** Try to cancel the process that the specified client initialized.
     *  @param client The DetectorCallbackHandler that was passed to the process function.
     *  @returns true if the service will try to abort the process, false if it is unable to.
     */
    bool cancel( Ice::Identity client );

    /** Return the default properties of the service.  Every service should be
        stateless, that is, present a clean slate to every connecting client.
    */
    DetectorProperties getDetectorProperties();
  };

  /** A trainer service that produces an object detection service
   */
  interface DetectorTrainer extends CVAlgorithmService {
    /** This call will block until after completedProcessing was called on
     *  the CallbackHandler (which needs to be of type TrainerCallbackHandler)
     * @param run  The images, videos, and their metadata from which to train a detector.
     * @param props Optionally configure the detector.
     */
    void process( Ice::Identity client, RunSet run, TrainerProperties props );

    /** Try to cancel the process that the specified client initialized.
     *  @param client The TrainerCallbackHandler that was passed to the process function.
     *  @returns true if the service will try to abort the process, false if it is unable to.
     */
    bool cancel( Ice::Identity client );

    /** Return the default properties of the service.  Every service should be
        stateless, that is, present a clean slate to every connecting client.
    */
    TrainerProperties getTrainerProperties();
  };

};

#endif
