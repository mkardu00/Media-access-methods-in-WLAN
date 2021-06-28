#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>

// KONSTANTE
const int slotTime = 9; 	// 9us
const int SIFS = 10; 	// us
const int DIFS = 2 * slotTime + SIFS; // us 
const int frameSize = 1040 * 8; // bit
const int dataRate = 6 * 1000000; 	// 6Mbps
const int timeACK = 50; // (us)
const int timeToSend = 1454; //us, 

//const int CWmax = 1023;
//const int CWmin = 15;
const int retryLimit = 7;
const int stationNumberOfPackets = 100000;
int HIGH_PRIORITY_PACKETS = 50000;
int LOW_PRIORITY_PACKETS = 50000;
int CW_MIN_HIGH_PRIORITY = 0;
int CW_MAX_HIGH_PRIORITY = 256;
int CW_MIN_LOW_PRIORITY = 128;
int CW_MAX_LOW_PRIORITY = 1024;
int HIGH_PRIORITY = 1;
int LOW_PRIORITY = 2;
int SF_HIGH_PRIORITY = 16;
int SF_LOW_PRIORITY = 128;
int SCW_SIZE_HIGH_PRIORITY = 32;
int SCW_SIZE_LOW_PRIORITY = 256;

int ALPHA_HIGH_PRIORITY = 5;
int ALPHA_LOW_PRIORITY = 99999;//ne postoji oganicenje

double NETWORK_LOAD_TRESHOLD = 0.7;
double NETWORK_LOAD_SATURATION = 0.9;

// AKUMULATORI
int slotTimeCounter = 0; //broj nadmetanja
int mediumBusyCounter = 0;

int droppedPackets = 0;
int transmittedPackets = 0;
int numberOfCollisions = 0;
int competitionTime = 0;//trajanje nadmetanja
int competitionCounter = 0;
long long int simulationTime = DIFS;
long long int transmittedDataSize = 0;

// OSTALO
int numberOfStations;
double collisionProbability;
double packetSendProbability;
double throughput; //propusnost
int slotTimeCounterLimit;

typedef struct _station {
	char name[20];
	int remainingHighPriorityPackets;
	int remainingLowPriorityPackets;
	int CWHighPriorityLB;
	int CWHighPriorityUB;
	int CWLowPriorityLB;
	int CWLowPriorityUB;
	int backoffTimeHighPriority;
	int backoffTimeLowPriority;
	int collisionCounter;
	int droppedPackets;

	
	
} Station;


void generateBackoffTime(Station* station, int priority) {
	if (priority == HIGH_PRIORITY) {
		station->backoffTimeHighPriority = (((rand() % (station->CWHighPriorityUB + 1 - station->CWHighPriorityLB)) + station->CWHighPriorityLB) * slotTime);
	}
	else if (priority == LOW_PRIORITY) {
		station->backoffTimeLowPriority = (((rand() % (station->CWLowPriorityUB + 1 - station->CWLowPriorityLB)) + station->CWLowPriorityLB) * slotTime);
	}
	
}

void createStations(Station* stations) {
	for (int i = 0; i < numberOfStations; i++) {
		sprintf_s(stations[i].name, "%s_%d", "STANICA", i);
		stations[i].collisionCounter = 0;
		stations[i].remainingHighPriorityPackets = HIGH_PRIORITY_PACKETS;
		stations[i].remainingLowPriorityPackets = LOW_PRIORITY_PACKETS;
		stations[i].droppedPackets = 0;

		stations[i].CWHighPriorityLB = CW_MIN_HIGH_PRIORITY;
		stations[i].CWHighPriorityUB = CW_MIN_HIGH_PRIORITY + 2 * SF_HIGH_PRIORITY;
		stations[i].CWLowPriorityLB = CW_MIN_LOW_PRIORITY;
		stations[i].CWLowPriorityUB = CW_MIN_LOW_PRIORITY + 2 * SF_LOW_PRIORITY;

		generateBackoffTime(&stations[i], HIGH_PRIORITY);
		generateBackoffTime(&stations[i], LOW_PRIORITY);
		
	}
}

void processPacket(Station* station, const char* packetStatus, int priority) {
	if (priority == HIGH_PRIORITY) {
		station->remainingHighPriorityPackets--;
	}
	else if (priority == LOW_PRIORITY) {
		station->remainingLowPriorityPackets--;
	}

	/* printf("\nPAKET %s ", packetStatus);
	printf("\nPreostali broj paketa: %d\n ", station->remainingPackets);
	printf("\nPreostalo %d paketa na mrezi ", numberOfPacketsOnNetwork); */
}

