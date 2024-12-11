#pragma once


#include <UGVModule.h>


#define MAX_SPEED		1
#define MAX_STEERING	40


ref class Controller :public UGVModule {

public:
	Controller(SM_ThreadManagement^ SM_TM, SM_VehicleControl^ SM_VehicleControl_);

	void threadFunction() override;

	error_state processHeartbeats();

	void shutdownThreads();

	bool getShutdownFlag() override;

	error_state processSharedMemory() override;

	error_state communicate() override;


private:

	SM_ThreadManagement^ SM_TM_;
	Stopwatch^ Watch;
	ControllerInterface* Mycontroller;
	SM_VehicleControl^ SM_VehicleControl_;

};