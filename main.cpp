

// This is the Gas station computer. Main process that creates all child 
// threads that creates datapools and communicates with pumps and fuel tanks.

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "C:\\RTExamples\\rt.h"
#include "FuelTankMonitor.h"
#include "Pump.h"
#include "Customer.h"
#include <windows.h>

#define ADD1 35
#define ADD2 75
#define YVAL 13


/* Not needed because all FuelTank data is stored within the FuelTank Monitor
struct fueltankdatapool {
	int Octane87Level;
	int Octane89Level;
	int Octane91Level;
	int DieselLevel;
};
*/

// FuelTankMonitor object included in pump.h

// Main Mutex to control printing to the DOS window.
CMutex DOSMutex("DOSMutex");

// Semaphores to control datapools for pump and GSC communication
/* Producer waits on ps1, produces data and signals cs1. Consumer waits on cs1, consumes data, then signals ps1.
This resets back to the original values of ps1 and cs1 which means producer/consumer arrangement
can start again with the same wait and signal process.
*/
CSemaphore ps1("PS1", 1, 1);  // pump1
CSemaphore cs1("CS1", 0, 1);

CSemaphore ps2("PS2", 1, 1); // pump2
CSemaphore cs2("CS2", 0, 1);

CSemaphore ps3("PS3", 1, 1); // pump3
CSemaphore cs3("CS3", 0, 1);

CSemaphore ps4("PS4", 1, 1); // pump4
CSemaphore cs4("CS4", 0, 1);



struct Customer1Data {
	int MyCreditNum;
	char MyName[14];
	int FuelNeed;
	int FuelChoice;
	BOOL refuelComplete;
};
struct Customer2Data {
	int MyCreditNum;
	char MyName[14];
	int FuelNeed;
	int FuelChoice = 87;
	BOOL refuelComplete;
};
struct Customer3Data {
	int MyCreditNum;
	char MyName[14];
	int FuelNeed;
	int FuelChoice;
	BOOL refuelComplete;
};
struct Customer4Data {
	int MyCreditNum;
	char MyName[14];
	int FuelNeed;
	int FuelChoice;
	BOOL refuelComplete;
};

//All 5 child threads required to communicate between pumps and fuel tanks