void countZeroBackoffTimes(Station* stations, int* zeroBackoffTimeCounter) {
	for (int i = 0; i < numberOfStations; i++) {
		if (stations[i].backoffTimeHighPriority == 0 ) {
			(*zeroBackoffTimeCounter)++;
		}
		if (stations[i].backoffTimeLowPriority == 0) {
			(*zeroBackoffTimeCounter)++;
		}
	}
}

void SCWDecreasingProcedureHighPriority(Station* station){
	if (station->CWHighPriorityLB - SF_HIGH_PRIORITY >= CW_MIN_HIGH_PRIORITY) {
		station->CWHighPriorityLB = station->CWHighPriorityLB - SF_HIGH_PRIORITY;
		station->CWHighPriorityUB = station->CWHighPriorityUB - SF_HIGH_PRIORITY;
	}
	else { 
		station->CWHighPriorityLB = CW_MIN_HIGH_PRIORITY;
		station->CWHighPriorityUB = CW_MIN_HIGH_PRIORITY + SCW_SIZE_HIGH_PRIORITY;
	}
}

void SCWIncreasingProcedureHighPriority(Station* station){
	if (station->CWHighPriorityUB + SF_HIGH_PRIORITY <= CW_MAX_HIGH_PRIORITY) {
		station->CWHighPriorityLB = station->CWHighPriorityLB + SF_HIGH_PRIORITY;
		station->CWHighPriorityUB = station->CWHighPriorityUB + SF_HIGH_PRIORITY;
	}
	else {
		station->CWHighPriorityLB = CW_MAX_HIGH_PRIORITY - SCW_SIZE_HIGH_PRIORITY;
		station->CWHighPriorityUB = CW_MAX_HIGH_PRIORITY;
	}
}

void SCWDecreasingProcedureLowPriority(Station* station){
	if (station->CWLowPriorityLB - SF_LOW_PRIORITY >= CW_MIN_LOW_PRIORITY) {
		station->CWLowPriorityLB = station->CWLowPriorityLB - SF_LOW_PRIORITY;
		station->CWLowPriorityUB = station->CWLowPriorityUB - SF_LOW_PRIORITY;
	}
	else {
		station->CWLowPriorityLB = CW_MIN_LOW_PRIORITY;
		station->CWLowPriorityUB = CW_MIN_LOW_PRIORITY + SCW_SIZE_LOW_PRIORITY;
	}
}

void SCWIncreasingProcedureLowPriority(Station* station){
	if (station->CWLowPriorityUB + SF_HIGH_PRIORITY <= CW_MAX_LOW_PRIORITY) {
		station->CWLowPriorityLB = station->CWLowPriorityLB + SF_LOW_PRIORITY;
		station->CWLowPriorityUB = station->CWLowPriorityUB + SF_LOW_PRIORITY;
	}
	else {
		station->CWLowPriorityLB = CW_MAX_LOW_PRIORITY - SCW_SIZE_LOW_PRIORITY;
		station->CWLowPriorityUB = CW_MAX_LOW_PRIORITY;
	}
}

void calculateSCWHighPriority(Station* station) {
	double lossRate = (double) station->droppedPackets / (HIGH_PRIORITY_PACKETS - station->remainingHighPriorityPackets);
	if (lossRate >= ALPHA_HIGH_PRIORITY) {
		SCWDecreasingProcedureHighPriority(station);
	}
	else if (lossRate <= (double) ALPHA_HIGH_PRIORITY / 2) {
		SCWIncreasingProcedureHighPriority(station);
	}

}

void calculateSCWLowPriority(Station* station) {
	double networkLoad = (double) mediumBusyCounter / slotTimeCounter; // B(T)

	if (networkLoad <= NETWORK_LOAD_SATURATION) {
		SCWDecreasingProcedureLowPriority(station);
	}
	else if (networkLoad  >= NETWORK_LOAD_SATURATION) {
		SCWIncreasingProcedureLowPriority(station);
	}

}

void printStationState(Station* station) {
	printf("\n\n -----------");
	printf("\n| %s |\n", station->name);
	printf(" -----------");
	printf("\nBackoff time HP: %d ", station->backoffTimeHighPriority);
	printf("\nBackoff time LP: %d ", station->backoffTimeLowPriority);
}

void printBackofTime(Station* stations) {
	printf("\nBACKOFF VREMENA:\n");
	for (int i = 0; i < numberOfStations; i++) {
		printf("\n%s backofftime HP : %d\n ", stations[i].name, stations[i].backoffTimeHighPriority);
		printf("\n%s backofftime LP : %d\n ", stations[i].name, stations[i].backoffTimeLowPriority);
	}
}

