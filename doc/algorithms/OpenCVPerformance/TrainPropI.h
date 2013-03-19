#ifndef _TrainPropI_H__
#define _TrainPropI_H__


#include <Data.h>
#include <Services.h>
#include <Ice/Ice.h>
#include <IceBox/IceBox.h>
#include <IceUtil/UUID.h>
#include "CVAC_cvPerformanceTrainer.h"

class TrainPropI : public cvac::TrainerProperties
{
public:
    TrainPropI();
    ~TrainPropI();

    virtual void setWindowSize(cvac::Size size, const ::Ice::Current& = ::Ice::Current() );
    virtual bool canSetWindowSize(const ::Ice::Current& = ::Ice::Current() );
    virtual cvac::Size getWindowSize(const ::Ice::Current& = ::Ice::Current() );
    virtual void setNumberStages(int stages, const ::Ice::Current& = ::Ice::Current() );
    virtual bool canSetNumberStages(const ::Ice::Current& = ::Ice::Current() );
    virtual int getNumberStages(const ::Ice::Current& = ::Ice::Current() );
    virtual void setMaxFalseAlarm(float falseAlarmRate, const ::Ice::Current& = ::Ice::Current() );
    virtual bool canSetMaxFalseAlarm(const ::Ice::Current& = ::Ice::Current() );
    virtual float getMaxFalseAlarm(const ::Ice::Current& = ::Ice::Current() );
    virtual void setMinHitRate(float minRate, const ::Ice::Current& = ::Ice::Current() );
    virtual bool canSetMinHitRate(const ::Ice::Current& = ::Ice::Current() );
    virtual float getMinHitRate(const ::Ice::Current& = ::Ice::Current() );
    virtual void setSymmetric(bool isSymmetric, const ::Ice::Current& = ::Ice::Current() );
    virtual bool canSetSymmetric(const ::Ice::Current& = ::Ice::Current() );
    virtual bool getSymmetric(const ::Ice::Current& = ::Ice::Current() );
    virtual void setBoostType(CvPerfTrainer::TrainerBoostType btype, const ::Ice::Current& = ::Ice::Current() );
    virtual bool canSetBoostType(const ::Ice::Current& = ::Ice::Current() );
    virtual CvPerfTrainer::TrainerBoostType getBoostType(const ::Ice::Current& = ::Ice::Current() );

private:
    int numStages;
    cvac::Size winSize;
    float maxFalseAlarm;
    float minHitRate;
    bool symmetric;
    CvPerfTrainer::TrainerBoostType boostType;

    
};

#endif //_TrainPropI_H__
