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
const int TIME_TO_SEND = 1454; //us,
const int CW_MAX = 1024;
const int CW_MIN = 16; // 16,32
const int RETRY_LIMIT = 7;
const int STATION_NUMBER_OF_PACKETS = 100000; 
const int FREEZING_LIMIT = 2; //2,6



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
double throughput; //propusnost
int slotTimeCounterLimit = 99999;

typedef struct _station {
	char name[20];
	int remainingPackets;// broj paketa za svaku stanicu
	int CW; //us, postavljanje contention windowa na minimalnu vrijednost
	int backoffTime; // backoff timer za svaku stanicu
	int collisionCounter; //brojac uzastopnih kolizija
	int freezingCounter; //FC - brojac zamrzavanja, koliko je puta stanica izgubila u natjecanju za medij od posljednje izabranog bc
} Station;

void generateBackoffTime(Station* station) {
	station->backoffTime = ((rand() % station->CW)) * SLOT_TIME;
	station->freezingCounter = 0;
}

void createStations(Station* stations) {
	for (int i = 0; i < numberOfStations; i++) {
		sprintf_s(stations[i].name, "%s_%d", "STANICA", i);
		stations[i].collisionCounter = 0;
		stations[i].remainingPackets = STATION_NUMBER_OF_PACKETS;
		stations[i].CW = CW_MIN;
		generateBackoffTime(&stations[i]);
		stations[i].freezingCounter = 0;
	}
}

void processPacket(Station* station, const char* packetStatus) {
	station->CW = CW_MIN;
	station->remainingPackets--;
}

void countZeroBackoffTimes(Station* stations, int* zeroBackoffTimeCounter) {
	for (int i = 0; i < numberOfStations; i++) {
		if (stations[i].backoffTime == 0) {
			(*zeroBackoffTimeCounter)++;
		}
	}
}

void calculateColisionCW(Station* station) {
	int newCW = (station->CW * 2);

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
						// printf("Dogodila se kolizija\n");

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
						// printBackofTime(stations);
					}
					else {
						stations[i].backoffTime = -1;
					}
				}
				else {
					if (zeroBackoffTimeCounter >= 1) {
						stations[i].freezingCounter++;
						if (stations[i].freezingCounter > FREEZING_LIMIT) {
							generateBackoffTime(&stations[i]);
						}
					}
					else {
						stations[i].backoffTime -= SLOT_TIME;
					}
				}
			}
			if (zeroBackoffTimeCounter > 0) {
				competitionCounter++;
				simulationTime += DIFS + TIME_TO_SEND + SIFS;

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

		printf("\n*** REZULTATI SIMULACIJE ZA CPCF (CWmin = %d, k = %d) ***\n", CW_MIN, FREEZING_LIMIT);
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