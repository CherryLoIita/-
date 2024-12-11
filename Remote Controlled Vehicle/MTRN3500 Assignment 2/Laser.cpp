#include "Laser.h"



Laser::Laser(SM_ThreadManagement^ SM_TM, SM_Laser^ SM_Laser) {

	Watch = gcnew Stopwatch();
	my_SM_TM = SM_TM;
	my_Laser = SM_Laser;

	SendData = gcnew array<unsigned char>(64);
	ReadData = gcnew array<unsigned char>(2000);

}


void Laser::threadFunction() {

	// Connecting
	if (connect(WEEDER_ADDRESS, LASER_PORT) == SUCCESS) {

		Console::WriteLine("Laser thread starting");

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

	Console::WriteLine("Laser thread terminating");

}


error_state Laser::processHeartbeats() {

	if ((my_SM_TM->heartbeat & bit_LASER) == 0) {

		Monitor::Enter(my_Laser->lockObject);
		try {
			my_SM_TM->heartbeat |= bit_LASER;
		}
		finally {
			Monitor::Exit(my_Laser->lockObject);
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


error_state Laser::processSharedMemory() {

	String^ response_data = Encoding::ASCII->GetString(ReadData);
	// get every single subdata from the reponse data stream
	array<String^>^ subdata = response_data->Split(' ');
	// wrong data
	if (subdata->Length < 26)	return ERR_INVALID_DATA;

	start_angle = Convert::ToInt32(subdata[23], 16);
	angle_space = (Convert::ToInt32(subdata[24], 16)) / 10000.0;
	
	unsigned int num_points = Convert::ToInt32(subdata[25], 16);

	for (int n = 0; n < num_points; n++) {

		unsigned int range = Convert::ToInt32(subdata[26 + n], 16);
		my_Laser->x[n] = range * Math::Cos(start_angle + (angle_space*n) * Math::PI / 180.0);
		my_Laser->y[n] = range * Math::Sin(start_angle + (angle_space*n) * Math::PI / 180.0);
	}
	
	return SUCCESS;
}


void Laser::shutdownThreads() {

	Monitor::Enter(my_Laser->lockObject);
	try {
		my_SM_TM->shutdown |= bit_LASER;
	}
	finally {
		Monitor::Exit(my_Laser->lockObject);
	}
}


bool Laser::getShutdownFlag() {

	return my_SM_TM->shutdown & bit_LASER;
}


error_state Laser::connect(String^ hostName, int portNum) {

	// initialization TCP parameters
	Client = gcnew TcpClient(hostName, portNum);

	Client->SendBufferSize = 1024;
	Client->ReceiveBufferSize = 1024;

	Client->SendTimeout = 200;
	Client->ReceiveTimeout = 200;

	Client->NoDelay = true;

	Stream = Client->GetStream();

	String^ send_command = "5527261\n";
	// obtain SendData
	SendData = Encoding::ASCII->GetBytes(send_command);
	// sending command
	Stream->Write(SendData, 0, SendData->Length);

	Thread::Sleep(30);

	// get response data
	Stream->Read(ReadData, 0, ReadData->Length);
	String^ response_data = Encoding::ASCII->GetString(ReadData);

	// return value 
	if (response_data[0] == 'O') {
		return SUCCESS;
	}
	else {
		return ERR_CONNECTION;
	}
}


error_state Laser::communicate() {

	String^ send_command = "sRN LMDscandata";
	SendData = Encoding::ASCII->GetBytes(send_command);
	// head char surround the sending command
	Stream->WriteByte(0x02);
	// sending command
	Stream->Write(SendData, 0, SendData->Length);
	// tail char surround the sending command
	Stream->WriteByte(0x03);

	Thread::Sleep(30);

	// get response data (LiDAR data)
	Stream->Read(ReadData, 0, ReadData->Length);
	String^ response_data = Encoding::ASCII->GetString(ReadData);
	// return 
	if (response_data->Length > 20) {
		return SUCCESS;
	}
	else {
		return ERR_NO_DATA;
	}

}



