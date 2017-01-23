//PARA EVITAR CONFLICTOS ESTABLECEREMOS EL 0 COMO FALSE Y 1 COMO TRUE PARA LOS INT BINARIOS.

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <string.h>

#define NC 5
#define FALSE 0
#define TRUE 1

void writeLogMessage(char * id, char * msg);
void borrarCorredor(int posicion);
void *hiloCorredor(void *ptr);
void nuevoCorredor(int sig);
void finCarrera(int sig);
void *hiloBox(void *ptr);
void *hiloJuez(void *ptr);

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
pthread_mutex_t mutexFin;
pthread_cond_t fin;
int numeroCorredores, numeroCorredoresTotal;
struct corredor corredores[NC];
char * idGanador;
double segGanador;
FILE * logFile;

int main(void) {

	struct sigaction sNuevo, sFin;
	sNuevo.sa_handler = nuevoCorredor;
	sFin.sa_handler = finCarrera;
	if (sigaction(SIGUSR1, &sNuevo, NULL) == -1) {
		perror("Error in signal call");
		exit(-1);
	}
	if (sigaction(SIGINT, &sFin, NULL) == -1) {
		perror("Error in signal call");
		exit(-1);
	}

	int i;
	struct corredor miCorredor;

	char * idGanador = malloc(sizeof(char) * 15);

	srand(time(NULL));

	logFile = fopen("registroTiempos.log", "w");

	miCorredor.box = FALSE;
	miCorredor.sancionado = FALSE;
	miCorredor.irreparable = FALSE;
	miCorredor.id = "Corredor_0";
	miCorredor.numID = 0;

	for (i = 0; i < NC; i++) {

		corredores[i] = miCorredor;

		if (pthread_cond_init(&(corredores[i].go), NULL) != 0)
			exit(-1);

		if (pthread_cond_init(&(corredores[i].stop), NULL) != 0)
			exit(-1);
	}

	pthread_mutex_init(&mutexLista, NULL);
	pthread_mutex_init(&mutexGo, NULL);
	pthread_mutex_init(&mutexStop, NULL);
	pthread_mutex_init(&mutexLog, NULL);
	pthread_mutex_init(&mutexFin, NULL);

	if (pthread_cond_init(&(fin), NULL) != 0)
		exit(-1);

	numeroCorredores = 0;
	numeroCorredoresTotal = 0;
	segGanador = 0.0;

	boxesAbiertos[0] = TRUE;
	boxesAbiertos[1] = TRUE;
	int a = 0, b = 1;

	pthread_t box1, box2, juez;

	pthread_create(&box1, NULL, hiloBox, (void*) &a);
	pthread_create(&box2, NULL, hiloBox, (void*) &b);
	pthread_create(&juez, NULL, hiloJuez, NULL);

	pthread_mutex_lock(&mutexFin);
	pthread_cond_wait(&fin, &mutexFin);
	pthread_mutex_unlock(&mutexFin);

}

void writeLogMessage(char * id, char * msg) {
	pthread_mutex_lock(&mutexLog);
// Calculamos la hora actual
	time_t now = time(0);
	struct tm * tlocal = localtime(&now);
	char stnow[19];
	strftime(stnow, 19, "%d/%m/%y %H:%M:%S", tlocal);

// Escribimos en el log
	logFile = fopen("registroTiempos.log", "a");
	fprintf(logFile, "[%s] %s: %s\n", stnow, id, msg);
	fclose(logFile);
	pthread_mutex_unlock(&mutexLog);
}

void borrarCorredor(int posicion) {
	pthread_mutex_lock(&mutexLista);
	corredores[posicion].box = FALSE;
	corredores[posicion].sancionado = FALSE;
	corredores[posicion].irreparable = FALSE;
	corredores[posicion].id = "Corredor_0";
	corredores[posicion].numID = 0;
	numeroCorredores--;
	pthread_mutex_unlock(&mutexLista);
}

void *hiloCorredor(void *ptr) {
	int i = 0;
	int problemasMecanicos, tVuelta;
	int posCorredor = *(int*) ptr;
	time_t ti_vuelta, tf_vuelta, ti_carrera, tf_carrera;
	double segVuelta, segCarrera;
	char * msg = malloc(sizeof(char) * 50);

	ti_carrera = time(0);

	writeLogMessage(corredores[posCorredor].id, "Entra a pista.");

	for (i; i < 5; i++) {

		ti_vuelta = time(0);

		tVuelta = rand() % 4 + 2;
		problemasMecanicos = rand() % 10 + 1;

		sleep(tVuelta);

		if (problemasMecanicos < 6) {

			writeLogMessage(corredores[posCorredor].id,
					"Esperando a ser reparado.");

			pthread_mutex_lock(&mutexLista);
			corredores[posCorredor].box = TRUE;
			pthread_mutex_unlock(&mutexLista);

			while (corredores[posCorredor].box == TRUE) {
				sleep(1);
			}

			writeLogMessage(corredores[posCorredor].id,
					"Comienza su reparación.");

			while (corredores[posCorredor].box != FALSE) {
				sleep(1);
			}

			if (corredores[posCorredor].irreparable == TRUE) {

				writeLogMessage(corredores[posCorredor].id,
						"No se puede reparar y abandona.");

				borrarCorredor(posCorredor);

				pthread_exit(NULL);

			}

			writeLogMessage(corredores[posCorredor].id, "Sale de boxes.");

		}

		if (corredores[posCorredor].sancionado == TRUE) {

			pthread_mutex_lock(&mutexStop);

			pthread_cond_signal(&(corredores[posCorredor].stop));

			pthread_mutex_unlock(&mutexStop);

			writeLogMessage(corredores[posCorredor].id,
					"Empieza a cumplir la sanción.");

			pthread_mutex_lock(&mutexGo);

			pthread_cond_wait(&(corredores[posCorredor].go), &mutexGo);

			pthread_mutex_unlock(&mutexGo);

			writeLogMessage(corredores[posCorredor].id,
					"Ha cumplido la sanción.");

		}

		tf_vuelta = time(0);

		segVuelta = difftime(tf_vuelta, ti_vuelta);

		sprintf(msg, "Termina la vuelta %d en %.2f segundos.", i + 1,
				segVuelta);

		writeLogMessage(corredores[posCorredor].id, msg);

	}

	tf_carrera = time(0);

	segCarrera = difftime(tf_carrera, ti_carrera);

	sprintf(msg, "Finaliza la carrera en %.2f segundos.", segCarrera);

	writeLogMessage(corredores[posCorredor].id, msg);

	if (segGanador == 0 || segCarrera < segGanador) {
		segGanador = segCarrera;
		idGanador = corredores[posCorredor].id;
	}

	borrarCorredor(posCorredor);

	pthread_exit(NULL);

}

