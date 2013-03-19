/*
 */
package wirediagram;

import com.googlecode.javacpp.FloatPointer;
import com.googlecode.javacv.cpp.opencv_objdetect.HOGDescriptor;
import static com.googlecode.javacv.cpp.opencv_core.*;
import static com.googlecode.javacv.cpp.opencv_imgproc.*;
import static com.googlecode.javacv.cpp.opencv_highgui.*;
import com.googlecode.javacv.cpp.opencv_ml.CvSVM;
import com.googlecode.javacv.cpp.opencv_ml.CvSVMParams;
import cvac.Labelable;
import cvac.PurposeType;
import cvac.ResultRect;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.io.File;

/**
 * ICE-wrapped Wire Diagram detector.
 *
 * @author matz
 */
public class WireDiagram {
  
  public static final Logger logger = Logger.getLogger(WireDiagram.class.getName());
  private static WireDiagram s_wd;
  private static final int NUM_FEATURES = 147;
  String mediaRootPath = null;

  static WireDiagram getRef() {
    if (null==s_wd)
    {
      s_wd = new WireDiagram();
    }
// ?? why was this here??    s_wd.train();
    return s_wd;
  }
  
  
  protected CvSVM svm = null;
  private List<TrainingSample> training_samples;
  
  public WireDiagram()
  {
    svm = new CvSVM();
    training_samples = new ArrayList<TrainingSample>();
  }
  
  void setMediaRootPath( String path )
  {
    mediaRootPath = path;
  }
  String getMediaRootPath()
  {
    if (null==mediaRootPath)
      throw new RuntimeException("mediaRootPath not set in WireDiagram");
    return mediaRootPath;
  }  

  protected CvMat calcFeatures( IplImage image )
  {
    // each trainData matrix row is a sample, hence need a 1xn feature vector
    CvMat features = cvCreateMat( 1, NUM_FEATURES, CV_32FC1 );

    
    IplImage gray = cvCreateImage( image.cvSize(), IPL_DEPTH_8U, 1 );
    cvCvtColor( image, gray, CV_BGR2GRAY );
    int featcnt = 0;
    {
      // determine contrast features:
      // overall contrast in the image, mean local contrast, their ratio
      IplImage ctrst = cvCloneImage( gray );
      cvSmooth(ctrst, ctrst, CV_GAUSSIAN, 9);
      cvSub( gray, ctrst, ctrst, null );
      double global_contrast = calcContrast( ctrst );
      logger.log(Level.FINER, "global contrast: {0}", global_contrast);
      features.put( featcnt++, global_contrast );
      final int num_rows = 10, num_cols = 10;
      final int roicols = ctrst.width()/num_cols;
      final int roirows = ctrst.height()/num_rows;
      CvMat local_contrasts = cvCreateMat( num_rows*num_cols, 1, CV_32FC1 );
      int roicnt = 0;
      for (int row=0; row<num_rows; row++)
      {
        for (int col=0; col<num_cols; col++)
        {
          CvRect roi_rect = cvRect( col*roicols, row*roirows, roicols, roirows );
          cvSetImageROI( ctrst, roi_rect );
          double contrast = calcContrast( ctrst );
          local_contrasts.put( roicnt++, contrast );
        }
      }
      cvResetImageROI( ctrst );
      CvScalar mean = cvScalarAll(0);
      CvScalar stddev = cvScalarAll(0);
      cvAvgSdv( local_contrasts, mean, stddev, null );
      logger.log(Level.FINER, "local contrast: ({0}, {1})", 
              new Object[]{mean.val(0), stddev.val(0)});
      features.put( featcnt++, mean.val(0) );
      features.put( featcnt++, stddev.val(0) );
    }
    
    // determine line and edge features:
    // how many rather thin edges, lines, curves;
    // see if HoG is bimodal
    {
      CvSize winSize = new CvSize(24, 24);    // must be multiple of cellSize
      CvSize blockSize = new CvSize(16, 16);  // must be multiple of cellSize
      CvSize blockStride = new CvSize(8, 8);
      CvSize cellSize = new CvSize(8, 8);
      int nbins = 9;
      int derivAperture = 1;
      double winSigma = -1;
      int histogramNormType = HOGDescriptor.L2Hys;
      double l2HysThreshold = 0.2;
      boolean gammaCorrection = false;
      int nlevels = HOGDescriptor.DEFAULT_NLEVELS;
      
//      HOGDescriptor hog = new HOGDescriptor();
      HOGDescriptor hog = new HOGDescriptor( winSize, blockSize, blockStride, cellSize, 
              nbins, derivAperture, winSigma, histogramNormType, l2HysThreshold, gammaCorrection, nlevels );
      
      //IplImage gradient = cvCloneImage( gray );
      //cvSobel( gray, gradient, 1, 0, 1 );
      FloatPointer descriptors = new FloatPointer();
      CvSize padding = cellSize; // pad a little
      CvPoint locations = new CvPoint(0, 0);
           
      hog.compute( gray, descriptors, blockStride, padding, locations );
      
      FloatBuffer fb = descriptors.asBuffer();
      int num_descs = 0;
      double sum = 0;
      while (fb.hasRemaining())
      {
        double val = fb.get();
        features.put( featcnt++, val );
        sum += val;
        //System.out.println("fb: " + num_descs + ", " + val );
        num_descs++;
      }
      logger.log(Level.FINER, "HOG computed {0} descriptors, average {1}", 
              new Object[]{num_descs, sum/(double)num_descs});
//      features.put( featcnt++, sum/(double)num_descs );
    }
    
    // color segmentation:
    // cluster with k-means, calc between-to-within distance ratio
    
    assert( NUM_FEATURES==featcnt );
    
    return features;
  }
  
