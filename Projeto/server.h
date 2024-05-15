//server.h

#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define BUFLEN 1024  // Tamanho do buffer
#define TAM 64      // Tamanho do username/password
#define MAX_USERS 50
#define MAX_CLASSES 50 
#define MAX_USERS_P_CLASS 100

#define MULTICAST_PORT 8888

#define USER_SEM "user_sem"
#define CLASS_SEM "class_sem"

// Estrutura para armazenar informações do utilizador
typedef struct {
    char username[TAM];
    char password[TAM];
    char role[TAM];
    int online_status;
}user;

// Estrutura para armazenar informações das aulals
typedef struct {
    char nome[TAM];
    char ip[TAM];
    int tam_max;
    int tam_current;
}class;

// Shared Memory struct
typedef struct{
    user *users;
    class *classes;
}shm;

shm *shared_m;
int shm_id;

sem_t *sem_user, *sem_class;

// Protótipos das funções
void erro(char *s);
// ==============================
void ler_ficheiro(char *ficheiro);
int confirmar_login(char username_login[TAM], char password_login[TAM]);
void escrever_lista_para_arquivo();
// ============== UDP ==============
int eliminar_utilizador(char *buffer);
void adicionar_utilizador(char *buffer);
void insere_utilizador(user u);
void listar_utilizadores();
// ============== TCP ==============
void process_client(int client_fd, char *ficheiro);
void treat_signal(int fd);

void subscribe_class(int client_fd, char *buffer);
void creat_class(int client_fd, char *buffer);
int increment_class_size(const char *class_name);

#endif // SERVER_H
