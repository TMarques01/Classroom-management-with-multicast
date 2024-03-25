//server.c

#include "server.h"
#include <stdlib.h>
#include <string.h>

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
    char line[BUFLEN];

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

        //DEBUG
        //printf("(DEBUG)\n USERNAME %s\n PASSWORD %s\n",aux->u.username, aux->u.password);

        if (strcmp(aux -> u.username, username_login) == 0 && strcmp(aux -> u.password, password_login) == 0){
                //Se for administrador
            if (strcmp(aux -> u.role, "administrador") == 0){
                count = 0;
            }
            //Se tiver outro cargo
            else {
                count = 2;
            }
        }
        aux = aux -> proximo;
    }
    return count;
}

//Função para debug (listar todos os utlizadores do ficheiro)
void listar_utilizadores(lista l) {
    //Saltar o primeiro valor
    no_lista *atual = l->proximo;
    while (atual != NULL) {
        printf("Username: %s, Password: %s, Role: %s\n", atual->u.username, atual->u.password, atual->u.role);
        atual = atual->proximo; // Move para o próximo nó da lista
    }
}

void process_client(int client_fd){
}

int main(int argc, char *argv[]){


    if (argc != 4) {
        printf("class_server {PORTO_TURMAS} {PORTO_CONFIG} {ficheiro configuracao}\n");
        exit(-1);
    }

    // Verificar se o ficheiro de configuração existe
    FILE *f = fopen(argv[3], "r");
    if (f == NULL) {
        erro("Erro ao abrir ficheiro\n");
    }

    lista lista_utilizadores = cria(); //Cria a lista ligada já com um elemento para evitar o caso de ser nulo
    ler_ficheiro(f,lista_utilizadores);
    
    //SERVIDOR TCP
    if (fork() == 0){
        struct sockaddr_in addr, client_addr;
        int fd, client;
        int client_addr_size;

        bzero((void *) &addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(atoi(argv[1]));

        if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            erro("na funcao socket");
        if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
            erro("na funcao bind");
        if( listen(fd, 5) < 0)
            erro("na funcao listen");
        client_addr_size = sizeof(client_addr);
        while (1) {

            while(waitpid(-1,NULL,WNOHANG)>0);
            client = accept(fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addr_size); 
            if (client > 0) {
                if (fork() == 0) {
                    close(fd);
                    process_client(client);
                    close(client);
                    exit(0);
                }
            }
        }

    //SERVIDOR UDP
    } else {
        struct sockaddr_in addr_server, addr_client;
        int s,recv_len;
        socklen_t slen = sizeof(addr_client);

        char buf[BUFLEN];

        // Cria um socket para recepção de pacotes UDP
        if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
            erro("Erro na criação do socket"); //DFNEKFNEKFNQEFKEFNE
        }

        // Preenchimento da socket address structure (UDP)
        addr_server.sin_family = AF_INET;
        addr_server.sin_port = htons(atoi(argv[2])); // PORTO_CONFIG
        addr_server.sin_addr.s_addr = htonl(INADDR_ANY);

        // Associa o socket à informação de endereço
        if(bind(s,(struct sockaddr*)&addr_server, sizeof(addr_server)) == -1) {
            erro("Erro no bind");
        }

        // Variáveis auxiliares
        int login = 0;
        int login_verificar = 0;

        while (1){

            //Mensagem recebida
            if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&addr_client, &slen)) == -1) {
                erro("Erro no recvfrom");  //DDJWDQWJBDQWJDBQWJDBWQJDBQWJBDQ
            }

            // Limpa o buffer buf para receber mais mensagens
            buf[recv_len] = '\0';

            printf("Recebi: %s", buf);

            if (strcmp(buf,"QUIT_SERVER\n") != 0){

                // Condição para fazer login
                if (login == 0){

                    char username[TAM];
                    char password[TAM];

                    // Validar os dados
                    // Guardar primeiramente os dados
                    int aux_dados = 0;
                    char *token = strtok(buf," ");
                    while (token != NULL){
                        if (aux_dados == 0){
                            if (strcmp(token,"LOGIN") == 0){
                                token =strtok(NULL," ");
                                aux_dados++;
                                login_verificar = 1;
                            } else {
                                sendto(s, "LOGIN {username} {password}\n", strlen("LOGIN {username} {password}\n"), 0, (struct sockaddr *) &addr_client, slen);
                                login_verificar = 0;
                                break;
                            }
                        }
                        else if (aux_dados == 1){
                            strcpy(username, token);
                            username[strcspn(username, "\n")] = '\0';
                            token = strtok(NULL," ");
                            aux_dados++;
                        } else if (aux_dados == 2){
                            strcpy(password, token);
                            password[strcspn(password, "\n")] = '\0';  
                            token = strtok(NULL," ");
                        }  
                    }

                    if(login_verificar == 1){
                        //Validação dos dados
                        int confirmacao_login = confirmar_login_administrador(lista_utilizadores, username, password);
                        if (confirmacao_login == 0){
                            login = 1;
                            sendto(s, "Login efetuado com sucesso. Bem-vindo\n", strlen("Login efetuado com sucesso. Bem-vindo\n"), 0, (struct sockaddr *) &addr_client, slen);

                        } else if (confirmacao_login == 2){
                            sendto(s, "Cargo nao admitido.\n", strlen("Cargo nao admitido.\n"), 0, (struct sockaddr *) &addr_client, slen);
                        } else {
                            login = 0;
                            sendto(s, "Dados incorretos. Tente outra vez.\n", strlen("Dados incorretos. Tente outra vez.\n"), 0, (struct sockaddr *) &addr_client, slen);
                            memset(username, 0, sizeof(username));
                            memset(password, 0, sizeof(password));

                        }
                    }

                    
                // SE JÁ ESTIVER LOGADO
                } else if (login == 1){
                    if (strncmp (buf,"ADD_USER", 8) == 0){
                        printf("ADD_USER entrou\n");
                    } else if (strncmp(buf, "DEL", 3) == 0){
                        printf("DEL entrou\n");
                    } else if (strncmp(buf,"LIST", 4) == 0){
                        printf("LIST entrou\n");
                        listar_utilizadores(lista_utilizadores);
                    } else {
                        sendto(s, "Argumentos incorretos\n", strlen("Argumentos incorretos\n"), 0, (struct sockaddr *) &addr_client, slen);
                    }   
                }  

            //SAIR DO SERVIDOR
            } else {
                sendto(s, "A fechar o servidor...\n", strlen("A fechar o servidor...\n"), 0, (struct sockaddr *) &addr_client, slen);
                break;
            }
        }
        //Fechar socket UDP
        printf("A fechar servidor UDP\n");
        close (s);
        
    }

    return 0;
}


