/*
 * Runs a (usually local) server to access files in "corpora."
 *
 * For now, run from command line with:
 * java -cp "src/Corpus.jar:/opt/Ice-3.4.2/lib/Ice.jar" cvac.corpus.CorpusServer
 * java -cp "src/Corpus.jar;c:\program files (x86)\zeroc\Ice-3.4.2\lib\Ice.jar" cvac.corpus.CorpusServer
 */

package cvac.corpus;

//import Ice.Application;
import Ice.InitializationData;
//import Ice.Properties;

import cvac.CorpusService;

/**
 *
 * @author matz
 */
public class CorpusServer extends Ice.Application 
{
    @Override
    public int run(String[] args)
    {
        System.out.println("Started CorpusServer with arguments: " + args);
        
        CorpusService cs = new CorpusServiceI();

        // this starts the actual service
        Ice.Communicator ic = communicator();
        Ice.ObjectAdapter adapter =
            ic.createObjectAdapterWithEndpoints("CorpusServiceAdapter", "default -p 10011");
        adapter.add( cs, ic.stringToIdentity("CorpusServer") );
        ic.getProperties().setProperty( "CVAC.DataDir", "data" );
        adapter.activate();

        // now just wait for a shutdown signal
        ic.waitForShutdown();
        return 0;
    }

    public static void main(String[] args)
    {
        CorpusServer app = new CorpusServer();
        // int status = app.main("CorpusServer", args, "config.service");
        int status = app.main("CorpusServer", args);
        System.exit(status);
    }
}
