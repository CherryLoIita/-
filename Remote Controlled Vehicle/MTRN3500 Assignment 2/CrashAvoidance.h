#pragma once


#include <UGVModule.h>
#include <cmath>
#include <vector>



ref class CrashAvoidance :public UGVModule {

public:

	CrashAvoidance(SM_ThreadManagement^ SM_TM, SM_Laser^ SM_Laser, SM_VehicleControl^ SM_VehicleControl);

	void threadFunction() override;

	error_state processHeartbeats();

	void shutdownThreads();

	bool getShutdownFlag() override;

	error_state processSharedMemory() override;

	bool CrashChecking();


private:

	SM_ThreadManagement^ SM_TM_;
	SM_Laser^ SM_Laser_;
	Stopwatch^ Watch;
	SM_VehicleControl^ SM_VehicleControl_;

	double check_distance = 1000;
	double half_width = 280;

};