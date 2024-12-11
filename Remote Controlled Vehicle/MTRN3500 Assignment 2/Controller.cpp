#include "Controller.h"



Controller::Controller(SM_ThreadManagement^ SM_TM, SM_VehicleControl^ SM_VehicleControl) {

	Watch = gcnew Stopwatch();

	SM_TM_ = SM_TM;
	Mycontroller = new ControllerInterface(1, 0);
	SM_VehicleControl_ = SM_VehicleControl;

}


void Controller::threadFunction() {

	Console::WriteLine("Controller thread starting");

	SM_TM_->ThreadBarrier->SignalAndWait();

	Watch->Start();

	
	while (!getShutdownFlag()) {

		processSharedMemory();
		processHeartbeats();

		Thread::Sleep(20);
	}

	Console::WriteLine("Controller thread terminating");

}


void Controller::shutdownThreads() {

	SM_TM_->shutdown |= bit_CONTROLLER;

}


bool Controller::getShutdownFlag() {

	return (SM_TM_->shutdown & bit_CONTROLLER);

}


error_state Controller::processSharedMemory() {

	if (communicate()) {

		controllerState controller_state = Mycontroller->GetState();

		double ctrl_speed, ctrl_steering;

		if (controller_state.rightTrigger && controller_state.leftTrigger)	{ ctrl_speed = 0; }

		else if (controller_state.rightTrigger)								{ ctrl_speed = controller_state.rightTrigger * MAX_SPEED; }
		
		else if (controller_state.leftTrigger)								{ ctrl_speed = -(controller_state.leftTrigger * MAX_SPEED); }

		else																{ ctrl_speed = 0; }


		if (controller_state.rightThumbX)	{ ctrl_steering = controller_state.rightThumbX * MAX_STEERING; }
		
		else								{ ctrl_steering = 0; }


		SM_VehicleControl_->Speed = ctrl_speed;
		SM_VehicleControl_->Steering = ctrl_steering;

	}
	
	else {

		SM_VehicleControl_->Speed = 0;
		SM_VehicleControl_->Steering = 0;
		
	}

	return SUCCESS;

}


error_state Controller::processHeartbeats() {

	if ((SM_TM_->heartbeat & bit_CONTROLLER) == 0) {

		SM_TM_->heartbeat |= bit_CONTROLLER;

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


error_state Controller::communicate() {

	return error_state(Mycontroller->IsConnected());

}