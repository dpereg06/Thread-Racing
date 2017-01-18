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

struct corredor {
	char * id;
	int box, sancionado, irreparable, numID;
	pthread_cond_t go;
	pthread_cond_t stop;
};

int boxesAbiertos[2];

pthread_mutex_t mutexGo;
pthread_mutex_t mutexStop;
pthread_mutex_t mutexLista;
pthread_mutex_t mutexLog;
int numeroCorredores, numeroCorredoresTotal;
struct corredor corredores[NC];
FILE * logFile;

void writeLogMessage(char * id, char * msg) {
// Calculamos la hora actual
	time_t now = time(0);
	struct tm * tlocal = localtime(&now);
	char stnow[19];
	strftime(stnow, 19, "%d/ %m/ %y %H: %M: %S", tlocal);

// Escribimos en el log
	logFile = fopen("registrosTiempos.log", "a");
	fprintf(logFile, "[ %s ] %s : %s \n ", stnow, id, msg);
	fclose(logFile);
}

void borrarCorredor(int posicion) {
	corredores[posicion].box = FALSE;
	corredores[posicion].sancionado = FALSE;
	corredores[posicion].irreparable = FALSE;
	sprintf(corredores[posicion].id, "Corredor_0");
	corredores[posicion].numID = 0;
	numeroCorredores--;
}

void *hiloCorredor(void *ptr) {
	int i = 0;
	int problemasMecanicos, tVuelta;
	int posCorredor = *(int*) ptr;

	pthread_mutex_lock(&mutexLog);

	writeLogMessage(corredores[posCorredor].id, "Entra a pista.");

	pthread_mutex_unlock(&mutexLog);

	for (i; i < 5; i++) {

		tVuelta = rand() % 4 + 2;
		problemasMecanicos = rand() % 10 + 1;

		sleep(tVuelta);

		if (problemasMecanicos < 6) {
			corredores[posCorredor].box = TRUE;
			while (corredores[posCorredor].box == TRUE) {
				sleep(1);
			}

			if (corredores[posCorredor].irreparable == TRUE) {

				pthread_mutex_lock(&mutexLog);

				writeLogMessage(corredores[posCorredor].id,
						"No se puede reparar.");

				pthread_mutex_unlock(&mutexLog);

				borrarCorredor(posCorredor);

				pthread_exit(NULL);

			}

		}

		if (corredores[posCorredor].sancionado == TRUE) {

			pthread_mutex_lock(&mutexStop);

			pthread_cond_signal(&(corredores[posCorredor].stop));

			pthread_mutex_unlock(&mutexStop);

			pthread_mutex_lock(&mutexGo);

			pthread_cond_wait(&(corredores[posCorredor].go), &mutexGo);

			pthread_mutex_unlock(&mutexGo);
		}

	}

	pthread_mutex_lock(&mutexLog);

	writeLogMessage(corredores[posCorredor].id, "Finaliza la carrera.");

	pthread_mutex_unlock(&mutexLog);

	borrarCorredor(posCorredor);

	pthread_exit(NULL);

}

//time_t t1,t2
//t1=time(0);
//t2=time(0);
//vuelta=t2-t1;

void nuevoCorredor(int sig) {

	if (signal(SIGUSR1, nuevoCorredor) == SIG_ERR) {
		perror("Error in signal call");
		exit(-1);
	}

	if (numeroCorredores == NC) {
		return;
	}

	pthread_t hilo;

	int i;

	pthread_mutex_lock(&mutexLista);

	for (i = 0; i < NC; i++) {
		if (corredores[i].numID == 0) {
			numeroCorredores++;
			corredores[i].numID = ++numeroCorredoresTotal;
			sprintf(corredores[i].id, "Corredor_%d", numeroCorredoresTotal);
			break;
		}
	}

	pthread_mutex_unlock(&mutexLista);

	pthread_create(&hilo, NULL, hiloCorredor, (void*) &i);

}

void finCarrera(int sig) {
	//.........................................................................................
}

