#include "CrashAvoidance.h"



CrashAvoidance::CrashAvoidance(SM_ThreadManagement^ SM_TM, SM_Laser^ SM_Laser, SM_VehicleControl^ SM_VehicleControl) {
	
	Watch = gcnew Stopwatch();
	SM_TM_ = SM_TM;
	SM_Laser_ = SM_Laser;
	SM_VehicleControl_ = SM_VehicleControl;

}


void CrashAvoidance::threadFunction() {

	Console::WriteLine("CrashAvoidance thread starting");

	SM_TM_->ThreadBarrier->SignalAndWait();

	Watch->Start();

	while (!getShutdownFlag()) {

		processSharedMemory();
		processHeartbeats();

	}

	Console::WriteLine("CrashAvoidance Thread terminating");

}


void CrashAvoidance::shutdownThreads() {

	SM_TM_->shutdown |= bit_CRASHAVOIDANCE;

}


bool CrashAvoidance::getShutdownFlag() {

	return (SM_TM_->shutdown & bit_CRASHAVOIDANCE);

}


error_state CrashAvoidance::processSharedMemory() {

	if (CrashChecking()) {

		SM_VehicleControl_->braking = TRUE;

		Console::WriteLine("Crash forward! Stop!");

	}

	else {
		SM_VehicleControl_->braking = FALSE;
	}

	return SUCCESS;
}


error_state CrashAvoidance::processHeartbeats() {

	if ((SM_TM_->heartbeat & bit_CRASHAVOIDANCE) == 0) 
	{
		SM_TM_->heartbeat |= bit_CRASHAVOIDANCE;

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


bool CrashAvoidance::CrashChecking() {

	if (SM_VehicleControl_->Speed == 0)		return FALSE;
	
	double X = 0;
	double Y = 0;
	double x_point = 0;
	double y_point = 0;
	double x_distance = 0;
	double y_distance = 0;

	double dt = 5;

	double current_speed = SM_VehicleControl_->Speed*1000;

	double current_steering = SM_VehicleControl_->Steering * Math::PI / 180.0;

	while (X * X + Y * Y < check_distance * check_distance) {

		Y = Y + dt * current_speed * std::cos(current_steering);
		X = X + dt * current_speed * std::sin(current_steering);
			
		for (int i = 0; i < STANDARD_LASER_LENGTH; i++)
		{
			x_point = SM_Laser_->x[i];
			y_point = SM_Laser_->y[i];
			
			// case of infinite range
			if (x_point == 0 && y_point == 0)		{ continue; }
			
			x_distance = x_point - X;
			y_distance = y_point - Y;

			if (x_distance * x_distance + y_distance * y_distance < half_width * half_width)	return TRUE;
		}

	}

	return FALSE;

}