UINT __stdcall Pump1Thread(void *args)
{
	struct Customer1Data *Customer1PTR = new struct Customer1Data;

	CDataPool pumpstatus1("PumpStatus1", sizeof(struct pumpdatapool));
	struct pumpdatapool *PumpStatus1DP = (struct pumpdatapool *)(pumpstatus1.LinkDataPool());

	while (1) {
		// Producer (pump) consumer(gsc) problem
		cs1.Wait(); // Wait for pump to signal cs2 so gsc can consume data


		Customer1PTR->MyCreditNum = PumpStatus1DP->MyCreditNum;
		for (int i = 0; i < 14; i++) {
			Customer1PTR->MyName[i] = PumpStatus1DP->MyName[i];
		}


		// Now print customer data to the DOS window
		DOSMutex.Wait(); // Gain DOS window writing premission

		MOVE_CURSOR(ADD1, 5 - 5);
		printf("Customer Name: ");
		for (int i = 0; i < 14; i++) {
			printf("%c", Customer1PTR->MyName[i]);
			fflush(stdout);
		}
		MOVE_CURSOR(ADD1, 6 - 5);
		printf("Credit Card Num: %d\n", Customer1PTR->MyCreditNum);
		fflush(stdout);

		DOSMutex.Signal(); // Release DOS window writing permission

		PumpStatus1DP->CreditValid = TRUE;

		cs1.Signal(); // signal producer semphore when done consuming data.

		ps1.Wait(); // gain control of producing 
		// DO SOMETHING TO CHECK IF CREDIT CARD NUMBER IS VALID

		SLEEP(200);

		cs1.Wait();
		Customer1PTR->FuelNeed = PumpStatus1DP->FuelNeed;
		Customer1PTR->FuelChoice = PumpStatus1DP->FuelChoice;

		DOSMutex.Wait(); // Gain DOS window writing premission

		MOVE_CURSOR(ADD1, 3);
		printf("Fuel Choice is: %d\n", Customer1PTR->FuelChoice);
		fflush(stdout);
		MOVE_CURSOR(ADD1, 4);
		printf("Fuel Need is: %d\n", Customer1PTR->FuelNeed);
		fflush(stdout);

		DOSMutex.Signal(); // Release DOS window writing permission

		BOOL j = FALSE;
		if (FuelTankMonitor.readLevel(Customer1PTR->FuelChoice) >= Customer1PTR->FuelNeed)
		{


			while (j != TRUE) {
				DOSMutex.Wait(); // Gain DOS window writing premission
				MOVE_CURSOR(ADD1, 12 - 5);
				printf("Enter P1 to start dispensing fuel");
				fflush(stdout);
				MOVE_CURSOR(ADD1, 13 - 5);
				char temp = getchar();
				if (temp == 'P') {
					char temp2 = getchar();
					if (temp2 == '1') {
						j = TRUE;
						PumpStatus1DP->Dispense = TRUE;
						MOVE_CURSOR(ADD1, 14 - 5);
						printf("Customer has begun dispensing fuel");
						fflush(stdout);
						MOVE_CURSOR(ADD1, 13 - 5);
						printf("                       ");
						fflush(stdout);
						temp = FALSE;
						temp2 = FALSE;

					}
					else {
						MOVE_CURSOR(ADD1, 12 - 5);
						printf("     ");
						fflush(stdout);
					}
				}
				else {
					MOVE_CURSOR(ADD1, 12 - 5);
					printf("     ");
					fflush(stdout);
				}
				DOSMutex.Signal();
			}
		}

		else if (FuelTankMonitor.readLevel(Customer1PTR->FuelChoice) <= Customer1PTR->FuelNeed)
		{
			PumpStatus1DP->Dispense = FALSE;
		}



		// READ COMMANDS FROM THE DOS WINDOW FOR REFUEL 2 KEY COMMAND
		// Then signals cs2 so pump can read whether GSC allowed pump to refuel or not
		cs1.Signal();
		SLEEP(200);
		cs1.Wait();
		Customer1PTR->refuelComplete = PumpStatus1DP->refuelComplete;
		if (Customer1PTR->refuelComplete == TRUE) {
			DOSMutex.Wait();
			MOVE_CURSOR(ADD1, 12 - 5);
			printf("                                 ");
			MOVE_CURSOR(ADD1, 14 - 5);
			printf("Customer is done                  ");
			fflush(stdout);
			ps1.Signal();
			DOSMutex.Signal();
		}

	}

	return 0;
}

