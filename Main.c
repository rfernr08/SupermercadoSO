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

pthread_cond_t repCondicion;

int contadorIds = 0;


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

struct cliente *clientes; // Array de clientes

void *Reponedor (void *arg){
    while (1){
        pthread_mutex_lock(&repSemaforo);
        pthread_cond_wait(&Reponedor, &repSemaforo);
        sleep(randomizer(5, 1));
        writeLogMessage("Reponedor", "El reponedor ha terminado de trabajar");
        pthread_cond_signal(&Reponedor);
        pthread_mutex_unlock(&repSemaforo);
    }
}

void *Cajero (int *arg){
    int clientesAtendidos = 0;
    int cajeroID = *arg;
    while(1){
        struct cliente *clienteSeleccionado = menorIDCliente();
        int idCliente = clienteSeleccionado->id;
        if (clienteSeleccionado == NULL) pthread_exit(NULL);
        // TO DO Contactar con el cliente encontrado
        cambiarEstadoCliente(idCliente, ESTADO_1); // Marcar al cliente como atendido
        char atendiendo[100];
        sprintf(atendiendo, "Atendiendo a cliente %d", clienteSeleccionado->id);
        writeLogMessage(cajeroID, atendiendo);
        int cooldown = randomizer(5, 1);
        sleep(cooldown);
        int random = randomizer(100, 1);
        if(random >= 96 && random <= 100) {
            char problema[200];
            sprintf(problema, "Cliente %d tiene problemas y no puede realizar la compra.", clienteSeleccionado->id);
            writeLogMessage(cajeroID, problema);
        }else{
            if(random >= 71 && random <= 95){
                writeLogMessage(cajeroID, "Aviso al reponedor para comprobar un precio.");
                pthread_cond_signal(&Reponedor);
                pthread_cond_wait(&Reponedor, &repSemaforo);
            }
            int precio = randomizer(100, 1);
            char compra[100];
            sprintf(compra, "El precio de la compra es de %d", precio);
            writeLogMessage(clienteSeleccionado->id, compra);
        }
        // TO DO Escribir informacion en el log
        cambiarEstadoCliente(idCliente, ESTADO_2);
        clientesAtendidos++;
        if(clientesAtendidos % 10 == 0){
            writeLogMessage(cajeroID, "Me tomo un descanso 20 seg");
            sleep(20);
        }
        writeLogMessage(cajeroID, "Acabe mi descanso");
    }
}

void *Cliente(int arg) {
    struct cliente *clienteSeleccionado = buscarCliente(arg);
    int clienteID = clienteSeleccionado->id;
    int clienteEstado = clienteSeleccionado->estado;
    if(clienteSeleccionado == NULL) pthread_exit(NULL);
    writeLogMessage(clienteID, "Cliente en la fila");

    //int esperaMaxima = randomizer(10, 1);
    int comprobarSalida = 0;
    while(comprobarEstadoCliente(clienteID) != ESTADO_1){
        sleep(1);
        comprobarSalida++;
        if(comprobarSalida == 10){
            int posibleSalida = randomizer(10, 1);
            if(posibleSalida == 10){
                writeLogMessage(clienteID, "Cliente se va de la tienda");
                sacarCola(clienteID);
                pthread_exit(NULL);
            }
            comprobarSalida = 0;
        }
    }
    //sleep(esperaMaxima);
    // int probabilidadSalida = randomizer(10, 1);
    // if(esperaMaxima == 1){
    //     writeLogMessage(clienteID, "Cliente se va de la tienda");
    //     pthread_exit(NULL);
    // }
    
    // TO DO: Esperar si un agente atienda al cliente
    while(comprobarEstadoCliente(clienteID) != ESTADO_2){
        sleep(1);
    }
    writeLogMessage(clienteID, "Cliente atendido y finaliza compra");
    pthread_exit(NULL);
}

// La cola de clientes se puede implementar con un array con los pids y expulsar al de menor pid, o una COLA como estructura de datos.
int main(int argc, char* argv){
    // if(argc==1 || atoi(argv[1]) < 1){
    //     printf("Argumento introducido incorrecto. Debes introducir un número de asistentes que sea mayor de 1.\n");
    //     return 1;
    // }
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

    for(int i = 0; i < MAX_CLIENTS; i++){
        (clientes+i)->id = 0;
        (clientes+i)->estado = ESTADO_2;
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

// TO DO: Descubrir cuando se tiene que llamar a esta funcion, ya que no hace falta que se corte al meter a 1 cliente
void añadirCliente(int sig){
    // Meter en la cola de clientes si todos lo cajeros estan ocupados
    // Si no, meter en algun cajero
    pthread_mutex_lock(&cliSemaforo); // Lock the cliSemaforo semaphore
    for(int i = 0; i < MAX_CLIENTS; i++){
        if((clientes+i)->estado == ESTADO_2){
            pthread_t cliente;
            (clientes+i)->id = ++contadorIds;
            (clientes+i)->estado = ESTADO_0;
            pthread_create(&cliente, NULL, Cliente, contadorIds);
            break;
        }
    }
    pthread_mutex_unlock(&cliSemaforo); // Unlock the cliSemaforo semaphore
}

struct cliente *menorIDCliente(){
    struct cliente *clienteSeleccionado = NULL;
    int menorId = INT_MAX;
    pthread_mutex_lock(&cliSemaforo); // Lock the cliSemaforo semaphore
    for(int i = 0; i < MAX_CLIENTS; i++){
        if((clientes+i)->estado == ESTADO_0){
            if((clientes+i)->id < menorId && (clientes+i)->id != 0){
                menorId = (clientes+i)->id;
                clienteSeleccionado = (clientes+i);
            }
        }
    }
    pthread_mutex_unlock(&cliSemaforo); // Unlock the cliSemaforo semaphore
    return clienteSeleccionado;
}

void cambiarEstadoCliente(int clienteID, int estado){
    pthread_mutex_lock(&cliSemaforo); // Lock the cliSemaforo semaphore
    for(int i = 0; i < MAX_CLIENTS; i++){
        if((clientes+i)->id == clienteID){
            (clientes+i)->estado = estado;
        }
    }
    pthread_mutex_unlock(&cliSemaforo); // Unlock the cliSemaforo semaphore
}   

int comprobarEstadoCliente(int clienteID){
    int estado = -1;
    pthread_mutex_lock(&cliSemaforo); // Lock the cliSemaforo semaphore
    for(int i = 0; i < MAX_CLIENTS; i++){
        if((clientes+i)->id == clienteID){
            estado = (clientes+i)->estado;
        }
    }
    pthread_mutex_unlock(&cliSemaforo); // Unlock the cliSemaforo semaphore
    return estado;
}

void sacarCola(int clienteID){
    for(int i = 0; i < MAX_CLIENTS; i++){
        if((clientes+i)->id == clienteID){
            (clientes+i)->id = 0;
            (clientes+i)->estado = ESTADO_2;
        }
    }
}

struct cliente *buscarCliente(int clienteID){
    for(int i = 0; i < MAX_CLIENTS; i++){
        if((clientes+i)->id == clienteID){
            return (clientes+i);
        }
    }
    return NULL;
}

int randomizer(int max, int min){
    srand(gettid());
    return rand() % (max - min +1) + min;
}