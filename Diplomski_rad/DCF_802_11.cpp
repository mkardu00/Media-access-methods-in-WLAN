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

int CWmax = 1023; // * slotTime; 	// us
int CWmin = 15; // *slotTime; 	// us---zasto množi s time slotom

int numberOfPacketsOnNetwork = 0;
int retryLimit = 4;

typedef struct _station {
	char name[20];
	int numberOfPackets;// broj paketa za svaku stanicu
	int CW; //us, postavljanje contention windowa na minimalnu vrijednost
	double backoffTime; // backoff timer za svaku stanicu
	int counterOfCollision; //brojac uzastopnih kolizija
	bool iteratonCollision; //provjerava koliziju za svaku iteraciju
} Station;

void generateBackoffTime(Station* stations, int stationIndex) {

	stations[stationIndex].backoffTime = ((rand() % stations[stationIndex].CW)) * slotTime; //provjerit je li se ovako racuna!!!!
}

void setCW(Station* stations, int stationIndex) {

	if (stations[stationIndex].counterOfCollision == 0) {
		stations[stationIndex].CW = CWmin;
	}
	else {
		stations[stationIndex].CW = stations[stationIndex].CW * 2;
		if (stations[stationIndex].counterOfCollision > retryLimit) {
			stations[stationIndex].numberOfPackets--;
			numberOfPacketsOnNetwork--;
			stations[stationIndex].CW = CWmin;
		}
		else if (stations[stationIndex].CW >= CWmax) {
			stations[stationIndex].CW = CWmax;
		}
	}
}

void createStations(Station* stations) {
	//postavljanje pocetnih vrijednosti za svaku stanicu
	for (int i = 0; i < numberOfStations; i++) {
		sprintf_s(stations[i].name, "%s_%d", "STANICA", i);
		stations[i].counterOfCollision = 0; //brojac kolizija
		stations[i].numberOfPackets = 2; //broj paketa za stanicu
		stations[i].iteratonCollision = 0;
		setCW(stations, i);
		generateBackoffTime(stations, i);
		numberOfPacketsOnNetwork = numberOfPacketsOnNetwork + stations[i].numberOfPackets;
		//printf("\nGenerirani backofftime %2f \n", stations[i].backoffTime);	
	}
}

int compareBackoffTime(const void* station_one, const void* station_two) {
	return (
		((Station*)station_one)->backoffTime - ((Station*)station_two)->backoffTime
	);
}

void checkCollision(Station* stations, int stationIndex){
	for (int i = 0; i < numberOfStations; i++) {
		if (
			stations[stationIndex].iteratonCollision == 0 &&
			stationIndex != i &&
			stations[stationIndex].backoffTime == stations[i].backoffTime
		) {
			stations[stationIndex].counterOfCollision++;
			stations[stationIndex].iteratonCollision = 1;
			//printf("\nDogodila se kolizija i brojac kolizija : %d ", stations[i].counterOfColision);
		}
		else {
				
			//printf("\nNema kolizije za i");
		}
		
	}
	if (stations[stationIndex].iteratonCollision == 0) {
		stations[stationIndex].counterOfCollision = 0; //nema kolizije u ivij iteraciji pa se brojac restira na nula
	}
}

int main() {
	srand(time(0));

	printf("\nUnesite broj stanica u mrezi (4, 9, 25, 49 ili 100):\n");
	scanf_s("%d", &numberOfStations);

	Station* stations = (Station*) malloc(sizeof(Station) * numberOfStations);

	createStations(stations);
	qsort(stations, numberOfStations, sizeof(Station), compareBackoffTime);

	printf("\n\nBackofftime nakon sorta: ");
	for (int i = 0; i < numberOfStations; i++) {
		printf("\n%s: %2f \n", stations[i].name, stations[i].backoffTime);
	}

	while (numberOfPacketsOnNetwork > 0){
		printf("\n**************  %d  ****************", numberOfPacketsOnNetwork);
		for (int i = 0; i < numberOfStations; i++) {
			checkCollision(stations, i);	
		}
	
		//postavljanje cw i ponovno određivanje backofftimea
		//nakon sta se for izvrši opet sortiram
		for (int i = 0; i < numberOfStations; i++) {
			if (stations[i].numberOfPackets > 0) {
				if (stations[i].counterOfCollision == 0) { //stanica nije bila u koliziji
					setCW(stations, i);
					generateBackoffTime(stations, i);
					numberOfPacketsOnNetwork--;
					stations[i].numberOfPackets--;
					printf("\n %s JE POSLALA PAKET", stations[i].name);
					printf("\n%s: broj paketa stanice %d ", stations[i].name, stations[i].numberOfPackets);
					//pošalji pakt
					//resetiraj cw na min
					//novi backoff
				}
				else {
					setCW(stations, i);
					generateBackoffTime(stations, i);
					printf("\n------------------------\n");
					printf("\n %s NIJE POSLALA PAKET", stations[i].name);
					printf("\n%s: broj paketa stanice %d ", stations[i].name, stations[i].numberOfPackets);
					//uduplat cw ovih i odredit nove backoffe
				}
			}
			
			printf("\n\nNovi backofftime %2f \n", stations[i].backoffTime);
			printf("\n\nNovi CW %d \n", stations[i].CW);
			printf("\n------------------------\n");
			stations[i].iteratonCollision = 0;
		}
		qsort(stations, numberOfStations, sizeof(Station), compareBackoffTime);
		printf("\n\nBackofftime nakon sorta iduceg: ");
		for (int i = 0; i < numberOfStations; i++) {
			printf("\n%s: %2f \n", stations[i].name, stations[i].backoffTime);
		}
		
		//printf("\n\n Brojaci ");
		//for (int i = 0; i < numberOfStations; i++) {

			//printf("\n%d\n", stations[i].counterOfColision);
		//}
		printf("\n\n Trenutni broj paketa je: %d ", numberOfPacketsOnNetwork);
	}
	printf("\n\n Broj paketa koji je preostao je: %d ",numberOfPacketsOnNetwork);
}