void remainingNumberOfPackets(Station* stations) {
	printf("\nPreostali broj paketa po stanicama:\n");
	for (int i = 0; i < numberOfStations; i++) {
		printf("\n%s Preostalo %d HP paketa\n ", stations[i].name, stations[i].remainingHighPriorityPackets);
		printf("\n%s Preostalo %d LP paketa\n ", stations[i].name, stations[i].remainingLowPriorityPackets);
	}
}

int main() {
	srand(time(0));

	printf("\nUnesite broj stanica u mrezi:\n");
	scanf_s("%d", &numberOfStations);

	printf("\nUnesite timeSlotCounter limit:\n");
	scanf_s("%d", &slotTimeCounterLimit);

	Station* stations = (Station*)malloc(sizeof(Station) * numberOfStations);
	createStations(stations);
	

	while (slotTimeCounter < slotTimeCounterLimit) {

		slotTimeCounter++;
		int zeroBackoffTimeCounter = 0;

		countZeroBackoffTimes(stations, &zeroBackoffTimeCounter);

		for (int i = 0; i < numberOfStations; i++) {
			int priority = 0;
			
			if (stations[i].backoffTimeHighPriority == 0 || stations[i].backoffTimeLowPriority == 0) {
				if (stations[i].backoffTimeHighPriority == 0) {
					priority = HIGH_PRIORITY;
				}
				else {
					priority = LOW_PRIORITY;
				}
				// printStationState(&stations[i]);
				if (zeroBackoffTimeCounter == 1) {
					processPacket(&stations[i], "POSLAN", priority);
					simulationTime += timeACK;
					transmittedDataSize += frameSize;
					transmittedPackets++;
					stations[i].collisionCounter = 0;
				}
				else {
					stations[i].collisionCounter++;
					//CalculateLr() aka Loss Rate;
					// printf("Dogodila se kolizija\n");

					if (stations[i].collisionCounter >= retryLimit) {
						droppedPackets++;
						processPacket(&stations[i], "ODBACEN", priority);
						stations[i].droppedPackets++;
					}
				}
				if (priority == HIGH_PRIORITY) {
					calculateSCWHighPriority(&stations[i]);
					if (stations[i].remainingHighPriorityPackets > 0) {
						generateBackoffTime(&stations[i], priority);
					
					}
					else {
						stations[i].backoffTimeHighPriority = -1;
					}
				}
				else if (priority == LOW_PRIORITY){
					calculateSCWLowPriority(&stations[i]);
					if (stations[i].remainingLowPriorityPackets > 0) {
						generateBackoffTime(&stations[i], priority);
					}
					else {
						stations[i].backoffTimeLowPriority = -1;
					}
				}	
			}
			else {
				if (zeroBackoffTimeCounter == 0) {//medij je slobodan
					stations[i].backoffTimeHighPriority -= slotTime;
					stations[i].backoffTimeLowPriority -= slotTime;
				}
				else {
					//medij nije slobodan
					mediumBusyCounter++;
				}
			}
		}
		if (zeroBackoffTimeCounter > 0) {
			competitionCounter++;
			simulationTime += timeToSend + SIFS + DIFS;

			if (zeroBackoffTimeCounter > 1) {
				numberOfCollisions++;
			}
		}
	}

	competitionTime = slotTime * slotTimeCounter;
	simulationTime += competitionTime;

	collisionProbability = (double)numberOfCollisions / competitionCounter;
	packetSendProbability = 1 - collisionProbability;
	throughput = (double)transmittedDataSize / simulationTime;

	printf("\n********** REZULTATI SIMULACIJE ZA SCW **********\n");
	printf("\nBroj uspjesno poslanih paketa: %d ", transmittedPackets);
	printf("\nBroj odbacenih paketa: %d ", droppedPackets);
	printf("\nBroj kolizija: %d ", numberOfCollisions);
	printf("\nBroj nadmetanja: %d ", competitionCounter);
	printf("\nVrijeme nadmetanja: %2f (s) ", (double)competitionTime / 1000000);
	printf("\nVrijeme trajanja simulacije: %2f (s)", (double)simulationTime / 1000000);
	printf("\n");
	printf("\nVjerojatnost kolizije: %2f ", collisionProbability);
	printf("\nVjerojatnost uspjesnog slanja: %2f ", packetSendProbability);
	printf("\n\nUkupna velicina poslanih podataka: %2f (Mb) ", (double)transmittedDataSize / 1000000);
	printf("\nPropusnost: %2f (Mb/s)\n ", throughput);
	printf("\n*********************************************************\n");
}