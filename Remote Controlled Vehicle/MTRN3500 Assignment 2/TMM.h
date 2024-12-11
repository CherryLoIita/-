#pragma once


#include <UGVModule.h>
#include "Laser.h"
#include "GNSS.h"
#include "Display.h"
#include "VC.h"
#include "Controller.h"
#include "CrashAvoidance.h"


ref struct ThreadProperties {
    ThreadStart^ ThreadStart_;
    bool Critical;
    String^ ThreadName;
    uint8_t BitID;

    ThreadProperties(ThreadStart^ Tstart, bool TCritical, uint8_t Tbitid, String^ TthreadName) {
        ThreadStart_ = Tstart;
        Critical = TCritical;
        ThreadName = TthreadName;
        BitID = Tbitid;
    }
};


ref class ThreadManagement : public UGVModule {
public:

    ThreadManagement(SM_ThreadManagement^ SM_TM_In, SM_Laser^ SM_Laser_In, SM_GPS^ SM_GPS_In, SM_VehicleControl^ SM_VehicleControl_In);

    error_state processSharedMemory() override;

    error_state processHeartbeats();

    void shutdownAllThreads();

    bool getShutdownFlag() override;

    void threadFunction() override;


private:

    SM_ThreadManagement^ SM_TM_;
    SM_Laser^ SM_Laser_;
    SM_GPS^ SM_GPS_;
    SM_VehicleControl^ SM_VehicleControl_;
    Display^ Display_;

    array<Stopwatch^>^ StopwatchList;
    array<Thread^>^ ThreadList;
    array<ThreadProperties^>^ ThreadPropertiesList;

    unsigned int ThreadNum;


};
