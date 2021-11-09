#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>

// KONSTANTE
const int SLOT_TIME = 9; // us
const int SIFS = 10; // us
const int DIFS = 2 * SLOT_TIME + SIFS; // us 
const int FRAME_SIZE = 1040 * 8; // bit
const int TIME_ACK = 50; // us
const int TIME_TO_SEND = 1454; // us, 
const int RETRY_LIMIT = 7;
const int STATION_NUMBER_OF_PACKETS = 200000;

/*
W0 = [0, 31], a ostali prozori su
W1 = [32, 63]
W2 = [64, 127]
W3 = [128, 255]
W4 = [256, 511]
W5 = [512, 1023]
*/

// AKUMULATORI
int slotTimeCounter = 0; 
int mediumBusyCounter = 0;
int droppedPackets = 0;
int transmittedPackets = 0;
int numberOfCollisions = 0;
int slotTimeTotal = 0;
int competitionCounter = 0;
int lastSlotTimeCounterWithCompetition = 0;
int competitionTime = 0;
long long int simulationTime = DIFS;
long long int transmittedDataSize = 0;


// OSTALO
int numberOfStations;
double collisionProbability;
double packetSendProbability;
double throughput; //propusnost
int slotTimeCounterLimit = 199999;

typedef struct _contentionWindow {
	const int shiftedMin;
	const int shiftedMax;
} ContentionWindow;

ContentionWindow CW[] = { {0, 31}, {32, 63}, {64, 127}, {128, 255}, {256, 511}, {512, 1023} };

typedef struct _station {
	char name[20];
	int remainingPackets;
	int CWIndex;
	int backoffTime;
	int collisionCounter;
	int droppedPackets;
} Station;

void generateBackoffTime(Station* station) {
	ContentionWindow cw = CW[station->CWIndex];

	station->backoffTime = (
		(
			(rand() % (cw.shiftedMax + 1 - cw.shiftedMin)) + cw.shiftedMin
		) * SLOT_TIME
	);
}

void createStations(Station* stations) {
	for (int i = 0; i < numberOfStations; i++) {
		sprintf_s(stations[i].name, "%s_%d", "STANICA", i);
		stations[i].collisionCounter = 0;
		stations[i].remainingPackets = STATION_NUMBER_OF_PACKETS;
		stations[i].droppedPackets = 0;
		stations[i].CWIndex = 0;
		generateBackoffTime(&stations[i]);
	}
}

void countZeroBackoffTimes(Station* stations, int* zeroBackoffTimeCounter) {
	for (int i = 0; i < numberOfStations; i++) {
		if (stations[i].backoffTime == 0) {
			(*zeroBackoffTimeCounter)++;
		}
	}
}

void printStationState(Station* station) {
	printf("\n\n -----------");
	printf("\n| %s |\n", station->name);
	printf(" -----------");
	printf("\nBackoff time: %d ", station->backoffTime);
}

int main() {
	srand(time(0));

	printf("\nUnesite broj stanica u mrezi:\n");
	scanf_s("%d", &numberOfStations);

	Station* stations = (Station*)malloc(sizeof(Station) * numberOfStations);
	createStations(stations);

	while (slotTimeCounter < slotTimeCounterLimit) {

		slotTimeCounter++;
		int zeroBackoffTimeCounter = 0;

		countZeroBackoffTimes(stations, &zeroBackoffTimeCounter);

		for (int i = 0; i < numberOfStations; i++) {

			if (stations[i].backoffTime == 0) {
				if (zeroBackoffTimeCounter == 1) {
					stations[i].remainingPackets--;
					simulationTime += TIME_ACK;
					transmittedDataSize += FRAME_SIZE;
					transmittedPackets++;
					stations[i].collisionCounter = 0;
					if (stations[i].CWIndex != 0)
						stations[i].CWIndex--;
				}
				else {
					stations[i].collisionCounter++;
					if (stations[i].collisionCounter >= RETRY_LIMIT) {
						droppedPackets++;
						stations[i].remainingPackets--;
						stations[i].droppedPackets++;
						stations[i].CWIndex = 0;
					}
					else {
						if (stations[i].CWIndex != 5)
							stations[i].CWIndex++;
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
					stations[i].backoffTime -= SLOT_TIME;
				}
				else {
					//medij nije slobodan
				}
			}
		}
		if (zeroBackoffTimeCounter > 0) {
			lastSlotTimeCounterWithCompetition = slotTimeCounter;
			competitionCounter++;
			simulationTime += TIME_TO_SEND + SIFS + DIFS;

			if (zeroBackoffTimeCounter > 1) {
				numberOfCollisions++;
			}
		}
	}

	slotTimeTotal = SLOT_TIME * slotTimeCounter;
	simulationTime += slotTimeTotal;
	collisionProbability = (double)numberOfCollisions / competitionCounter;
	packetSendProbability = 1 - collisionProbability;
	throughput = (double)transmittedDataSize / simulationTime;
	competitionTime = SLOT_TIME * lastSlotTimeCounterWithCompetition;
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
	printf("\nProsjecno trajanje jednog nadmetanja: %3f (ms) ", ((double)competitionTime / 1000) / competitionCounter);
	printf("\nPropusnost: %2f (Mb/s)\n ", throughput);
	printf("\n*********************************************************\n");
}