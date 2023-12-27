#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>

#define MAX_CLIENTS 20
#define MAX_CAJEROS 3
// PracticaFinal
// char logFilePath[] = "/ruta/del/archivo.log"; No hay ruta de archivo ya que vamos a considerar que esta en la misma ruta que el ejecutable

FILE *logFile;
pthread_mutex_t logSemaforo;
pthread_mutex_t repSemaforo;
pthread_mutex_t cliSemaforo;

struct cliente *clientes; // Array de clientes
//semaforo 1 y 2 y 3
//Variable de condicion para reponedor
//char clientes = (char*) malloc(sizeof(char) * MAX_CLIENTS * 2);
enum Estado {
    ESTADO_0, // esperando
    ESTADO_1, // en cajero
    ESTADO_2, // atendido
};

// Los clientes tiene que ser un array de estructuras que almacene el id y su estado
struct cliente{
    int id;
    int estado;
};


void *Reponedor (void *arg){

}

void *Cajero (void *arg){
    int clientesAtendidos = 0;
    int cajeroID = *((int *)arg);

    while(1){
        int indexCliente = -1;
        for(int i = 0; i < MAX_CLIENTS; i++){
            if(cliente[i].ESTADO == 0){
                if(indexCliente = -1 || cliente[i].id < cliente[indexCliente].id){
                    indexCliente = i;
                }
            }
        }
        if(indexCliente != -1){
            cliente[indexCliente].ESTADO = 1;
            writeLogMessage(cajeroID, "Atendiendo a cliente");
            int cooldown = randomizer(5, 1);
            sleep(cooldown);
            inr random = randomizer(100, 1);
            if (random >= 71 && random <= 95) {
                writeLogMessage(cajeroID, "Aviso al reponedor para comprobar un precio.");
                pthread_cond_signal(&Reponedor);
                pthread_cond_wait(&Reponedor, &repSemaforo);
            } else if (random >= 96 && random <= 100) {
                writeLogMessage(cajeroID, "Cliente tiene problemas y no puede realizar la compra.");
            }
            // TO DO Escribir informacion en el log
            cliente[indexCliente].ESTADO = 2;
            clientesAtendidos++;
            if(clientesAtendidos % 10 == 0){
                writeLogMessage(cajeroID, "Descanso 20 seg");
                sleep(20);
            }
        }
    }
}

int randomizer(int max, int min){
    srand(gettid());
    return rand() % (max - min +1) + min;
}

// La cola de clientes se puede implementar con un array con los pids y expulsar al de menor pid, o una COLA como estructura de datos.
int main(int argc, char* argv){
    if(argc==1 || numClientes < 1){
        printf("Argumento introduccido incorrecto. Debes introducir un número de asistentes y que se mayor de 1.\n");
        return 1;
    }
    pthread_mutex_init(&logSemaforo, NULL);
    pthread_mutex_init(&repSemaforo, NULL);
    pthread_mutex_init(&cliSemaforo, NULL);

    pthread_cond_t repCondicion;
    pthread_cond_init(&repCondicion, NULL);

    logFile = fopen("logFile.txt", "w");
    if (logFile == NULL) {
        printf("Error opening log file.\n");
        return 1;
    }

    clientes = (struct cliente*) malloc(sizeof(struct cliente) * MAX_CLIENTS); // Asignación de memoria en la función main

    for (int i = 0; i < MAX_CLIENTS; i++) {
        (clientes+i)->id = i;
        (clientes+i)->estado = ESTADO_0;
    }
    
    struct sigaction ss;
    ss.sa_handler = añadirCliente;
    sigaction(SIGUSR1, &ss, NULL);
    pthread_t cajero1, cajero2, cajero3, reponedor;
    pthread_attr_t attr;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); 
    pthread_create(&cajero1, &attr, Cajero, "Cajero 1 en su puesto");
    pthread_create(&cajero2, &attr, Cajero, "Cajero 2 en su puesto");
    pthread_create(&cajero3, &attr, Cajero, "Cajero 3 en su puesto");
    pthread_create(&reponedor, &attr, Reponedor, "Reponedor listo para ser util");
    pause();
    if(pthread_mutex_destroy(&logSemaforo) != 0) exit(-1);
    if(pthread_mutex_destroy(&repSemaforo) != 0) exit(-1);
    if(pthread_mutex_destroy(&cliSemaforo) != 0) exit(-1);
    free(clientes);
    return 0;
}


void writeLogMessage ( char * id , char * msg ) {
    pthread_mutex_lock(&logSemaforo);
    // Calculamos la hora actual
    time_t now = time (0) ;
    struct tm * tlocal = localtime (& now ) ;
    char stnow [25];
    strftime ( stnow , 25 , " %d/ %m/ %y %H: %M: %S", tlocal );
    // Escribimos en el log
    logFile = fopen ( logFile , "a");
    fprintf ( logFile , "[ %s] %s: %s\n", stnow , id , msg );
    fclose ( logFile );
    pthread_mutex_unlock(&logSemaforo);
}

void añadirCliente(int sig){
    // Meter en la cola de clientes si todos lo cajeros estan ocupados
    // Si no, meter en algun cajero
    int i;
    for(i = 0; i < MAX_CLIENTS; i++){
        if((clientes+i)->id == 0){
            pthread_t cliente;
            // client.id -> +, Sacar el id del cliente para sumar 1 y sacar el id del siguiente cliente.
            // Hacer metodo para buscar el mayor id de cliente en el array para determinar id del siguiente.
            pthread_create(&cliente, NULL, cliente, (void*)i);
            (clientes+i)->id = 1;
            break;
        }
    }
}

int menorIDCliente(){
    int menorId = INT_MAX;
    for(int i = 0; i < MAX_CLIENTS; i++){
        if((clientes+i)->id < menorId && (clientes+i)->id != 0){
            menorId = (clientes+i)->id ;
        }
    }
    return menorId;
}

