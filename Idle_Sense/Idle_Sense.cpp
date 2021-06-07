#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>

// KONSTANTE
const int slotTime = 9; 	// 9us(for 802.11g)
const int SIFS = 10; 	// us, 802.11g
const int DIFS = 2 * slotTime + SIFS; // us 
const int frameSize = 1040 * 8; // bit
const int dataRate = 6 * 1000000; 	// 6Mbps for 802.11g
const int timeACK = 50; // (us)
const int timeToSend = 1454; //us, 802.11g

const int CWmax = 1023 * slotTime; 	// us
const int CWmin = 15 * slotTime; 	// us
const int retryLimit = 7;
const int stationNumberOfPackets = 100;// 100000;

// AKUMULATORI
int slotTimeCounter = 0; //broj nadmetanja
int numberOfPacketsOnNetwork = 0;
int droppedPackets = 0;
int transmittedPackets = 0;
int numberOfCollisions = 0;
int competitionTime = 0;//trajanje nadmetanja
int competitionCounter = 0;
long long int simulationTime = DIFS;
long long int transmittedDataSize = 0;

// OSTALO
int numberOfStations;	// broj stanica u mrezi (networkSize x networkSize)
double collisionProbability;
double packetSendProbability;
double throughput; //propusnost

// IDLE SENSE
int numberOfConsecutiveIdleSlots = 0;
int slotTimeCounterPrevious = 0;

int maxtrans = 2 * numberOfStations;
int sum = 0;
int ntrans = 0;
double nTarget = 3.91; // 802.11g
double alpha = 1 / 1.0666;
double beta = 0.75;
double gamma = 4;
double epsilon = 6.0;

typedef struct _station {
	char name[20];
	int remainingPackets;// broj paketa za svaku stanicu
	int CW; //us, postavljanje contention windowa na minimalnu vrijednost
	int backoffTime; // backoff timer za svaku stanicu
	int collisionCounter; //brojac uzastopnih kolizija
	int n;// broj uzastopnih idle slotova između dva pokusaja prijenosa
	int nAvg; 

} Station;

void generateBackoffTime(Station* station) {
	int backoffTime = -1;

	do {
		backoffTime = ((rand() % station->CW)) * slotTime;
	} while (backoffTime == 0);

	station->backoffTime = backoffTime;
}

void createStations(Station* stations) {
	for (int i = 0; i < numberOfStations; i++) {
		sprintf_s(stations[i].name, "%s_%d", "STANICA", i);
		stations[i].collisionCounter = 0;
		stations[i].remainingPackets = stationNumberOfPackets;
		stations[i].CW = CWmin;
		generateBackoffTime(&stations[i]);
		numberOfPacketsOnNetwork = numberOfPacketsOnNetwork + stations[i].remainingPackets;
	}
}

void processPacket(Station* station, const char* packetStatus) {
	station->remainingPackets--;
	numberOfPacketsOnNetwork--;

	/* printf("\nPAKET %s ", packetStatus);
	printf("\nPreostali broj paketa: %d\n ", station->remainingPackets);
	printf("\nPreostalo %d paketa na mrezi ", numberOfPacketsOnNetwork); */
}

void decrementBackoffTimes(Station* stations) {
	for (int i = 0; i < numberOfStations; i++) {
		if (stations[i].backoffTime != -1) {
			stations[i].backoffTime -= slotTime;
		}
	}
}

void countZeroBackoffTimes(Station* stations, int* zeroBackoffTimeCounter) {
	for (int i = 0; i < numberOfStations; i++) {
		if (stations[i].backoffTime == 0) {
			(*zeroBackoffTimeCounter)++;
		}
	}
}

void calculateCW(Station* station, int numberOfConsecutiveIdleSlots) {
	station->n = numberOfConsecutiveIdleSlots;
	sum += station->n;
	ntrans += 1;

	if (ntrans >= maxtrans) {
		station->nAvg = sum / ntrans;
		sum = 0;
		ntrans = 0;

		if (station->nAvg < nTarget) {
			station->CW += epsilon;
		}
		else {
			station->CW = station->CW * alpha;
		}

		if (abs(nTarget - station->nAvg) < beta) {
			maxtrans = station->CW / gamma;
		}
		else {
			maxtrans = 2 * numberOfStations; //JEDNAK BROJU STANICA
		}
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

	printf("\nUnesite broj stanica u mrezi :\n");
	scanf_s("%d", &numberOfStations);

	printf("\nUkupan broj paketa na mrezi: %d\n ", numberOfStations * stationNumberOfPackets);

	Station* stations = (Station*)malloc(sizeof(Station) * numberOfStations);
	createStations(stations);

	while (numberOfPacketsOnNetwork > 0) {

		slotTimeCounter++;
		int zeroBackoffTimeCounter = 0;

		// printf("\n---------------------TIMESLOT %d--------------------", slotTimeCounter);

		decrementBackoffTimes(stations);
		countZeroBackoffTimes(stations, &zeroBackoffTimeCounter);

		for (int i = 0; i < numberOfStations; i++) {
			numberOfConsecutiveIdleSlots = 0;

			if (stations[i].backoffTime == 0) {
				numberOfConsecutiveIdleSlots = slotTimeCounter - slotTimeCounterPrevious;
				slotTimeCounterPrevious = slotTimeCounter;
				// printStationState(&stations[i]);
				if (zeroBackoffTimeCounter == 1) {
					processPacket(&stations[i], "POSLAN");
					simulationTime += (timeToSend + SIFS + timeACK);
					transmittedDataSize += frameSize;
					transmittedPackets++;
					stations[i].collisionCounter = 0;
				}
				else {
					stations[i].collisionCounter++;
					// printf("Dogodila se kolizija\n");

					if (stations[i].collisionCounter >= retryLimit) {
						droppedPackets++;
						processPacket(&stations[i], "ODBACEN");
					}
					else {
						//kolizija
					}
				}
				calculateCW(&stations[i], numberOfConsecutiveIdleSlots);

				if (stations[i].remainingPackets > 0) {
					generateBackoffTime(&stations[i]);
					// printBackofTime(stations);

				}
				else {
					stations[i].backoffTime = -1;
				}
			}
		}
		if (zeroBackoffTimeCounter > 0) {
			competitionCounter++;
			simulationTime += DIFS;

			if (zeroBackoffTimeCounter > 1) {
				numberOfCollisions++;
			}
		}
	}

	competitionTime = slotTime * slotTimeCounter;
	simulationTime += (slotTime * slotTimeCounter);

	collisionProbability = (double)numberOfCollisions / competitionCounter;
	packetSendProbability = 1 - collisionProbability;
	throughput = (double)transmittedDataSize / simulationTime;

	printf("\n\n********** REZULTATI SIMULACIJE IDLE SENSE **********\n");
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
}