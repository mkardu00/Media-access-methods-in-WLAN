#include <stdio.h>
#include <math.h>
#include <time.h>
#include<iostream>

int networkSize;// ?? mislim da mi ovo uopce ne treba

int numberOfStations;	// broj stanica u mrezi (networkSize x networkSize)
int slotTime = 9; 	// 20us(for 802.11b); 9us(for 802.11g)
int dataRate = 54; 	// 11Mbps for 802.11b; 54Mbps for 802.11g
int SIFS = 10; 	// us
int DIFS = 2 * slotTime + SIFS; // us 
int maxFrameSize = 1500; // bytes

int CWmax = 1023 * slotTime; 	// us
int CWmin = 15 * slotTime; 	// us



int CW[100]; // us, contention window
int numberOfPackets[100];// broj paketa za svaku stanicu
double backoffTime[100]; // backoff timer za svaku stanicu
int numberOfPacketsOnNetwork = 0;

int main() {
	printf("\nUnesite broj stanica u mrezi (4, 9, 25, 49 ili 100):\n");
	scanf_s("%d", &numberOfStations);
	srand(time(0));
	//postavljanje pocetnih vrijednosti za svaku stanicu
	for (int i = 0; i < numberOfStations; i++) {
		CW[i] = CWmin; // postavljanje contention windowa na minimalnu vrijednost
		numberOfPackets[i] = 1000; //dodjeljivanje 1000 paketa svakoj stanici
		numberOfPacketsOnNetwork = numberOfPacketsOnNetwork + 1000;
	
		
		backoffTime[i] = (rand() % CW[i]);//provjerit je li se ovako racuna!!!!
		printf("\n%2f \n", backoffTime[i]);
	
	}

	while (numberOfPacketsOnNetwork > 0)
	{

	}

}