//server.c

#include "server.h"
#include <stdlib.h>
#include <string.h>

struct sockaddr_in addr_server, addr_client;
int s,recv_len;
socklen_t slen = sizeof(addr_client);

void erro(char *s){ 
	perror(s);
	exit(1);
}

//Funcao que cria uma lista com 1 elemento e retorna essa mesma lista
lista cria (){  
    lista aux;
    struct utilizador u1 = {"", "", ""};
    aux = (lista)malloc(sizeof (no_lista));
    if (aux != NULL) {
        aux->u = u1;
        aux->proximo = NULL;
    }
    return aux;
}

//Função para ler ficheiro e adicionar à lista ligada
void ler_ficheiro(FILE *file, lista lista_utilizadores) {
    char *token;
    char line[TAM];

    struct utilizador u;
    while (fgets(line, BUFLEN, file)) {
        token = strtok(line, ";");

        int aux = 0;
        while (token != NULL) {
            if (aux == 0) {
                strcpy(u.username, token);
            } else if (aux == 1) {
                strcpy(u.password, token);
            } else if (aux == 2) {
                strcpy(u.role, token);
            }
            aux++;
            token = strtok(NULL, ";\n");
        }
        insere_utilizador(lista_utilizadores, u);
    }
    fclose(file);
}

//Função para inserir utilizador na lista ligada
void insere_utilizador(lista lista_utilizadores, struct utilizador u) {
    lista no = (lista) malloc(sizeof(no_lista)); // aloca espaço para um novo nó
    no->u = u; // armazena a struct utilizador no campo 'u' do novo nó
    no->proximo = lista_utilizadores->proximo; // define o próximo ponteiro do novo nó para o primeiro nó da lista 
    lista_utilizadores->proximo = no; // define o próximo ponteiro do nó anterior para o novo nó, adicionando-o na lista
}

//Funcao para confirmar o login do administrador
int confirmar_login_administrador(lista lista_utilizadores, char username_login[TAM], char password_login[TAM]){

    lista aux = lista_utilizadores -> proximo;
    int count = 1;

    while(aux){
        if (strcmp(aux -> u.username, username_login) == 0 && strcmp(aux -> u.password, password_login) == 0){
            if (strcmp(aux -> u.role, "administrador") == 0){
                count = 0;
            }
            else {
                count = 2;
            }
        }
        aux = aux -> proximo;
    }
    return count;
}

int main(int argc, char *argv[]){

    char buf[BUFLEN];

    if (argc != 4) {
        printf("class_server {PORTO_NOTICIAS} {PORTO_CONFIG} {ficheiro configuracao}\n");
        exit(-1);
    }

    // Verificar se o ficheiro de configuração existe
    FILE *f = fopen("ficheiro.txt", "r");
    if (f == NULL){
        erro("Erro ao abrir ficheiro\n");
    }

    // Cria um socket para recepção de pacotes UDP
	if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		erro("Erro na criação do socket"); //DFNEKFNEKFNQEFKEFNE
	}

    // Preenchimento da socket address structure (UDP)
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = htons(9876); // PORTO_CONFIG
	addr_server.sin_addr.s_addr = htonl(INADDR_ANY);

	// Associa o socket à informação de endereço
	if(bind(s,(struct sockaddr*)&addr_server, sizeof(addr_server)) == -1) {
		erro("Erro no bind");
	}

    lista lista_utilizadores = cria(); //Cria a lista ligada já com um elemento para evitar o caso de ser nulo
    ler_ficheiro(f,lista_utilizadores);

    // Variáveis aux
    int first_login  = 0;
    int login = 0;
    int valores_verficar = 0;

    while (1){

        //Mensagem recebida
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&addr_client, &slen)) == -1) {
            erro("Erro no recvfrom");  //DDJWDQWJBDQWJDBQWJDBWQJDBQWJBDQ
        }

        // Limpa o buffer buf para receber mais mensagens
        buf[recv_len] = '\0';

        printf("Recebi: %s\n", buf);

        // Condição para fazer login
        if (login == 0){

            char username[TAM];
            char password[TAM];
            char role[TAM];
            char mensagem[BUFLEN];

            // Primeira parte para pedir valores ao utilizador
            if (valores_verficar == 0){
                strcpy(mensagem, "Insira os dados {nome utilizador} {password}: ");
                sendto(s, mensagem, strlen(mensagem), 0, (struct sockaddr *) &addr_client, slen);
                valores_verficar++;
                memset(mensagem, 0, sizeof(mensagem));

            // Validar os dados
            // Guardar primeiramente os dados
            } else if (valores_verficar == 1){
                int aux_dados = 0;
                char *token = strtok(buf," ");
                while (token != NULL){
                    if (aux_dados == 0){
                        strcpy(username, token);
                        token = strtok(NULL," ");
                        aux_dados++;
                    } else if (aux_dados == 1){
                        strcpy(password, token);
                        token = strtok(NULL," ");
                    }  
                }
                //Validação dos dados
                if (confirmar_login_administrador(lista_utilizadores, username, password) == 0){
                    login = 1;
                    char mensagem[BUFLEN];
                    strcpy(mensagem, "Login efetuado com sucesso. Bem-vindo\n");
                    sendto(s, mensagem, strlen(mensagem), 0, (struct sockaddr *) &addr_client, slen);
                    memset(mensagem, 0, sizeof(mensagem));
                } else if (confirmar_login_administrador(lista_utilizadores, username, password) == 2){
                    char mensagem_erro[BUFLEN];
                    strcpy(mensagem_erro, "Cargo nao admitido.\n Insira os dados {nome do utilizador} {password}: ");
                    sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &addr_client, slen);
                    memset(mensagem_erro, 0, sizeof(mensagem_erro));
                } else {
                    login = 0;
                    valores_verficar = 0;
                    char mensagem_erro[BUFLEN];
                    strcpy(mensagem_erro, "Dados incorretos. Tente outra vez.\nInsira os dados {nome do utilizador} {password}: ");
                    sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &addr_client, slen);
                    memset(mensagem_erro, 0, sizeof(mensagem_erro));
                    memset(username, 0, sizeof(username));
                    memset(password, 0, sizeof(password));
                }
            }
            
        } else if (login == 1){
            break;
        }        


    }
    close (s);
    return 0;
}


