/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

import cvac.*;
import java.util.List;
import java.util.ArrayList;
import java.util.logging.Level;
//import org.sleuthkit.autopsy.coreutils.Logger;
//import inspect;

/**
 *
 * @author tomb
 */
public class DetectorCallbackHandlerI extends _DetectorCallbackHandlerDisp{
     
    ArrayList<cvac.ResultSet> allResults ; 
    //Logger _logger;


    public DetectorCallbackHandlerI(){
        allResults =  new ArrayList<cvac.ResultSet>();
        //_logger = Logger.getLogger("EASYCV Logger");
    }

    public void log(String messString) {
        /*
        _logger.logp(java.util.logging.Level.INFO, "DetectorCallbackHandlerI",
                     "log", null, //inspect.stack()[1][3],
                     messString);
         */
    }

    public void estimatedTotalRuntime(int seconds, Ice.Current current){
        
    }
    public void estimatedRuntimeLeft(int seconds, Ice.Current current){
        
    }
    public void cancelled(Ice.Current current){
    }
    
    public void completedProcessing(Ice.Current current) {
    }
    public void message(int level, String messString, Ice.Current current){
    
    }
    public void foundNewResults(cvac.ResultSet results, Ice.Current current)    {
        allResults.add(results);
    }   
    public List<cvac.ResultSet> getResults()
    {
        return allResults;
    }
}
