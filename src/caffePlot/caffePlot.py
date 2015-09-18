#!/usr/bin/env python


import os
import re
import collections
import sys
import getopt
import ast
import io
import math
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

'''
Finds the next iteration number in the log file that is greater than the passed in iterator and
returns the following tuple (iter, loss, accuracy, vloss, vaccuracy, done, pos)
'''
def getNextLoss(iteration, logfile, pos):
    iterEx = re.compile(".* Iteration (\d+), .*")
    #/[+\-]?(?:0|[1-9]\d*)(?:\.\d*)?(?:[eE][+\-]?\d+)?/
    trainAccEx = re.compile(".*? Train net output #\d+: (.*?) = (\d+\.?\d*(?:[eE][+\-]?\d+)?)\n")
    trainLossEx = re.compile(".*? Train net output #\d+: (.*?) = (\d+\.?\d*(?:[eE][+\-]?\d+)?) (\(.*\))")
    testAccEx = re.compile(".*? Test net output #\d+: (.*?) = (\d+\.?\d*?(?:[eE][+\-]?\d+)?)\n")
    testLossEx = re.compile(".*? Test net output #\d+: (.*?) = (\d+\.?\d*?(?:[eE][+\-]?\d+)?) (\(.*\))")
    #ittLossEx = re.compile(".*? loss = (\d+\.?\d*)(?:[eE][+\-]?\d+)?")
    ittLossEx = re.compile(".*? loss = ((?:0|[1-9]\d*)(?:\.\d*)?(?:[eE][+\-]?\d+)?)")
    itDoneEx = re.compile(".*? (Optimization Done.)")
    logf = open(logfile, "r")
    if not logf:
        raise RuntimeError("Could not open log file " + logfile)
    res_iter = -1
    res_tloss = []
    res_taccuracy  = []
    res_vloss = []
    res_vaccuracy = []
    nextIter = None
    if pos > 0:
        logf.seek(pos,io.SEEK_SET)
    for line in logf:
        #print(line)
        itIter = iterEx.findall(line)
        tAccuracy = trainAccEx.findall(line)
        tLoss = trainLossEx.findall(line)
        vAccuracy = testAccEx.findall(line)
        vLoss = testLossEx.findall(line)
        itLoss = ittLossEx.findall(line)
        itDone = itDoneEx.findall(line)

        if len(itDone) > 0:
            res_done = True
            break
        else:
            res_done = False
        if len(itIter) > 0:
            nextIter = int(itIter[0])
        elif res_iter > 0:
            nextIter = res_iter
        if nextIter == None:
            pos += len(line)
            continue
        if (nextIter > iteration) and (res_iter == -1):
            res_iter = nextIter
        elif res_iter >= 0 and nextIter > res_iter:
            break
        pos += len(line)
        if len(itLoss) > 0:
            res_tloss.append(('loss', itLoss[0]))
        elif res_iter >= 0 and nextIter <= res_iter:
            if len(tAccuracy) > 0:
                res_taccuracy.append(tAccuracy[0])
            if len(tLoss) > 0:
                res_tloss.append(tLoss[0])
            if len(vAccuracy) > 0:
                res_vaccuracy.append(vAccuracy[0])
            if len(vLoss) > 0:
                res_vloss.append(vLoss[0])

    #print (res_iter, res_tloss, res_taccuracy, res_vloss, res_vaccuracy)
    return res_iter, res_tloss, res_taccuracy, res_vloss, res_vaccuracy, res_done, pos


