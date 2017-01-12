//PARA EVITAR CONFLICTOS ESTABLECEREMOS EL 0 COMO FALSE Y 1 COMO TRUE PARA LOS INT BINARIOS.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>

#define NC 5
#define FALSE 0
#define TRUE 1

struct corredor
{
	pthread_t hilo;
	int atendido,sancionado,irreparable;
	char * id;
	
};

struct box
{
	char * id;
	int ocupado;
};

pthread_mutex_t mutexCircuito;
pthread_mutex_t mutexBoxes;
pthread_mutex_t mutexLog;
int numeroCorredores = 0;
struct corredor corredores[NC];
struct corredor corredoresBoxes[NC];
FILE * logFile;

int main(void){

}

void nuevoCorredor(){

}

void *hiloCorredor(void *ptr){
	char *log;
	int i = 0;
	int problemasMecanicos,tVuelta;

	srand(time (NULL));

	*log = "Entra a pista.";

	//pthread_mutex_lock(log);

	//writeLogMessage(*(struct*)ptr.id,log);

	//pthread_mutex_unlock(log);

	for(i;i<5;i++){

		tVuelta=rand()%4+2;
		problemasMecanicos=rand()%10+1;

		sleep(tVuelta);

		if(problemasMecanicos<6){
			//entrar cola boxes

			/*if(*(struct*)ptr.irreparable == TRUE){
				*log = "No se puede reparar.";

				//pthread_mutex_lock(log);

				//writeLogMessage(*(struct*)ptr.id,log);

				//pthread_mutex_unlock(log);

				pthread_exit (NULL);

			}*/

		}


	}



}

void *hiloBox(void *ptr){
	
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