  public String getFilename( Labelable sample )
  {
    return mediaRootPath + File.separatorChar
            + sample.sub.path.directory.relativePath + File.separatorChar
            + sample.sub.path.filename + "\n";
  }
  
  public ResultRect[] detect( String filename ) 
  {
//    String filename = getFilename( sample );
    ResultRect reg[] = null;
    //int windowSize = getWindowSize();
    IplImage image = cvLoadImage( filename );
    if (image != null) {
      boolean detected = detect(image);
      if (detected)
      {
        reg = new ResultRect[1];
        reg[0] = new ResultRect(0, 0, image.width(), image.height(), 0);
      }
      cvReleaseImage(image);
    }
    return reg;
  }
  
  boolean detect(IplImage image) 
  {  
    CvMat features = calcFeatures( image );
    // classify
    double result = svm.predict( features, true );
    logger.log(Level.FINEST, "SVM result: {0}", result);
    
    return result<0.0;
  }

  /** this trains this detector, then clears out all the previously added
   * training samples
   */
  public void train()
  {
    if (training_samples.isEmpty())
    {
      logger.log(Level.WARNING, "no training samples to train with");
      return;
    }
    
    int num_samples = training_samples.size();
    // Only the CV_ROW_SAMPLE data layout is supported:
    // each trainData matrix row is a sample
    CvMat trainData = cvCreateMat( num_samples, NUM_FEATURES, CV_32FC1 );
    CvMat responses = cvCreateMat( num_samples, 1, CV_32FC1 );
    int samplenum = 0;
    for (TrainingSample ts : training_samples)
    {
      IplImage image = cvLoadImage( ts.path );
      if (null==image)
      {
        logger.log(Level.WARNING, "cvLoadImage could not open input image {0}", ts.path);
        File file = new File( ts.path );
        if (file.exists()) System.out.println("file exists");
        else System.out.println("File doesn't exist");
        if (file.canRead()) System.out.println("file can be read");
        else System.out.println("File can't be read");
        
        continue;
      }
      logger.log(Level.INFO, "loaded input image {0}", ts.path);
      CvMat features = calcFeatures( image );
      cvReleaseImage(image);
      for (int k=0; k<NUM_FEATURES; k++)
      {
        trainData.put(samplenum, k, features.get(k));
      }
      switch (ts.purpose) {
        case POSITIVE: {
          responses.put(samplenum, 1.0);
          break;
        } 
        case NEGATIVE: {
          responses.put(samplenum, -1.0);
          break;
        } 
        default:
        {
          logger.log(Level.WARNING, "not training with sample '{0}' because Purpose is invalid",
                  ts.path);
          continue;
        }
      }
      samplenum++;
    }
    if (samplenum!=num_samples)
    {
      logger.log(Level.SEVERE, "error loading samples (see earlier warnings), cannot train");
      return;
    }

    CvSVMParams params = new CvSVMParams();
    params.kernel_type( CvSVM.LINEAR );
    params.svm_type( CvSVM.C_SVC );
    params.C( 1.0 );
    params.term_crit( cvTermCriteria(CV_TERMCRIT_ITER,100,0.000001) );
    logger.log(Level.FINER, "WireDiagram training SVM");
    svm.train( trainData, responses, null, null, params );
    logger.log(Level.INFO, "WireDiagram training done.");
  }
  

  private double calcContrast(IplImage image) 
  {
    CvScalar sum = cvSum(image);
    double contrast = sum.val(0)/(image.width()*image.height());
    return contrast;
  }
  
  class TrainingSample
  {
    String path;
    PurposeType purpose;

    private TrainingSample(String _path, PurposeType _purposeType) {
      path = _path;
      purpose = _purposeType;
    }
  }

  public void addToTraining(String path, PurposeType purpose) 
  {
    training_samples.add(new TrainingSample(path, purpose));
  }

  public static void main( String[] args )
  {
    WireDiagram detector = new WireDiagram();
    
    // train with two simple images
    String neg1 = "/Users/matz/Analyst_Media/WiringDiagrams/neg/horseshoebend.jpg";
    String pos1 = "/Users/matz/Analyst_Media/WiringDiagrams/pos/improved-circuit-sketch2.jpg";
    detector.addToTraining(neg1, PurposeType.NEGATIVE);
    detector.addToTraining(pos1, PurposeType.POSITIVE);
    detector.train();
    
    // now test with the same images
    {
      IplImage image = cvLoadImage( neg1 );
      if (null==image)
      {
        System.out.println("could not open input image " + neg1 );
        return;
      }
      System.out.println("loaded input image " + neg1 );
      detector.detect(image);
      cvReleaseImage(image);
    }

    {
      IplImage image = cvLoadImage( pos1 );
      if (null==image)
      {
        System.out.println("could not open input image " + pos1 );
        return;
      }
      System.out.println("loaded input image " + pos1 );
      detector.detect(image);
      cvReleaseImage(image);
    }
  }
}
