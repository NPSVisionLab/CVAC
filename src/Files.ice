#ifndef _FILES_ICE
#define _FILES_ICE

/** File handling and remote sychronization for CVAC.
 * A remote site that provides CVAC services should run a FileService if
 * it is expected to process media that is not already stored at the remote
 * site, and/or if the clients are permitted to obtain the actual files on
 * which processing was performed.
 * The clients need to be configured to know about the FileServer location
 * (host, port) next to the CVAC service.  Utility functions exist that
 * implement the client functionality.
 *
 * A custom FileService is necessary as IcePatch2 does not meet our needs.
 * To explain the differences:
 * FileService can send and receive files, bidirectionally.
 * FileService provides some security and privacy to shield multiple
 *  clients from each other.
 * With FileService, clients can obtain media-related metadata without
 *  downloading the entire image or video.
 * FileService can create and transfer a thumbnail image.
 * Two IcePatch2 instances, one for each direction, could be used
 *  as the transport mechanism underneath FileService.
 */

module cvac {

  /**
   *  This assumes that the client and service each have a "media root path"
   *  which gets prepended to the relativePath.
   *  "up" paths and absolute paths are not permitted: 
   *  "../../" and "/somepath" are illegal.
   */
  struct DirectoryPath {
    string relativePath;  // without the filename
  };

  /** Where the image or video file is
   */
  struct FilePath {
    DirectoryPath directory;
    string filename;      // just the filename, such as image.jpg
  };

  /** Note: this might end up being a duplicate to what's defined in Data.ice
   */
  struct VideoSeekTime {
    long time;
    long framecnt;
  };

  /** Metadata about a media file: byte size, image size, video length etc.,
    * as well as some file attributes.
    */
  struct FileProperties {
    bool isImage;
    bool isVideo;
    long bytesize;
    int width;
    int height;
    VideoSeekTime videoLength;
    bool readPermitted;
    bool writePermitted;  // this includes file deletion
  };

  /** This is a placeholder for the algorithm-internal data that
   *  turns a generic "object" detection algorithm into a detector for, say, faces, or cars, etc.;
   *  We also use it to transfer files.
   */
  sequence<byte> ByteSeq;

  exception FileServiceException {
    string msg;
  };

  /** FileService facilitates upload and download of files, particularly
    * media files such as images and videos.
    * It can also produce a snapshot of an image or video.
    * A FileService will grant read permissions to any file that is readable
    * by the FileService process.  It will grant write/delete permissions only
    * to files that have been created by the same client.
    */
  class FileService {
    /**
      * True if the file exists on the FileServer.
      * The FileService might not permit clients to query for the existence of
      * arbitrary files, instead, it will grant permissions only to files that
      * were uploaded by the respective client.
      */
    bool exists( FilePath file ) throws FileServiceException;

    /** 
      * copies a local file at the specified FilePath to the same location
      * on the remote file FileService.
      */
    void putFile( FilePath file, ByteSeq bytes ) throws FileServiceException;

    /** 
      * copies a remote file at the specified FilePath to the same location
      * on the local hard disk.
      */
    ByteSeq getFile( FilePath file ) throws FileServiceException;

    /**
      * Do the obvious.  Not permitted unless this client put the file there earlier.
      */
    void deleteFile( FilePath file ) throws FileServiceException;

    /**
      * Creates a new file *_snap.jpg next to the original file.  The download
      * request needs to be made separately.
      */
    FilePath createSnapshot( FilePath file ) throws FileServiceException;

    /** Obtains read/write permissions of a file or directory.
      * To obtain the permissions of a directory, use an empty
      * filename component or a dot "."
      */
    FileProperties getProperties( FilePath file ) throws FileServiceException;
  };

};

#endif  // _FILES_ICE
