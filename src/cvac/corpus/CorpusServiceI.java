/**
 * Implementation for the CorpusService
 */

package cvac.corpus;

import cvac.CorpusService;
import cvac.CorpusCallback;
import cvac.Corpus;
import cvac._CorpusServiceDisp;

/**
 * Implementation for the CorpusService
 *
 * @author matz
 */
public class CorpusServiceI extends _CorpusServiceDisp implements IceBox.Service 
{
    private Ice.ObjectAdapter mAdapter;
    private CorpusService mService = null;
    
    public void start(String name, Ice.Communicator communicator, String[] args)
    {
        mAdapter = communicator.createObjectAdapter(name);
        if (mService == null)
            mService = new CorpusServiceI();
        mAdapter.add(mService, communicator.stringToIdentity("CorpusServer"));
        mAdapter.activate();
    }
    
    public void stop()
    {
         mAdapter.deactivate();  
    }
    /**
     * Opens the Corpus from a metadata file.  This does not download,
     * extract, or otherwise prepare the Corpus, just create a Corpus object.
     * @param __current The Current object for the invocation.
     **/
    public Corpus openCorpus(cvac.FilePath file, Ice.Current __current) 
    {
        System.out.println("request for openCorpus( " + file.filename + " )");
        return new Corpus();
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