UINT __stdcall Pump2Thread(void *args)
{
	
	struct Customer2Data *Customer2PTR = new struct Customer2Data;

	CDataPool pumpstatus2("PumpStatus2", sizeof(struct pumpdatapool));
	struct pumpdatapool *PumpStatus2DP = (struct pumpdatapool *)(pumpstatus2.LinkDataPool());
	
	while (1) {
		// Producer (pump) consumer(gsc) problem
		cs2.Wait(); // Wait for pump to signal cs2 so gsc can consume data


		Customer2PTR->MyCreditNum = PumpStatus2DP->MyCreditNum;
		for (int i = 0; i < 14; i++) {
			Customer2PTR->MyName[i] = PumpStatus2DP->MyName[i];
		}


		// Now print customer data to the DOS window
		DOSMutex.Wait(); // Gain DOS window writing premission

		MOVE_CURSOR(ADD2, 5-5);
		printf("Customer Name: ");
		for (int i = 0; i < 14; i++) {
			printf("%c", Customer2PTR->MyName[i]);
			fflush(stdout);
		}
		MOVE_CURSOR(ADD2, 6-5);
		printf("Credit Card Num: %d\n", Customer2PTR->MyCreditNum);
		fflush(stdout);

		DOSMutex.Signal(); // Release DOS window writing permission

		PumpStatus2DP->CreditValid = TRUE;

		cs2.Signal(); // signal producer semphore when done consuming data.

		ps2.Wait(); // gain control of producing 
		// DO SOMETHING TO CHECK IF CREDIT CARD NUMBER IS VALID

		SLEEP(200);

		cs2.Wait();
		Customer2PTR->FuelNeed = PumpStatus2DP->FuelNeed;
		Customer2PTR->FuelChoice = PumpStatus2DP->FuelChoice;

		DOSMutex.Wait(); // Gain DOS window writing premission

		MOVE_CURSOR(ADD2, 3);
		printf("Fuel Choice is: %d\n", Customer2PTR->FuelChoice);
		fflush(stdout);
		MOVE_CURSOR(ADD2, 4);
		printf("Fuel Need is: %d\n", Customer2PTR->FuelNeed);
		fflush(stdout);

		DOSMutex.Signal(); // Release DOS window writing permission

		BOOL j = FALSE;
		if (FuelTankMonitor.readLevel(Customer2PTR->FuelChoice) >= Customer2PTR->FuelNeed)
		{
			

			while (j != TRUE) {
				DOSMutex.Wait(); // Gain DOS window writing premission
				MOVE_CURSOR(ADD2, 12-5);
				printf("Enter P2 to start dispensing fuel  ");
				fflush(stdout);
				MOVE_CURSOR(ADD2, 13-5);
				char temp = getchar();
				if (temp == 'P') {
					char temp2 = getchar();
					if (temp2 == '2') {
						j = TRUE;
						PumpStatus2DP->Dispense = TRUE;
						MOVE_CURSOR(ADD2, 14-5);
						printf("Customer has begun dispensing fuel");
						fflush(stdout);
						MOVE_CURSOR(ADD2, 13-5);
						printf("                       ");
						fflush(stdout);
						temp = FALSE;
						temp2 = FALSE;
						
					}
					else {
						MOVE_CURSOR(ADD2, 12-5);
						printf("     ");
						fflush(stdout);
					}
				}
				else {
					MOVE_CURSOR(ADD2, 12-5);
					printf("     ");
					fflush(stdout);
				}
				DOSMutex.Signal();
			}
		}

		else if (FuelTankMonitor.readLevel(Customer2PTR->FuelChoice) <= Customer2PTR->FuelNeed)
		{
			PumpStatus2DP->Dispense = FALSE;
		}

		

		// READ COMMANDS FROM THE DOS WINDOW FOR REFUEL 2 KEY COMMAND
		// Then signals cs2 so pump can read whether GSC allowed pump to refuel or not
		cs2.Signal();
		SLEEP(200);
		cs2.Wait();
		Customer2PTR->refuelComplete = PumpStatus2DP->refuelComplete;
		if (Customer2PTR->refuelComplete == TRUE) {
			DOSMutex.Wait();
			MOVE_CURSOR(ADD2, 12-5);
			printf("                                 ");
			MOVE_CURSOR(ADD2, 14-5);
			printf("Customer is done                  ");
			fflush(stdout);
			ps2.Signal();
			DOSMutex.Signal();
		}

	}

	return 0;
}

