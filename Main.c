#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <limits.h>
#include <stdbool.h>

FILE *logFile;
pthread_mutex_t logSemaforo;
pthread_mutex_t repSemaforo;
pthread_mutex_t cliSemaforo;
pthread_cond_t repCondicion;
int contadorIdsClientes = 0;
bool final = false;

int MAX_CLIENTS;
int MAX_CAJEROS;
// Nombramos las funciones
void *Reponedor(void *arg);
void *Cajero(void *arg);
void *Cliente(void *arg);
void writeLogMessage(int id, char *msg, char *source);
void añadirCliente(int sig);
void aumentarColaYCajero(int sig);
struct cliente *menorIDCliente();
void cambiarEstadoCliente(int clienteID, int estado);
int comprobarEstadoCliente(int clienteID);
void sacarCola(int clienteID);
struct cliente *buscarCliente(int clienteID);
int randomizer(int max, int min);
void acabarPrograma(int sig);

enum Estado {
    ESTADO_0,
    ESTADO_1,
    ESTADO_2,
};
struct cliente {
    int id;
    int estado;
};
struct cliente *clientes;
int *contadorIdsCajeros;

void *Reponedor(void *arg) {
    while (1) {
        pthread_mutex_lock(&repSemaforo);
        pthread_cond_wait(&repCondicion, &repSemaforo);
        sleep(randomizer(5, 1));
        pthread_cond_signal(&repCondicion);
        pthread_mutex_unlock(&repSemaforo);
    }
}
void *Cajero(void *arg) {
    int clientesAtendidos = 0;
    int cajeroID = *(int*) arg;
    while (1 && !final) {
        struct cliente *clienteSeleccionado = menorIDCliente();
        if (clienteSeleccionado != NULL) {        
            int idCliente = clienteSeleccionado->id;
            cambiarEstadoCliente(idCliente, ESTADO_1);
            char atendiendo[100];
            sprintf(atendiendo, "Atendiendo a cliente %d", idCliente);
            writeLogMessage(cajeroID, atendiendo, "Cajero");
            int cooldown = randomizer(5, 1);
            sleep(cooldown);
            int random = randomizer(100, 1);
            if (random >= 96 && random <= 100) {
                char problema[200];
                sprintf(problema, "Cliente %d tiene problemas y no puede realizar la compra.", idCliente);
                writeLogMessage(cajeroID, problema, "Cajero");
            } else {
                if (random >= 71 && random <= 95) {
                    pthread_cond_signal(&repCondicion);
                    pthread_cond_wait(&repCondicion, &repSemaforo);
                }
                int precio = randomizer(100, 1);
                char compra[100];
                sprintf(compra, "El precio de la compra es de %d", precio);
                writeLogMessage(cajeroID, compra, "Cajero");
            }
            cambiarEstadoCliente(idCliente, ESTADO_2);
            clientesAtendidos++;
            if (clientesAtendidos % 10 == 0) {
                writeLogMessage(cajeroID, "Me voy a tomar un descanso", "Cajero");
                sleep(20);
                writeLogMessage(cajeroID, "Acabe mi descanso", "Cajero");
            }
        }
    }
    char clientesAtendidosNumero[100];
    sprintf(clientesAtendidosNumero, "He atendido a %d clientes", clientesAtendidos);
    writeLogMessage(cajeroID, clientesAtendidosNumero, "Cajero");
}

