#include "TrainPropI.h"
#include <iostream>
#include <vector>

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>

TrainPropI::TrainPropI()
{
}

TrainPropI::~TrainPropI()
{
}

void TrainPropI:: setWindowSize(cvac::Size size, const ::Ice::Current&)
{
    winSize = size;
}

bool TrainPropI::canSetWindowSize(const ::Ice::Current&)
{
    return true;
}

cvac::Size TrainPropI::getWindowSize(const ::Ice::Current&)
{
    return winSize;
}

void TrainPropI::setNumberStages(int stages, const ::Ice::Current&)
{
    numStages = stages;
}

bool TrainPropI::canSetNumberStages(const ::Ice::Current&)
{
    return true;
}

int TrainPropI::getNumberStages(const ::Ice::Current&)
{
    return numStages;
}

void TrainPropI::setMaxFalseAlarm(float fAlarmRate, const ::Ice::Current&)
{
    maxFalseAlarm = fAlarmRate;    
}

bool TrainPropI::canSetMaxFalseAlarm(const ::Ice::Current&)
{
    return true;
}

float TrainPropI::getMaxFalseAlarm(const ::Ice::Current&)
{
    return maxFalseAlarm;
}

void TrainPropI::setMinHitRate(float minRate, const ::Ice::Current&)
{
    minHitRate = minRate;
}

bool TrainPropI::canSetMinHitRate(const ::Ice::Current&)
{
    return true;
}

float TrainPropI::getMinHitRate(const ::Ice::Current&)
{
    return minHitRate;
}

void TrainPropI::setSymmetric(bool isSymmetric, const ::Ice::Current&)
{
    symmetric = isSymmetric;
}

bool TrainPropI::canSetSymmetric(const ::Ice::Current&)
{
    return true;
}

bool TrainPropI::getSymmetric(const ::Ice::Current&)
{
    return symmetric;
}

void TrainPropI::setBoostType(CvPerfTrainer::TrainerBoostType btype, const ::Ice::Current&)
{
    boostType = btype;
}

bool TrainPropI::canSetBoostType(const ::Ice::Current&)
{
    return true;
}

CvPerfTrainer::TrainerBoostType TrainPropI::getBoostType(const ::Ice::Current&)
{
    return boostType;
}

