#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>

int networkSize;// ?? mislim da mi ovo uopce ne treba

int numberOfStations;	// broj stanica u mrezi (networkSize x networkSize)
int slotTime = 9; 	// 20us(for 802.11b); 9us(for 802.11g)
int dataRate = 54; 	// 11Mbps for 802.11b; 54Mbps for 802.11g
int SIFS = 10; 	// us
int DIFS = 2 * slotTime + SIFS; // us 
int maxFrameSize = 1500; // bytes

int CWmax = 1023; // * slotTime; 	// us
int CWmin = 15; // *slotTime; 	// us---zasto množi s time slotom


int numberOfPacketsOnNetwork = 0;

typedef struct _station {
	int numberOfPackets;// broj paketa za svaku stanicu
	int CW; //us, postavljanje contention windowa na minimalnu vrijednost
	double backoffTime; // backoff timer za svaku stanicu
	int counterOfColision; //brojac kolizija
} Station;

void generateBackoffTime(Station* stations, int stationIndex) {
	

	stations[stationIndex].backoffTime = ((rand() % stations[stationIndex].CW)) * slotTime; //provjerit je li se ovako racuna!!!!

}

void setCW(Station* stations, int stationIndex) {
	
	if (stations[stationIndex].counterOfColision == 0) {
		stations[stationIndex].CW = CWmin;
	}
	else
	{
		if (stations[stationIndex].CW >= CWmax) {
			stations[stationIndex].CW = CWmax;
			
		}
		else{
			stations[stationIndex].CW = stations[stationIndex].CW * 2;
			
		}
	}
}

void createStations(Station* stations) {

	//postavljanje pocetnih vrijednosti za svaku stanicu
	for (int i = 0; i < numberOfStations; i++) {
		stations[i].counterOfColision = 0; //brojac kolizija
		stations[i].numberOfPackets = 3; //broj paketa za stanicu
		setCW(stations, i);
		generateBackoffTime(stations, i);
		numberOfPacketsOnNetwork = numberOfPacketsOnNetwork + stations[i].numberOfPackets;
		printf("\n%2f \n", stations[i].backoffTime);
		


	}
}

int compareBackoffTime(const void* station_one, const void* station_two) {
	return (
		((Station*)station_one)->backoffTime - ((Station*)station_two)->backoffTime
	);
}

void checkCollision(Station* stations, int stationIndex){
	for (int i = 0; i < numberOfStations; i++) {
		printf("\n\n index stanice %d     %d ", stationIndex, i);
		if (stations[stationIndex].counterOfColision == 0) {
			if (stationIndex != i && stations[stationIndex].backoffTime == stations[i].backoffTime) {
				stations[stationIndex].counterOfColision++;
				stations[i].counterOfColision++;
				printf("\nDogodila se kolizija i brojac kolizija : %d ", stations[i].counterOfColision);
			}
			else {
				
				printf("\nNema kolizije");
			}
		}
		//printf("\n Station index %d \n", stations[stationIndex].counterOfColision);
	}


	//printf("\n\n funkcija u kojoj se provjeravaju kolizije ");
	//printf("\n Station index %d \n", stations[stationIndex].counterOfColision);

}

int main() {
	srand(time(0));

	printf("\nUnesite broj stanica u mrezi (4, 9, 25, 49 ili 100):\n");
	scanf_s("%d", &numberOfStations);

	Station* stations = (Station*) malloc(sizeof(Station) * numberOfStations);

	createStations(stations);
	qsort(stations, numberOfStations, sizeof(Station), compareBackoffTime);

	printf("\n\n Nakon sorta ");
	for (int i = 0; i < numberOfStations; i++) {
		printf("\n%2f \n", stations[i].backoffTime);
	}

	//while (numberOfPacketsOnNetwork > 0){
		for (int i = 0; i < numberOfStations; i++) {
			
			checkCollision(stations, i);
			
		}
	
	//postavljanje cw i ponovno određivanje backofftimea
	//nakon sta se for izvrši opet sortiram
		for (int i = 0; i < numberOfStations; i++) {
			if (stations[i].counterOfColision == 0) { //stanica nije bila u koliziji
				setCW(stations, i);
				generateBackoffTime(stations, i);
				numberOfPacketsOnNetwork--;
				stations[i].numberOfPackets--;
				printf("\n------------------------\n");
				printf("\n\nSTANICA %d JE POSLALA PAKET ", i);
				//pošalji pakt
				//resetiraj cw na min
				//novi backoff
			}
			else {
				setCW(stations, i);
				generateBackoffTime(stations, i);
				printf("\n------------------------\n");
				printf("\n\nSTANICA %d NIJE POSLALA PAKET ", i);
			    //uduplat cw ovih i odredit nove backoffe
			}
			
			printf("\n\nNovi backofftime %2f \n", stations[i].backoffTime);
			printf("\n\nNovi CW %d \n", stations[i].CW);
			printf("\n------------------------\n");
		}

	printf("\n\n Brojaci ");
	for (int i = 0; i < numberOfStations; i++) {

		printf("\n%d\n", stations[i].counterOfColision);
	}
	

	//}


}