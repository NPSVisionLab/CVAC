/*
 * Runs a (usually local) server to access files in "corpora."
 *
 * For now, run from command line with:
 * java -cp "src/Corpus.jar:/opt/Ice-3.4.2/lib/Ice.jar" cvac.corpus.CorpusServer
 */

package cvac.corpus;

//import Ice.Application;
import Ice.InitializationData;
//import Ice.Properties;

import cvac.CorpusService;
import cvac.BBox;

/**
 *
 * @author matz
 */
public class CorpusServer extends Ice.Application {
    public int
    run(String[] args)
    {
        System.out.println("we're in run()");
        System.out.println("args: " + args.length);
        BBox box = new BBox(1, 2, 3, 4);
        System.out.println("created BBox: x=" + box.x);
        
        CorpusService cs = new CorpusServiceI();

	// this starts the actual service
        Ice.Communicator ic = communicator();
        Ice.ObjectAdapter adapter =
            ic.createObjectAdapterWithEndpoints("CorpusServerAdapter", "default -p 10011");
        adapter.add( cs, ic.stringToIdentity("CorpusServer") );
        adapter.activate();

	// now just wait for a shutdown signal
        ic.waitForShutdown();
        return 0;
    }

    public static void main(String[] args)
    {
        CorpusServer app = new CorpusServer();
	//        int status = app.main("CorpusServer", args, "config.service");
        int status = app.main("CorpusServer", args);
        System.exit(status);
    }
}
