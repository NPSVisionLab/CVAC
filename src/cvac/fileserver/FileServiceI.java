package cvac.fileserver;

import Ice.Current;
import Ice.Endpoint;
import cvac.FilePath;
import cvac.FileProperties;
import cvac.FileService;
import cvac.FileServiceException;
import cvac.VideoSeekTime;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 * This FileService implementation keeps a history of which files a client
 * put on the server so that only those files may be removed again.
 * It does not permit "up" and absolute paths: ../path and /path.
 * 
 * @author matz
 */
public class FileServiceI extends FileService implements IceBox.Service {

    protected static final Logger logger = Logger.getLogger(FileServiceI.class.getName());
    private Ice.ObjectAdapter mAdapter = null;
    Map<Endpoint, HashSet<File>> ownedFiles = null;
    String dataDir = "";
    static Set<String> videoExtensions = null;
    static Set<String> imageExtensions = null;

    /** Perform the initialization steps common to being started within or
     * outside an IceBox.  mAdapter must have been set prior to calling this.
     */
    private void initialize()
    {
        if (null!=ownedFiles)
        {
            throw new RuntimeException("FileServiceI has been initialized already");
        }
        ownedFiles = new HashMap<Endpoint, HashSet<File>>(0);
        dataDir = mAdapter.getCommunicator().getProperties().getProperty("CVAC.DataDir");
        logger.log(Level.FINE, "FileService found CVAC.DataDir={0}", dataDir);
        logger.setLevel(Level.FINEST);
        logger.log(Level.INFO, "FileService initialized" );
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
        mAdapter.add( this, communicator.stringToIdentity("FileService"));
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

    @Override
    public boolean exists(FilePath file, Current __current) throws FileServiceException 
    {
        if (null==mAdapter) 
        {
            mAdapter = __current.adapter;
            initialize();
        }
        
        // Read the file into memory and ship off as return value. 
        // This probably won't work for large files.
        // get local path and check for permissions
        File localFile = getLocalPath( file );
        logger.log(Level.INFO, "FileService exists: {0}", 
                localFile.getAbsolutePath());

        return localFile.exists();
    }
    
    @Override
    public void putFile(FilePath file, byte[] bytes, Current __current) throws FileServiceException
    {
        if (null==mAdapter) 
        {
            mAdapter = __current.adapter;
            initialize();
        }
        
        // get local path and check for permissions
        File localFile = getLocalPath( file );
        logger.log(Level.INFO, "FileService putFile: {0}", 
                localFile.getAbsolutePath());

        // don't overwrite existing files unless this client owns it
        if (null==__current.con)
        {
            // connection was optimized away since it's a local invocation.
            // This shouldn't happen: clients shouldn't ask for files if they're
            // local anyway.
            throw new FileServiceException("FileService doesn't make sense for local clients.");
        }
        Endpoint endpoint = __current.con.getEndpoint();
        
        if (localFile.exists() && !ownsFile( endpoint, localFile ))
        {
            if (localFile.exists()) 
                logger.log(Level.FINE, "file exists");
            if (!ownsFile( endpoint, localFile )) 
                logger.log(Level.FINE, "file not owned by endpoint");
            throw new FileServiceException("no put permissions to this file");
        }
        
        try {
            if (!localFile.createNewFile() || !localFile.canWrite())
            {
                logger.log(Level.INFO, "cannot write to file");
                throw new FileServiceException("no put permissions to this file");
            }

            FileOutputStream fos = new FileOutputStream( localFile );
            BufferedOutputStream bos = new BufferedOutputStream( fos );
            bos.write( bytes );
            bos.close();
        } catch (IOException ex) {
            throw new FileServiceException(ex.getMessage());
        }
        
        addOwner( endpoint, localFile );
    }

    @Override
    public byte[] getFile(FilePath file, Current __current) throws FileServiceException 
    {
        if (null==mAdapter) 
        {
            mAdapter = __current.adapter;
            initialize();
        }
        
        // Read the file into memory and ship off as return value. 
        // This probably won't work for large files.
        // get local path and check for permissions
        File localFile = getLocalPath( file );
        logger.log(Level.INFO, "FileService getFile: {0}", 
                localFile.getAbsolutePath());

        if (!localFile.canRead())
        {
            throw new FileServiceException("no read permissions to this file");
        }
        long flen = localFile.length();
        if (flen>Integer.MAX_VALUE)
        {
            throw new FileServiceException("file too large, cannot transfer yet");
        }
        
        try {
            FileInputStream fis = new FileInputStream( localFile );
            BufferedInputStream bis = new BufferedInputStream( fis );
            byte[] bytes = new byte[(int)flen];
            int sum = 0;
            do {
                int cnt = bis.read(bytes, sum, (int)flen-sum);
                if (-1==cnt) break;
                sum += cnt;
            } while (sum<flen);
            bis.close();
            if (sum<flen)
            {
                throw new FileServiceException("could not read entire file");
            }
            return bytes;
        } catch (IOException ex) {
            throw new FileServiceException(ex);
        }
    }

    /**
     * Note that we don't report specifically if this file doesn't exist
     * so that clients can't query for the existence of files.
     * 
     * @param file
     * @param __current
     * @throws FileServiceException 
     */
    @Override
    public void deleteFile(FilePath file, Current __current) throws FileServiceException 
    {
        if (null==mAdapter) 
        {
            mAdapter = __current.adapter;
            initialize();
        }
        
        // get local path and check for permissions
        File localFile = getLocalPath( file );
        logger.log(Level.INFO, "FileService deleteFile: {0}", 
                localFile.getAbsolutePath());

        // don't delete files unless this client owns it
        if (null==__current.con)
        {
            // connection was optimized away since it's a local invocation.
            // This shouldn't happen: clients shouldn't ask for files if they're
            // local anyway.
            throw new FileServiceException("FileService doesn't make sense for local clients.");
        }
        Endpoint endpoint = __current.con.getEndpoint();
        if (localFile.exists() && !ownsFile( endpoint, localFile ))
        {
            throw new FileServiceException("no delete permissions to this file");
        }
        
        localFile.delete();
    }

    @Override
    public FilePath createSnapshot(FilePath file, Current __current) throws FileServiceException {
        if (null==mAdapter) 
        {
            mAdapter = __current.adapter;
            initialize();
        }

        File localFile = getLocalPath( file );
        logger.log(Level.INFO, "FileService createSnapshot: {0}", 
                localFile.getAbsolutePath());
        
        throw new FileServiceException("Not supported yet.");
    }

    @Override
    public FileProperties getProperties(FilePath file, Current __current) throws FileServiceException {
        if (null==mAdapter) 
        {
            mAdapter = __current.adapter;
            initialize();
        }

        // get local path and check for permissions
        File localFile = getLocalPath( file );
        logger.log(Level.INFO, "FileService getProperties: {0}", 
                localFile.getAbsolutePath());

        // get the owner so we can report write permissions
        if (null==__current.con)
        {
            // connection was optimized away since it's a local invocation.
            // This shouldn't happen: clients shouldn't ask for files if they're
            // local anyway.
            throw new FileServiceException("FileService doesn't make sense for local clients.");
        }
        Endpoint endpoint = __current.con.getEndpoint();
        if (localFile.exists() && !ownsFile( endpoint, localFile ))
        {
            throw new FileServiceException("no delete permissions to this file");
        }

        FileProperties fp = new FileProperties();
        fp.bytesize = localFile.length();
        fp.height = -1;
        fp.width = -1;
        fp.isImage = hasImageExtension(localFile);
        fp.isVideo = hasVideoExtension(localFile);
        fp.readPermitted = localFile.canRead();
        fp.writePermitted = ownsFile(endpoint, localFile);
        fp.videoLength = new VideoSeekTime(-1, -1);
        
        return fp;
    }
    
    /** Resolves the relative FilePath into an absolute local path.
     * This uses the global CVAC.DataDir variable, checks for "up" and
     * absolute paths (.., /path), but it does not check whether the
     * local file exists already or whether this client has write permissions
     * to it.
     * @param relative
     * @return the absolute local path
     * @throws IllegalFileOperationException 
     */
    private File getLocalPath(FilePath relative) throws FileServiceException
    {
        String fpath = relative.directory.relativePath + File.separator
                + relative.filename;
        if (fpath.startsWith("/"))
        {
            throw new FileServiceException("absolute paths not permitted");
        }
        if (fpath.contains(".."))
        {
            throw new FileServiceException("up paths not permitted");
        }        
        assert(!dataDir.equals(""));
        
        // all pre-conditions met, create local file reference and return it
        File localFile = new File(dataDir + File.separator + fpath);
        return localFile;
    }
    
    /** 
     * @return true if @endpoint has putFile @localFile on this FileService before,
     *          false otherwise
     */
    private boolean ownsFile( Endpoint endpoint, File localFile )
    {
        Set<File> files = ownedFiles.get(endpoint);
        if (null==files)
        {
            return false;
        }
        return files.contains( localFile );
    }

    /** Remember this @endpoint as owner of the @localFile.
     * 
     * @param endpoint The client that connected to the FileService.
     * @param localFile Absolute path to the local file.
     */
    private void addOwner(Endpoint endpoint, File localFile) 
    {
        HashSet<File> list = ownedFiles.get(endpoint);
        if (null==list)
        {
            list = new HashSet<File>(1);
            ownedFiles.put(endpoint, list);
        }
        list.add(localFile);
    }

    private boolean hasVideoExtension(File localFile) 
    {
        if (null==videoExtensions)
        {
            String[] values = new String[] {"avi", "wmv", "mpg"};
            videoExtensions = new HashSet<String>(Arrays.asList(values));
        }
        String extension = localFile.getName();
        int i = extension.lastIndexOf('.');
        if (i > 0) {
            extension = extension.substring(i+1);
        }
        return videoExtensions.contains( extension );
    }

    private boolean hasImageExtension(File localFile) 
    {
        if (null==imageExtensions)
        {
            String[] values = new String[] {"jgp", "jpeg", "png", "gif"};
            imageExtensions = new HashSet<String>(Arrays.asList(values));
        }
        String extension = localFile.getName();
        int i = extension.lastIndexOf('.');
        if (i > 0) {
            extension = extension.substring(i+1);
        }
        return imageExtensions.contains( extension );
    }
}
