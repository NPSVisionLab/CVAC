#!/bin/bash

# command arguments must be:
# $1: super-dir to bin/startIcebox.sh
# $2: dir where tests are located
# $3: ctest arguments, such as "-R File --verbose"
# $4: remote file service test machine to connect to

ROOT_DIR=$1
BUILD_DIR=$2
CTEST_ARGS=$3

#Tell the Tests what machine to use for remote fileserver tests
export CVAC_REMOTE_TEST_SERVER=$4

# start up services
cd ${BUILD_DIR}
bin/startIcebox.sh
sleep 15

# run the tests, capture exit status
cd ${BUILD_DIR}
export PYTHONPATH=${BUILD_DIR}/etc/python/easyPkg:${BUILD_DIR}/etc/python/icePkg
ctest ${CTEST_ARGS}
EXIT_STATUS=$?

# irrespective of exit status: shut down services
# then return exit status
cd ${BUILD_DIR}
bin/stopIcebox.sh
exit ${EXIT_STATUS}