UINT __stdcall Pump3Thread(void *args)
{
	struct Customer3Data *Customer3PTR = new struct Customer3Data;

	CDataPool pumpstatus3("PumpStatus3", sizeof(struct pumpdatapool));
	struct pumpdatapool *PumpStatus3DP = (struct pumpdatapool *)(pumpstatus3.LinkDataPool());

	while (1) {
		// Producer (pump) consumer(gsc) problem
		cs3.Wait(); // Wait for pump to signal cs2 so gsc can consume data


		Customer3PTR->MyCreditNum = PumpStatus3DP->MyCreditNum;
		for (int i = 0; i < 14; i++) {
			Customer3PTR->MyName[i] = PumpStatus3DP->MyName[i];
		}


		// Now print customer data to the DOS window
		DOSMutex.Wait(); // Gain DOS window writing premission

		MOVE_CURSOR(ADD1, YVAL);
		printf("Customer Name: ");
		for (int i = 0; i < 14; i++) {
			printf("%c", Customer3PTR->MyName[i]);
			fflush(stdout);
		}
		MOVE_CURSOR(ADD1, YVAL+1);
		printf("Credit Card Num: %d\n", Customer3PTR->MyCreditNum);
		fflush(stdout);

		DOSMutex.Signal(); // Release DOS window writing permission

		PumpStatus3DP->CreditValid = TRUE;

		cs3.Signal(); // signal producer semphore when done consuming data.

		ps3.Wait(); // gain control of producing 
		// DO SOMETHING TO CHECK IF CREDIT CARD NUMBER IS VALID

		SLEEP(200);

		cs3.Wait();
		Customer3PTR->FuelNeed = PumpStatus3DP->FuelNeed;
		Customer3PTR->FuelChoice = PumpStatus3DP->FuelChoice;

		DOSMutex.Wait(); // Gain DOS window writing premission

		MOVE_CURSOR(ADD1, YVAL+2);
		printf("Fuel Choice is: %d\n", Customer3PTR->FuelChoice);
		fflush(stdout);
		MOVE_CURSOR(ADD1, YVAL+3);
		printf("Fuel Need is: %d\n", Customer3PTR->FuelNeed);
		fflush(stdout);

		DOSMutex.Signal(); // Release DOS window writing permission

		BOOL j = FALSE;

		if (FuelTankMonitor.readLevel(Customer3PTR->FuelChoice) >= Customer3PTR->FuelNeed)
		{
			

			while (j != TRUE) {
				DOSMutex.Wait(); // Gain DOS window writing premission
				MOVE_CURSOR(ADD1, YVAL+7);
				printf("Enter P3 to start dispensing fuel");
				fflush(stdout);
				MOVE_CURSOR(ADD1, YVAL+8);
				char temp = getchar();
				if (temp == 'P') {
					char temp2 = getchar();
					if (temp2 == '3') {
						j = TRUE;
						PumpStatus3DP->Dispense = TRUE;
						MOVE_CURSOR(ADD1, YVAL+9);
						printf("Customer has begun dispensing fuel");
						fflush(stdout);
						MOVE_CURSOR(ADD1, YVAL+8);
						printf("                     ");
						fflush(stdout);
						temp = 0;
						temp2 = 0;
						
					}
					else {
						MOVE_CURSOR(ADD1, YVAL+7);
						printf("     ");
						fflush(stdout);
					}
				}
				else {
					MOVE_CURSOR(ADD1, YVAL+7);
					printf("     ");
					fflush(stdout);
				}
				DOSMutex.Signal();
			}
			
		}

		else if (FuelTankMonitor.readLevel(Customer3PTR->FuelChoice) <= Customer3PTR->FuelNeed)
		{
			PumpStatus3DP->Dispense = FALSE;
		}


		

		// READ COMMANDS FROM THE DOS WINDOW FOR REFUEL 2 KEY COMMAND
		// Then signals cs2 so pump can read whether GSC allowed pump to refuel or not
		cs3.Signal();
		SLEEP(200);
		cs3.Wait();
		Customer3PTR->refuelComplete = PumpStatus3DP->refuelComplete;
		if (Customer3PTR->refuelComplete == TRUE) {
			DOSMutex.Wait();
			MOVE_CURSOR(ADD1, YVAL+7);
			printf("                                 ");
			MOVE_CURSOR(ADD1, YVAL+9);
			printf("Customer is done                     ");
			fflush(stdout);
			ps3.Signal();
			DOSMutex.Signal();
		}

	}

	return 0;
}

