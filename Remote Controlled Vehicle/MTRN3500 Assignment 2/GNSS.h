#pragma once


#include<NetworkedModule.h>


#define CRC32_POLYNOMIAL 0xEDB88320L
#define NORTHING_ADDRESS	44
#define EASTING_ADDRESS		52
#define HEIGHT_ADDRESS		60
#define CRC_ADDRESS			108



ref class GNSS :public NetworkedModule {

public:
	GNSS(SM_ThreadManagement^ SM_TM, SM_GPS^ SM_GPS);

	void threadFunction() override;

	error_state processHeartbeats();

	void shutdownThreads();

	bool getShutdownFlag() override;

	error_state processSharedMemory() override;

	virtual error_state communicate() override;

	virtual error_state connect(String^ hostName, int portNum) override;

	unsigned long CRC32Value(int i);

	unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer);

private:

	SM_ThreadManagement^ my_SM_TM;
	SM_GPS^ my_GPS;
	Stopwatch^ Watch;


};