#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>

int numberOfStations;	// broj stanica u mrezi (networkSize x networkSize)
int slotTime = 9; 	// 20us(for 802.11b); 9us(for 802.11g)
int dataRate = 54; 	// 11Mbps for 802.11b; 54Mbps for 802.11g
int SIFS = 10; 	// us, 802.11g
int DIFS = 2 * slotTime + SIFS; // us 
int maxFrameSize = 1500; // bytes//1040
int ACK = 15;			//bytes
int timeToSend = 1454; //us, 802.11g


int CWmax = 1023; // * slotTime; 	// us
int CWmin = 15; // *slotTime; 	// us---zasto množi s time slotom

int numberOfPacketsOnNetwork = 0;

int retryLimit = 7;
int numberOfPackets = 2;

double competitionTime = 0;//trajanje nadmetanja
double simulationTime = 0;
int droppedPackets = 0;
int transmittedPackets = 0;

double arrayOfBackoffTime[50];
int numberOfCollision = 0;

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
		stations[i].remainingPackets = numberOfPackets; //broj paketa za stanicu
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
	int br = 0; 
	for (int i = 0; i < numberOfStations; i++) {
		if (isCollisionDetected(&stations[stationIndex], &stations[i])) {

			stations[stationIndex].counterOfCollision++;
			stations[stationIndex].iteratonCollision = 1;
			
			for (int j = 0; j < 50; j++) {
				if (stations[stationIndex].backoffTime == arrayOfBackoffTime[j]) {
					br++;
					
				}
			}
			if (br == 0) {
				for (int j = 0; j < 50; j++) {
					if (arrayOfBackoffTime[j] == -1) {
						arrayOfBackoffTime[j] = stations[stationIndex].backoffTime;
						break;
						
					}
					
				}
			}
			printf("\nDogodila se kolizija i brojac kolizija : %d, %s ", br, stations[stationIndex].name);
		}
	}
	if (stations[stationIndex].iteratonCollision == 0) {
		stations[stationIndex].counterOfCollision = 0; //nema kolizije u ovoj iteraciji pa se brojac restira na nula
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
	printf("\nUkupan broj paketa na mrezi: %d ", numberOfStations*numberOfPackets);

	for (int i = 0; i < 50; i++) {
			arrayOfBackoffTime[i] = -1;
	}

	
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
		//niz s backoffima vratit na nulu
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
					transmittedPackets += 1;
					stations[i].elapsedTime += DIFS + stations[i].backoffTime + timeToSend + SIFS + ACK;
					simulationTime += stations[i].elapsedTime;
					competitionTime += stations[i].elapsedTime;
					printf("\nProteklo vrijeme: %2f (us) ", stations[i].elapsedTime);
				}
				else {//stanica je bila u koliziji
					
					if (stations[i].counterOfCollision >= retryLimit) {
						droppedPackets += 1; 
						processPacket(&stations[i]);
					}
					else {
						int newCW = (stations[i].CW * 2) + 1;

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
					stations[i].elapsedTime += DIFS + stations[i].backoffTime + timeToSend + SIFS;
					simulationTime += stations[i].elapsedTime;
					competitionTime += stations[i].elapsedTime;
					printf("\nProteklo vrijeme: %2f (us) ", stations[i].elapsedTime);
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
		for (int i = 0; i < 50; i++) {
			if (arrayOfBackoffTime[i] != -1) {
				printf("\nNiz za kolizije %2f: ", arrayOfBackoffTime[i]);
				numberOfCollision += 1;
				arrayOfBackoffTime[i] = -1;
				
			}

		}
		competitionTime = 0;
		qsort(stations, numberOfStations, sizeof(Station), compareBackoffTime);
		printf("\n\nSortirani Backofftime: ");
		for (int i = 0; i < numberOfStations; i++) {
			printf("\n%s: %2f \n", stations[i].name, stations[i].backoffTime);
		}

		//printf("\nTrenutni broj paketa je: %d ", numberOfPacketsOnNetwork);
	}
	//printf("\nUkupan broj paketa na mrezi: %d ",numberOfPacketsOnNetwork);
	printf("\nBroj uspjesno poslanih paketa: %d ", transmittedPackets);
	printf("\nBroj odbacenih paketa: %d ", droppedPackets);
	printf("\nBroj kolizija: %d ", numberOfCollision);
	printf("\nVrijeme trajanja simulacije: %2f (us) \n", simulationTime);// zbrojena vremena uspjesno i neuspjesno poslanih paketa
}