//PARA EVITAR CONFLICTOS ESTABLECEREMOS EL 0 COMO FALSE Y 1 COMO TRUE PARA LOS INT BINARIOS.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <string.h>

#define NC 5
#define FALSE 0
#define TRUE 1

/*typedef*/
struct corredor
{
	pthread_t hilo;
	char * id;
	int atendido,sancionado,irreparable,posicionLista;
	
};//Corredor;

struct box
{
	char * id;
	int cerrado;
};

pthread_mutex_t mutexCircuito;
pthread_mutex_t mutexBoxes;
pthread_mutex_t mutexLog;
pthread_cond_t salida;
pthread_cond_t condicionesBoxes[NC];
int numeroCorredores;
int posicionColaBox;
struct corredor *corredores[NC];
struct corredor *corredoresBoxes[NC];
FILE * logFile;


void nuevoCorredor(int sig){

	char * id;
	struct corredor nuevoCorredor;
	pthread_t hilo;	

	pthread_mutex_lock(&mutexCircuito);

	if(numeroCorredores == NC - 1){
		if(signal(SIGUSR1,SIG_IGN)==SIG_ERR){
		perror("Error in signal call");
		exit(-1);
		}
	}
	else{
		if(signal(SIGUSR1,nuevoCorredor)==SIG_ERR){
		perror("Error in signal call");
		exit(-1);
		}
	}

		nuevoCorredor.hilo=hilo;
		nuevoCorredor.atendido=FALSE;
		nuevoCorredor.sancionado=FALSE;
		nuevoCorredor.irreparable=FALSE;
		nuevoCorredor.posicionLista=numeroCorredores;
		sprintf(id,"Corredor_%d",numeroCorredores+1);
		nuevoCorredor.id=id;

		//Se añade el corredor
		corredores[numeroCorredores++]=&nuevoCorredor;

		pthread_create(&hilo, NULL, hiloCorredor, (void*)&nuevoCorredor);
		
		pthread_mutex_unlock(&mutexCircuito);
}

void *hiloCorredor(void *ptr){
	int i = 0;
	//int indice = *(int*)ptr;
	int problemasMecanicos,tVuelta;
	struct corredor *esteCorredor = (struct corredor*) ptr;
	//Corredor *esteCorredor = &corredores[indice];
	//struct corredor *esteCorredor;
	//esteCorredor=&corredores[indice];
	
	if(numeroCorredores==NC){
		pthread_cond_signal(&salida);
	}
	else{
		pthread_mutex_lock(&mutexCircuito);
		pthread_cond_wait(&salida,&mutexCircuito);
		pthread_mutex_unlock(&mutexCircuito);
	}

	srand(time (NULL));

	pthread_mutex_lock(&mutexLog);

	writeLogMessage((*esteCorredor).id,"Entra a pista.");

	pthread_mutex_unlock(&mutexLog);

	for(i;i<5;i++){

		tVuelta=rand()%4+2;
		problemasMecanicos=rand()%10+1;

		sleep(tVuelta);

		if(problemasMecanicos<6){
			pthread_mutex_lock(&mutexBoxes);
			corredoresBoxes[posicionColaBox++]=esteCorredor;
			pthread_cond_wait(&condicionesBoxes[(*esteCorredor).posicionLista],&mutexBoxes);
			pthread_mutex_unlock(&mutexBoxes);


			if((*esteCorredor).irreparable == TRUE){

				pthread_mutex_lock(&mutexLog);

				writeLogMessage((*esteCorredor).id,"No se puede reparar.");

				pthread_mutex_unlock(&mutexLog);

				pthread_exit (NULL);

			}

		}

		if((*esteCorredor).sancionado == TRUE){
			sleep(3);
		}


	}

	pthread_mutex_lock(&mutexLog);

	writeLogMessage((*esteCorredor).id,"Finaliza la carrera.");

	pthread_mutex_unlock(&mutexLog);

	pthread_exit (NULL);

}

void *hiloBox(void *ptr) {
	struct box *box = struct box* ptr;
	char * msg;
	struct corredor corredor;
	srand(time(NULL));
	int corredoresAtendidos = 0;

	// INCIO BUCLE GENERAL
	// ESPERA / BUSCA
	&corredor = corredoresBoxes[0];
	// ACTUALIZA CORREDORESBOXES

	pthread_mutex_lock(&mutexLog);
	// sprintf(msg, "Atiende a %s", corredor.id);
	sprintf(msg, "Atiende a ...");
	writeLogMessage((*box).id, msg);
	pthread_mutex_unlock(&mutexLog);

	sleep(rand() % 3 + 1);
	if (rand() % 10 >= 3)
		corredor.irreparable = TRUE;

	if (++corredoresAtendidos >= 3 /*&& PUEDE CERRAR*/) {
		corredoresAtendidos = 0;

		pthread_mutex_lock(&mutexLog);
		writeLogMessage((*box).id, "Cierra");
		pthread_mutex_unlock(&mutexLog);

		(*box).cerrado = TRUE;
		sleep(20);

		pthread_mutex_lock(&mutexLog);
		writeLogMessage((*box).id, "Reabre");
		pthread_mutex_unlock(&mutexLog);

		(*box).cerrado = FALSE;
	}
	// FIN BUCLE GENERAL
}

void *hiloJuez(void *ptr){
	
}

void writeLogMessage ( char * id , char * msg ) {
// Calculamos la hora actual
	time_t now = time (0) ;
	struct tm * tlocal = localtime (& now ) ;
	char stnow [19];
	strftime (stnow,19,"%d/ %m/ %y %H: %M: %S" ,tlocal) ;

// Escribimos en el log
	logFile = fopen ("log.txt","a");
	fprintf (logFile,"[ %s ] %s : %s \n " ,stnow,id,msg) ;
	fclose (logFile);
}


int main(void){

	int i;

	if(signal(SIGUSR1,nuevoCorredor)==SIG_ERR){
		perror("Error in signal call");
		exit(-1);
	}
	
	if(pthread_cond_init(&salida, NULL)!=0){
		exit(-1);
	}

	for(i=0;i<NC;i++){
		if(pthread_cond_init(&condicionesBoxes[i], NULL)!=0){
		exit(-1);
		}
	}
	
	pthread_mutexCircuito_init(&mutexCircuito, NULL);
	pthread_mutexBoxes_init(&mutexBoxes, NULL);
	pthread_mutexLog_init(&mutexLog, NULL);
	numeroCorredores = 0;
	posicionColaBox = 0;
	struct box param_box1 = {"Box_1", FALSE};
	struct box param_box2 = {"Box_2", FALSE};
	pthread_t box1, box2, juez;
	

	pthread_create(&box1, NULL, hiloBox, (void*)&param_box1);
	pthread_create(&box2, NULL, hiloBox, (void*)&param_box2);
	pthread_create(&juez, NULL, hiloJuez, NULL);

	
}


