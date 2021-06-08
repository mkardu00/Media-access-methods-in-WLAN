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
const int CWmin = 16 * slotTime; 	// us //to mj je W0
const int retryLimit = 7;
const int stationNumberOfPackets = 100; // 100000;

const int freezingLimit = 3; //k - granica zamrzavanja
const int FCgr = 4;
const int CWshifted = 32 * slotTime; 	// us //W1-velicina pomoknutog prozora
const int CWshiftedMIN = 8 * slotTime; //Wmin - donja granica pomaknutig prozora


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
int numberOfActiveStations; // broj aktivnih stanica, potrebo da bi se detktirala zagusenost mreze


typedef struct _station {
	char name[20];
	int remainingPackets;// broj paketa za svaku stanicu
	int CW; //us, postavljanje contention windowa na minimalnu vrijednost
	int backoffTime; // backoff timer za svaku stanicu
	int collisionCounter; //brojac uzastopnih kolizija
	int congestion; // detektiranje zagusenja
	int freezingCounter; //FC - brojac zamrzavanja, koliko je puta stanica izgubila u natjecanju za medij od posljednje izabranog bc
} Station;

void generateBackoffTime(Station* station) {
	int backoffTime = -1;

	do {
		backoffTime = ((rand() % (station->CW - 1))) * slotTime;
	} while (backoffTime == 0);
	station->backoffTime = backoffTime;
	station->freezingCounter = 0;
	if (station->congestion == 1) {
		station->backoffTime += CWshiftedMIN;
	}
	station->congestion = 0;

}

void createStations(Station* stations) {
	for (int i = 0; i < numberOfStations; i++) {
		sprintf_s(stations[i].name, "%s_%d", "STANICA", i);
		stations[i].collisionCounter = 0;
		stations[i].remainingPackets = stationNumberOfPackets;
		stations[i].CW = CWmin;
		generateBackoffTime(&stations[i]);
		stations[i].freezingCounter = 0;
		stations[i].congestion = 0;
		numberOfPacketsOnNetwork = numberOfPacketsOnNetwork + stations[i].remainingPackets;
		
	}
}

void processPacket(Station* station, const char* packetStatus) {
	station->CW = CWmin;
	station->remainingPackets--;
	numberOfPacketsOnNetwork--;
	if (station->remainingPackets == 0) {
		numberOfActiveStations--;
	}
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

void calculateColisionCW(Station* station) {
	int newCW = (station->CW * 2);

	if (newCW >= CWmax) {
		station->CW = CWmax;
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

void incrementFreezingCounter(Station* stations, Station* station) {
	for (int i = 0; i < numberOfStations; i++) {
		if (stations[i].name != station->name) {
			stations[i].freezingCounter++;
			
			if (stations[i].freezingCounter > freezingLimit) {
				station[i].CW = CWshifted;
				station[i].congestion = 1;
				generateBackoffTime(&stations[i]);
			}
		}
	}
}

void checkCongestionForWinningStation(Station* station){
	if (station->freezingCounter >= FCgr) {
		station->CW = CWshifted;
		station->congestion = 1;
	}
	else {
		station->CW = CWmin;
		station->congestion = 0;
	}

}

int main() {
	srand(time(0));

	printf("\nUnesite broj stanica u mrezi:\n");
	scanf_s("%d", &numberOfStations);
	numberOfActiveStations = numberOfStations;

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

			if (stations[i].backoffTime == 0) {
				// printStationState(&stations[i]);
				if (zeroBackoffTimeCounter == 1) {
					processPacket(&stations[i], "POSLAN");
					checkCongestionForWinningStation(&stations[i]);
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
				//implementacija dijela za cpcf
				incrementFreezingCounter(stations, &stations[i]);
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

	printf("\n********** REZULTATI SIMULACIJE ZA SaMAC **********\n");
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