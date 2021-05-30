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

int CWmax = 1023 * slotTime; 	// us
int CWmin = 15; // *slotTime; 	// us---zasto množi s time slotom


int numberOfPacketsOnNetwork = 0;

typedef struct _station {
	int numberOfPackets;// broj paketa za svaku stanicu
	int CW; //us, postavljanje contention windowa na minimalnu vrijednost
	double backoffTime; // backoff timer za svaku stanicu
} Station;

void createStations(Station* stations) {
	srand(time(0));
	//postavljanje pocetnih vrijednosti za svaku stanicu
	for (int i = 0; i < numberOfStations; i++) {
		stations[i].numberOfPackets = 1000;
		stations[i].CW = CWmin;
		stations[i].backoffTime = ((rand() % stations[i].CW)) * slotTime; //provjerit je li se ovako racuna!!!!
		printf("\n%2f \n", stations[i].backoffTime);
	}
}

int compareBackoffTime(const void* station_one, const void* station_two) {
	return (
		((Station*)station_one)->backoffTime - ((Station*)station_two)->backoffTime
	);
}
int main() {

	printf("\nUnesite broj stanica u mrezi (4, 9, 25, 49 ili 100):\n");
	scanf_s("%d", &numberOfStations);

	Station* stations = (Station*) malloc(sizeof(Station) * numberOfStations);

	createStations(stations);
	qsort(stations, numberOfStations, sizeof(Station), compareBackoffTime);
	printf("\n\n Nakon sorta ");

	for (int i = 0; i < numberOfStations; i++) {
		printf("\n%2f \n", stations[i].backoffTime);
	}

}