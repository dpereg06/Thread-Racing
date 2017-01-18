//PARA EVITAR CONFLICTOS ESTABLECEREMOS EL 0 COMO FALSE Y 1 COMO TRUE PARA LOS INT BINARIOS.

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <string.h>

#define NC 5
#define FALSE 0
#define TRUE 1

/*typedef*/
struct corredor
{
	char * id;
	int numID;
	int box,sancionado,irreparable;
	pthread_cond_t go;
	pthread_cond_t stop;
	
};//Corredor;

struct box
{
	char * id;
	int cerrado;
};

pthread_mutex_t mutexGo;
pthread_mutex_t mutexStop;
pthread_mutex_t mutexCircuito;
pthread_mutex_t mutexLog;
int numeroCorredores,numeroCorredoresTotal;
struct corredor corredores[NC];
FILE * logFile;


void writeLogMessage ( char * id , char * msg ) {
// Calculamos la hora actual
	time_t now = time (0) ;
	struct tm * tlocal = localtime (& now ) ;
	char stnow [19];
	strftime (stnow,19,"%d/ %m/ %y %H: %M: %S" ,tlocal) ;

// Escribimos en el log
	logFile = fopen ("registrosTiempos.log","a");
	fprintf (logFile,"[ %s ] %s : %s \n " ,stnow,id,msg) ;
	fclose (logFile);
}

void borrarCorredor(int posicion){
	corredores[posicion].box=FALSE;
	corredores[posicion].sancionado=FALSE;
	corredores[posicion].irreparable=FALSE;
	sprintf(corredores[posicion].id,"Corredor_0");
	corredores[posicion].numID=0;
}


void *hiloCorredor(void *ptr){
	int i = 0;
	int problemasMecanicos,tVuelta;
	int posCorredor = *(int*)ptr;

	pthread_mutex_lock(&mutexLog);

	writeLogMessage(corredores[posCorredor].id,"Entra a pista.");

	pthread_mutex_unlock(&mutexLog);

	for(i;i<5;i++){

		tVuelta=rand()%4+2;
		problemasMecanicos=rand()%10+1;

		sleep(tVuelta);

		if(problemasMecanicos<6){
			corredores[posCorredor].box=TRUE;
			while(corredores[posCorredor].box==TRUE){
				sleep(1);
			}

			if(corredores[posCorredor].irreparable == TRUE){

				pthread_mutex_lock(&mutexLog);

				writeLogMessage(corredores[posCorredor].id,"No se puede reparar.");

				pthread_mutex_unlock(&mutexLog);

				borrarCorredor(posCorredor);

				pthread_exit (NULL);

			}

		}

		if(corredores[posCorredor].sancionado==TRUE){

			pthread_mutex_lock(&mutexStop);

			pthread_cond_signal(&(corredores[posCorredor].stop));

			pthread_mutex_unlock(&mutexStop);

			pthread_mutex_lock(&mutexGo);

			pthread_cond_wait(&(corredores[posCorredor].go), &mutexGo);

			pthread_mutex_unlock(&mutexGo);
		}


	}

	pthread_mutex_lock(&mutexLog);

	writeLogMessage(corredores[posCorredor].id,"Finaliza la carrera.");

	pthread_mutex_unlock(&mutexLog);

	borrarCorredor(posCorredor);

	pthread_exit (NULL);

}

//time_t t1,t2
//t1=time(0);
//t2=time(0);
//vuelta=t2-t1;

void nuevoCorredor(int sig){

	
	if(signal(SIGUSR1,nuevoCorredor)==SIG_ERR){
		perror("Error in signal call");
		exit(-1);
	}

	if(numeroCorredores==NC){
		return;
	}

	pthread_t hilo;

	int i;

	pthread_mutex_lock(&mutexCircuito);

	for(i=0;i<NC;i++){
		if(corredores[i].numID == 0){
			numeroCorredores++;
			corredores[i].numID=++numeroCorredoresTotal;
			sprintf(corredores[i].id,"Corredor_%d",numeroCorredoresTotal);
			break;
		}
	}

	pthread_mutex_unlock(&mutexCircuito);

	pthread_create(&hilo, NULL, hiloCorredor, (void*)&i);
	
}



void *hiloBox(void *ptr) {
	struct box *box = (struct box*) ptr;
	char * msg;
	struct corredor *corredor;
	int corredoresAtendidos = 0;

	do{
		while(*corredoresBoxes[0]==NULL){
			sleep(1);
		}
		corredor = corredoresBoxes[0];
		// ACTUALIZA CORREDORESBOXES

		pthread_mutex_lock(&mutexLog);
		sprintf(msg, "Atiende a %s", (*corredor).id);
		writeLogMessage((*box).id, msg);
		pthread_mutex_unlock(&mutexLog);

		sleep(rand() % 3 + 1);
		if (rand() % 10 >= 3)
			(*corredor).irreparable = TRUE;

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
		(*corredor).box=TRUE;
	}while(finCarrera==FALSE);
}

void *hiloJuez(void *ptr){
	sleep(10);
	int aleatorio;
	do{
		aleatorio=rand()%NC;
	}while(corredores[aleatorio].numID==0);

	corredores[aleatorio].sancionado=TRUE;

	pthread_mutex_lock(&mutexStop);

	pthread_cond_wait(&(corredores[aleatorio].stop), &mutexStop);

	pthread_mutex_unlock(&mutexStop);

	sleep(3);

	corredores[aleatorio].sancionado=FALSE;

	pthread_mutex_lock(&mutexGo);

	pthread_cond_signal(&(corredores[aleatorio].go));

	pthread_mutex_unlock(&mutexGo);
}


int main(void){

	int i;

	srand(time(NULL));

	if(signal(SIGUSR1,nuevoCorredor)==SIG_ERR){
		perror("Error in signal call");
		exit(-1);
	}

	for(i=0;i<NC;i++){

		struct corredor miCorredor;
		
		if (pthread_cond_init(&(miCorredor.go), NULL)!=0) exit(-1);

		if (pthread_cond_init(&(miCorredor.stop), NULL)!=0) exit(-1);

		miCorredor.box=FALSE;
		miCorredor.sancionado=FALSE;
		miCorredor.irreparable=FALSE;
		sprintf(id,"Corredor_0");
		miCorredor.id=id;
		miCorredor.numID=0;
	}

	pthread_mutex_init(&mutexCircuito, NULL);
	pthread_mutex_init(&mutexGo, NULL);
	pthread_mutex_init(&mutexStop, NULL);
	pthread_mutex_init(&mutexLog, NULL);

	numeroCorredores = 0;
	numeroCorredoresTotal = 0;

	struct box param_box1 = {"Box_1", FALSE};
	struct box param_box2 = {"Box_2", FALSE};

	pthread_t box1, box2, juez;
	

	pthread_create (&box1, NULL, hiloBox, (void*)&param_box1);
	pthread_create (&box2, NULL, hiloBox, (void*)&param_box2);
	pthread_create (&juez, NULL, hiloJuez, NULL);
	
}


