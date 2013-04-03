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
package cvacutil;

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import cvac.*;
import cvac.Labelable;
import cvac.Purpose;
import cvac.PurposeType;
import cvac.PurposedLabelableSeq;
import cvac.PurposedList;
import java.util.HashMap;
import java.util.Map;
import java.util.NoSuchElementException;

/**
 * RunSetWrapper wraps the ICE-generated cvac.RunSet with Java utility functions.
 * @author matz
 */
public class RunSetWrapper implements Iterable {

  public static class RunSetIterator implements Iterator<Labelable> 
  {
    // currentLabelableIdx is set to the current index, or -1 if not started yet
    PurposedList[] lists;
    int currentPurposedListIdx;
    int currentLabelableIdx;
    Purpose constraint;
    String mediaRootPath;
    Map<PurposedDirectory, ArrayList<Labelable>> dirmap;

    public RunSetIterator( PurposedList[] _lists, Purpose _constraint, String _mediaRootPath )
    {          
      lists = _lists;
      currentPurposedListIdx = 0;
      constraint = _constraint;
      currentLabelableIdx = -1;
      mediaRootPath = _mediaRootPath;
      dirmap = new HashMap<PurposedDirectory, ArrayList<Labelable>>();
    }

    @Override
    public boolean hasNext() 
    {
      if (currentPurposedListIdx>=lists.length)
      {
//        System.out.println("RunSetWrapper: current list > length");
        return false;
      }

      int numInList = getNumItems( lists[currentPurposedListIdx] );
//      System.out.println("RunSetWrapper: " + numInList + " items in current list");
      if ( currentLabelableIdx+1 < numInList )
      {
        return true;
      }

      // if there's another list, try in that
      int nextList = currentPurposedListIdx+1;
      while (nextList<lists.length)
      {
        numInList = getNumItems( lists[nextList] );
//        System.out.println("RunSetWrapper: trying next list: " + nextList + ", found " + numInList + " items");
        if (numInList>0) return true;
        nextList++;
      }

      // so we won't check everything again next time:
      currentPurposedListIdx = lists.length;
      return false;
    }

    private int getNumItems( PurposedList list )
    {
      if (!compatiblePurpose(list.pur, constraint))
      {
        return 0;
      }
      
      if (list.getClass()==PurposedLabelableSeq.class)
      {
        PurposedLabelableSeq seq = (PurposedLabelableSeq) list;
        return seq.labeledArtifacts.length;
      }
      else if (list.getClass()==PurposedDirectory.class)
      {
        PurposedDirectory dir = (PurposedDirectory) list;
        return countFiles( dir );
      }
      else
      {
        throw new RuntimeException("Unknown PurposedList subclass encountered");
      }
    }


    @Override
    public Labelable next()
    {
      if ( !hasNext() )
      {
        throw new NoSuchElementException();
      }
      PurposedList list = lists[currentPurposedListIdx];
      while ( !compatiblePurpose(list.pur, constraint) )
      {
        currentPurposedListIdx++;
        if (currentPurposedListIdx>=lists.length)
          throw new RuntimeException("should have caught this in hasNext!!");
        list = lists[currentPurposedListIdx];
        currentLabelableIdx = -1;
      }
      if (list.getClass()==PurposedLabelableSeq.class)
      {
        PurposedLabelableSeq seq = (PurposedLabelableSeq) list;
        currentLabelableIdx++;
        return seq.labeledArtifacts[currentLabelableIdx];
      }
      else if (list.getClass()==PurposedDirectory.class)
      {
        PurposedDirectory dir = (PurposedDirectory) list;
        currentLabelableIdx++;
        return getLabelable( dir, currentLabelableIdx );
      }
      else
      {
        throw new RuntimeException("Unknown PurposedList subclass encountered");
      }
    }

    @Override
    public void remove() {
      throw new UnsupportedOperationException("The Labelable iterator does not support removal.");
    }
    
    /** recursively count the number of files in the specified directory;
     * side effect: adds them to the dirmap
     * @param dir
     * @return 
     */
    private int countFiles( PurposedDirectory dir )
    {
      ArrayList<Labelable> lls = dirmap.get( dir );
      if (lls!=null)
      {
        return lls.size();
      }
      lls = new ArrayList<Labelable>(1);
      dirmap.put(dir, lls);
      
      String path = mediaRootPath + File.separatorChar + dir.directory.relativePath;
      File dirfile = new File( path );
      addFiles( lls, dirfile, dir.directory.relativePath, dir.recursive );
      
      return lls.size();
    }
    
