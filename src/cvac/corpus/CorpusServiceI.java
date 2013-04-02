/**
 * Implementation for the CorpusService
 */

package cvac.corpus;

import cvacslice.CorpusService;
import cvacslice.CorpusCallback;
import cvacslice.Corpus;

/**
 * Implementation for the CorpusService
 *
 * @author matz
 */
public class CorpusServiceI extends cvacslice._CorpusServiceDisp {
    /**
     * Opens the Corpus from a metadata file.  This does not download,
     * extract, or otherwise prepare the Corpus, just create a Corpus object.
     * @param __current The Current object for the invocation.
     **/
    public Corpus openCorpus(cvac.FilePath file, Ice.Current __current) 
    {
        return new cvacslice.Corpus();
    }

    /**
     * Write Corpus to a metadata file. 
     * @param __current The Current object for the invocation.
     **/
    public void saveCorpus(Corpus corp, cvac.FilePath file, Ice.Current __current)
    {
        
    }

    /**
     * Download, extract, and keep caller informed via CorpusCallback.
     * A mirror can contain the actual files or just enough metadata about
     * the files so as to construct a LabelableList.
     * @param __current The Current object for the invocation.
     **/
    public void createLocalMirror(Corpus corp, CorpusCallback cb, Ice.Current __current)
    {
        
    }

    /**
     * Obtain the actual data items in the corpus.  This will fail if
     * the Corpus isImmutableMirror and createLocalMirror has not been
     * completed.
     * @param __current The Current object for the invocation.
     **/
    public cvac.Labelable[] getDataSet(Corpus corp, Ice.Current __current)
    {
        return null;
    }

    /**
     * Add a labeled or unlabeled artifact(s) to the Corpus.  This method will
     * fail if the Corpus isImmutableMirror.
     * @param __current The Current object for the invocation.
     **/
    public void addLabelable(Corpus corp, cvac.Labelable[] addme, Ice.Current __current)
    {
        
    }
}
