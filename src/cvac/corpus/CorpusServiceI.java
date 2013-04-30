/**
 * Implementation for the CorpusService
 */

package cvac.corpus;

import cvac.CorpusService;
import cvac.CorpusCallback;
import cvac.Corpus;
import cvac.CorpusCallbackPrx;
import cvac.CorpusCallbackPrxHelper;
import cvac.CorpusServicePrxHelper;
import cvac._CorpusServiceDisp;
import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Implementation for the CorpusService
 *
 * @author matz
 */
public class CorpusServiceI extends _CorpusServiceDisp implements IceBox.Service
{
    protected static final Logger logger = Logger.getLogger(CorpusServiceI.class.getName());
    private Ice.ObjectAdapter mAdapter = null;
    private String dataDir = "";
    private CorpusConfig cc = null;
    private Map<String, CorpusI> corpToImp = null;

    /** Perform the initialization steps common to being started within or
     * outside an IceBox.  mAdapter must have been set prior to calling this.
     */
    private void initialize()
    {
        if (null!=cc)
        {
            throw new RuntimeException("CorpusServiceI has been initialized already");
        }
        dataDir = mAdapter.getCommunicator().getProperties().getProperty("CVAC.DataDir");
        logger.log(Level.FINE, "CorpusService found CVAC.DataDir={0}", dataDir);
        logger.setLevel(Level.FINEST);
        CorpusI.rootDataDir = dataDir;
        cc = new CorpusConfig();
        corpToImp = new HashMap<String, CorpusI>();
        logger.log(Level.INFO, "CorpusService initialized" );
    }
    
    /** IceBox calls this method
     * 
     * @param name
     * @param communicator
     * @param args 
     */
    @Override
    public void start(String name, Ice.Communicator communicator, String[] args)
    {
        mAdapter = communicator.createObjectAdapter(name);
        mAdapter.add( this, communicator.stringToIdentity("CorpusServer"));
        initialize();
        mAdapter.activate();
    }
    
    /** IceBox calls this method
     * 
     */
    @Override
    public void stop()
    {
         mAdapter.deactivate();  
    }
    /**
     * Opens the Corpus from a metadata file.  This does not download,
     * extract, or otherwise prepare the Corpus, just create a Corpus object.
     * @param __current The Current object for the invocation.
     **/
    @Override
    public Corpus openCorpus(cvac.FilePath file, Ice.Current __current) 
    {
        if (null==mAdapter) 
        {
            mAdapter = __current.adapter;
            initialize();
        }
        
        String filename = dataDir + File.separator
                + file.directory.relativePath + File.separator
                + file.filename;
        logger.log(Level.INFO, "request for openCorpus( {0} )", filename);
        try {
            CorpusI cs = cc.addCorpusFromConfig( new File(filename) );
            CorpusI cs_orig = corpToImp.get(cs.name);
            if (null!=cs_orig)
            {
                logger.log(Level.WARNING, 
                   "a corpus with the name {0} exists already, discarding the new one.", cs.name );
                return cs_orig;
            }
            corpToImp.put( cs.name, cs );
            return cs;
        } 
        catch ( CorpusI.CorpusConfigurationException ex) {
            logger.log(Level.WARNING, "could not open or parse Corpus property file {0}: {1}",
                    new Object[]{filename, ex.toString()} );
        }
        return null;
    }

    /**
     * Write Corpus to a properties file. 
     * @param __current The Current object for the invocation.
     **/
    @Override
    public void saveCorpus(Corpus corp, cvac.FilePath file, Ice.Current __current)
    {
        
    }

    /**
     * Download, extract, and keep caller informed via CorpusCallback.
     * A mirror can contain the actual files or just enough metadata about
     * the files so as to construct a LabelableList.
     * @param cb A CorpusCallback reference.
     * @param __current The Current object for the invocation.
     **/
    @Override
    public void createLocalMirror(Corpus corp, Ice.Identity cb, Ice.Current __current)
    {
        if (null==mAdapter) 
        {
            logger.log(Level.WARNING, "CorpusServiceI has not been initialized yet, "
                       + "no corpus is open.  Ignoring request to getDataSet.");
            return;
        }
        if (null==corp) 
        {
            logger.log(Level.WARNING, "CorpusServiceI.getDataSet called with null corpus.");
            return;
        }
        CorpusI cs = corpToImp.get( corp.name );
        if (null==cs)
        {
            logger.log(Level.WARNING, "nothing known about corpus {0}", corp.name);
            return;
        }
        Ice.ObjectPrx base = __current.con.createProxy( cb );
        CorpusCallbackPrx client = CorpusCallbackPrxHelper.uncheckedCast(base);
        //client.corpusMirrorCompleted( );
        cs.createLocalMirror( null );
    }

    /**
     * Obtain the actual data items in the corpus.  This will fail if
     * the Corpus isImmutableMirror and createLocalMirror has not been
     * completed.
     * @param __current The Current object for the invocation.
     **/
    @Override
    public cvac.Labelable[] getDataSet(Corpus corp, Ice.Current __current)
    {
        if (null==mAdapter) 
        {
            logger.log(Level.WARNING, "CorpusServiceI has not been initialized yet, "
                       + "no corpus is open.  Ignoring request to getDataSet.");
            return null;
        }
        if (null==corp) 
        {
            logger.log(Level.WARNING, "CorpusServiceI.getDataSet called with null corpus.");
            return null;
        }
        CorpusI cs = corpToImp.get( corp.name );
        if (null==cs)
        {
            logger.log(Level.WARNING, "nothing known about corpus {0}", corp.name);
            return null;
        }
        if ( !cs.hasFinishedLoading())
        {
            logger.log(Level.WARNING, "corpus {0} is still loading, cannot obtain labels", cs.name);
            return null;
        }
        return cs.getLabels();
    }

    /**
     * Add a labeled or unlabeled artifact(s) to the Corpus.  This method will
     * fail if the Corpus isImmutableMirror.
     * @param __current The Current object for the invocation.
     **/
    @Override
    public void addLabelable(Corpus corp, cvac.Labelable[] addme, Ice.Current __current)
    {
        CorpusI cs = corpToImp.get( corp.name );
        if (null==cs)
        {
            logger.log(Level.WARNING, "nothing known about corpus {0}", corp.name);
            return;
        }
        if ( cs.isImmutableMirror || !cs.hasFinishedLoading())
        {
            logger.log(Level.WARNING, "corpus {0} is immutabel or still loading, cannot add labels", cs.name);
        }
//        cs.addSample( addme );
    }
}
