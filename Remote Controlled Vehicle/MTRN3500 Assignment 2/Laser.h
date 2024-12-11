#pragma once


#include<NetworkedModule.h>



ref class Laser : public NetworkedModule {

public:

	Laser(SM_ThreadManagement^ SM_TM, SM_Laser^ SM_Laser);

	void threadFunction() override;

	virtual error_state connect(String^ hostName, int portNum) override;

	virtual error_state communicate() override;

	error_state processSharedMemory() override;

	error_state processHeartbeats();

	void shutdownThreads();

	bool getShutdownFlag() override;



private:

	SM_ThreadManagement^ my_SM_TM;
	SM_Laser^ my_Laser;
	Stopwatch^ Watch;

	double start_angle;
	double angle_space;
};