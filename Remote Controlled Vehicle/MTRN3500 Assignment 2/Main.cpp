#include "TMM.h"
#include "Laser.h"
#include "GNSS.h"
#include "VC.h"
#include "controller.h"
#include "CrashAvoidance.h"
#include "Display.h"



int main() {

	SM_ThreadManagement^ myTMM	= gcnew SM_ThreadManagement();
	SM_Laser^ myLaser			= gcnew SM_Laser();
	SM_GPS^ myGps				= gcnew SM_GPS();
	SM_VehicleControl^ myVC		= gcnew SM_VehicleControl();
	ThreadManagement^ AllTM		= gcnew ThreadManagement(myTMM, myLaser, myGps, myVC);

	Thread^ All_Threads = gcnew Thread(gcnew ThreadStart(AllTM, &ThreadManagement::threadFunction));
	
	All_Threads->Start();

}