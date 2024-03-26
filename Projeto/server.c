//server.c

#include "server.h"
#include <stdlib.h>
#include <string.h>

struct sockaddr_in addr, client_addr, addr_server, addr_client;
int fd, client, client_addr_size, s,recv_len;
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
void ler_ficheiro(lista lista_utilizadores, char *ficheiro) {

    FILE *file;
    file = fopen(ficheiro, "r");

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
int confirmar_login(lista lista_utilizadores, char username_login[TAM], char password_login[TAM]){

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
            else if (strcmp(aux->u.role,"aluno") == 0){
                count = 2;
            } else if (strcmp(aux->u.role,"professor") == 0){
                count = 3;
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

//Função para clientes TCP
void process_client(int client_fd, char *ficheiro){

    lista lista_utilizadores_tcp = cria(); //Cria a lista ligada já com um elemento para evitar o caso de ser nulo
    ler_ficheiro(lista_utilizadores_tcp, ficheiro);

    int login = 0;
    int login_verificar = 0;
    int nread = 1;
    char buffer[BUFLEN];

    while(1) {
        nread = read(client_fd, buffer, BUFLEN-1);
        if (nread <= 0) break; 
        buffer[nread] = '\0';

        char username[TAM];
        char password[TAM];

        printf("%s\n", buffer);

        // Validar os dados (0 = nao logado; 1 = aluno; 2 = professor)
        if (login == 0){

            // Guardar primeiramente os dados
            int aux_dados = 0;
            char *token = strtok(buffer," ");
            while (token != NULL){
                if (aux_dados == 0){
                    if (strcmp(token,"LOGIN") == 0){
                        token =strtok(NULL," ");
                        aux_dados++;
                        login_verificar = 1;
                    } else {
                        write(client_fd, "LOGIN {username} {password}: ", strlen("LOGIN {username} {password}: "));
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

            if (login_verificar == 1){
                //Validação dos dados
                int confirmacao_login = confirmar_login(lista_utilizadores_tcp, username, password);
                if (confirmacao_login == 2){ // caso seja aluno
                    login = 1;
                    write(client_fd, "OK", strlen("OK"));

                } else if (confirmacao_login == 3){ // caso seja professor
                    login = 2;
                    write(client_fd, "OK", strlen("OK"));

                } else if (confirmacao_login != 2 || confirmacao_login != 3){
                    login = 0;
                    write(client_fd, "REJECTED", strlen("REJECTED"));
                    memset(username, 0, sizeof(username));
                    memset(password, 0, sizeof(password));
                }
            }
        // Login efetuado
        } else if (login == 1 || login == 2){

            if (strncmp (buffer,"LIST_CLASSES", 12) == 0){
                write(client_fd, "LIST_CLASSES entrou", strlen("LIST_CLASSES entrou"));
            } else if (strncmp(buffer, "LIST_SUBSCRIBED", 15) == 0){
                write(client_fd, "LIST_SUBSCRIBED entrou", strlen("LIST_SUBSCRIBED entrou"));
            } else {
                if (login == 1){ // Caso seja aluno
                //printf("Aluno entrou\n");

                    if (strncmp(buffer,"SUBSCRIBE_CLASS", 15) == 0){
                        write(client_fd, "SUBSCRIBE_CLASS entrou", strlen("SUBSCRIBE_CLASS entrou"));
                    } else {
                        write(client_fd, "Argumentos incorretos", strlen("Argumentos incorretos"));
                    } 
                }
                else if (login == 2){ // Caso seja professor
                    //printf("Professor entrou\n");
                    if (strncmp (buffer,"CREAT_CLASS", 11) == 0){
                        write(client_fd,"CREAT_CLASS entrou",strlen("CREAT_CLASS entrou"));
                    } else if (strncmp(buffer, "SEND", 4) == 0){
                        write(client_fd,"SEND entrou", strlen("SEND entrou"));
                    } else {
                        write(client_fd, "Argumentos incorretos", strlen("Argumentos incorretos"));
                    }       
                } 
            }
        }
    }

    printf("A fechar cliente...\n");
    close(client_fd);
}

//Funcao para lidar com o sinal (TCP)
void treat_signal(int sig){
    printf("Fechar o socket\n");
    close(fd);
    exit(0);
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
    fclose(f);
    
    //SERVIDOR TCP
    pid_t TCP_process = fork();
    if (TCP_process == 0){
        
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

        //Sinal a ser recebido para terminar processo do TCP
        signal(SIGTERM, treat_signal);

        while (1){

            while(waitpid(-1,NULL,WNOHANG)>0);
            client = accept(fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addr_size); 
            if (client > 0) {
                if (fork() == 0) {
                    
                    close(fd);
                    printf("Novo cliente!\n");
                    process_client(client,argv[3]);
                    close(client);
                    exit(0);
                }
            }
        }
        
        close(fd);
          
    //SERVIDOR UDP
    } else {

        lista lista_utilizadores = cria(); //Cria a lista ligada já com um elemento para evitar o caso de ser nulo
        ler_ficheiro(lista_utilizadores, argv[3]);

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

            printf("Administrador: %s", buf);

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
                    int confirmacao_login = confirmar_login(lista_utilizadores, username, password);
                    if (confirmacao_login == 0){
                        login = 1;
                        sendto(s, "OK\n", strlen("OK\n"), 0, (struct sockaddr *) &addr_client, slen);
                    } else {
                        login = 0;
                        sendto(s, "REJECTED\n", strlen("REJECTED\n"), 0, (struct sockaddr *) &addr_client, slen);
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
                } else if (strncmp(buf,"QUIT_SERVER",11) == 0) { //SAIR DO SERVIDOR
                    sendto(s, "A fechar o servidor...\n", strlen("A fechar o servidor...\n"), 0, (struct sockaddr *) &addr_client, slen);
                    kill(TCP_process, SIGTERM);
                    waitpid(TCP_process, NULL, 0); // Espera o filho terminar
                    break;
                } else {
                    sendto(s, "Argumentos incorretos\n", strlen("Argumentos incorretos\n"), 0, (struct sockaddr *) &addr_client, slen);
                }   
            }
        }
        //Fechar socket UDP
        printf("A fechar servidor UDP\n");
        close (s);  
    }
    return 0;
}