void nuevoCorredor(int sig) {

	pthread_mutex_lock(&mutexLista);
	if (numeroCorredores < NC) {

		pthread_t hilo;

		int i;

		char * id = malloc(sizeof(char) * 15);

		for (i = 0; i < NC; i++) {
			if (corredores[i].numID == 0) {
				numeroCorredores++;
				corredores[i].numID = ++numeroCorredoresTotal;
				sprintf(id, "Corredor_%d", numeroCorredoresTotal);
				corredores[i].id = id;
				break;
			}
		}

		pthread_create(&hilo, NULL, hiloCorredor, (void*) &i);
	}
	pthread_mutex_unlock(&mutexLista);

}

void finCarrera(int sig) {
	char * msg = malloc(sizeof(char) * 150);
	if (segGanador == 0.0) {
		sprintf(msg,
				"Fin de la carrera / Nº total de corredores: %d / Ningún corredor ha terminado la carrera.",
				numeroCorredoresTotal);
		writeLogMessage("Dirección de carrera", msg);
	} else {
		sprintf(msg,
				"Fin de la carrera / Nº total de corredores: %d / Ganador: %s (%.2f segundos).",
				numeroCorredoresTotal, idGanador, segGanador);
		writeLogMessage("Dirección de carrera", msg);
	}
	pthread_cond_signal(&fin);
}

void *hiloBox(void *ptr) {
	char * id = malloc(sizeof(char) * 15);
	char * msg = malloc(sizeof(char) * 50);
	int corredoresAtendidos = 0, posBox = *(int*) ptr, trabajo = FALSE, i,
			posCorredor, idCorredor = 0;
	sprintf(id, "Box_%d", posBox + 1);

	while (TRUE) {
		while (TRUE) {
			pthread_mutex_lock(&mutexLista);
			for (i = 0; i < NC; i++) {
				if (corredores[i].box == TRUE && corredores[i].numID != 0
						&& (idCorredor == 0 || corredores[i].numID < idCorredor)) {
					trabajo = TRUE;
					idCorredor = corredores[i].numID;
					posCorredor = i;
				}
			}
			idCorredor = 0;
			if (trabajo == TRUE)
				corredores[posCorredor].box = -1;
			pthread_mutex_unlock(&mutexLista);
			if (trabajo == TRUE)
				break;
			sleep(1);
		}
		trabajo = FALSE;

		sprintf(msg, "Se prepara para atender a %s.",
				corredores[posCorredor].id);
		writeLogMessage(id, msg);

		sleep(rand() % 3 + 1);

		writeLogMessage(id, "Termina la reparación.");

		pthread_mutex_lock(&mutexLista);
		corredores[posCorredor].box = FALSE;
		if (rand() % 10 >= 7)
			corredores[posCorredor].irreparable = TRUE;
		pthread_mutex_unlock(&mutexLista);

		if (++corredoresAtendidos >= 3) {
			if (boxesAbiertos[(posBox + 1) % 2] == TRUE) {
				boxesAbiertos[posBox] = FALSE;
				corredoresAtendidos = 0;

				writeLogMessage(id, "Cierra temporalmente.");

				sleep(20);

				writeLogMessage(id, "Reabre.");

				boxesAbiertos[posBox] = TRUE;
			} else
				writeLogMessage(id, "No puede cerrar.");
		}
	}
}

void *hiloJuez(void *ptr) {
	while (TRUE) {
		sleep(10);
		int aleatorio;
		char * msg = malloc(sizeof(char) * 50);

		do {
			aleatorio = rand() % NC;
		} while (corredores[aleatorio].numID == 0);

		pthread_mutex_lock(&mutexLista);
		corredores[aleatorio].sancionado = TRUE;
		pthread_mutex_unlock(&mutexLista);

		sprintf(msg, "Sanciona a %s.", corredores[aleatorio].id);
		writeLogMessage("Juez", msg);

		pthread_mutex_lock(&mutexStop);
		pthread_cond_wait(&(corredores[aleatorio].stop), &mutexStop);
		pthread_mutex_unlock(&mutexStop);

		sleep(3);

		writeLogMessage("Juez", "Sanción cumplida.");

		pthread_mutex_lock(&mutexLista);
		corredores[aleatorio].sancionado = FALSE;
		pthread_mutex_unlock(&mutexLista);

		pthread_mutex_lock(&mutexGo);
		pthread_cond_signal(&(corredores[aleatorio].go));
		pthread_mutex_unlock(&mutexGo);
	}
}

