#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_CLIENTS 20
#define MAX_CAJEROS 3
// PracticaFinal
FILE *logFile
//semaforo 1 y 2 y 3
//Variable de condicion para reponedor
char clientes = (char*)malloc(sizeof(char) * MAX_CLIENTS * 2);

void *Reponedor (void *arg){

}

void *Cajero (void *arg){

}

int randomizer(int max, int min){
    srand(gettid());
    return rand() % (max - min +1) + min;
}

int main(int argc, char* argv){
    if(argc==1){
        printf("No has introducido ningún argumento. Debes introducir un número de asistentes mayor que 1.\n");
        return 1;
    }
    struct sigaction ss;
    // if(numClientes<1){
    //     printf("El valor introducido es incorrecto. Debes introducir un número de asistentes mayor que 1.\n");
    //     return 1;
    // }
    ss.sa_handler = addClient;
    sigaction(SIGUSR1, &ss, NULL);
    pthread_t cajero1, cajero2, cajero3, reponedor;
    pthread_attr_t attr;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); 
    pthread_create (&cajero1, &attr, Cajero, "Cajero 1 en su puesto");
    pthread_create (&cajero2, &attr, Cajero, "Cajero 2 en su puesto");
    pthread_create (&cajero3, &attr, Cajero, "Cajero 3 en su puesto");
    pthread_create (&reponedor, &attr, Reponedor, "Reponedor listo para ser util");
    pause();

}


void writeLogMessage ( char * id , char * msg ) {
    // Calculamos la hora actual
    time_t now = time (0) ;
    struct tm * tlocal = localtime (& now ) ;
    char stnow [25];
    strftime ( stnow , 25 , " %d/ %m/ %y %H: %M: %S", tlocal );
    // Escribimos en el log
    logFile = fopen ( logFile , "a");
    fprintf ( logFile , "[ %s] %s: %s\n", stnow , id , msg );
    fclose ( logFile );
}

void addClient(int sig){
    // Meter en la cola de clientes si todos lo cajeros estan ocupados
    // Si no, meter en algun cajero
    clients;
    cambio;
}

