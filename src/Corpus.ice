#ifndef _CORPUS_ICE
#define _CORPUS_ICE

#include "Data.ice"

module cvac {


  /** A Corpus, which describes the metadata for a collection of
    * images or videos.
    * This object mainly acts as an ID to perform tasks on the corpus, but
    * it includes a few more data members for convenience.  The data members
    * must not be modified by any client, only by CorpusService.
    * The actual Labelable artifacts in the corpus are accessed via the
    * CorpusService.
    */
  class Corpus {
    string name;
    string description;
    string homepageURL  ;
    bool   isImmutableMirror;
  };


  /** CorpusCallback informs the caller of CorpusService's createLocalMirror
   *  function about the progress of downloading and extracting the dataset.
   */
  interface CorpusCallback {
    /** The CorpusService might or might not call this function to provide
      * updates about the mirror creation progress.
      * @param corpus Which Corpus this progress report is for.
      * @param numtasks How many tasks there are in total, e.g. download
      *               and extract would be numtasks==2.
      * @param currtask What the number of the current task is, e.g. download==1.
      * @param taskname For example, "downloading".
      * @param details  For example, "45MB of 375MB at 2.3MB/sec".
      * @param percentCompleted A percentage between 0.0 and 1.0, concerning
                      only the current task.
    */
    void corpusMirrorProgress( Corpus corp, 
            int numtasks, int currtask, string taskname, string details,
            float percentCompleted );

    /** Called once the createLocalMirror function has completed.
     */
    void corpusMirrorCompleted( Corpus corp );
  };


  /** Handles download, mirror, extraction, modification of a Corpus.
   *  Corpus data items and functions are separated into "class Corpus" and
   *  "interface CorpusService" because of two reasons:
   *  1) ICE has to create fewer proxy classes, and
   *  2) only one CorpusService has to run and can deal with many corpora.
   *
   *  Construction of a new Corpus can currently only be accomplished through
   *  editing of a property file which contains all relevant information.
   *  
   *  A single CorpusService instance generally runs alongside the client code,
   *  but in the case of a remote Corpus, that instance might talk to other
   *  CorpusService instances running on remote machines.  The client should
   *  never have to deal with a remote Corpus directly.
   */
  interface CorpusService  {
    /** Opens the Corpus from a metadata file.  This does not download,
     *  extract, or otherwise prepare the Corpus, just create a Corpus object.
     */
    Corpus openCorpus( FilePath file );

    /** Write Corpus to a metadata file. 
     */
    void saveCorpus( Corpus corp, FilePath file );

    /** Download, extract, and keep caller informed via CorpusCallback.
      * A mirror can contain the actual files or just enough metadata about
      * the files so as to construct a LabelableList.
      */
    void createLocalMirror( Corpus corp, CorpusCallback cb );

    /** Obtain the actual data items in the corpus.  This will fail if
     *  the Corpus isImmutableMirror and createLocalMirror has not been
     *  completed.
     */
    LabelableList getDataSet( Corpus corp );

    /** Add a labeled or unlabeled artifact(s) to the Corpus.  This method will
     *  fail if the Corpus isImmutableMirror.
     */
    void addLabelable( Corpus corp, LabelableList addme );
  };
};

#endif  // _CORPUS_ICE
