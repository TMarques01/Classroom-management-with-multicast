// server.h

#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>


#define BUFLEN 512  // Tamanho do buffer
#define TAM 64      // Tamanho do username/password

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
} no_lista;

// Tipo lista para facilitar o manuseio da lista ligada de utilizadores
typedef no_lista *lista;

// Protótipos das funções
void erro(char *s);
lista cria();
void ler_ficheiro(FILE *file, lista lista_utilizadores);
void insere_utilizador(lista lista_utilizadores, struct utilizador u);
int confirmar_login_administrador(lista lista_utilizadores, char username_login[TAM], char password_login[TAM]);

#endif // SERVER_H
