#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>

// KONSTANTE
const int SLOT_TIME = 9; 	// 9us
const int SIFS = 10; 	// us
const int DIFS = 2 * SLOT_TIME + SIFS; // us 
const int FRAME_SIZE = 1040 * 8; // bit
//const int dataRate = 6 * 1000000; 	// 6Mbps
const int TIME_ACK = 50; // (us)
const int TIME_TO_SEND = 1454; //us, 
const int CW_MAX = 1023;
const int CW_MIN = 15; //MIJENJA SE ZA TESTIRANJE
const int RETRY_LIMIT = 7;
const int STATION_NUMBER_OF_PACKETS = 100000;

// AKUMULATORI
int slotTimeCounter = 0; //broj nadmetanja
int droppedPackets = 0;
int transmittedPackets = 0;
int numberOfCollisions = 0;
int competitionTime = 0; //trajanje nadmetanja
int competitionCounter = 0;
long long int simulationTime = DIFS;
long long int transmittedDataSize = 0;

// OSTALO
int numberOfStations;	
double collisionProbability;
double packetSendProbability;
double throughput;
int slotTimeCounterLimit = 99999;

typedef struct _station {
	char name[20];
	int remainingPackets;
	int CW; 
	int backoffTime; 
	int collisionCounter; 
} Station;

void generateBackoffTime(Station* station) {
	station->backoffTime = ((rand() % station->CW)) * SLOT_TIME;
}
	
void createStations(Station* stations) {
	for (int i = 0; i < numberOfStations; i++) {
		sprintf_s(stations[i].name, "%s_%d", "STANICA", i);
		stations[i].collisionCounter = 0;
		stations[i].remainingPackets = STATION_NUMBER_OF_PACKETS;
		stations[i].CW = CW_MIN;
		generateBackoffTime(&stations[i]);
	}
}

void processPacket(Station* station, const char* packetStatus) {
	station->CW = CW_MIN;
	station->remainingPackets--;
	/* printf("\nPAKET %s ", packetStatus);
	printf("\nPreostali broj paketa: %d\n ", station->remainingPackets);
	printf("\nPreostalo %d paketa na mrezi ", numberOfPacketsOnNetwork); */
}

void countZeroBackoffTimes(Station* stations, int* zeroBackoffTimeCounter) {
	for (int i = 0; i < numberOfStations; i++) {
		if (stations[i].backoffTime == 0) {
			(*zeroBackoffTimeCounter)++;
		}
	}
}

void calculateColisionCW(Station* station) {
	int newCW = (station->CW * 2) + 1;

	if (newCW >= CW_MAX) {
		station->CW = CW_MAX;
	}
	else {
		station->CW = newCW;
	}
}

void printStationState(Station* station) {
	printf("\n\n -----------");
	printf("\n| %s |\n", station->name);
	printf(" -----------");
	printf("\nBackoff time: %d ", station->backoffTime);
}

void printBackofTime(Station* stations) {
	printf("\nBACKOFF VREMENA:\n");
	for (int i = 0; i < numberOfStations; i++) {
		printf("\n%s backofftime : %d\n ", stations[i].name, stations[i].backoffTime);
	}
}

void remainingNumberOfPackets(Station* stations) {
	printf("\nPreostali broj paketa po stanicama:\n");
	for (int i = 0; i < numberOfStations; i++) {
		printf("\n%s Preostalo %d paketa\n ", stations[i].name, stations[i].remainingPackets);
	}
}

int main() {
	srand(time(0));

	printf("\nUnesite broj stanica u mrezi:\n");
	scanf_s("%d", &numberOfStations);

	//printf("\nUnesite timeSlotCounter limit:\n");
	//scanf_s("%d", &slotTimeCounterLimit);

	Station* stations = (Station*)malloc(sizeof(Station) * numberOfStations);
	createStations(stations);
	
	while (slotTimeCounter < slotTimeCounterLimit) {

		slotTimeCounter++;
		int zeroBackoffTimeCounter = 0;

		countZeroBackoffTimes(stations, &zeroBackoffTimeCounter);

		for (int i = 0; i < numberOfStations; i++) {

			if (stations[i].backoffTime == 0) {
				// printStationState(&stations[i]);
				if (zeroBackoffTimeCounter == 1) {
					processPacket(&stations[i], "POSLAN");
					simulationTime += TIME_ACK;
					transmittedDataSize += FRAME_SIZE;
					transmittedPackets++;
					stations[i].collisionCounter = 0;
				}
				else {
					stations[i].collisionCounter++;
					
					if (stations[i].collisionCounter >= RETRY_LIMIT) {
						droppedPackets++;
						processPacket(&stations[i], "ODBACEN");
					}
					else {
						calculateColisionCW(&stations[i]);
					}
				}
				if (stations[i].remainingPackets > 0) {
					generateBackoffTime(&stations[i]);
				}
				else {
					stations[i].backoffTime = -1;
				}
			}
			else {
				if (zeroBackoffTimeCounter == 0) {
					//medij je slobodan
					stations[i].backoffTime -= SLOT_TIME;
				}
				else {
					//medij nije slobodan
				}
			}
		}
		if (zeroBackoffTimeCounter > 0) {
			competitionCounter++;
			simulationTime += TIME_TO_SEND + SIFS + DIFS;

			if (zeroBackoffTimeCounter > 1) {
				numberOfCollisions++;
			}
		}
	}

	competitionTime = SLOT_TIME * slotTimeCounter;
	simulationTime += competitionTime;

	collisionProbability = (double)numberOfCollisions / competitionCounter;
	packetSendProbability = 1 - collisionProbability;
	throughput = (double)transmittedDataSize / simulationTime;

	printf("\n********** REZULTATI SIMULACIJE ZA DCF 802.11 **********\n");
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