#!/bin/bash
# Usage: runIceTest [ctest] det1 det2 ..."
#
#export PATH=$PATH:/opt/local/bin
#export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/Users/tomb/git/myCVAC/CVAC/lib"
if [ "$#" == "0" ]; then
    echo "Pass in the detectors run"
    exit 1
fi
CTEST="testImg"
VERIFY=""
if [ "$1" == "CTest" ]; then
    CTEST=$1
    VERIFY="verifyresults"
    shift;
fi
/Users/tomb/git/myCVAC/CVAC/runIcebox.sh
sleep 1
echo "runIcebox returned"
while (( "$#" ));do
    echo "Running detector $1"
    /Users/tomb/git/myCVAC/CVAC/bin/detectorClient $1 $CTEST /Users/tomb/git/myCVAC/CVAC/config.client $VERIFY
    if [ "$?" == "1" ]; then
        echo "Detector $1 failed"
	/Users/tomb/git/myCVAC/CVAC/stopIcebox.sh
        exit 1
    fi
    shift
done
echo "Stopping icebox"
/Users/tomb/git/myCVAC/CVAC/stopIcebox.sh
exit 0