void *Cliente(void *arg) {
    int clienteID = *(int*) arg;
    struct cliente *clienteSeleccionado = buscarCliente(clienteID);
    if (clienteSeleccionado == NULL)
        pthread_exit(NULL);
    writeLogMessage(clienteID, "Cliente en la fila", "Cliente");
    int comprobarSalida = 0;
    while (comprobarEstadoCliente(clienteID) != ESTADO_1) {
        sleep(1);
        comprobarSalida++;
        if (comprobarSalida == 10) {
            int posibleSalida = randomizer(10, 1);
            if (posibleSalida == 10) {
                writeLogMessage(clienteID, "Cliente se va de la tienda", "Cliente");
                sacarCola(clienteID);
                pthread_exit(NULL);
            }
            comprobarSalida = 0;
        }
    }
    while (comprobarEstadoCliente(clienteID) != ESTADO_2) {
        sleep(1);
    }
    writeLogMessage(clienteID, "Cliente atendido y finaliza compra", "Cliente");
    pthread_exit(NULL);
}
int main(int argc, char *argv[]) {
    pthread_mutex_init(&logSemaforo, NULL);
    pthread_mutex_init(&repSemaforo, NULL);
    pthread_mutex_init(&cliSemaforo, NULL);
    pthread_cond_init(&repCondicion, NULL);

    if(argc == 1){
        MAX_CLIENTS = 20;
        MAX_CAJEROS = 3;
    } else if(argc == 3){
        if(atoi(argv[1]) < 1 || atoi(argv[2]) < 1){
            printf("Argumento introducido incorrecto. Debes introducir un tamaño de cola y numero cajeros que sea mayor de 1.\n");
            return 1;
        }
        MAX_CLIENTS = atoi(argv[1]);
        MAX_CAJEROS = atoi(argv[2]);
    } else{
        printf("Numero de argumentos invalido.\n");
        return 0;
    }

    logFile = fopen("registroCaja.log", "w");
    if (logFile == NULL) {
        printf("Error opening log file.\n");
        return 1;
    }
    clientes = (struct cliente *)malloc(sizeof(struct cliente) * MAX_CLIENTS);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        (clientes + i)->id = 0;
        (clientes + i)->estado = ESTADO_2;
    }

    struct sigaction ss;
    ss.sa_handler = añadirCliente;
    sigaction(SIGUSR1, &ss, NULL);
   
    ss.sa_handler = aumentarColaYCajero;
    sigaction(SIGUSR2, &ss, NULL);

    ss.sa_handler = acabarPrograma;
    sigaction(SIGINT, &ss, NULL);

    contadorIdsCajeros = malloc(sizeof(int) * MAX_CAJEROS);
    for(int i = 0; i < MAX_CAJEROS; i++){
        contadorIdsCajeros[i] = i+1;
    }

    for(int i = 0; i < MAX_CAJEROS; i++){
        pthread_t cajero;
        pthread_create(&cajero, NULL, Cajero, &contadorIdsCajeros[i]);
    }

    pthread_t reponedor;
    pthread_create(&reponedor, NULL, Reponedor, "Reponedor listo para ser util");

    while(1){
        pause();
    }
    return 0;
}
void writeLogMessage(int id, char *msg, char *source) {
    pthread_mutex_lock(&logSemaforo);
    time_t now = time(0);
    struct tm *tlocal = localtime(&now);
    char stnow[25];
    strftime(stnow, 25, " %d/ %m/ %y %H: %M: %S", tlocal);
    logFile = fopen("registroCaja.log", "a");
    fprintf(logFile, "[%s] %s %d: %s\n", stnow, source, id, msg);
    fclose(logFile);
    pthread_mutex_unlock(&logSemaforo);
}

void añadirCliente(int sig) {
    pthread_mutex_lock(&cliSemaforo);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if ((clientes + i)->estado == ESTADO_2) {
            pthread_t cliente;
            (clientes + i)->id = ++contadorIdsClientes;
            (clientes + i)->estado = ESTADO_0;
            pthread_create(&cliente, NULL, Cliente, &contadorIdsClientes);
            break;
        }
    }
    pthread_mutex_unlock(&cliSemaforo);
}

void aumentarColaYCajero(int sig){
    clientes = realloc(clientes, (MAX_CLIENTS + 1) * sizeof(struct cliente));
    (clientes + MAX_CLIENTS)->id = 0;
    (clientes + MAX_CLIENTS)->estado = ESTADO_2;
    MAX_CLIENTS++;
    pthread_t nuevoCajero;
    contadorIdsCajeros = realloc(contadorIdsCajeros, (MAX_CAJEROS + 1) * sizeof(int));
    contadorIdsCajeros[MAX_CAJEROS] = MAX_CAJEROS + 1;
    pthread_create(&nuevoCajero, NULL, Cajero, &contadorIdsCajeros[MAX_CAJEROS]);
    MAX_CAJEROS++;
}


struct cliente *menorIDCliente() {
    struct cliente *clienteSeleccionado = NULL;
    int menorId = INT_MAX;
    pthread_mutex_lock(&cliSemaforo);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if ((clientes + i)->estado == ESTADO_0) {
            if ((clientes + i)->id < menorId && (clientes + i)->id != 0) {
                menorId = (clientes + i)->id;
                clienteSeleccionado = (clientes + i);
            }
        }
    }
    pthread_mutex_unlock(&cliSemaforo);
    return clienteSeleccionado;
}

void cambiarEstadoCliente(int clienteID, int estado) {
    pthread_mutex_lock(&cliSemaforo);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if ((clientes + i)->id == clienteID) {
            (clientes + i)->estado = estado;
        }
    }
    pthread_mutex_unlock(&cliSemaforo);
}

int comprobarEstadoCliente(int clienteID) {
    int estado = -1;
    pthread_mutex_lock(&cliSemaforo);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if ((clientes + i)->id == clienteID) {
            estado = (clientes + i)->estado;
        }
    }
    pthread_mutex_unlock(&cliSemaforo);
    return estado;
}
void sacarCola(int clienteID) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if ((clientes + i)->id == clienteID) {
            (clientes + i)->id = 0;
            (clientes + i)->estado = ESTADO_2;
        }
    }
}
struct cliente *buscarCliente(int clienteID) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if ((clientes + i)->id == clienteID) {
            return (clientes + i);
        }
    }
    return NULL;
}
int randomizer(int max, int min) {
    pid_t threadId = syscall(SYS_gettid);
    srand(threadId);
    return rand() % (max - min + 1) + min;
}
void acabarPrograma(int sig) {
    final = true;
    if (pthread_mutex_destroy(&logSemaforo) != 0)
        exit(-1);
    if (pthread_mutex_destroy(&repSemaforo) != 0)
        exit(-1);
    if (pthread_mutex_destroy(&cliSemaforo) != 0)
        exit(-1);
    free(clientes);
    fclose(logFile);
    exit(0);
}