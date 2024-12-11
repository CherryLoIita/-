#pragma once


#include <NetworkedModule.h>


ref class Display : public NetworkedModule {

public:

	Display(SM_ThreadManagement^ SM_TM, SM_Laser^ SM_Laser);

	void threadFunction() override;

	error_state processHeartbeats();

	void shutdownThreads();

	bool getShutdownFlag() override;

	error_state processSharedMemory() override;

	virtual error_state communicate() override;

	virtual error_state connect(String^ hostName, int portNum) override;


private:

	SM_ThreadManagement^ SM_TM_;
	SM_Laser^ SM_Laser_;
	Stopwatch^ Watch;
	array<Byte>^ Display_X;
	array<Byte>^ Display_Y;
};