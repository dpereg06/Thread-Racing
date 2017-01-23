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
	// valores del campo box: 0 = en pista / 1 = en espera / -1 = en reparación
	int box, sancionado, irreparable, numID;
	// variables condición para comunicarse con el juez
	pthread_cond_t go;
	pthread_cond_t stop;
};

// array para comprobar el estado de los boxes (1 = abierto / 0 = cerrado)
int boxesAbiertos[2];

// declaración de mutex
pthread_mutex_t mutexGo;
pthread_mutex_t mutexStop;
pthread_mutex_t mutexLista;
pthread_mutex_t mutexLog;
pthread_mutex_t mutexFin;
pthread_cond_t fin;
// contadores de corredores
int numeroCorredores, numeroCorredoresTotal, NC;
// lista de corredores
struct corredor * corredores;
// id y tiempo del ganador
char * idGanador;
double segGanador;
// fichero de log
FILE * logFile;

int main(int argc, char*argv[]) {

	// Comprobamos si se ha introducido por parámatro un numero distinto de corredores
	if (argc > 1)
		NC = atoi(argv[1]);
	else
		NC = 5;

	corredores = (struct corredor *) malloc(NC * sizeof(struct corredor));

	struct sigaction sNuevo, sFin;
	sNuevo.sa_handler = nuevoCorredor;
	sFin.sa_handler = finCarrera;

	// Esperamos a la señal SIGUSR1	
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

	// Atributos que tomaran por defecto los corredores al comienzo de la carrera

	miCorredor.box = FALSE;
	miCorredor.sancionado = FALSE;
	miCorredor.irreparable = FALSE;
	miCorredor.id = "Corredor_0";
	miCorredor.numID = 0;

	// Inicializamos los corredores, variables de condición y mutex
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

	// Al comienzo, los dos boxes estan abiertos
	boxesAbiertos[0] = TRUE;
	boxesAbiertos[1] = TRUE;
	int a = 0, b = 1;

	pthread_t box1, box2, juez;

	pthread_create(&box1, NULL, hiloBox, (void*) &a);
	pthread_create(&box2, NULL, hiloBox, (void*) &b);
	pthread_create(&juez, NULL, hiloJuez, NULL);

	// Espera por el hilo ganador si lo hay, sino se indicara
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

// función que restaura a los valores por defecto los campos del corredor
// que ocupa en la lista la posición indicada como parámetro (numID a 0 simboliza corredor vacío)
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

		//Entrada a boxes por problemas mecánicos.

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

			//Comprueba si puede continuar la carrera después de su paso por boxes.

			if (corredores[posCorredor].irreparable == TRUE) {

				writeLogMessage(corredores[posCorredor].id,
						"No se puede reparar y abandona.");

				borrarCorredor(posCorredor);

				//Se hace signal para hacer saber al juez que no puede cumplir la sanción porque abandona la carrera y así no quede bloqueado.

				pthread_mutex_lock(&mutexStop);
				pthread_cond_signal(&(corredores[posCorredor].stop));
				pthread_mutex_unlock(&mutexStop);

				pthread_exit(NULL);

			}

			writeLogMessage(corredores[posCorredor].id, "Sale de boxes.");

		}

		//Comprueba si esta sancionado y en caso afirmativo la cumple.

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

		//Se calcula el tiempo que tarda en dar una vuelta

		segVuelta = difftime(tf_vuelta, ti_vuelta);

		sprintf(msg, "Termina la vuelta %d en %.2f segundos.", i + 1,
				segVuelta);

		writeLogMessage(corredores[posCorredor].id, msg);

	}

	tf_carrera = time(0);

	//Se calcula el tiempo que tarda en terminar la carrera

	segCarrera = difftime(tf_carrera, ti_carrera);

	sprintf(msg, "Finaliza la carrera en %.2f segundos.", segCarrera);

	writeLogMessage(corredores[posCorredor].id, msg);

	//Si ha sido el más rapido se guarda su tiempo y su id.

	if (segGanador == 0 || segCarrera < segGanador) {
		segGanador = segCarrera;
		idGanador = corredores[posCorredor].id;
	}

	borrarCorredor(posCorredor);

	pthread_exit(NULL);
}