UINT __stdcall Pump4Thread(void *args)
{
	struct Customer4Data * Customer4PTR = new struct Customer4Data;
	
	CDataPool pumpstatus4("PumpStatus4", sizeof(struct pumpdatapool));
	struct pumpdatapool *PumpStatus4DP = (struct pumpdatapool *)(pumpstatus4.LinkDataPool());
	
	while (1) {
		// Producer (pump) consumer(gsc) problem
		cs4.Wait(); // Wait for pump to signal cs2 so gsc can consume data


		Customer4PTR->MyCreditNum = PumpStatus4DP->MyCreditNum;
		for (int i = 0; i < 14; i++) {
			Customer4PTR->MyName[i] = PumpStatus4DP->MyName[i];
		}


		// Now print customer data to the DOS window
		DOSMutex.Wait(); // Gain DOS window writing premission

		MOVE_CURSOR(ADD2, YVAL);
		printf("Customer Name: ");
		for (int i = 0; i < 14; i++) {
			printf("%c", Customer4PTR->MyName[i]);
			fflush(stdout);
		}
		MOVE_CURSOR(ADD2, YVAL + 1);
		printf("Credit Card Num: %d\n", Customer4PTR->MyCreditNum);
		fflush(stdout);

		DOSMutex.Signal(); // Release DOS window writing permission

		PumpStatus4DP->CreditValid = TRUE;

		cs4.Signal(); // signal producer semphore when done consuming data.

		ps4.Wait(); // gain control of producing 
		// DO SOMETHING TO CHECK IF CREDIT CARD NUMBER IS VALID

		SLEEP(200);

		cs4.Wait();
		Customer4PTR->FuelNeed = PumpStatus4DP->FuelNeed;
		Customer4PTR->FuelChoice = PumpStatus4DP->FuelChoice;

		DOSMutex.Wait(); // Gain DOS window writing premission

		MOVE_CURSOR(ADD2, YVAL + 2);
		printf("Fuel Choice is: %d\n", Customer4PTR->FuelChoice);
		fflush(stdout);
		MOVE_CURSOR(ADD2, YVAL + 3);
		printf("Fuel Need is: %d\n", Customer4PTR->FuelNeed);
		fflush(stdout);

		DOSMutex.Signal(); // Release DOS window writing permission

		BOOL j = FALSE;

		if (FuelTankMonitor.readLevel(Customer4PTR->FuelChoice) >= Customer4PTR->FuelNeed)
		{


			while (j != TRUE) {
				DOSMutex.Wait(); // Gain DOS window writing premission
				MOVE_CURSOR(ADD2, YVAL + 7);
				printf("Enter P4 to start dispensing fuel    ");
				fflush(stdout);
				MOVE_CURSOR(ADD2, YVAL + 8);
				char temp = getchar();
				if (temp == 'P') {
					char temp2 = getchar();
					if (temp2 == '4') {
						j = TRUE;
						PumpStatus4DP->Dispense = TRUE;
						MOVE_CURSOR(ADD2, YVAL + 9);
						printf("Customer has begun dispensing fuel");
						fflush(stdout);
						MOVE_CURSOR(ADD2, YVAL + 8);
						printf("                     ");
						fflush(stdout);
						temp = 0;
						temp2 = 0;

					}
					else {
						MOVE_CURSOR(ADD2, YVAL + 7);
						printf("     ");
						fflush(stdout);
					}
				}
				else {
					MOVE_CURSOR(ADD2, YVAL + 7);
					printf("     ");
					fflush(stdout);
				}
				DOSMutex.Signal();
			}

		}

		else if (FuelTankMonitor.readLevel(Customer4PTR->FuelChoice) <= Customer4PTR->FuelNeed)
		{
			PumpStatus4DP->Dispense = FALSE;
		}




		// READ COMMANDS FROM THE DOS WINDOW FOR REFUEL 2 KEY COMMAND
		// Then signals cs2 so pump can read whether GSC allowed pump to refuel or not
		cs4.Signal();
		SLEEP(200);
		cs4.Wait();
		Customer4PTR->refuelComplete = PumpStatus4DP->refuelComplete;
		if (Customer4PTR->refuelComplete == TRUE) {
			DOSMutex.Wait();
			MOVE_CURSOR(ADD2, YVAL + 7);
			printf("                                 ");
			MOVE_CURSOR(ADD2, YVAL + 9);
			printf("Customer is done                     ");
			fflush(stdout);
			ps4.Signal();
			DOSMutex.Signal();
		}

	}

	return 0;
}

