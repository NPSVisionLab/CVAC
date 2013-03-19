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
import cvac.Labelable;
import cvac.Purpose;
import cvac.PurposeType;
import cvac.RunSet;
import cvac.TrainerCallbackHandlerPrx;
import cvac.DetectorData;
import cvac.DetectorDataType;
import cvac.DetectorPrxHolder;
import cvac.*;
import cvac._DetectorTrainerDisp;
import java.util.Iterator;
import cvacutil.RunSetWrapper;

/**
 *
 * @author matz
 */
public class WireDiagramTrainer extends _DetectorTrainerDisp implements IceBox.Service {
  private int verbosity = 3;
  private boolean m_isRunning = false;
  WireDiagram wd = null;
  
  public WireDiagramTrainer()
  {
    wd = WireDiagram.getRef();
  }

  @Override
  public void start( String name, Ice.Communicator _com, String args[] )
  {
    System.out.println("WireDiagramTrainer::start called");
    Ice.ObjectAdapter adapter = _com.createObjectAdapter("WireDiagramTrainer");
    adapter.add( this, _com.stringToIdentity("WireDiagramTrainer") );
    Ice.Properties props = _com.getProperties();
    String mediaRootPath = props.getProperty( "CVAC.MediaRootPath" );
    wd.setMediaRootPath( mediaRootPath );
    adapter.activate();
  }

  @Override
  public void stop()
  {
    System.out.println("WireDiagramTrainer::stop called");
  }

  @Override
  public void initialize(int verbosity, Current __current) {
    // nothing to do
  }

  @Override
  public void process(TrainerCallbackHandlerPrx client, RunSet cvacRunset, Current __current)
  {
    m_isRunning = true;
    client.message(verbosity, "Wire diagram training started");

    RunSetWrapper runset = new RunSetWrapper( cvacRunset, wd.getMediaRootPath() );
    
    // this can be a rather costly operation:
    if (verbosity>1)
    {
      int numItems = runset.getSampleCount();
      client.message(4, "received " + numItems + " items in RunSet");
    }

    // insert positives into training data set
    Iterator<Labelable> it = runset.iterator( new Purpose( PurposeType.POSITIVE, -1) );
    if (!it.hasNext()) {
      throw new RuntimeException("need positive samples for training WireDiagram");
    }
    while (it.hasNext())
    {
      Labelable sample = it.next();
      String path = wd.getFilename( sample );
      wd.addToTraining( path, PurposeType.POSITIVE );
    }
    
    // insert negatives into training data set
    it = runset.iterator( new Purpose( PurposeType.NEGATIVE, -1) );
    if (!it.hasNext()) {
      throw new RuntimeException("need negative samples for training WireDiagram");
    }
    while (it.hasNext())
    {
      Labelable sample = it.next();
      String path = wd.getFilename( sample );
      wd.addToTraining( path, PurposeType.NEGATIVE );
    }
    wd.train();
    
    m_isRunning = false;
    client.message(verbosity, "Wire diagram training finished");
    
    DetectorData ddata = new DetectorData();
    ddata.type = DetectorDataType.FILE;
    ddata.file = new FilePath( new DirectoryPath( "someRelativePath" ), "someFile.txt" );
    client.createdDetector(ddata);
  }

  @Override
  public boolean isInitialized(Current __current) {
    return true;
  }

  @Override
  public void destroy(Current __current) {
    // nothing to do
  }

  @Override
  public String getName(Current __current) {
    return "Wire Diagram Trainer";
  }

  @Override
  public String getDescription(Current __current) {
    return "This trainer builds a model to detect wiring diagrams";
  }

  @Override
  public void setVerbosity(int verbosity, Current __current) {
    this.verbosity = verbosity;
  }
  
}
