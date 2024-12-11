#include "TMM.h"



ThreadManagement::ThreadManagement(SM_ThreadManagement^ SM_TM, SM_Laser^ SM_Laser, SM_GPS^ SM_GPS,SM_VehicleControl^ SM_VehicleControl) {

	SM_TM_ = SM_TM;
	SM_Laser_ = SM_Laser;
	SM_GPS_ = SM_GPS;
	SM_VehicleControl_ = SM_VehicleControl;

	Display_ = gcnew Display(SM_TM, SM_Laser);

}


error_state ThreadManagement::processSharedMemory() {

	ThreadPropertiesList = gcnew array<ThreadProperties^>{

		gcnew ThreadProperties(gcnew ThreadStart(gcnew Laser(SM_TM_, SM_Laser_),								&Laser::threadFunction),			true,		bit_LASER,				"Laser thread"),
		gcnew ThreadProperties(gcnew ThreadStart(gcnew GNSS(SM_TM_, SM_GPS_),									&GNSS::threadFunction),				true,		bit_GPS,				"GNSS thread"),
		gcnew ThreadProperties(gcnew ThreadStart(gcnew VehicleControl(SM_TM_, SM_VehicleControl_),				&VehicleControl::threadFunction),	true,		bit_VC,					"VehicleControl thread"),
		gcnew ThreadProperties(gcnew ThreadStart(gcnew Controller(SM_TM_, SM_VehicleControl_),					&Controller::threadFunction),		true,		bit_CONTROLLER,			"Controller thread"),
		gcnew ThreadProperties(gcnew ThreadStart(gcnew CrashAvoidance(SM_TM_, SM_Laser_, SM_VehicleControl_),	&CrashAvoidance::threadFunction),	true,		bit_CRASHAVOIDANCE,		"CrashAvoidance thread"),
		gcnew ThreadProperties(gcnew ThreadStart(gcnew Display(SM_TM_, SM_Laser_),								&Display::threadFunction),			true,		bit_DISPLAY,			"Display thread"),

	};

	ThreadNum = ThreadPropertiesList->Length;

	StopwatchList = gcnew array<Stopwatch^>(ThreadNum);

	SM_TM_->ThreadBarrier = gcnew Barrier(ThreadNum + 1);

	ThreadList = gcnew array<Thread^>(ThreadNum);


	for (int i = 0; i < ThreadNum; i++) {

		StopwatchList[i] = gcnew Stopwatch();
		ThreadList[i] = gcnew Thread(ThreadPropertiesList[i]->ThreadStart_);

	}
	
	return error_state::SUCCESS;

}


void ThreadManagement::shutdownAllThreads() {

	SM_TM_->shutdown = 0xFF;

	for (int i = 0; i < ThreadNum; i++) {
		ThreadList[i]->Join();
	}

}


bool ThreadManagement::getShutdownFlag() {

	return (SM_TM_->shutdown & bit_TM);

}


void ThreadManagement::threadFunction() {

	Console::WriteLine("TMM thread starting");

	processSharedMemory();

	// Starting threads
	for (int i = 0; i < ThreadNum; i++) {

		ThreadList[i]->Start();
		StopwatchList[i]->Start();

	}

	SM_TM_->ThreadBarrier->SignalAndWait();
	

	// Threads loop
	while (!getShutdownFlag()) {

		if (Console::KeyAvailable) {
			ConsoleKeyInfo keyInfo = Console::ReadKey(true);    
			if (keyInfo.KeyChar == 'q')		break;
		}

		processHeartbeats(); 

		Thread::Sleep(30);
	}

	shutdownAllThreads();

	Console::WriteLine("TMM Thread terminating");

	Thread::Sleep(1000);

}


error_state ThreadManagement::processHeartbeats() {

	for (int i = 0; i < ThreadNum; i++) {
		
		// heartbeat=1 : pull the heartbeat signals down to 0 and reset stopwatches
		if (SM_TM_->heartbeat & ThreadPropertiesList[i]->BitID) {
			Monitor::Enter(SM_TM_->lockObject);
			try {
				SM_TM_->heartbeat ^= ThreadPropertiesList[i]->BitID;
			}
			finally {
				Monitor::Exit(SM_TM_->lockObject);
			}
			StopwatchList[i]->Restart();								
		}
		// heartbeat=0 : check the crash time
		else {
			if (StopwatchList[i]->ElapsedMilliseconds >= CRASH_TIME) {

				// Critical - Shutdown
				if (ThreadPropertiesList[i]->Critical) {	

					Console::WriteLine(ThreadPropertiesList[i]->ThreadName + " fail (Critical): All threads shutting down");
					
					shutdownAllThreads();

					return ERR_CRITICAL_PROCESS_FAILURE;
				}

				// Non-Critical: Restart
				else {
					Console::WriteLine(ThreadPropertiesList[i]->ThreadName + " fail (Non-Critical): Restarting");
					
					ThreadList[i]->Abort();
					ThreadList[i] = gcnew Thread(ThreadPropertiesList[i]->ThreadStart_);
					
					SM_TM_->ThreadBarrier = gcnew Barrier(1);			
					
					ThreadList[i]->Start();
				}
			}
		}
	}

	return SUCCESS;
}