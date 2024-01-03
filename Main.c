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

void *Cajero (void *arg){
    int clientesAtendidos = 0;
    //int cajeroID = *((int *)arg);
    while(1){
        pthread_mutex_lock(&cliSemaforo);
        struct cliente *clienteSeleccionado = menorIDCliente();
        if (clienteSeleccionado != NULL) {
            // TO DO Contactar con el cliente encontrado
            clienteSeleccionado->estado = ESTADO_1; // Marcar al cliente como atendido
            writeLogMessage(clienteSeleccionado->id, "Atendiendo a cliente");
            int cooldown = randomizer(5, 1);
            sleep(cooldown);
            int random = randomizer(100, 1);
            if(random >= 96 && random <= 100) {
                writeLogMessage(clienteSeleccionado->id, "Cliente tiene problemas y no puede realizar la compra.");
            }else{
<<<<<<< Updated upstream
                if(random >= 71 && random <= 95){
                    writeLogMessage(clienteSeleccionado->id, "Aviso al reponedor para comprobar un precio.");
                    pthread_cond_signal(&Reponedor);
                    pthread_cond_wait(&Reponedor, &repSemaforo);
                }
                int precio = randomizer(100, 1);
                char compra[100];
                sprintf(compra, "El precio de la compra es de %d", precio);
                writeLogMessage(clienteSeleccionado->id, compra);
            }
            // TO DO Escribir informacion en el log
            clienteSeleccionado->estado = 2;
            clientesAtendidos++;
            if(clientesAtendidos % 10 == 0){
                writeLogMessage(CajeroID, "Me tomo un descanso 20 seg");
                sleep(20);
=======
                // TO DO Escribir informacion en el log
                clienteSeleccionado->estado = 2;
                clientesAtendidos++;
                if(clientesAtendidos % 10 == 0){
                    writeLogMessage(clienteSeleccionado->id, "Descanso 20 seg");
                    sleep(20);
                }
>>>>>>> Stashed changes
            }
            writeLogMessage(CajeroID, "Acabe mi descanso");
        }
        pthread_mutex_unlock(&cliSemaforo);
    }
}

void *Cliente(int arg) {
    int clienteID = arg;
    writeLogMessage(clienteID, "Cliente en la fila");

    int esperaMaxima = randomizer(10, 1);
    sleep(esperaMaxima);
    
    // TO DO: Esperar a que un agente atienda al cliente

    // TO DO: Guardar en el log la hora de finalización
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

    // To DO: puede que haya que convertir el id en una variable global (Lo he hecho por ahora pero puede que no sea tan buena idea)
    for (int i = 0; i < MAX_CLIENTS; i++) {
        (clientes+i)->id = contadorIds++;
        (clientes+i)->estado = ESTADO_0;
        pthread_create(&clientes[i], NULL, Cliente, contadorIds);
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
    int i;
    for(i = 0; i < MAX_CLIENTS; i++){
        if((clientes+i)->id == 0){
            pthread_t cliente;
            (clientes+i)->id = contadorIds++;
            (clientes+i)->estado = ESTADO_0;
            pthread_create(&cliente, NULL, cliente, contadorIds);
            break;
        }
    }
}

struct cliente *menorIDCliente(){
    struct cliente *clienteSeleccionado = NULL;
    int menorId = INT_MAX;
    for(int i = 0; i < MAX_CLIENTS; i++){
        if((clientes+i)->estado == ESTADO_0){
            if((clientes+i)->id < menorId && (clientes+i)->id != 0){
                menorId = (clientes+i)->id;
                clienteSeleccionado = (clientes+i);
            }
        }
    }
    return clienteSeleccionado;
}

int randomizer(int max, int min){
    srand(gettid());
    return rand() % (max - min +1) + min;
}

