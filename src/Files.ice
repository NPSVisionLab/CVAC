#if 0
#ifndef _FILES_ICE
#define _FILES_ICE

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

  /** FileService facilitates upload and download of files, particularly
    * media files such as images and videos.
    * It can also produce a snapshot of an image or video.
    * A FileService will grant read permissions to any file that is readable
    * by the FileService process.  It will grant write/delete permissions only
    * to files that have been created by the same client.
    */
  class FileService {
    /** 
      * copies a local file at the specified FilePath to the same location
      * on the remote file FileService.
      */
    void putFile( FilePath file );
    /** 
      * copies a remote file at the specified FilePath to the same location
      * on the local hard disk.
      */
    void getFile( FilePath file );
    /**
      * Creates a new file *_snap.jpg next to the original file.
      */
    FilePath getSnapshot( FilePath file );
    FileProperties getProperties( FilePath file );

    /** to obtain read/write permissions of a directory
      */
    FileProperties getProperties( DirectoryPath dir );

    /**
      * The actual byte copy functions.
      */
    void putBytes( Byte[] bytes, FilePath where );
    Byte[] getBytes( FilePath file );
  };

};

#endif  // _FILES_ICE
