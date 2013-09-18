/**
 * Implementation for the CorpusService
 */

package cvac.corpus;

import cvac.Corpus;
import cvac.CorpusCallbackPrx;
import cvac.CorpusCallbackPrxHelper;
import cvac._CorpusServiceDisp;
import java.io.File;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;
import util.Data_IO_Utils;

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
        logger.log(Level.INFO, "CorpusService stopped" );
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
        
        String filename = Data_IO_Utils.getFSPath(file,  dataDir);
        logger.log(Level.INFO, "request for openCorpus( {0} )", filename);
        try {
            CorpusI cs = cc.addCorpusFromConfig( new File(filename) );
            if (null==cs)	    
            {
                logger.log(Level.WARNING, 
                   "Could not add corpus from config file {0}", filename );
                return null;
            }
            CorpusI cs_orig = corpToImp.get(cs.name);
            if (null!=cs_orig)
            {
                logger.log(Level.INFO, 
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

    /** A corpus, once opened, is cached in the CorpusService and does not
     *  update itself if the file system or properties file change.  To
     *  re-open the corpus and re-load the cache, close it first.  Otherwise,
     *  closing is optional.
     * @param __current The Current object for the invocation.
     **/
    @Override
    public void closeCorpus(Corpus corp, Ice.Current __current) 
    {
        logger.log(Level.INFO, "request to closeCorpus( {0} )", corp.name);
        CorpusI cs = checkValidityAndLookup( corp );
        if (null==cs)
        {
            logger.log(Level.WARNING, 
                       "CorpusServiceI.createLocalMirror: error with corpus {0}", 
                       corp.name);
            return;
        }
        corpToImp.remove( cs.name );
    }
        
    /**
     * Write Corpus to a properties file. 
     * @param __current The Current object for the invocation.
     **/
    @Override
    public void saveCorpus(Corpus corp, cvac.FilePath file, Ice.Current __current)
    {
        throw new RuntimeException("Not implemented yet");
    }
    
    private CorpusI checkValidityAndLookup( Corpus corp )
    {
        if (null==mAdapter) 
        {
            logger.log(Level.WARNING, "CorpusServiceI has not been initialized yet, "
                       + "no corpus is open.  Ignoring request.");
            return null;
        }
        if (null==corp) 
        {
            logger.log(Level.WARNING, "CorpusServiceI called with null corpus.");
            return null;
        }
        CorpusI cs = corpToImp.get( corp.name );
        if (null==cs)
        {
            logger.log(Level.WARNING, "nothing known about corpus {0}", corp.name);
            return null;
        }
        return cs;
    }
    
    /** Does this Corpus require a "download" before the meta data
      * can be obtained?  That's in general, independent on whether it
      * has been downloaded already or not.
      */
    @Override
    public boolean getDataSetRequiresLocalMirror( Corpus corp, Ice.Current __current )
    {
        CorpusI cs = checkValidityAndLookup( corp );
        // if you're changing this function, please also change localMirrorExists
        if (null!=cs && cs.isImmutableMirror)
        {
            return true;
        }
        return false;
    }

    /** Has a local mirror already been created?  This will return true only
      * if this corpus requires a download, not for one that is local to
      * begin with.
      */
    @Override
    public boolean localMirrorExists( Corpus corp, Ice.Current __current )
    {
        CorpusI cs = checkValidityAndLookup( corp );
        // don't query if no local mirror required in the first place;
        // this protects directory-based corpora from being queried
        if (null==cs || !cs.isImmutableMirror) return false;
        return cs.localMirrorExists();
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
        CorpusI cs = checkValidityAndLookup( corp );
        if (null==cs)
        {
            logger.log(Level.WARNING, 
                    "CorpusServiceI.createLocalMirror: error with corpus {0}", corp.name);
            return;
        }
        logger.log(Level.INFO, "request to createLocalMirror( {0} )", corp.name);
        Ice.ObjectPrx base = __current.con.createProxy( cb ).ice_oneway();
        CorpusCallbackPrx client = CorpusCallbackPrxHelper.uncheckedCast(base);
        cs.createLocalMirror( client );
        client.corpusMirrorCompleted( corp );
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
        CorpusI cs = checkValidityAndLookup( corp );
        if (null==cs)
        {
            logger.log(Level.WARNING, "nothing known about corpus {0}", corp.name);
            return null;
        }
        logger.log(Level.INFO, "request for getDataSet( {0} )", cs.name);
        if ( cs.isImmutableMirror && !cs.localMirrorExists())
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
        CorpusI cs = checkValidityAndLookup( corp );
        if (null==cs)
        {
            logger.log(Level.WARNING, "nothing known about corpus {0}", corp.name);
            return;
        }
        if ( cs.isImmutableMirror || !cs.localMirrorExists())
        {
            logger.log(Level.WARNING, "corpus {0} is immutabel or still loading, cannot add labels", cs.name);
        }
//        cs.addSample( addme );
    }

    /**
     * Write Corpus to a properties file. 
     * @param __current The Current object for the invocation.
     **/
    @Override
    public Corpus createCorpus( cvac.DirectoryPath dir, Ice.Current __current )
    {
        if (null==mAdapter) 
        {
            mAdapter = __current.adapter;
            initialize();
        }
        
        String dirname = dataDir + File.separator + dir.relativePath;
        logger.log(Level.INFO, "request to create Corpus from path {0}", dirname);
        try {
            CorpusI cs = cc.createCorpusFromPath( new File(dirname) );
            CorpusI cs_orig = corpToImp.get(cs.name);
            if (null!=cs_orig)
            {
                logger.log(Level.FINE, 
                   "a corpus with the name {0} exists already, discarding the new one.", cs.name );
                return cs_orig;
            }
            corpToImp.put( cs.name, cs );
            return cs;
        } 
        catch ( CorpusI.CorpusConfigurationException ex) {
            logger.log(Level.WARNING, "could not parse Corpus from path {0}", ex.toString() );
        }
        return null;
    }
}
