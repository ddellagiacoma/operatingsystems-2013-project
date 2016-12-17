#include <stdio.h>
#include <stdlib.h>
#include "Threads.h"
#include <getopt.h>

int main (int argc, char **argv){

	//Inizializzazione threads
	pthread_t Tr;
	pthread_t Te;
	pthread_t Td;
	pthread_t Tw;

	//guardo l'opzione presa in input
	int opt;
	int opzione=50;


	/*se l'opzione data è -q ed un intero allora verrà inizializzata la coda
	avente lunghezza uguale  all intero passato
	se l intero passato è minore di 0 si concuderà il programma
	se l opzione scelta non sarà q, verrà chiuso il programma con messaggio di errore
	se si esegue il file senza usare l'opzione la coda viene automaticamente inizializzata a 50
	 */
	while((opt=getopt(argc, argv, "q:"))!=-1){
		switch (opt){
		case 'q':
			opzione=atoi(optarg);
			if (opzione<0){
				exit(1);
			}
			if((atoi(optarg))==0){	
				printf("scelta sbagliata dopo aver digitato -q si prega di inserire un valore numerico maggiore di zero\n");
				exit(1);
			}	
			break;		
		case'?':
			printf("scelta sbagliata digitare -q ed un numero che sarà la grandezza della coda\n");
			exit(1);
			break;
		}
	}


	//Inizializzazione struttura
	init (&buffer, opzione); 

	// Creazione threads
	pthread_create (&Tr, NULL, Trbody,NULL);
	pthread_create (&Te, NULL, Tebody, NULL);
	pthread_create(&Td, NULL, Tdbody, NULL);
	pthread_create(&Tw, NULL, Twbody, NULL);

	// Attesa teminazione threads creati
	pthread_join (Tr, NULL);
	pthread_join (Te, NULL);
	pthread_join(Td, NULL);
	pthread_join(Tw, NULL);

return 0;
}
