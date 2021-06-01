#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>

int numberOfStations;	// broj stanica u mrezi (networkSize x networkSize)
int slotTime = 9; 	// 20us(for 802.11b); 9us(for 802.11g)
int dataRate = 54; 	// 11Mbps for 802.11b; 54Mbps for 802.11g
int SIFS = 10; 	// us
int DIFS = 2 * slotTime + SIFS; // us 
int maxFrameSize = 1500; // bytes
int ACK = 15;			//bytes
int timeToSend = 10; //us 


int CWmax = 1023; // * slotTime; 	// us
int CWmin = 15; // *slotTime; 	// us---zasto množi s time slotom

int numberOfPacketsOnNetwork = 0;
int retryLimit = 4;

typedef struct _station {
	char name[20];
	int remainingPackets;// broj paketa za svaku stanicu
	int CW; //us, postavljanje contention windowa na minimalnu vrijednost
	double backoffTime; // backoff timer za svaku stanicu
	int counterOfCollision; //brojac uzastopnih kolizija
	bool iteratonCollision; //provjerava koliziju za svaku iteraciju
	double elapsedTime; //us, koliko je vremena proslo

} Station;

void generateBackoffTime(Station* station) {

	station->backoffTime = ((rand() % station->CW)) * slotTime; //provjerit je li se ovako racuna!!!!
}

void createStations(Station* stations) {
	//postavljanje pocetnih vrijednosti za svaku stanicu
	for (int i = 0; i < numberOfStations; i++) {
		sprintf_s(stations[i].name, "%s_%d", "STANICA", i);
		stations[i].counterOfCollision = 0; //brojac kolizija
		stations[i].remainingPackets = 2; //broj paketa za stanicu
		stations[i].iteratonCollision = 0;
		stations[i].CW = CWmin;
		stations[i].elapsedTime = 0;
		generateBackoffTime(&stations[i]);
		numberOfPacketsOnNetwork = numberOfPacketsOnNetwork + stations[i].remainingPackets;
		//printf("\nGenerirani backofftime %2f \n", stations[i].backoffTime);	
	}
}

int compareBackoffTime(const void* stationOne, const void* stationTwo) {
	return (
		((Station*)stationOne)->backoffTime - ((Station*)stationTwo)->backoffTime
	);
}

bool isCollisionDetected(Station* mainStation, Station* stationToCompare) {

	return (
		mainStation->iteratonCollision == 0 &&
		strcmp(mainStation->name, stationToCompare->name) != 0 &&
		mainStation->backoffTime == stationToCompare->backoffTime &&
		stationToCompare->remainingPackets > 0
	);
}

void checkCollision(Station* stations, int stationIndex){
	for (int i = 0; i < numberOfStations; i++) {
		if (isCollisionDetected(&stations[stationIndex], &stations[i])) {
			stations[stationIndex].counterOfCollision++;
			stations[stationIndex].iteratonCollision = 1;
			//printf("\nDogodila se kolizija i brojac kolizija : %d ", stations[i].counterOfColision);
		}
	}
	if (stations[stationIndex].iteratonCollision == 0) {
		stations[stationIndex].counterOfCollision = 0; //nema kolizije u ivij iteraciji pa se brojac restira na nula
	}
}

void processPacket(Station* station) {
	station->CW = CWmin;
	station->remainingPackets--;
	numberOfPacketsOnNetwork--;
}

int main() {
	srand(time(0));

	printf("\nUnesite broj stanica u mrezi (4, 9, 25, 49 ili 100):\n");
	scanf_s("%d", &numberOfStations);

	Station* stations = (Station*) malloc(sizeof(Station) * numberOfStations);

	createStations(stations);
	qsort(stations, numberOfStations, sizeof(Station), compareBackoffTime);

	printf("\n\nSortirani Backofftime:\n");
	for (int i = 0; i < numberOfStations; i++) {
		printf("\n%s: %2f \n", stations[i].name, stations[i].backoffTime);
	}

	while (numberOfPacketsOnNetwork > 0){
		printf("\n*********************************************************");
		printf("\n               PREOSTALI BROJ PAKETA  %d                  ", numberOfPacketsOnNetwork);
		printf("\n*********************************************************");
		for (int i = 0; i < numberOfStations; i++) {
			if (stations[i].remainingPackets > 0) {
				checkCollision(stations, i);
			}		
		}
	
		//postavljanje cw i ponovno određivanje backofftimea
		//nakon sta se for izvrši opet sortiram
		for (int i = 0; i < numberOfStations; i++) {
			if (stations[i].remainingPackets > 0) {
				if (stations[i].counterOfCollision == 0) { //stanica nije bila u koliziji
					processPacket(&stations[i]);
					printf("\n -----------");
					printf("\n| %s |\n", stations[i].name);
					printf(" -----------");
					printf("\nPAKET POSLAN");
					printf("\nPreostali broj paketa: %d ",stations[i].remainingPackets);
					stations[i].elapsedTime += DIFS + stations[i].backoffTime + timeToSend + SIFS + ACK;
					printf("\nProteklo vrijeme: %2f ", stations[i].elapsedTime);
				}
				else {//stanica je bila u koliziji
					if (stations[i].counterOfCollision >= retryLimit) {
						processPacket(&stations[i]);
					}
					else {
						int newCW = stations[i].CW * 2;

						if (newCW >= CWmax) {
							stations[i].CW = CWmax;
						}
						else {
							stations[i].CW = newCW;
						}
					}

					printf("\n---------------------------------------------------------\n");
					printf("\n%s\n", stations[i].name);
					printf("\nPAKET NIJE POSLAN");
					printf("\nPreostali broj paketa: %d ",stations[i].remainingPackets);
					stations[i].elapsedTime += DIFS + stations[i].backoffTime + timeToSend + SIFS + ACK;
					printf("\nProteklo vrijeme: %2f ", stations[i].elapsedTime);
				}

				generateBackoffTime(&stations[i]);
			}
			if (stations[i].remainingPackets > 0) {
				printf("\n\nNovi backofftime %2f", stations[i].backoffTime);
				printf("\nNovi CW %d \n", stations[i].CW);
			}
			printf("\n---------------------------------------------------------\n");
			stations[i].iteratonCollision = 0;
		}
		qsort(stations, numberOfStations, sizeof(Station), compareBackoffTime);
		printf("\n\nSortirani Backofftime: ");
		for (int i = 0; i < numberOfStations; i++) {
			printf("\n%s: %2f \n", stations[i].name, stations[i].backoffTime);
		}

		//printf("\nTrenutni broj paketa je: %d ", numberOfPacketsOnNetwork);
	}
	printf("\nBroj paketa koji je preostao je: %d ",numberOfPacketsOnNetwork);
}