void *hiloBox(void *ptr) {
	char * id;
	char * msg;
	int corredoresAtendidos = 0, posBox = *(int*) ptr, trabajo = FALSE, i,
			posCorredor, idCorredor = 0;
	sprintf(id, "Box_%d", posBox + 1);

	while (TRUE) {
		while (TRUE) {
			pthread_mutex_lock(&mutexLista);
			for (i = 0; i < NC; i++)
				if (corredores[i].box == TRUE
						&& (idCorredor == 0 || corredores[i].numID < idCorredor)) {
					trabajo = TRUE;
					idCorredor = corredores[i].numID;
					posCorredor = i;
				}
			pthread_mutex_unlock(&mutexLista);
			if (trabajo == TRUE)
				break;
			sleep(1);
		}

		pthread_mutex_lock(&mutexLista);
		sprintf(msg, "Atiende a %s", corredores[posCorredor].id);
		pthread_mutex_unlock(&mutexLista);
		pthread_mutex_lock(&mutexLog);
		writeLogMessage(id, msg);
		pthread_mutex_unlock(&mutexLog);

		sleep(rand() % 3 + 1);

		pthread_mutex_lock(&mutexLista);
		if (rand() % 10 >= 3)
			corredores[posCorredor].irreparable = TRUE;
		else
			corredores[posCorredor].box = FALSE;
		pthread_mutex_unlock(&mutexLista);

		if ((++corredoresAtendidos >= 3)
				&& (boxesAbiertos[(posBox + 1) % 2] == TRUE)) {
			boxesAbiertos[posBox] = FALSE;
			corredoresAtendidos = 0;

			pthread_mutex_lock(&mutexLog);
			writeLogMessage(id, "Cierra");
			pthread_mutex_unlock(&mutexLog);

			sleep(20);

			pthread_mutex_lock(&mutexLog);
			writeLogMessage(id, "Reabre");
			pthread_mutex_unlock(&mutexLog);

			boxesAbiertos[posBox] = TRUE;
		}
	}
}

void *hiloJuez(void *ptr) {
	while (TRUE) {
		sleep(10);
		int aleatorio;

		pthread_mutex_lock(&mutexLista);
		do {
			aleatorio = rand() % NC;
		} while (corredores[aleatorio].numID == 0);
		corredores[aleatorio].sancionado = TRUE;
		pthread_mutex_unlock(&mutexLista);

		pthread_mutex_lock(&mutexStop);
		pthread_cond_wait(&(corredores[aleatorio].stop), &mutexStop);
		pthread_mutex_unlock(&mutexStop);

		sleep(3);

		pthread_mutex_lock(&mutexLista);
		corredores[aleatorio].sancionado = FALSE;
		pthread_mutex_unlock(&mutexLista);

		pthread_mutex_lock(&mutexGo);
		pthread_cond_signal(&(corredores[aleatorio].go));
		pthread_mutex_unlock(&mutexGo);
	}
}

int main(void) {

	int i;

	srand(time(NULL));

	if (signal(SIGUSR1, nuevoCorredor) == SIG_ERR) {
		perror("Error in signal call");
		exit(-1);
	}

	if (signal(SIGINT, finCarrera) == SIG_ERR) {
		perror("Error in signal call");
		exit(-1);
	}

	for (i = 0; i < NC; i++) {

		struct corredor miCorredor;

		if (pthread_cond_init(&(miCorredor.go), NULL) != 0)
			exit(-1);

		if (pthread_cond_init(&(miCorredor.stop), NULL) != 0)
			exit(-1);

		miCorredor.box = FALSE;
		miCorredor.sancionado = FALSE;
		miCorredor.irreparable = FALSE;
		sprintf(miCorredor.id, "Corredor_0");
		miCorredor.numID = 0;

		corredores[i] = miCorredor;
	}

	pthread_mutex_init(&mutexLista, NULL);
	pthread_mutex_init(&mutexGo, NULL);
	pthread_mutex_init(&mutexStop, NULL);
	pthread_mutex_init(&mutexLog, NULL);

	numeroCorredores = 0;
	numeroCorredoresTotal = 0;

	boxesAbiertos[0] = TRUE;
	boxesAbiertos[1] = TRUE;
	int a = 0, b = 1;

	pthread_t box1, box2, juez;

	pthread_create(&box1, NULL, hiloBox, (void*) &a);
	pthread_create(&box2, NULL, hiloBox, (void*) &b);
	pthread_create(&juez, NULL, hiloJuez, NULL);

}
