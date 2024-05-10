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
#define MAX_USERS 999
#define MAX_CLASSES 100 
#define MAX_USERS_P_CLASS 30

#define USER_SEM "user_sem"
#define CLASS_SEM "class_sem"

// Estrutura para armazenar informações do utilizador
struct utilizador {
    char username[TAM];
    char password[TAM];
    char role[TAM];
};

// Nó da lista ligada para armazenar utilizadores
typedef struct no_lista {
    struct utilizador u;
    struct no_lista *proximo;
    struct no_lista *anterior;
} no_lista;

// Estrutura para armazenar informações das aulals
struct class {
    struct user alunos[MAX_USERS_P_CLASS];
    struct utilizador professor;
    char nome[TAM];
    char ip[TAM];
    int tam_max;
};

// Nó da lista ligada para armazenar class
typedef struct no_class {
    struct class u;
    struct no_class *proximo;
} no_class;

// Tipo lista para facilitar o manuseio da lista ligada de utilizadores
typedef no_lista *lista;
// Tipo lista para facilitar o manuseio da lista ligada de aulas
typedef no_class *class_list;

typedef struct shared_memory{
    struct utilizador users[MAX_USERS];
    struct class classes[MAX_CLASSES];
} shm;

int shm_user, shm_class;

sem_t *sem_user, *sem_class;

// Protótipos das funções
void erro(char *s);
lista cria();
void ler_ficheiro(lista lista_utilizadores, char *ficheiro);
void insere_utilizador(lista lista_utilizadores, struct utilizador u);
int confirmar_login(lista lista_utilizadores, char username_login[TAM], char password_login[TAM]);
void listar_utilizadores(lista l);
void process_client(int client_fd, char *ficheiro);
void treat_signal(int fd);
no_lista* create_student_list();
void add_student_to_class(no_lista **head, char *nome);
void add_class_to_list(class_list list, struct class new_class);
void load_classes_from_file(const char *filename, class_list list);
void print_students(const no_lista *head);
void print_classes(const class_list list);
#endif // SERVER_H
