#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>

// KONSTANTE
const int slotTime = 9; 	// 20us(for 802.11b); 9us(for 802.11g)
const int SIFS = 10; 	// us, 802.11g
const int DIFS = 2 * slotTime + SIFS; // us 
const int maxFrameSize = 1040; // bytes//1040
const int dataRate = 54 * 1000000; 	// 11Mbps for 802.11b; 54Mbps for 802.11g
const int ACK = 15 * 8;			//bit
const int timeACK = (ACK/dataRate) * 1000000; // (us)
const int timeToSend = 1454; //us, 802.11g
const int CWmax = 1023 * slotTime; 	// us
const int CWmin = 15 * slotTime; 	// us
const int retryLimit = 7;
const int stationNumberOfPackets = 2;

// AKUMULATORI
int slotTimeCounter = 0; //broj nadmetanja
int numberOfPacketsOnNetwork = 0;
int droppedPackets = 0;
int transmittedPackets = 0;
int numberOfCollisions = 0;
int competitionTime = 0;//trajanje nadmetanja
int competitionCounter = 0;
int simulationTime = DIFS;

// OSTALO
int numberOfStations;	// broj stanica u mrezi (networkSize x networkSize)
double collisionProbability;
double packetSendProbability;


typedef struct _station {
	char name[20];
	int remainingPackets;// broj paketa za svaku stanicu
	int CW; //us, postavljanje contention windowa na minimalnu vrijednost
	int backoffTime; // backoff timer za svaku stanicu
	int collisionCounter; //brojac uzastopnih kolizija
} Station;

void generateBackoffTime(Station* station) {

	station->backoffTime = ((rand() % station->CW)) * slotTime; //provjerit je li se ovako racuna!!!!
	printf("\n%s backofftime : %d\n ", station->name, station->backoffTime);
}

void createStations(Station* stations) {
	//postavljanje pocetnih vrijednosti za svaku stanicu
	for (int i = 0; i < numberOfStations; i++) {
		sprintf_s(stations[i].name, "%s_%d", "STANICA", i);
		stations[i].collisionCounter = 0; //brojac kolizija
		stations[i].remainingPackets = stationNumberOfPackets; //broj paketa za stanicu
		stations[i].CW = CWmin;
		generateBackoffTime(&stations[i]);
		numberOfPacketsOnNetwork = numberOfPacketsOnNetwork + stations[i].remainingPackets;
	}
}

void processPacket(Station* station, const char* packetStatus) {
	station->CW = CWmin;
	station->remainingPackets--;
	numberOfPacketsOnNetwork--;

	printf("\nPAKET %s ", packetStatus);
	printf("\nPreostali broj paketa: %d\n ", station->remainingPackets);
	printf("\nPreostalo %d paketa na mrezi ", numberOfPacketsOnNetwork);
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
	int newCW = (station->CW * 2) + 1;

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

int main() {
	srand(time(0));

	printf("\nUnesite broj stanica u mrezi (4, 9, 25, 49 ili 100):\n");
	scanf_s("%d", &numberOfStations);

	printf("\nUkupan broj paketa na mrezi: %d\n ", numberOfStations * stationNumberOfPackets);
	
	Station* stations = (Station*) malloc(sizeof(Station) * numberOfStations);
	createStations(stations);
	
	while (numberOfPacketsOnNetwork > 0){
		
		slotTimeCounter++;
		int zeroBackoffTimeCounter = 0;

		printf("\n---------------------TIMESLOT %d--------------------", slotTimeCounter);

		decrementBackoffTimes(stations);
		countZeroBackoffTimes(stations, &zeroBackoffTimeCounter);

		for (int i = 0; i < numberOfStations; i++) {
			
			if (stations[i].backoffTime == 0 ) {
				printStationState(&stations[i]);
				if (zeroBackoffTimeCounter == 1) {
					processPacket(&stations[i], "POSLAN");
					simulationTime += (timeToSend + SIFS + timeACK);
					transmittedPackets++;
					stations[i].collisionCounter = 0;
				}
				else {
					stations[i].collisionCounter++;
					printf("Dogodila se kolizija\n");

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
					printBackofTime(stations);

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

	collisionProbability = double(numberOfCollisions / competitionCounter);
	packetSendProbability = 1 - collisionProbability;

	printf("\n\n\nBroj uspjesno poslanih paketa: %d ", transmittedPackets);
	printf("\nBroj odbacenih paketa: %d ", droppedPackets);
	printf("\nBroj kolizija: %d ", numberOfCollisions);
	printf("\nBroj nadmetanja: %d ", competitionCounter);
	printf("\nVrijeme nadmetanja: %d (us) ", competitionTime);
	printf("\nVrijeme trajanja simulacije: %d (us) \n", simulationTime);
	printf("\n\n");
	printf("\nVjerojatnost kolizije: %2f ", collisionProbability);
	printf("\nVjerojatnost uspjesnog slanja: %2f ", packetSendProbability);
	
}