#ifndef _FILESERVER_ICE
#define _FILESERVER_ICE

#include <Ice/Identity.ice>
#include "Data.Ice"

module cvac {

    sequence<FilePath> FileList;
    sequence<DirectoryPath> DirectoryList;

    /**
     * Information about a FileRequest is returned.
     * This called from the File Server to the client
     */
    interface FileRequestCallback {
        /**
         * Here are file bytes for the given file starting at offset.
         */
        void fileBytes(FilePath file, int offset, ByteSeq bytes);
        /**
         * File copy is complete.
         */
        void fileComplete(FilePath file);
        void infoComplete(DirectoryPath dir, DirectoryList dirs, 
                          FileList files);
        /**
         * Error occured while for this file request. 
         */
        void errorFile(FilePath file, string errorString);
        void errorInfo(DirectoryPath dir, string errorString);

    };


    /**
     *  Transfer either a file or the contents of a directory
     */
    interface FileRequest {
        void getFile(FileRequestCallback callback, FilePath remoteFile);    
        void getInfo(FileRequestCalllback callback, DirectoryPath dir);
    };

};

#endif
