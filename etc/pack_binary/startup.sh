#!/bin/bash
MYPYTHON=""
RES=0
Dir=$(cd "$(dirname "$0")";pwd)
# turn off error reporting
launchctl unload -w /System/Library/LaunchAgents/com.apple.ReportCrash.plist
#First look for python 2.7
for p in `which -a python2.7`
do
    ver=`$p -c "import sys; print hex(sys.hexversion)"`
    if [[ "$ver" -ge 0x2070000 ]]
    then
        if [[ "$ver"  -lt 0x2080000 ]]
        then
            # verify that we can load ice with this python
            RES=`(cd $Dir/../Resources/3rdparty/Ice/python; $p -c "import Ice";echo $?)`
            if [ $RES == 0 ]
            then
                MYPYTHON=$p
                break
            fi
         fi
    fi
done
if [ "$MYPYTHON" == "" ]
then
    for p in `which -a python`
    do
        ver=`$p -c "import sys; print hex(sys.hexversion)"`
        if [[ "$ver" -ge 0x2070000 ]]
        then
            if [[ "$ver" -lt 0x2080000 ]]
            then
                # verify that we can load ice with this python
                (cd $Dir/../Resources/3rdparty/Ice/python; $p -c "import Ice";RES=$?)
                if [ $RES == 0 ]
                then
                    MYPYTHON=$p
                    break
                fi
            fi
        fi
    done
fi
if [ "$MYPYTHON" == "" ]
then
    echo "Could not find a valid python using default python"
    python $Dir/startup.py
else
    echo "Found python = $MYPYTHON"
    $MYPYTHON $Dir/startup.py 
fi
launchctl load -w /System/Library/LaunchAgents/com.apple.ReportCrash.plist
    
