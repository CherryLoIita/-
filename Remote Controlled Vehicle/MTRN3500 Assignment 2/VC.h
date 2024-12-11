#pragma once


#include<NetworkedModule.h>


ref class VehicleControl :public NetworkedModule {

public:
	VehicleControl(SM_ThreadManagement^ SM_TM, SM_VehicleControl^ SM_VehicleControl);

	void threadFunction() override;

	error_state processHeartbeats();

	void shutdownThreads();

	bool getShutdownFlag() override;

	error_state processSharedMemory() override;

	virtual error_state communicate() override;

	virtual error_state connect(String^ hostName, int portNum) override;


private:

	SM_ThreadManagement^ my_SM_TM;
	SM_VehicleControl^ my_VC;
	Stopwatch^ Watch;
};