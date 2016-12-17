#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "Threads.h"

//dichiaro variabili globali
char S[500];
int controllo=0;
char *R;
char *Se;
char *Sd;
int lungS;

//dichiaro semafori globali
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond, cond1, cond2;

/*funzione dataora, crea se non c'è già la cartella mkdir 
problema: segnalerà sempre errore dicendo che la cartella è già stata creata, il programma procede normalmente l'esecuzione comunque.
 una volta creata la cartella crea i vari file di log richieste in appendice
 legge dalla libreria time.h  data ed ora e in base all int passato come parametro scrive nei vari thread il loro log nei vari file log creati*/
void dataora(int nthread){
	//creo cartella e gestisco errori
	if ((mkdir("/var/log/Threads/", S_IRWXU | S_IRWXG | S_IRWXO)) != 0){
		//perror("Errore mkdir()");
	}
	//dichiaro file
	FILE *fp1;
	FILE *fp2;
	FILE *fp3;
	FILE *fp4;
	




	// leggo dalla libreria time.h
	int log;
	time_t rawtime;
	if (time(&rawtime)==-1){
		perror("Errore time()");
		exit(1);
	}
	struct tm* leggibile; 
	leggibile = localtime(&rawtime);
	// metto data e ora nella stringa orario
	char orario[30];
	sprintf(orario,"data %d/%d/%d ora %d:%d:%d",leggibile->tm_mday,leggibile->tm_mon+1,leggibile->tm_year+1900,leggibile->tm_hour,leggibile->tm_min,leggibile->tm_sec);

	// apro i vari file e stampo orario nei file di log aperti in base a int preso come parametro e gestisco eventuali errori
	switch(nthread){
	case 1:
		if ((fp1=fopen("/var/log/Threads/Tr.log","a+"))==NULL){
			perror("Errore fopen() Tr.log");
			exit(1);
		}
		fprintf(fp1,"%s\n",orario);
		if(fclose(fp1)==-1){
			perror("Errore close() Tr.log");
			exit(1);
		}
		break;
	case 2:
		if ((fp2=fopen("/var/log/Threads/Te.log","a+"))==NULL){
			perror("Errore fopen() Te.log");
			exit(1);
		}
		fprintf(fp2,"%s\n",orario);
		if(fclose(fp2)==-1){
			perror("Errore close() Te.log");
			exit(1);
		}
		break;
	case 3:
		if ((fp3=fopen("/var/log/Threads/Td.log","a+"))==NULL){
			perror("Errore fopen() Td.log");
			exit(1);
		}
		fprintf(fp3,"%s\n",orario);
		if (fclose(fp3)==-1){
			perror("Errore close() Td.log");
			exit(1);
		}
		break;
	case 4:
		if ((fp4=fopen("/var/log/Threads/Tw.log","a+"))==NULL){
			perror("Errore fopen() Tw.log");
			exit(1);
		}
		fprintf(fp4,"%s\n",orario);
		if (fclose(fp4)==-1){
			perror("Errore close() Tw.log");
			exit(1);
		}
		break;
	}
}



/* la top serve ritorna l elemento in testa alla coda, senza toglierlo da essa.Se la coda è vuota non ritorna niente */
char * top(prodcons *b){
	if (b->size!=0){
		return b->elem[b->testa];
	}
}

/*la funzione dequeue toglie l elemento in testa alla coda se la coda è vuota non toglie niente*/
void dequeue(prodcons *b){
	if (b->size!=0){
		b->testa=(b->testa+1)%b->mas;
		b->size--;
	}
}


/*la funzione enqueue aggiunge l' elemento in fondo alla coda */
void enqueue(char  *S,prodcons *b){
	if (b->size!=b->mas){
		b->elem[b->coda]=S;
		b->coda=(b->coda+1)%b->mas;
		b->size++;
	}
}

/*la funzione ctrlS controlla se la scritta inserita dall utente è quit, abbiamo fatto questa scelta di controllo
 dopo aver visto che la funzione strcmp ritorna valori diversi ad ogni dispositivo dove
 viene compilato il programma*/
int ctrlS(char *d){
	if(d[0]=='q'){
		if(d[1]=='u'){
			if (d[2]=='i'){
				if (d[3]=='t'){
					if(d[4]=='\n'){
						return 1;					
					}else {return 2;}			
				}else {return 2;}
			}else {return 2;}
		}else {return 2;}
	}else {return 2;}
}


