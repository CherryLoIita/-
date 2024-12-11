#include "VC.h"



VehicleControl::VehicleControl(SM_ThreadManagement^ SM_TM, SM_VehicleControl^ SM_VehicleControl) {

	Watch = gcnew Stopwatch();
	my_SM_TM = SM_TM;
	my_VC = SM_VehicleControl;

	SendData = gcnew array<unsigned char>(64);
	ReadData = gcnew array<unsigned char>(2000);

}


void VehicleControl::threadFunction() {

	if(connect(WEEDER_ADDRESS, VC_PORT) == SUCCESS){

		Console::WriteLine("VehicleControl thread starting");
		Thread::Sleep(1000);
	}

	my_SM_TM->ThreadBarrier->SignalAndWait();

	Watch->Start();

	// thread loop
	while (!getShutdownFlag()) {

		if (processSharedMemory() == SUCCESS) {
			communicate();
			processHeartbeats();
		}

	}

	Console::WriteLine("VC thread terminating");

}


void VehicleControl::shutdownThreads() {

	Monitor::Enter(my_VC->lockObject);
	try {
		my_SM_TM->shutdown |= bit_VC;
	}
	finally {
		Monitor::Exit(my_VC->lockObject);
	}

}


bool VehicleControl::getShutdownFlag() {

	return (my_SM_TM->shutdown & bit_VC);

}


error_state VehicleControl::processSharedMemory() {

	my_VC->Speed = my_VC->Speed;
	my_VC->Steering = - my_VC->Steering;

	if ((my_VC->braking == TRUE) && my_VC->Speed > 0) {
		my_VC->Speed = 0;
		my_VC->Steering = 0;
	}

	return SUCCESS;
}


error_state VehicleControl::communicate() {

	// obtain sending data
	String^ VC_Data = String::Format("# {0} {1} {2} #", my_VC->Steering, my_VC->Speed, 1);
	array<unsigned char>^ Sending_Data = Encoding::ASCII->GetBytes(VC_Data);
	// sending data
	Stream->Write(Sending_Data, 0, Sending_Data->Length);
	
	return SUCCESS;
}


error_state VehicleControl::connect(String^ hostName, int portNum) {

	// initialization TCP parameters
	Client = gcnew TcpClient(hostName, portNum);

	Client->SendBufferSize = 1024;
	Client->ReceiveBufferSize = 1024;

	Client->SendTimeout = 500;
	Client->ReceiveTimeout = 500;

	Client->NoDelay = true;

	Stream = Client->GetStream();

	String^ send_command = "5527261\n";
	// obtain send data
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


error_state VehicleControl::processHeartbeats() {

	if ((my_SM_TM->heartbeat & bit_VC) == 0) {

		Monitor::Enter(my_VC->lockObject);
		try {
			my_SM_TM->heartbeat |= bit_VC;
		}
		finally {
			Monitor::Exit(my_VC->lockObject);
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