if __name__ == '__main__':

    resFileExt = '.cdata'
    IterFile = '.caffe_iter'
    argv = sys.argv[1:]
    try:
        opts, args = getopt.getopt(argv, "l:fp:")
    except getopt.GetoptError:
        print (sys.argv[0] + ' -l logfile' + '[-f (force log read)' + '[-p (plot only label)...]')
        sys.exit(2)  # Error

    logfile = None
    force = False
    plotOnly = []
    for opt, arg in opts:
        if opt == '-l':
            logfile = arg
        if opt == '-f':
            force = True
        if opt == '-p':
            plotOnly.append('t-' + arg)
            plotOnly.append('v-' + arg)

    if not logfile:
        print("-l option required")
        sys.exit(1) # Warning


    pos = 0
    resfile = logfile + resFileExt
    rF = None

    iter = 0
    iterList = []
    tlossList = []
    taccuracyList = []
    vlossList = []
    vaccuracyList = []
    columns = []
    lastIter = -1
    while iter >= 0:
        iter, tloss, taccuracy, vloss, vaccuracy, done, pos = getNextLoss(lastIter, logfile, pos)
        lastIter = iter

        iterList.append(iter)
        if tloss:
            for tup in tloss:
                temp = 't-'+ tup[0]
                if temp not in columns:
                    columns.append(temp)
            tlossList.append(tloss)
        else:
            tlossList.append('nan')
        if vloss:
            for tup in vloss:
                temp = 'v-'+ tup[0]
                if temp not in columns:
                    columns.append(temp)
            vlossList.append(vloss)
        else:
            vlossList.append('nan')
        if taccuracy:
            for tup in taccuracy:
                temp = 't-'+ tup[0]
                if temp not in columns:
                    columns.append(temp)
            taccuracyList.append(taccuracy)
        else:
            taccuracyList.append('nan')
        if vaccuracy:
            for tup in vaccuracy:
                temp = 'v-' + tup[0]
                if temp not in columns:
                    columns.append(temp)
            vaccuracyList.append(vaccuracy)
        else:
            vaccuracyList.append('nan')

        if done:
            break



    if os.path.exists(resfile) == False or force == True:
        rF = open(resfile, 'w+')
        rF.write('Iters ')
        for name in columns:
            rF.write(name + " ")
        rF.write("\n")

        cnt = len(iterList)
        i = 0
        while i < cnt:
            rF.write(str(iterList[i]) + " ")
            for name in columns:
                valid = False
                if taccuracyList[i] != 'nan':
                    for item in taccuracyList[i]:
                        if name == 't-' + item[0]:
                            rF.write(item[1] + " ")
                            valid = True
                            break
                if tlossList[i] != 'nan':
                    for item in tlossList[i]:
                        if name == 't-' + item[0]:
                            rF.write(item[1] + " ")
                            valid = True
                            break
                if vaccuracyList[i] != 'nan':
                    for item in vaccuracyList[i]:
                        if name == 'v-' + item[0]:
                            rF.write(item[1] + " ")
                            valid = True
                            break
                if vlossList[i] != 'nan':
                    for item in vlossList[i]:
                        if name == 'v-' + item[0]:
                            rF.write(item[1] + " ")
                            valid = True
                            break
                if not valid:
                    rF.write('nan ')
            rF.write('\n')
            i += 1
        rF.close()

    ''' Create a plot of the result file
    '''
    logdata = pd.read_csv(resfile, delim_whitespace=True)
    dataShape = logdata.shape
    _, ax1 = plt.subplots(figsize=(15, 10))
    ax2 = ax1.twinx()
    ax1.set_ylim([0.0, 2.0])
    columns = list(logdata)
    # create colors
    cmap = plt.get_cmap('jet')
    # count number of plots in ax2
    ax2_cnt = 0;
    for name in columns:
        if 'acc' in name:
            ax2_cnt += 1
    ax1_cnt = len(columns) - ax2_cnt;
    ax1.set_color_cycle([cmap(1.*i/ax1_cnt) for i in range(ax1_cnt)])
    ax2.set_color_cycle([cmap(1.*i/ax2_cnt) for i in range(ax1_cnt, ax1_cnt + ax2_cnt)])
    handles = []

    for name in columns:
        if name == 'Iters':
            continue
        if plotOnly:
            if name not in plotOnly:
                continue
        mask = logdata[name].notnull()
        if 'acc' in name:
            h, = ax2.plot(logdata.ix[mask, 'Iters'], logdata.ix[mask, name], label=name)
            #ax1.plot(logdata.ix[0:,'Iters'], logdata.ix[0:,mask])
        else:
            h, = ax1.plot(logdata.ix[mask, 'Iters'], logdata.ix[mask,name], label=name)
            #ax2.plot(logdata.ix[0:,'Iters'], logdata.ix[0:,mask])
        handles.append(h)
    ax1.set_xlabel('iteration')
    ax1.set_ylabel('loss')
    ax2.set_ylabel('accuracy')
    #handles_ax1, labels_ax1 = ax1.get_legend_handles_labels()
    #handles_ax2, labels_ax2 = ax2.get_legend_handles_labels()
    #ax1.legend(handles_ax1, labels_ax1, loc="lower_right")
    #ax2.legend(handles_ax2, labels_ax2, loc="upper_right")
    plt.title(logfile)
    plt.legend(handles=handles, loc="right")
    #plt.legend(bbox_to_anchor=(1,1), loc="lower_right", borderaxespad=0.)
    plt.show()
    sys.exit(0)


