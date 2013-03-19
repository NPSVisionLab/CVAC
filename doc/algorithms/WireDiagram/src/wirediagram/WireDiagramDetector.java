/*******************************************************************************
 * CVAC Software Disclaimer
 * 
 * This software was developed at the Naval Postgraduate School, Monterey, CA,
 * by employees of the Federal Government in the course of their official duties.
 * Pursuant to title 17 Section 105 of the United States Code this software
 * is not subject to copyright protection and is in the public domain. It is 
 * an experimental system.  The Naval Postgraduate School assumes no
 * responsibility whatsoever for its use by other parties, and makes
 * no guarantees, expressed or implied, about its quality, reliability, 
 * or any other characteristic.
 * We would appreciate acknowledgement and a brief notification if the software
 * is used.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above notice,
 *       this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Naval Postgraduate School, nor the name of
 *       the U.S. Government, nor the names of its contributors may be used
 *       to endorse or promote products derived from this software without
 *       specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE NAVAL POSTGRADUATE SCHOOL (NPS) AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL NPS OR THE U.S. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
package wirediagram;

import Ice.Current;
import IceBox.Service;
import cvac.BBox;
import cvac.DetectorCallbackHandlerPrx;
import cvac.DetectorData;
import cvac.DetectorPropertiesPrx;
import cvac.Label;
import cvac.Labelable;
import cvac.LabeledLocation;
import cvac.Result;
import cvac.ResultSetV2;
import cvac.ResultRect;
import cvac.RunSet;
import cvac._DetectorDisp;
import java.util.Iterator;
import java.util.logging.Logger;
import cvacutil.RunSetWrapper;

/**
 * To run icebox on mac:
 * export DYLD_FALLBACK_LIBRARY_PATH=/usr/local/lib (or wherever you have the opencv dylibs
 * java -cp "/opt/Ice-3.4.2/lib/Ice.jar:WireDiagram.jar:javacv.jar" IceBox.Server --Ice.Config=config.icebox
 * You might need paths for the WireDiagram.jar and javacv.jar files.
 * @author matz
 */
public class WireDiagramDetector extends _DetectorDisp implements IceBox.Service {
  
  protected static final Logger logger = WireDiagram.logger;
  protected WireDiagram wd = null;
  int verbosity = 2;
  
  public WireDiagramDetector()
  {
    super();
    wd = WireDiagram.getRef();
  }
  
  @Override
  public void start( String name, Ice.Communicator _com, String args[] )
  {
    System.out.println("WireDiagramDetector::start called");
    Ice.ObjectAdapter adapter = _com.createObjectAdapter("WireDiagramDetector");
    adapter.add( this, _com.stringToIdentity("WireDiagramDetector") );
    adapter.activate();
    Ice.Properties props = _com.getProperties();
    String mediaRootPath = props.getProperty( "CVAC.MediaRootPath" );
    wd.setMediaRootPath( mediaRootPath );
  }

  @Override
  public void stop()
  {
    System.out.println("WireDiagramDetector::stop called");
  }

  @Override
  public void initialize(int verbosity, DetectorData data, Current __current) {
    System.out.println("WireDiagramDetector::initialize called");
    this.verbosity = verbosity;
  }

  @Override
  public String getName(Current __current) {
    return "Wire Diagram";
  }

  @Override
  public String getDescription(Current __current) {
    return "Wire Diagram detector detects hand-drawn or machine-drawn circuit diagrams";
  }

  @Override
  public boolean isInitialized(Current __current) {
    return wd!=null;
  }

  @Override
  public void process(DetectorCallbackHandlerPrx client, RunSet cvacRunset, Current __current)
  {
    System.out.println("WireDiagramDetector::process called");
    RunSetWrapper runset = new RunSetWrapper( cvacRunset, wd.getMediaRootPath() );
    Iterator<Labelable> it = runset.iterator();
    
    // this can be a rather costly operation:
    if (verbosity>1)
    {
      int numItems = runset.getSampleCount();
      client.message(4, "received " + numItems + " items in RunSet");
    }

    while (it.hasNext()) {
      System.out.println("WireDiagramDetector: has a next");
      Labelable next = it.next();
      String filename = wd.getFilename( next );
      if (!next.sub.isImage)
      {
        System.out.println( "WireDiagram can only detect in images, skipping " + filename);
        continue;
      }

      try {
        ResultRect reg[] = null;
        reg = wd.detect( filename );
        if (null == reg || reg.length == 0) {
          System.out.println("WireDiagramDetector: no detections");
          client.message(3, "Detections = 0 for " + next.sub.path.filename );
        } else {
          System.out.println("WireDiagramDetector: found something");
          client.message(3, "Detections = " + String.valueOf(reg.length) + " for " + next.sub.path.filename );
          ResultSetV2 rs = new ResultSetV2();
          rs.results = new Result[1];
          rs.results[0].original = next;
          rs.results[0].foundLabels = new Labelable[ reg.length ];
          float confidence = 0.9f;
          for (int i=0; i<reg.length; i++)
          {
            BBox bbox = new BBox( 
                    reg[i].x, reg[i].y, reg[i].width, reg[i].height );
            Label lab = new Label("wire diagram", null);
            rs.results[0].foundLabels[i] = new LabeledLocation( confidence, lab, next.sub, bbox );
          }
          client.foundNewResults( rs );
        }

      } catch (Exception e) {
        client.message(2, e.getMessage());
      }
    }
    client.message(3, "Run Complete!");
    System.out.println("WireDiagramDetector::process finished.");
  }

  @Override
  public DetectorPropertiesPrx getDetectorProperties(Current __current) {
    throw new UnsupportedOperationException("Not supported yet.");
  }

  @Override
  public void destroy(Current __current) {
    throw new UnsupportedOperationException("Not supported yet.");
  }

  @Override
  public void setVerbosity(int verbosity, Current __current) {
    throw new UnsupportedOperationException("Not supported yet.");
  }
}
