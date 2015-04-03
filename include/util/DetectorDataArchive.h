// This should be in a header eventually for the lib wrapper
/****
 *CVAC Software Disclaimer
 *
 *This software was developed at the Naval Postgraduate School, Monterey, CA,
 *by employees of the Federal Government in the course of their official duties.
 *Pursuant to title 17 Section 105 of the United States Code this software
 *is not subject to copyright protection and is in the public domain. It is 
 *an experimental system.  The Naval Postgraduate School assumes no
 *responsibility whatsoever for its use by other parties, and makes
 *no guarantees, expressed or implied, about its quality, reliability, 
 *or any other characteristic.
 *We would appreciate acknowledgement and a brief notification if the software
 *is used.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above notice,
 *      this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above notice,
 *      this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the Naval Postgraduate School, nor the name of
 *      the U.S. Government, nor the names of its contributors may be used
 *      to endorse or promote products derived from this software without
 *      specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY THE NAVAL POSTGRADUATE SCHOOL (NPS) AND CONTRIBUTORS
 *"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL NPS OR THE U.S. BE LIABLE FOR ANY
 *DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****/
#ifndef __DETECTORDATAARCHIVE_H_INCLUDED__
#define __DETECTORDATAARCHIVE_H_INCLUDED__

#include <string>
#include <vector>


/**
 *  Common file identifiers. 
 */
#define XMLID "XMLFILE"
#define RESID "RESFILE"
#define VOCID "VOCFILE"
#define SVMID "SVMFILE"
#define MODELID "MODELFILE"
#define WEIGHTID "WEIGHTFILE"
#define LOOKUPID "LOOKUPFILE"
#define MEANID "MEANFILE"


namespace cvac
{
    class DetectorDataArchive
    {
    public:
        /**
         * Construct a DetectorDataArchive class
         */
        DetectorDataArchive();

        /**
         * Archive the detector data using relDirectory.  This throws an exception
         * if the archive cannot be created.
         */
        void createArchive(const std::string &relDirectory);

        /**
         * Unarchive the passed in archive file into dir.   This throws an
         * exception of the file cannot be found or compressed.
         * If any trainer.property file existed in the archive then 
         * this DetectorDataArchive's properties are set
         * to the contents of that file.
         * @param archiveFile the file to unarchive.
         * @param dir Write all unarchived files into this directory.
         */
        void unarchive(const std::string &archiveFile, const std::string &dir);

        /**
         * Save a property which is a name, value pair.  When
         * the trainer data is archived these attributes are written out to
         * the archive.
         */
        void setProperty(const std::string &name,
                                  const std::string &value);   
        /**
         * Get the list of properties currently defined.
         * These can be set from an archive file or from the user or both.
         */
        std::vector<std::string> getProperties();

        /**
         * Get a value of a trainer property value defined by name.
         */
        std::string getProperty(const std::string &name) const;

        /**
         * Set the archive filename.  This should be the complete path
         * and filename.
         */
        void setArchiveFilename(std::string &filename);

        /**
         * Add a file to the archive. A file has two parts.  An identifier 
         * string that tells what this file is and the name of the file
         * itself.  This assumes that it might not be possible to 
         * determine the purpose of the file just by given its name.
         * @param identifier - the identifier string for the file
         * @param filename - the file name.
         * @return - returns true if the file exists and no other
         * file in the archive has this identifier else it returns false.
         */
        bool addFile(const std::string &identifier, 
                     const std::string &filename);

        /**
         * Remove the file that maps to this identifier from the archive
         * @param identifier - the file identifier  and file to remove
         * @return - returns true if the identifier was removed false
         * if the identifier was not in the archive.
         */
        bool removeFile(const std::string &identifier);

        /**
         * Get the list of file identifiers in a archive.
         * @return a vector of file identifiers contained in the archive.
         */
        const std::vector<std::string> getFileIds();

        /**
         * Get a file name in the archive based on its identifier
         * @param identifier - the identifier string for the file
         * @return - the filename in the archive that matches the identifier.
         * The  return string is empty if there is no match.
         */
        const std::string getFile(const std::string &identifier) const;


    private:
        
       
        std::string mArchiveName;
        std::vector<std::string> mPropNames;
        std::vector<std::string> mPropValues;
        std::vector<std::string> mFileNames;
        std::vector<std::string> mFileIds;

        
    };
}

    void expandSeq_fromFile(
              const std::string& filename, const std::string& expandSubfolder);
    void expandSeq_fromFile(const std::string& filename);
    bool writeZipArchive(const std::string& _outpath,
              const std::vector<std::string>& _inPaths);

#endif // __DETECTORDATAARCHIVE_H_INCLUDED__