UINT __stdcall FuelTankThread(void *args)
{
	while (1) {
		DOSMutex.Wait();
		MOVE_CURSOR(0, 17);
		printf("Octane 87 Level: %.3lf", FuelTankMonitor.readLevel(87));
		fflush(stdout);
		MOVE_CURSOR(0, 18);
		printf("Octane 89 Level: %.3lf", FuelTankMonitor.readLevel(89));
		fflush(stdout);
		MOVE_CURSOR(0, 19);
		printf("Octane 91 Level: %.3lf", FuelTankMonitor.readLevel(91));
		fflush(stdout);
		MOVE_CURSOR(0, 20);
		printf("Octane 93 Level: %.3lf", FuelTankMonitor.readLevel(93));
		fflush(stdout);
		DOSMutex.Signal();

		if (FuelTankMonitor.readLevel(87) < 200)
		{
			system("Color 0C");
			SLEEP(300);
			system("Color 0A");
			SLEEP(300);
			system("Color 0C");
			SLEEP(300);
			system("Color 0A");
			SLEEP(300);
			system("Color 0C");
			SLEEP(300);
			system("Color 0A");
			DOSMutex.Wait();
			MOVE_CURSOR(0, 24);
			printf("Octane 87 Fuel Tank Low. Press RF1 to refill Immediately");
			fflush(stdout);
			DOSMutex.Signal();
			BOOL leave = FALSE;
			while (leave == FALSE) {
				DOSMutex.Wait();
				MOVE_CURSOR(0, 25);
				char temp = getchar();
				if (temp == 'R') {
					char temp2 = getchar();
					if (temp2 == 'F') {
						char temp3 = getchar();
						if (temp3 == '1') {
							FuelTankMonitor.refill(87);
							leave = TRUE;
							MOVE_CURSOR(0, 24);
							printf("                                                         ");
							fflush(stdout);
							MOVE_CURSOR(0, 25);
							printf("   ");
							fflush(stdout);
							system("Color 0A");
						}
					}
				}
				DOSMutex.Signal();
			}
			
		}
		if (FuelTankMonitor.readLevel(89) < 200)
		{

			system("Color 0C");
			SLEEP(300);
			system("Color 0A");
			SLEEP(300);
			system("Color 0C");
			SLEEP(300);
			system("Color 0A");
			SLEEP(300);
			system("Color 0C");
			SLEEP(300);
			system("Color 0A");
			DOSMutex.Wait();
			MOVE_CURSOR(0, 26);
			printf("Octane 87 Fuel Tank Low. Press RF2 to refill Immediately");
			fflush(stdout);
			DOSMutex.Signal();
			BOOL leave = FALSE;
			while (leave == FALSE) {
				DOSMutex.Wait();
				MOVE_CURSOR(0, 27);
				char temp = getchar();
				if (temp == 'R') {
					char temp2 = getchar();
					if (temp2 == 'F') {
						char temp3 = getchar();
						if (temp3 == '2') {
							FuelTankMonitor.refill(89);
							leave = TRUE;
							MOVE_CURSOR(0, 26);
							printf("                                                         ");
							fflush(stdout);
							MOVE_CURSOR(0, 27);
							printf("   ");
							fflush(stdout);
							system("Color 0A");
						}
					}
				}
				DOSMutex.Signal();
			}
		}
		if (FuelTankMonitor.readLevel(91) < 200)
		{
			system("Color 0C");
			SLEEP(300);
			system("Color 0A");
			SLEEP(300);
			system("Color 0C");
			SLEEP(300);
			system("Color 0A");
			SLEEP(300);
			system("Color 0C");
			SLEEP(300);
			system("Color 0A");
			DOSMutex.Wait();
			MOVE_CURSOR(0, 28);
			printf("Octane 87 Fuel Tank Low. Press RF3 to refill Immediately");
			fflush(stdout);
			DOSMutex.Signal();
			BOOL leave = FALSE;
			while (leave == FALSE) {
				DOSMutex.Wait();
				MOVE_CURSOR(0, 29);
				char temp = getchar();
				if (temp == 'R') {
					char temp2 = getchar();
					if (temp2 == 'F') {
						char temp3 = getchar();
						if (temp3 == '3') {
							FuelTankMonitor.refill(91);
							leave = TRUE;
							MOVE_CURSOR(0, 28);
							printf("                                                         ");
							fflush(stdout);
							MOVE_CURSOR(0, 29);
							printf("   ");
							fflush(stdout);
							system("Color 0A");
						}
					}
				}
				DOSMutex.Signal();
			}
		}
		if (FuelTankMonitor.readLevel(93) < 200)
		{
			system("Color 0C");
			SLEEP(300);
			system("Color 0A");
			SLEEP(300);
			system("Color 0C");
			SLEEP(300);
			system("Color 0A");
			SLEEP(300);
			system("Color 0C");
			SLEEP(300);
			system("Color 0A");
			system("Color 0C");
			DOSMutex.Wait();
			MOVE_CURSOR(0, 30);
			printf("Octane 87 Fuel Tank Low. Press RF4 to refill Immediately");
			fflush(stdout);
			DOSMutex.Signal();
			BOOL leave = FALSE;
			while (leave == FALSE) {
				DOSMutex.Wait();
				MOVE_CURSOR(0, 31);
				char temp = getchar();
				if (temp == 'R') {
					char temp2 = getchar();
					if (temp2 == 'F') {
						char temp3 = getchar();
						if (temp3 == '4') {
							FuelTankMonitor.refill(93);
							leave = TRUE;
							MOVE_CURSOR(0, 30);
							printf("                                                         ");
							fflush(stdout);
							MOVE_CURSOR(0, 31);
							printf("   ");
							fflush(stdout);
							system("Color 0A");
						}
					}
				}
				DOSMutex.Signal();
			}

		}
		
		
		SLEEP(1000);
	}
	return 0;
}

