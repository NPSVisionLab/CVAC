/*
 * Runs a (usually local) server to access files in "corpora."
 */

package cvac.corpus;

//import Ice.Application;
//import Ice.InitializationData;
//import Ice.Properties;

import cvacslice.CorpusService;
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
        return 0;
    }

    public static void main(String[] args)
    {
        CorpusServer app = new CorpusServer();
//        InitializationData idata = new InitializationData();
//        if (null==idata.properties)
//        {
//            System.out.println("yes it's zero");
//            idata.properties = app.
//        }
//        idata.properties.setProperty("CorpusServer.Endpoints", "tcp -p 10011");
//        int status = app.main("CorpusServer", args, idata);
        int status = app.main("CorpusServer", args, "myconfig.txt");
        System.exit(status);
    }
}
