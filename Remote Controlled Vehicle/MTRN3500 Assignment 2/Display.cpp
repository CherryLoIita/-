#include "Display.h"
#include <SMObjects.h>



Display::Display(SM_ThreadManagement^ SM_TM, SM_Laser^ SM_Laser) {
	
	Watch = gcnew Stopwatch;

	SM_TM_ = SM_TM;
	SM_Laser_ = SM_Laser;

	SendData = gcnew array<unsigned char>(64);
	ReadData = gcnew array<unsigned char>(2000);

}


void Display::threadFunction() {

	if (connect(DISPLAY_ADDRESS, DISPLAY_PORT) == SUCCESS) {

		Console::WriteLine("Display thread starting");
	}

	SM_TM_->ThreadBarrier->SignalAndWait();

	Watch->Start();

	while (!getShutdownFlag()) {

		if (processSharedMemory() == SUCCESS) {
			communicate();
			processHeartbeats();
		}
	}

	Console::WriteLine("Display thread terminating");

}


error_state Display::processHeartbeats() {

	if ((SM_TM_->heartbeat & bit_DISPLAY) == 0) {

		SM_TM_->heartbeat |= bit_DISPLAY;

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


void Display::shutdownThreads() {

	SM_TM_->shutdown |= bit_DISPLAY;

}


bool Display::getShutdownFlag() {

	return (SM_TM_->shutdown & bit_DISPLAY);

}


error_state Display::processSharedMemory() {

	// obtain Display_X data from SM_Laser
	Display_X = gcnew array<Byte>(sizeof(double) * SM_Laser_->x->Length);
	Buffer::BlockCopy(SM_Laser_->x, 0, Display_X, 0, Display_X->Length);
	// obtain Display_Y data from SM_Laser
	Display_Y = gcnew array<Byte>(sizeof(double) * SM_Laser_->y->Length);
	Buffer::BlockCopy(SM_Laser_->y, 0, Display_Y, 0, Display_Y->Length);

	return SUCCESS;
}


error_state Display::communicate() {
	
	// sending Display_X data
	Stream->Write(Display_X, 0, Display_X->Length);
	Thread::Sleep(20);
	// sending Display_Y data
	Stream->Write(Display_Y, 0, Display_Y->Length);
	Thread::Sleep(20);

	return SUCCESS;
}


error_state Display::connect(String^ hostName, int portNum) {

	// initialization TCP parameters
	Client = gcnew TcpClient(hostName, portNum);

	Client->SendBufferSize = 1024;
	Client->ReceiveBufferSize = 1024;

	Client->SendTimeout = 200;
	Client->ReceiveTimeout = 200;

	Client->NoDelay = true;

	Stream = Client->GetStream();

	return SUCCESS;
}
