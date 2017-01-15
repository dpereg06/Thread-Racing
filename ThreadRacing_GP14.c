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

typedef struct corredor
{
	pthread_t hilo;
	int atendido,sancionado,irreparable, id;
	
}Corredor;

struct box
{
	
	char * id;
	int ocupado;
};

pthread_mutex_t mutexCircuito;
pthread_mutex_t mutexBoxes;
pthread_mutex_t mutexLog;
int numeroCorredores;
struct corredor corredores[NC];
struct corredor corredoresBoxes[NC];
FILE * logFile;


void manejadorSignal(){
	if(signal(SIGUSR1, manejadorSignal) == SIG_ERR){
	   printf("Error en la llamada a signal.\n");
	}

	//int i = 0;
	//char *id;
	//itoa(i, id, 10);
	//pthread_create(&corredores[i].hilo, NULL, nuevoCorredor(id), NULL);
	//i++;
	//if(i == 5){
	//	i == 0;
	//}
	
}

int main(void){
	/*if(signal(SIGUSR1, manejadorSignal) == SIG_ERR){
	   printf("Error en la llamada a signal.\n");
	}

	pthread_mutexCircuito_init(&mutexCircuito, NULL);
	pthread_mutexBoxes_init(&mutexBoxes, NULL);
	pthread_mutexLog_init(&mutexLog, NULL);
	numeroCorredores = 0;
	logFile = fopen("FicheroLog.log", "a");
	struct box param_box1 = {"box 1", FALSE};
	struct box param_box2 = {"box 2", FALSE};
	pthread_t box1, box2;
	

	//pthread_create(&box1, NULL, hiloBox, (void*)&param_box1);
	//pthread_create(&box2, NULL, hiloBox, (void*)&param_box2);
*/
	
}


void nuevoCorredor(int identificador){
	
	/*if(signal(SIGUSR1, nuevoCorredor) == SIG_ERR){
	   printf("Error en la llamada a signal.\n");
	}*/
	
	if(numeroCorredores<5){

		//Crear corredor y asignarle sus atributos		
		Corredor nuevoCorredor;
		pthread_t hilo;		
		numeroCorredores++;
		identificador=numeroCorredores;		
		nuevoCorredor.hilo=hilo;
		nuevoCorredor.atendido=FALSE;
		nuevoCorredor.sancionado=FALSE;
		nuevoCorredor.irreparable=FALSE;
		nuevoCorredor.id=identificador;

		//Se añade el corredor
		corredores[numeroCorredores]=nuevoCorredor;
		
	}else{

		//No hacer nada
	
	}
}

void *hiloCorredor(void *ptr){
	char *log;
	int i = 0;
	int indice = *(int*)ptr;
	int problemasMecanicos,tVuelta;
	Corredor *esteCorredor = &corredores[indice];
	//struct corredor *esteCorredor;
	//esteCorredor=&corredores[indice];

	srand(time (NULL));

	*log = "Entra a pista.";

	pthread_mutex_lock(&mutexLog);

	writeLogMessage(esteCorredor->id,log);

	pthread_mutex_unlock(&mutexLog);

	for(i;i<5;i++){

		tVuelta=rand()%4+2;
		problemasMecanicos=rand()%10+1;

		sleep(tVuelta);

		if(problemasMecanicos<6){
			//entrar cola boxes

			if(esteCorredor->irreparable == TRUE){
				*log = "No se puede reparar.";

				pthread_mutex_lock(&mutexLog);

				writeLogMessage(esteCorredor->id,log);

				pthread_mutex_unlock(&mutexLog);

				pthread_exit (NULL);

			}

		}

		if(esteCorredor->sancionado == TRUE){
			sleep(3);
		}


	}

	*log = "Finaliza la carrera.";

	pthread_mutex_lock(&mutexLog);

	writeLogMessage(esteCorredor->id,log);

	pthread_mutex_unlock(&mutexLog);

	pthread_exit (NULL);

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

