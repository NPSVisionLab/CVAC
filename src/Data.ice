#ifndef _DATA_ICE
#define _DATA_ICE

module cvac {
  // should this be UNPURPOSED instead of UNLABELED?
  enum PurposeType {UNLABELED, POSITIVE, NEGATIVE, MULTICLASS, ANY};

  struct Purpose {
    PurposeType ptype = UNLABELED;
    int classID = 0; // if MULTICLASS
  };

  class PurposedList {
    Purpose pur;
  };

  class Labelable;

/* sequences default to arrays but List is more like c++'s std::vector */
  ["clr:generic:List"] sequence<Labelable> LabelableList;

  class PurposedLabelableSeq extends PurposedList {
    LabelableList labeledArtifacts;
  };

  struct DirectoryPath {
    string relativePath;  // without the filename
  };

  sequence<string> StringList;

  class PurposedDirectory extends PurposedList {
    DirectoryPath directory;
    StringList fileSuffixes;
    bool recursive;
  };

  /* sequences default to arrays but List is more like c++'s std::vector */
  ["clr:generic:List"] sequence<PurposedList> PurposedListSequence;

  /** RunSet is what is passed to trainers and detectors: one or more
   * purposed lists of labeled things
   */
  struct RunSet {
    PurposedListSequence purposedLists;
  };

  /** Where the image or video file is:
   *  This assumes that the client and service each have a "media root path"
   *  which gets prepended to the relativePath.
   */
  struct FilePath {
    DirectoryPath directory;
    string filename;      // just the filename, such as image.jpg
  };

  /** the image or video, formerly called ImageModel
   */
  struct Substrate {
    bool isImage = true;
    bool isVideo = false;

    FilePath path;
    int width = 0;
    int height = 0;
  };

  dictionary<string, string> LabelProperties;
  struct Semantics {
    string url;
  };

  struct Label {
    bool hasLabel;
    string name;
    LabelProperties properties;
    Semantics semantix;
  };

  /** Any image or video artifact that can have a label.
   * This is intended to be data that gets copied between client and server,
   * not interfaces with method calls.
   */
  class Labelable {
    float confidence;
    Label lab;
    Substrate sub;
  };

  struct Size {
    int width;
    int height;
  };


  /** A location in an image, such as a bounding box or just x/y coordinates;
   * assuming zero-based indexing into the image;
   * all numbers are in pixel units
   */
  class Location {
  };
  class Point2D extends Location {
    int x;
    int y;
  };
  sequence<Point2D> Point2DList;
  class BBox extends Location {
    int x;
    int y;
    int width;
    int height;
  };
  class PreciseLocation extends Location {
    float centerX;
    float centerY;
  };
  class PreciseBBox extends PreciseLocation {
    float width;
    float height;
  };
  class PreciseCircle extends PreciseLocation {
    float radius;
  };
  class Silhouette extends Location {
    Point2DList points;
  };
  /** if the entire image or video has a label
   */
  class LabeledFullSubstrate extends Labelable {
  };
  /** Data structure that combines a Location with a Label
   */
  class LabeledLocation extends Labelable {
    Location loc;
  };

  struct VideoSeekTime {
    long time;
    long framecnt;
  };
  struct FrameLocation {
    VideoSeekTime frame;
    Location loc;
  };

  /** Baseclass for video (temporal) annotations.  This permits labeling
   * a single frame (toFrame==NULL) and temporal segments of videos with
   * a single, constant label.
   */
  class LabeledVideoSegment extends Labelable {
    VideoSeekTime start;    // must be specified; if this segments gets faded in (or other soft transition),
                            // and last are expected to be all-inclusive (including the transition)
    VideoSeekTime last;     // ==NULL iff only one frame, otherwise inclusive to last frame of this segment
    VideoSeekTime startAfterTx;  // if smooth transition, when is the transition done and when does the segment really starts?
    VideoSeekTime lastBeforeTx;  // if smooth transition, what is the last non-transition frame of the segment
    Location loc;                  // ==NULL iff the entire spatial extent in the frames has a label,
                                   // otherwise the Location is constant from start->last frame .
  };

  sequence<FrameLocation> FrameLocationList;
  /** A Track:
   *  A sequence of Locations in a Video - note that all Locations have the same label.
   *  The annotations can be interpreted as DISCRETE annotations on the keyframes,
   *  as linearly interpolated in frames between the specified keyframes; or
   *  polynomially interpolated.  Interpolation only makes sense for some types of Locations.
   */
  enum Interpolation { DISCRETE, LINEAR, POLYNOMIAL };
  class LabeledTrack extends Labelable {
    FrameLocationList keyframesLocations;
    Interpolation interp = DISCRETE;
  };

  /** The original is the Labelable artifact that was to be tested, which
   * might contain a ground truth label or not.  It does always point to the
   * original image or video.
   * The foundLabels are all things that the detector found.
   */
  struct Result {
    Labelable original;
    LabelableList foundLabels;
  };

  /** One result for each Labelable in the RunSet, albeit, they might
   * be reported bit by bit.
   */
  sequence<Result> ResultList;
  struct ResultSetV2 {
    ResultList results;
  };

  /** This is a placeholder for the algorithm-internal data that
   *  turns a generic "object" detection algorithm into a detector for, say, faces, or cars, etc.
   */
  sequence<byte> ByteSeq;
};

#endif