/*la funzione inserisci va quando la coda non è piena.Se la coda risulta piena blocca l inserimento, fa partire il thread 2 che si occupa 
 dell estrazione fino a che la coda non si svuota. se la coda non è piena allora inseirsce la stringa S in coda*/

void inserisci (prodcons *b){
	if (pthread_mutex_lock (&b->lock)==-1){
		perror("Errore lock() inserisci");
		exit(1);
	}
	/* controlla che il buffer non sia pieno:*/
	while ( b->size==b->mas){
		if (pthread_cond_wait (&b->notfull, &b->lock)==-1){
			perror("Errore wait() inserisci");
			exit(1);
		}
	}
	/* scrivi data e aggiorna lo stato del buffer */

	enqueue(S,b);

	/* risveglia eventuali thread (consumatori) sospesi */
	if(pthread_cond_signal(&b->notempty)==-1){
		perror("Errore signal() inserisci");
		exit(1);
	}
	if(pthread_mutex_unlock (&b->lock)==-1){
		perror("Errore unlock() inserisci");
		exit(1);
	}
}


/*la funzione estrai va quando la coda non è vuota.Se la coda risulta vuota viene bloccata l estrazione,
 fa partire il thread 1 che si occupa  dell estrazione fino a che la coda non è in parte piena.
 se la coda non è vuota viene estratto l elemento in testa alla coda.
 */

char* estrai (prodcons *b){
	char *stringa;
	if(pthread_mutex_lock (&b->lock)==-1){
		perror("Errore lock() estrai");
		exit(1);
	}
	while (b->size==0){/* il buffer e` vuoto? */
		if (pthread_cond_wait (&b->notempty, &b->lock)==-1){
			perror("Errore wait() estrai");
			exit(1);
		}
	}
	/* Leggi l'elemento e aggiorna lo stato del buffer*/
	stringa=top(b);

	dequeue(b);

	/* Risveglia eventuali threads (produttori)*/
	if(pthread_cond_signal (&b->notfull)==-1){
		perror("Errore signal() estrai");
		exit(1);
	}
	if(pthread_mutex_unlock (&b->lock)==-1){
		perror("Errore unlock() estrai");
		exit(1);
	}
	return stringa;
}


/*Tr legge da tastiera una sequenza di caratteri (S) arbitrariamente lunga fino a che
non riconosce il carattere <CR> (ENTER), il thread si blocca solo quando la parola inseirta dall utente non sarà quit.
 il thread chiama la funzione inserimento per  ogni input immesso dall utente
 la chiamata alla funzione data ora(1) fa si che chiamando la funzione dataora, venga creato il file Tr.log in var/log/threads */
void *Trbody (void *arg)
{
	do{
	dataora(1);
	//prendo input
	int i=-1;

		do{
			i++;
			S[i]=getchar();
			
		}
		while(S[i]!='\n');
		S[i+1]='\0'; 
		
		
		/*Chiamata alla funzione inserisci*/
		inserisci (&buffer);

		//faccio controllo
		controllo=ctrlS(S);
	}
	while(controllo!=1);//esci se è uguale a quit

	return NULL;
}



/* 
 il thread continua a chiamare la funzione estrai, fino a quando  la parola estratta non è quit,o fino a quando la coda non è svuotata definitivamente,
 questo per permettere a Te di lavorare e svuotare la 
 coda anche se TR ha finito l esecuzione
 la parola estratta viene inserita in una stringa d.
 viene letto dal device “/dev/random” una stringa di caratteri R della dimensione di d, e,
 successivamente, ne fa lo XOR byte-per-byte con S ottenendo Se = XOR(R, S), e viene sbloccato il thread Td
 la chiamata alla funzione dataora(2) fa si che chiamando la funzione dataora, venga creato il file Te.log in var/log/threads */