    /** the actual recursion function for counting files and adding to the dirmap
     * 
     * @param lls
     * @param dirfile
     * @param relativePath
     * @param recursive 
     */
    private void addFiles( ArrayList<Labelable> lls, File dirfile, String relativePath, boolean recursive )
    {
      if (!dirfile.isDirectory()) 
      {
        System.out.println("expected directory, but it's not: " + dirfile);
        return;
      }

      File[] files = dirfile.listFiles();
      for (int i=0; i<files.length; i++)
      {
        if (files[i].isDirectory() && recursive)
        {
          String newpath = relativePath + File.separatorChar + files[i].getName();
          addFiles( lls, files[i], newpath, true );
        }
        else
        {
          FilePath mediaPath = new FilePath( new DirectoryPath(relativePath), files[i].getName() );
          Labelable lab = new Labelable(-1, null, new Substrate(true, false, mediaPath, 0, 0 ));
          lls.add(lab);
        }
      }
    }

    private Labelable getLabelable(PurposedDirectory dir, int currentLabelableIdx) {
      ArrayList<Labelable> lls = dirmap.get( dir );
      //if (lls==null)
      //{
      //  // this inserts all Labelables into the dirmap
      //  countFiles( dir );
      //}
      //lls = dirmap.get( dir );
      assert( null!=lls );
      
      return lls.get(currentLabelableIdx);
    }

    private boolean compatiblePurpose(Purpose actual, Purpose constraint) 
    {
      if (PurposeType.ANY==constraint.ptype) return true;
      if (actual.ptype!=constraint.ptype) return false;
      if (PurposeType.MULTICLASS==constraint.ptype && PurposeType.MULTICLASS==actual.ptype 
          && actual.classID!=constraint.classID)
        return false;
      return true;
    }
  }
        
    private cvac.RunSet runset = null;
    private String mediaRootPath = null;

    /**
     * Wraps a CVAC RunSet in Java utility functions; mainly an iterator.
     */
    public RunSetWrapper( cvac.RunSet runset, String mediaRootPath )
    {
        this.runset = runset;
        this.mediaRootPath = mediaRootPath;
    }
    
    /** count all samples (Labelables) in this list of lists.
     * Note that this might be an expensive operation.
     * @return 
     */
    public int getSampleCount() 
    {
      int total = 0;
      Purpose pur = new Purpose( PurposeType.ANY, 0 ); 
      RunSetIterator it = new RunSetIterator( runset.purposedLists, pur, mediaRootPath );
      for (int i = 0; i < runset.purposedLists.length; i++)
      {
        total += it.getNumItems( runset.purposedLists[i] );
      }
      return total;
    }    

    @Override
    public Iterator<Labelable> iterator()
    {
      Purpose pur = new Purpose( PurposeType.ANY, 0 ); 
      return new RunSetIterator( runset.purposedLists, pur, mediaRootPath );
    }
   
    /** iterate only over samples with the "constraint" purpose
     * @param constraint for example, only positive samples
     * @return 
     */
    public Iterator<Labelable> iterator( Purpose constraint )
    {
      return new RunSetIterator( runset.purposedLists, constraint, mediaRootPath );
    }
    
    @Override
    public String toString()
    {
      String out = "";
      Purpose pur = new Purpose( PurposeType.ANY, 0 ); 
      for (int i = 0; i < runset.purposedLists.length; i++)
      {
        PurposedList[] oneList = new PurposedList[1];
        oneList[0] = runset.purposedLists[i];
        out += runset.purposedLists[i].pur.ptype + " (" + runset.purposedLists[i].pur.classID + ")\n"; 
        RunSetIterator it = new RunSetIterator( oneList, pur, mediaRootPath );
        while (it.hasNext())
        {
          Labelable lab = it.next();
          out += "  " + lab.sub.path.directory.relativePath + "/" + 
                  lab.sub.path.filename + "\n";
        }
      }
      return out;
    }
}