void nuevoCorredor(int sig) {

	pthread_mutex_lock(&mutexLista);

//Controlamos que el corredor se cree solo si el numero de corredores es menor que el maximo permitido
	if (numeroCorredores < NC) {

		pthread_t hilo;

		int i;

		char * id = malloc(sizeof(char) * 15);

//Asignamos valores a los atributos de un corredor del array que estuviera a 0
		for (i = 0; i < NC; i++) {
			if (corredores[i].numID == 0) {
				numeroCorredores++;
				corredores[i].numID = ++numeroCorredoresTotal;
				sprintf(id, "Corredor_%d", numeroCorredoresTotal);
				corredores[i].id = id;
				break;
			}
		}
//Iniciamos el hilo que controlara las acciones del corredor
		pthread_create(&hilo, NULL, hiloCorredor, (void*) &i);
	}
	pthread_mutex_unlock(&mutexLista);

}

// manejadora de la señal SIGINT que detiene la carrera y vuelca al log el mensaje final con el ganador
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

// Acciones a realizar por los hilos de los boxes
// Recibe el índice del box en el array boxesAbiertos
void *hiloBox(void *ptr) {
	char * id = malloc(sizeof(char) * 15);
	char * msg = malloc(sizeof(char) * 50);
	int corredoresAtendidos = 0;     // contador para cerrar cada 3 reparaciones
	int posBox = *(int*) ptr; // posición del box en el array boxesAbiertos (0/1)
	int trabajo = FALSE; // variable para comprobar si hay corredores esperando atención
	int i;
	int posCorredor;     // posición del corredor al que atiende en la lista
	int idCorredor = 0; // almacena el id mínimo entre los corredores que esperan atención
	sprintf(id, "Box_%d", posBox + 1);

	while (TRUE) {
		/* Bucle para buscar un corredor que esté esperando a ser atendido
		 * Recorre la lista de corredores en busca del de menor id entre ellos (mayor tiempo de espera)
		 * Si no hay ninguno duerme 1 segundo y vuelve a buscar
		 */
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
			// Si ha encontrado un corredor para atender lo pone en reparación y sale del bucle
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

		// Duerme el tiempo de reparación
		sleep(rand() % 3 + 1);

		writeLogMessage(id, "Termina la reparación.");

		pthread_mutex_lock(&mutexLista);
		corredores[posCorredor].box = FALSE;
		// Comprueba si el problema es irreparable
		if (rand() % 10 >= 7)
			corredores[posCorredor].irreparable = TRUE;
		pthread_mutex_unlock(&mutexLista);

		// Comprueba si tiene que cerrar y si puede hacerlo (si el otro box no está cerrado)
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

		//Sanciona a un corredor en pista que no se encuentre en boxes

		do {
			aleatorio = rand() % NC;
		} while (corredores[aleatorio].numID == 0
				|| corredores[aleatorio].box != FALSE);

		pthread_mutex_lock(&mutexLista);
		corredores[aleatorio].sancionado = TRUE;
		pthread_mutex_unlock(&mutexLista);

		sprintf(msg, "Sanciona a %s.", corredores[aleatorio].id);
		writeLogMessage("Juez", msg);

		//Espera a que el corredor empiece la sanción para hacersela cumplir, excepto si ha abandonado por problemas mecánicos irreparables.

		pthread_mutex_lock(&mutexStop);
		pthread_cond_wait(&(corredores[aleatorio].stop), &mutexStop);
		pthread_mutex_unlock(&mutexStop);

		if (corredores[aleatorio].sancionado == TRUE) {
			sleep(3);
			pthread_mutex_lock(&mutexLista);
			corredores[aleatorio].sancionado = FALSE;
			pthread_mutex_unlock(&mutexLista);
			writeLogMessage("Juez", "Sanción cumplida.");
		} else
			writeLogMessage("Juez", "Abandono del corredor sancionado.");

		pthread_mutex_lock(&mutexGo);
		pthread_cond_signal(&(corredores[aleatorio].go));
		pthread_mutex_unlock(&mutexGo);
	}
}
