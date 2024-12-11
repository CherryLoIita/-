#include "GNSS.h"



GNSS::GNSS(SM_ThreadManagement^ SM_TM, SM_GPS^ SM_GPS) {

	Watch = gcnew Stopwatch();
	my_SM_TM = SM_TM;
	my_GPS = SM_GPS;

	SendData = gcnew array<unsigned char>(64);
	ReadData = gcnew array<unsigned char>(2000);

}


void GNSS::threadFunction() {
	
	// Connecting
	if (connect(WEEDER_ADDRESS, GNSS_PORT) == SUCCESS) {

		Console::WriteLine("GNSS thread starting");
	}

	my_SM_TM->ThreadBarrier->SignalAndWait();

	Watch->Start();

	// thread loop
	while (!getShutdownFlag()) {

		if (communicate() == SUCCESS) {
			processSharedMemory();
			processHeartbeats();
		}
	}

	Console::WriteLine("GNSS thread terminating");

}


void GNSS::shutdownThreads() {

	Monitor::Enter(my_GPS->lockObject);
	try {
		my_SM_TM->shutdown |= bit_GPS;
	}
	finally {
		Monitor::Exit(my_GPS->lockObject);
	}

}


bool GNSS::getShutdownFlag() {

	return (my_SM_TM->shutdown & bit_GPS);
}


error_state GNSS::processSharedMemory() {

	double CRC;
	double CRC_Calculated;
	
	// obtain GNSS data
	Stream->Read(ReadData, 0, ReadData->Length);

	my_GPS->Northing = BitConverter::ToDouble(ReadData, NORTHING_ADDRESS);
	my_GPS->Easting = BitConverter::ToDouble(ReadData, EASTING_ADDRESS);
	my_GPS->Height = BitConverter::ToDouble(ReadData, HEIGHT_ADDRESS);
	
	// Get CRC code
	CRC = BitConverter::ToInt32(ReadData, CRC_ADDRESS);
	// Calculate CRC code
	pin_ptr<unsigned char> CRC_ucBuffer = &ReadData[0];
	CRC_Calculated = CalculateBlockCRC32(CRC_ADDRESS, CRC_ucBuffer);
	
	// Printing
	Console::WriteLine("Northing:" + my_GPS->Northing + "  Easting:" + my_GPS->Easting + "  Height:" + my_GPS->Height);
	Console::WriteLine("CRC:" + CRC + "  CRC_Calculated:" + CRC_Calculated);
	if (CRC == CRC_Calculated) {
		Console::WriteLine("CRC Correct");
		Console::WriteLine();
	}
	else {
		Console::WriteLine("CRC Wrong");
		Console::WriteLine();
	}

	return SUCCESS;
}


error_state GNSS::communicate() {

	if (Stream->DataAvailable) 
		
		return SUCCESS;

	else {

		return ERR_NO_DATA;
	}

}


error_state GNSS::connect(String^ hostName, int portNum) {

	// initialization TCP parameters
	Client = gcnew TcpClient(hostName, portNum);

	Client->SendBufferSize = 1024;
	Client->ReceiveBufferSize = 1024;

	Client->SendTimeout = 500;
	Client->ReceiveTimeout = 500;

	Client->NoDelay = true;

	Stream = Client->GetStream();

	return SUCCESS;
}


error_state GNSS::processHeartbeats() {

	if ((my_SM_TM->heartbeat & bit_GPS) == 0) {

		Monitor::Enter(my_GPS->lockObject);
		try {
			my_SM_TM->heartbeat |= bit_GPS;
		}
		finally {
			Monitor::Exit(my_GPS->lockObject);
		}

		Watch->Restart();

		Thread::Sleep(20);

		return SUCCESS;
	}

	else {

		if (Watch->ElapsedMilliseconds >= CRASH_TIME) {

			shutdownThreads();

			Thread::Sleep(20);

			return ERR_CRITICAL_PROCESS_FAILURE;
		}
	}
}


unsigned long GNSS::CRC32Value(int i) {

	int j;
	unsigned long ulCRC = i;
	for (j = 8; j > 0; j--) {
		if (ulCRC & 1) {
			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		}
		else {
			ulCRC >>= 1;
		}
	}
	return ulCRC;
}


unsigned long GNSS::CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer) {

	unsigned long ulTemp1;
	unsigned long ulTemp2;
	unsigned long ulCRC = 0;

	while (ulCount-- != 0) {
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xff);
		ulCRC = ulTemp1 ^ ulTemp2;
	}

	return ulCRC;
}