void *Tebody (void *arg)
{
	char *d;
	int ctrlexit;
	while (1){
		dataora(2);
		d = estrai (&buffer); /*Legge gli elementi da buffer*/
		lungS=strlen(d);
		ctrlexit=ctrlS(d);
		if (ctrlexit==1){
			exit(0);
		}

		//allocazione dinamica R
		R=malloc(lungS*sizeof(char));
		//leggo da device dev/random
		int random=open("/dev/random",O_RDONLY);
		read(random,R,sizeof(char)*lungS);
		if (close(random)==-1){
			perror("Errore close() random");
		}

		//stampo R
		printf("R: ");
		int i=0;
		for(i;i<lungS;i++){
			printf("%c",R[i]);
		}
		printf("\n");

		//allocazione dinamica Se
		Se=malloc(lungS*sizeof(char));

		//Se = XOR(R, S)
		i=0;
		for(i;i<lungS;i++){
			Se[i]=R[i]^d[i];
		}

		//stampo Se
		printf("Se: ");
		i=0;
		for(i;i<lungS;i++){
			printf("%c",Se[i]);
		}
		printf("\n");

		//controllo eventuali errori di pthread_cond_signal
		if (pthread_cond_signal(&cond)==-1){
			perror("Errore signal() Te");
			exit(1);
		}//controllo eventuali errori di pthread_cond_wait
       		if (pthread_cond_wait(&cond2, &lock)==-1){
			perror("Errore wait() Te");
			exit(1);
		}

		if (buffer.size==-1) {/*Esce dal ciclo quando il buffer è vuoto*/
			break;
		}
	}
}


/*Td viene sbloccato alla fine del ciclo di Te
 Td legge R e Se e calcola Sd = XOR(R, Se), e sblocca il thread Tw una volta finita l'esecuzione
 Td finisce l'esecuzione una volta finito il programma
 la chiamata alla funzione dataora(3) fa si che chiamando la funzione dataora, venga creato il file Td.log in var/log/threads*/
void *Tdbody(void *arg)
{
	while(1){
		if (pthread_cond_wait(&cond, &lock)==-1){
			perror("Errore wait() Td");
			exit(1);
		}
		dataora(3);
		//allocazione dinamica Sd
		Sd=malloc(lungS*sizeof(char));
	
		//Sd = XOR(R, Se)
		int i=0;
		for (i;i<lungS;i++){
			Sd[i]=R[i]^Se[i];
		}
		//controllo eventuali errori di pthread_cond_signal
		if (pthread_cond_signal(&cond1)==-1){
			perror("Errore signal() Td");
			exit(1);
		}
	}
}


/*Tw viene sbloccato una volta finita l'esecuzione di Td
Tw stampa a schermo la stringa Sd
 Tw finisce l'esecuzione una volta finito il programma
 la chiamata alla funzione dataora(4) fa si che chiamando la funzione dataora, venga creato il file Tw.log in var/log/threads*/
void *Twbody(void *arg)
{
	while(1){
		//controllo eventuali errori di pthread_cond_signal
		if(pthread_cond_wait(&cond1, &lock)==-1){
			perror("Errore wait() Tw");
			exit(1);
		}

		//scrivo data e ora in Tw.log
		dataora(4);
	
		int i=0;
		//stampo la stringa Sd

		printf("Sd: ");
		for (i;i<lungS;i++){
			printf("%c",Sd[i]);
		}
		printf("\n");
		//chiamata a deinit
		deinit();
		//controllo eventuali errori di pthread_cond_signal
		if(pthread_cond_signal(&cond2)==-1){
			perror("Errore signal() Tw");
			exit(1);
		}
	}
}


//la funzione init inizializza la coda ed i semafori

void init(prodcons *b, int opzione){
	//inizializzo coda
	b->testa=0;
	b->mas=opzione;
	b->coda=0;
	b->size=0;
	b->elem=malloc(opzione*sizeof(int*));
	//inizializzo semafori e controllo eventuali errori
	if (pthread_mutex_init(&b->lock, NULL)==-1){
		perror("Errore init() lock");
		exit(1);
	}
	if (pthread_cond_init(&b->notempty, NULL)==-1){
		perror("Errore init() notempty");
		exit(1);
	}
	if (pthread_cond_init(&b->notfull, NULL)==-1){
		perror("Errore init() notfull");
		exit(1);
	}
}


//la funzione deinit riporta la lunghezza della stringa a 0 e disalloca le variabili contenenti le varie stringhe
void deinit()
{	
	lungS=0;
	free(Sd);
	free(Se);
	free(R);
}

