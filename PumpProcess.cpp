
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "Pump.h"
#include "Customer.h"
#include "FuelTankMonitor.h"
#include <windows.h>
#include "C:\\RTExamples\\rt.h"

int main(void)
{
	while (1) {
		Pump Pump1(1);
		Pump Pump2(2);
		Pump Pump3(3);
		Pump Pump4(4);
		Pump1.Resume();
		Pump2.Resume();
		Pump3.Resume();
		Pump4.Resume();
		Pump2.WaitForThread();
		Pump3.WaitForThread();
		Pump1.WaitForThread();
		Pump4.WaitForThread();
	}
	return 0;

}