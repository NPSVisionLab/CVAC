export PATH=$PATH:/opt/local/bin
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/Users/tomb/git/myCVAC/CVAC/lib"
cd /Users/tomb/git/myCVAC/CVAC
/opt/Ice-3.4/bin/icebox --Ice.Config=config.icebox &
echo CVAC services launched
exit