UINT __stdcall Customer1Thread(void *args)
{

	while (1) {
		if (rand()%20 == 10) {
			Customer1 Cust1;
			Cust1.Resume();
			Cust1.WaitForThread();
		}
	}

		
	return 0;
}

UINT __stdcall Customer2Thread(void *args)
{
	while (1) {
		if (rand() % 20 == 10) {
			Customer2 Cust1;
			Cust1.Resume();
			Cust1.WaitForThread();
		}
	}



	return 0;
}

UINT __stdcall Customer3Thread(void *args)
{

	while (1) {
		if (rand() % 20 == 10) {
			Customer3 Cust1;
			Cust1.Resume();
			Cust1.WaitForThread();
		}
	}

	return 0;
}

UINT __stdcall Customer4Thread(void *args)
{

	while (1) {
		if (rand() % 20 == 10) {
			Customer4 Cust1;
			Cust1.Resume();
			Cust1.WaitForThread();
		}
	}



	return 0;
}


// Main GSC Program
int main(void)
{
	system("Color 0A");
	// need to initialize child threads.
	CThread Pump1Thread(Pump1Thread, ACTIVE, NULL);
	CThread Pump2Thread(Pump2Thread, ACTIVE, NULL);
	CThread Pump3Thread(Pump3Thread, ACTIVE, NULL);
	CThread Pump4Thread(Pump4Thread, ACTIVE, NULL);
	SLEEP(1000);
	CThread FuelTankThread(FuelTankThread, ACTIVE, NULL);
	CThread Customer1Thread(Customer1Thread, ACTIVE, NULL);
	CThread Customer2Thread(Customer2Thread, ACTIVE, NULL);
	CThread Customer3Thread(Customer3Thread, ACTIVE, NULL);
	CThread Customer4Thread(Customer4Thread, ACTIVE, NULL);

	CProcess	PumpProcess("C:\\Users\\rickc\\Desktop\\School\\CPEN333\\Assignment 1\\Assignment_1\\Debug\\PumpProcess.exe", 
		NORMAL_PRIORITY_CLASS, 
		OWN_WINDOW,
		ACTIVE);

	while (1)
	{
		
	}

	Pump1Thread.WaitForThread();
	Pump2Thread.WaitForThread();
	Pump3Thread.WaitForThread();
	Pump4Thread.WaitForThread();
	Customer1Thread.WaitForThread();
	Customer2Thread.WaitForThread();
	Customer3Thread.WaitForThread();
	Customer4Thread.WaitForThread();
	FuelTankThread.WaitForThread();

	
	

}
