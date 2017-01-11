//PARA EVITAR CONFLICTOS ESTABLECEREMOS EL 0 COMO FALSE Y 1 COMO TRUE PARA LOS INT BINARIOS.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>

#define NC 5

struct corredor
{
	pthread_t hilo;
	int id,atendido,sancionado;
	
};

struct box
{
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
	strftime (stnow,19,"%d/%m/ %y %H: %M: %S" ,tlocal) ;

// Escribimos en el log
	logFile = fopen ("logFileName","a");
	fprintf (logFile,"[ %s ] %s : %s \n " ,stnow,id,msg) ;
	fclose (logFile);
}

