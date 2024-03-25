#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>

#define BUF_SIZE 1024	// Tamanho do buffer
#define PORT 9876	// Porto para recepção das mensagens
#define TAM 25      // Tamanho do username/password


struct utilizador {
    char username[TAM];
    char password[TAM];
    char role[TAM];
};

struct topico {  //Struct dos topicos
    char nome[TAM];
    char id_topico[BUF_SIZE];
    char porta_topico[BUF_SIZE];
};

struct noticia {  //Struct das noticia
    char nome_topico[TAM];
    char titulo[TAM];
    char texto[BUF_SIZE];
    char autor[BUF_SIZE];
};

struct cliente_topicos {  //Struct topicos associados ao cliente
    char username[TAM];
    char topicos[TAM][BUF_SIZE];
    int num_topicos;
};


typedef struct no_lista_utilizador { //Struct da lista_utilizadores ligada dos users
    struct utilizador u;
    struct no_lista_utilizador * proximo;
} no_lista_utilizador;

typedef struct no_lista_topico { //Struct da lista_u ligada dos topicos
    struct topico t;
    struct no_lista_topico * proximo;
} no_lista_topico;

typedef struct no_lista_noticia { //Struct da lista_u ligada das noticia
    struct noticia n;
    struct no_lista_noticia * proximo;
} no_lista_noticia;

typedef struct no_cliente_topicos { //Struct da lista_u ligada dos topicos_clientes
    struct cliente_topicos ct;
    struct no_cliente_topicos * proximo;
} no_cliente_topicos;


typedef no_lista_utilizador * lista_u;
typedef no_lista_topico * lista_t;
typedef no_lista_noticia * lista_n;
typedef no_cliente_topicos * lista_ct;



lista_u cria_u ();
lista_t cria_t ();
lista_ct cria_ct();
lista_n cria_n();

void ler_ficheiro_utilizadores(lista_u lista_utilizadores);
void escrever_ficheiro_utilizadores(lista_u lista_utilizadores);
void ler_ficheiro_topicos(lista_t lista_topicos, lista_n lista_noticias);
void escrever_ficheiro_topicos(lista_t lista_topicos, lista_n lista_noticias);
void ler_ficheiro_ct(lista_ct lista_cliente_topicos);
void escrever_ficheiro_ct(lista_ct lista_cliente_topicos);

void add_utilizador(lista_u lista_utilizadores, char username[TAM], char password[TAM], char role[TAM]);
void insere_utilizador (lista_u lista_utilizadores, struct utilizador u);
void remove_utilizador(lista_u lista_utilizadores, char username[TAM]);
int conf_nome(lista_u lista_utilizadores, char username[TAM]);
int verifica_role(char aux[TAM]);
void list(lista_u lista_utilizadores);
int confirmar_login(lista_u lista_utilizadores ,char *username_login, char *password_login);

void process_client(int client_fd);
int procura_topico(lista_t lista_topico, char nome[TAM]);
void insere_topico(lista_t lista_topico, struct topico t);
void insere_noticia(lista_n lista_noticia, struct noticia n);
void insere_cliente_topicos(lista_ct lista_cliente_topicos, struct cliente_topicos ct);
int e_leitor(lista_u lista_utilizadores ,char username_login[TAM]);
char *lista_topicos(lista_t lista_topico);
char *subscreve_topico(char username[BUF_SIZE], char topico[BUF_SIZE], lista_ct lista_cliente_topicos, lista_t lista_topico);
char *cria_topico(char novo_topico[BUF_SIZE], lista_t lista_topico);
char *cria_noticia(char username[BUF_SIZE], char topico[BUF_SIZE], char titulo[BUF_SIZE], char texto[BUF_SIZE], lista_n lista_noticia, lista_t lista_topico);
void erro(char *s);

struct sockaddr_in si_minha, client_sock, server_stock_sock, addr, client_addr;
int s,recv_len, fd_tcp, client, client_number = 0, client_addr_size;
socklen_t slen = sizeof(client_sock);


int main(int argc, char *argv[]) {
    char topicos[BUF_SIZE];

    if (argc != 4) {
        printf("news_server {PORTO_NOTICIAS} {PORTO_CONFIG} {ficheiro configuracao}\n"); //FEJFNEQJFNEQJDNQJQNF
        exit(-1);
    }

    if (fork() == 0) { //verificar isto
        // Cria um socket para recepção de pacotes TCP
        if ((fd_tcp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
            erro("Failed to create TCP socket");
        printf("TCP Socket created.\n");

        // Preenchimento da socket adress structure para bolsa
        server_stock_sock.sin_family = AF_INET;
        server_stock_sock.sin_port = htons((int) atoi(argv[1]));
        server_stock_sock.sin_addr.s_addr = ntohl(INADDR_ANY);

        // Associa o socket de market a informacao do endereco
        if (bind(fd_tcp, (struct sockaddr *) &server_stock_sock, sizeof server_stock_sock) < 0)
            erro("Failed to bind\n");
        printf("Successfully bound.\n");

        if (listen(fd_tcp, 5) < 0)
            erro("Failed to listen");
        client_addr_size = sizeof(client_addr);

        while (1) {
            while (waitpid(-1, NULL, WNOHANG) > 0);

            client = accept(fd_tcp, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_size);

            client_number++;
            if (client_number == 5) {
                printf("Numero maximo de clientes atingido.\n");
                break;
            }

            if (client > 0) {
                if (fork() == 0) {
                    printf("Novo cliente conectado\n");
                    close(fd_tcp);
                    process_client(client);
                    exit(0);
                }
                close(client);
            }
        }
        close(fd_tcp);
    } else {
        // Cria um socket para recepção de pacotes UDP
        if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
            erro("Erro na criação do socket"); //DFNEKFNEKFNQEFKEFNE
        }

        // Preenchimento da socket address structure
        si_minha.sin_family = AF_INET;
        si_minha.sin_port = htons(PORT);
        si_minha.sin_addr.s_addr = htonl(INADDR_ANY);

        // Associa o socket à informação de endereço
        if(bind(s,(struct sockaddr*)&si_minha, sizeof(si_minha)) == -1) {
            erro("Erro no bind"); //FENFENFJENFJWENFJWNFEJFW
        }
        char *token;
        char str[BUF_SIZE];

        lista_u lista_utilizadores = cria_u(); //Cria a lista_utilizadores ligada já com um elemento para evitar o caso de ser nulo
        ler_ficheiro_utilizadores(lista_utilizadores);

        int login = 1;
        int aux_login = 1;
        char username_login[TAM];
        char password_login[TAM];
        char mensagem[BUF_SIZE];
        int login_st = 0;

        while (1) {
            // Espera recepção de mensagem (a chamada é bloqueante)
            if ((recv_len = recvfrom(s, topicos, BUF_SIZE, 0, (struct sockaddr*)&client_sock, &slen)) == -1) {
                erro("Erro no recvfrom");  
            }

            // Limpa o buffer topicos para receber mais mensagens
            topicos[recv_len] = '\0';

            if (login_st == 0){
                char login_str[BUF_SIZE];
                strcpy(login_str, "Insira o nome do utilizador: ");
                sendto(s, login_str, strlen(login_str), 0, (struct sockaddr *) &client_sock, slen);
                login_st++; //Passa a ser 1
            }

            if (strcmp(topicos, "X") != 0 && strlen(topicos) != 1) {
                strcpy(str, topicos);
                token = strtok(str, " \n");
                int i = 1;
                char username[TAM];
                char password[TAM];
                char role[TAM];
                int aux = 0;

                if (login != 0){
                    if (aux_login == 1 && login_st == 1){
                        aux_login++;
                        login_st++;
                    }
                    if (aux_login == 1 && login_st > 1){
                        strcpy(mensagem, "Insira o nome do utilizador: ");
                        sendto(s, mensagem, strlen(mensagem), 0, (struct sockaddr *) &client_sock, slen);
                        aux_login++;
                        memset(mensagem, 0, sizeof(mensagem));
                    } else if (aux_login == 2){
                        strcpy(mensagem, "Insira a password: ");
                        sendto(s, mensagem, strlen(mensagem), 0, (struct sockaddr *) &client_sock, slen);
                        aux_login++;
                        strcpy(username_login, token);
                        memset(mensagem, 0, sizeof(mensagem));
                    } else if (aux_login == 3){
                        strcpy(password_login, token);
                        if (confirmar_login(lista_utilizadores, username_login, password_login) == 0){
                            login = 0;
                            char mensagem[BUF_SIZE];
                            strcpy(mensagem, "Login efetuado com sucesso. Bem-vindo\n");
                            sendto(s, mensagem, strlen(mensagem), 0, (struct sockaddr *) &client_sock, slen);
                            memset(mensagem, 0, sizeof(mensagem));
                        } else  if (confirmar_login(lista_utilizadores, username_login, password_login) == 2){
                            char mensagem_erro[BUF_SIZE];
                            strcpy(mensagem_erro, "Cargo nao admitido.\nInsira o nome de utilizador: ");
                            sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &client_sock, slen);
                            memset(mensagem_erro, 0, sizeof(mensagem_erro));
                        }else {
                            login = 1;
                            aux_login = 2;
                            char mensagem_erro[BUF_SIZE];
                            strcpy(mensagem_erro, "Dados incorretos. Tente outra vez.\nInsira o nome de utilizador: ");
                            sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &client_sock, slen);
                            memset(mensagem_erro, 0, sizeof(mensagem_erro));
                            memset(username_login, 0, sizeof(username_login));
                            memset(password_login, 0, sizeof(password_login));
                        }
                    }
                } else {
                    char menu[BUF_SIZE];
                    strcpy(menu, "Escreva a operacao que pretende fazer:\n"
                                " ADD_USER {username} {password} {administrador/cliente/jornalista}"
                                " DEL {username}"
                                " LIST"
                                " QUIT"
                                " QUIT SERVER");
                    sendto(s, menu, strlen(menu), 0, (struct sockaddr *) &client_sock, slen);
                    if (strcmp(token, "ADD_USER") == 0){
                        while (token != NULL && i <= 3) {
                            token = strtok(NULL, " \n"); // pega o proximo token
                            if (i == 1 && conf_nome(lista_utilizadores, token) == 0) {
                                strcpy(username, token);
                                aux = 1;
                            }
                            else if (i == 2 && aux == 1) {
                                strcpy(password, token);
                            }
                            else if (i == 3 && aux == 1) {
                                strcpy(role, token);
                            } else {
                                char mensagem_erro[BUF_SIZE];
                                strcpy(mensagem_erro, "Username ja existente.\n");
                                sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &client_sock, slen);
                                memset(mensagem_erro, 0, sizeof(mensagem_erro));
                                aux = 0;
                                break;
                            }
                            i++; // incrementa o contador de tokens
                        }
                        if (aux == 1 && i == 4){
                            add_utilizador(lista_utilizadores, username, password, role);
                            escrever_ficheiro_utilizadores(lista_utilizadores);
                        } else if (i != 4){
                            char mensagem_erro[BUF_SIZE];
                            strcpy(mensagem_erro, "Argumentos em falta ou em demasia.\n");
                            sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &client_sock, slen);
                            memset(mensagem_erro, 0, sizeof(mensagem_erro));
                            break;
                        }
                    } else if (strcmp(token, "DEL") == 0){
                        while (token != NULL && i <= 1){
                            token = strtok(NULL, " \n"); // pega o proximo token
                            if (i == 1 && conf_nome(lista_utilizadores, token) != 0){
                                if (strcmp(username_login, token) != 0){
                                    strcpy(username, token);
                                    aux = 1;
                                } else {
                                    char mensagem_erro[BUF_SIZE];
                                    strcpy(mensagem_erro, "Utilizador nao disponivel para ser eliminado.\n");
                                    sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &client_sock, slen);
                                    memset(mensagem_erro, 0, sizeof(mensagem_erro));
                                }
                            }else {
                                char mensagem_erro[BUF_SIZE];
                                strcpy(mensagem_erro, "Username nao existente.\n");
                                sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &client_sock, slen);
                                memset(mensagem_erro, 0, sizeof(mensagem_erro));
                            }
                            i++;
                        }
                        if (aux == 1){
                            remove_utilizador(lista_utilizadores, username);
                            escrever_ficheiro_utilizadores(lista_utilizadores);
                            char mensagem[BUF_SIZE];
                            strcpy(mensagem, "Utilizador eliminado com sucesso.\n");
                            sendto(s, mensagem, strlen(mensagem), 0, (struct sockaddr *) &client_sock, slen);
                            memset(mensagem, 0, sizeof(mensagem));
                        } else if (i != 2){
                            char mensagem_erro[BUF_SIZE];
                            strcpy(mensagem_erro, "Argumentos em falta ou em demasia.\n");
                            sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &client_sock, slen);
                            memset(mensagem_erro, 0, sizeof(mensagem_erro));
                            break;
                        }
                    } else if (strcmp(token, "LIST") == 0){
                        list(lista_utilizadores);
                    } else if (strcmp(token, "QUIT") == 0){
                        break;
                    } else if (strcmp(token, "QUIT_SERVER") == 0){
                        close(s);
                        break;
                    }

                    memset(token, 0, sizeof(token));
                    memset(username, 0, sizeof(username));
                    memset(password, 0, sizeof(password));
                    memset(role, 0, sizeof(role));

                    // Verifica se a conexão foi fechada pelo remetente
                    if (recv_len == 0) {
                        break; // Sai do loop se a conexão foi fechada
                    }
                }
            }
        }
    }
    // Fecha socket e termina programa
    close(s);
    return 0;
}

lista_u cria_u (){  //Funcao que cria uma lista_utilizadores com 1 elemento e retorna essa mesma lista_utilizadores
    lista_u aux;
    struct utilizador u1 = {"", "", ""};
    aux = (lista_u)malloc(sizeof(no_lista_utilizador));
    if (aux != NULL) {
        aux->u = u1;
        aux->proximo = NULL;
    }
    return aux;
}

lista_t cria_t (){  //Funcao que cria uma lista_topicos com 1 elemento e retorna essa mesma lista_u
    lista_t aux;
    struct topico t1 = {"", "", ""};
    aux = (lista_t)malloc(sizeof (no_lista_topico));
    if (aux != NULL) {
        aux->t = t1;
        aux->proximo = NULL;
    }
    return aux;
}

lista_n cria_n (){  //Funcao que cria uma lista_topicos com 1 elemento e retorna essa mesma lista_u
    lista_n aux;
    struct noticia n1 = {"", "", ""};
    aux = (lista_n)malloc(sizeof (no_lista_noticia));
    if (aux != NULL) {
        aux->n = n1;
        aux->proximo = NULL;
    }
    return aux;
}

lista_ct cria_ct (){  //Funcao que cria uma lista_ct com 1 elemento e retorna essa mesma lista_u
    lista_ct aux;
    struct cliente_topicos ct1 = {"", {}, 0};
    aux = (lista_ct)malloc(sizeof (no_cliente_topicos));
    if (aux != NULL) {
        aux->ct = ct1;
        aux->proximo = NULL;
    }
    return aux;
}


void ler_ficheiro_utilizadores(lista_u lista_utilizadores) {
    char *token;
    char line[TAM];

    FILE *file = fopen("utilizadores.txt", "r");

    if (file == NULL) {
        printf("Erro ao abrir o ficheiro.\n");
    }

    struct utilizador u;
    while (fgets(line, BUF_SIZE, file)) {
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

void escrever_ficheiro_utilizadores(lista_u lista_utilizadores) {
    FILE *file;

    file = fopen("utilizadores.txt", "w");

    if (file == NULL) {
        printf("Ocorreu um erro ao abrir o ficheiro de texto.\n"); //DEFWEFEWNFJEWFNEWJFEJFBWE
        exit(1);   //A execução do programa termina com erros.
    }

    lista_u aux = lista_utilizadores -> proximo;

    while(aux) {
        fprintf(file, "%s;%s;%s\n", aux -> u.username, aux -> u.password, aux -> u.role);

        aux = aux -> proximo;
    }

    fclose(file);
}

void ler_ficheiro_topicos(lista_t lista_topico, lista_n lista_noticia) {
    char *token;
    char line[BUF_SIZE];

    FILE *file = fopen("topicos.txt", "r");

    if (file == NULL) {
        printf("Erro ao abrir o ficheiro.\n");
    }

    struct topico t;
    struct noticia n;
    while (fgets(line, BUF_SIZE, file)) {
        token = strtok(line, "/");
        int aux = 0;
        while (token != NULL) {
            if (aux == 0) {
                strcpy(t.nome, token);
                strcpy(n.nome_topico, token);
            } else if (aux == 1) {
                strcpy(t.id_topico, token);
            } else if (aux == 2) {
                strcpy(t.porta_topico, token);
            } else if (aux == 3) {
                strcpy(n.titulo, token);
            } else if (aux == 4) {
                strcpy(n.texto, token);
            } else if (aux == 5) {
                strcpy(n.autor, token);
            }
            aux++;
            token = strtok(NULL, "/\n");
        }
        if (procura_topico(lista_topico, t.nome) != 0) {
            insere_topico(lista_topico, t);
        }
        insere_noticia(lista_noticia, n);
    }
    fclose(file);
}

void escrever_ficheiro_topicos(lista_t lista_topico, lista_n lista_noticia) {
    FILE *file;

    file = fopen("topicos.txt", "w");

    if (file == NULL) {
        printf("Ocorreu um erro ao abrir o ficheiro de texto.\n"); //DEFWEFEWNFJEWFNEWJFEJFBWE
        exit(1);   //A execução do programa termina com erros.
    }

    lista_t aux = lista_topico -> proximo;
    lista_n aux2 = lista_noticia -> proximo;
    
    while (aux){
        int count = 0;
        while (aux2){
            if (strcmp(aux -> t.nome, aux2 -> n.nome_topico) == 0){
                fprintf(file, "%s/%s/%s/%s/%s/%s\n", aux -> t.nome, aux -> t.id_topico, aux -> t.porta_topico, aux2 -> n.titulo, aux2 -> n.texto, aux2 -> n.autor);
                count ++;
            }
            aux2 = aux2 -> proximo;
        }
        aux2 = lista_noticia->proximo;
        if (count == 0){
            fprintf(file, "%s/%s/%s///\n", aux -> t.nome, aux -> t.id_topico, aux -> t.porta_topico);
        }
        aux = aux -> proximo;
    }

    fclose(file);
}

void ler_ficheiro_ct(lista_ct lista_cliente_topicos){
    char *token;
    char line[BUF_SIZE];

    FILE *file = fopen("utilizadores_topicos.txt", "r");

    if (file == NULL) {
        printf("Erro ao abrir o ficheiro.\n");
        exit(1);
    }

    //char topicos_aux[TAM][BUF_SIZE];
    struct cliente_topicos ct;

    while (fgets(line, BUF_SIZE, file)) {
        token = strtok(line, "/\n");
        bool user = true;
        int aux = 0;
        char topicos_aux[TAM][BUF_SIZE];

        while(token != NULL){
            if(user){
                strcpy(ct.username, token);
                user = false;
            }else {
                strcpy(topicos_aux[aux], token);
                aux++;
            }
            token = strtok(NULL, ";/\n");
        }

        int i = 0;
        for (i = 0; i < aux; i++) {
            strcpy(ct.topicos[i], topicos_aux[i]);
        }
        ct.num_topicos = atoi(topicos_aux[aux - 1]);
        insere_cliente_topicos(lista_cliente_topicos, ct);

        memset(topicos_aux, 0, sizeof(topicos_aux));
    }
    fclose(file);
}


void escrever_ficheiro_ct(lista_ct lista_cliente_topicos) {
    FILE *file = fopen("utilizadores_topicos.txt", "w");

    if (file == NULL) {
        printf("Ocorreu um erro ao abrir o ficheiro de texto.\n");
        exit(1);
    }

    lista_ct aux = lista_cliente_topicos->proximo;

    while (aux != NULL){
        char buffer[BUF_SIZE] = "";
        strcat(buffer, aux->ct.username);
        strcat(buffer, "/");
        int i = 0;
        for(i = 0; i < aux->ct.num_topicos; i++){
            strcat(buffer, aux->ct.topicos[i]);
            if (i == (aux->ct.num_topicos - 1)){
                strcat(buffer, "/");
            } else {
                strcat(buffer, ";");
            }
        }

        char num[BUF_SIZE];
        sprintf(num, "%d", aux ->ct.num_topicos);
        strcat(buffer, num);
        
        fprintf(file, "%s\n", buffer);
        aux = aux->proximo;
        memset(buffer, 0, sizeof(buffer));
    }

    if (fclose(file) != 0) {
        printf("Ocorreu um erro ao fechar o ficheiro de texto.\n");
        exit(1);
    }
}

void insere_utilizador(lista_u lista_utilizadores, struct utilizador u) {
    lista_u no = (lista_u)malloc(sizeof(no_lista_utilizador)); // aloca espaço para um novo nó
    no->u = u; // armazena a struct utilizador no campo 'u' do novo nó
    no->proximo = lista_utilizadores->proximo; // define o próximo ponteiro do novo nó para o primeiro nó da lista_utilizadores
    lista_utilizadores->proximo = no; // define o próximo ponteiro do nó anterior para o novo nó, adicionando-o na lista_utilizadores
}

void insere_topico(lista_t lista_topico, struct topico t) {
    lista_t no = (lista_t)malloc(sizeof(no_lista_topico)); // aloca espaço para um novo nó
    no->t = t; // armazena a struct utilizador no campo 'u' do novo nó
    no->proximo = lista_topico->proximo; // define o próximo ponteiro do novo nó para o primeiro nó da lista_utilizadores
    lista_topico->proximo = no; // define o próximo ponteiro do nó anterior para o novo nó, adicionando-o na lista_utilizadores
}

void insere_noticia(lista_n lista_noticia, struct noticia n) {
    lista_n no = (lista_n)malloc(sizeof(no_lista_noticia)); // aloca espaço para um novo nó
    no->n = n; // armazena a struct utilizador no campo 'u' do novo nó
    no->proximo = lista_noticia->proximo; // define o próximo ponteiro do novo nó para o primeiro nó da lista_utilizadores
    lista_noticia->proximo = no; // define o próximo ponteiro do nó anterior para o novo nó, adicionando-o na lista_utilizadores
}

void insere_cliente_topicos(lista_ct lista_cliente_topicos, struct cliente_topicos ct) {
    lista_ct no = (lista_ct) malloc(sizeof(no_cliente_topicos)); // aloca espaço para um novo nó
    no->ct = ct; // armazena a struct utilizador no campo 'u' do novo nó
    no->proximo = lista_cliente_topicos->proximo; // define o próximo ponteiro do novo nó para o primeiro nó da lista_utilizadores
    lista_cliente_topicos->proximo = no; // define o próximo ponteiro do nó anterior para o novo nó, adicionando-o na lista_utilizadores
}

int confirmar_login(lista_u lista_utilizadores ,char *username_login, char *password_login){
    lista_u aux = lista_utilizadores -> proximo;

    while(aux){
        if (strcmp(aux -> u.username, username_login) == 0 && strcmp(aux -> u.password, password_login) == 0){
            if (strcmp(aux -> u.role, "administrador") == 0){
                return 0;
            }
            else {
                return 2;
            }
        }
        aux = aux -> proximo;
    }
    return 1;
}

int conf_nome(lista_u lista_utilizadores, char username[TAM]){
    lista_u aux = lista_utilizadores -> proximo;
    int count = 0;

    while (aux){
        if (strcmp(aux -> u.username, username) == 0){
            count++;
        }

        aux = aux -> proximo;
    }
    return count;
}

int verifica_role(char aux[TAM]){
    int count = 1;

    if (strcmp(aux, "leitor") == 0 || strcmp(aux, "jornalista") == 0 || strcmp(aux, "administrador") == 0){
        count = 0;
    }

    return count;
}

void add_utilizador(lista_u lista_utilizadores, char username[TAM], char password[TAM], char role[TAM]){ //ver isto porque o verifica aparece ja em cima
    struct utilizador u;
    char aux[TAM];
    strcpy(aux, role);

    if (verifica_role(&aux[0]) == 0) { //Verifica se o 3 parametros é leitor, admin ou jornalista
        strcpy(u.username, username); //Guarda as 3 strings nos seus respetivos lugares na struct
        strcpy(u.password, password);
        strcpy(u.role, role);

        insere_utilizador(lista_utilizadores, u); //Adiciona-se essa struct à lista_utilizadores

        char mensagem[BUF_SIZE];
        strcpy(mensagem, "Utilizador adicionado com sucesso.\n");
        sendto(s, mensagem, strlen(mensagem), 0, (struct sockaddr *) &client_sock, slen);
        memset(mensagem, 0, sizeof(mensagem));
    } else {
        char mensagem_erro[BUF_SIZE];
        strcpy(mensagem_erro, "Cargo nao existente.\n");
        sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &client_sock, slen);
        memset(mensagem_erro, 0, sizeof(mensagem_erro));
    }
}

void remove_utilizador(lista_u lista_utilizadores, char username[TAM]) {
    lista_u aux = lista_utilizadores -> proximo;

    lista_u ant = aux;
    aux = aux->proximo;
    int count = 0;

    while (aux) {
        if (strcmp(aux->u.username, username) == 0) {
            ant->proximo = aux->proximo;
            free(aux);
            count++;
            return;
        }
        ant = aux;
        aux = aux->proximo;
    }
    if (count == 0){
        printf("Elemento não encontrado\n");  //EFNAKFNEFJNEWFJEWNFJEWF
    }
}

void list(lista_u lista_utilizadores){
    lista_u aux = lista_utilizadores -> proximo;

    while(aux){
        char topicos[BUF_SIZE];
        strcpy(topicos, aux -> u.username);
        strcat(topicos, "\n");
        sendto(s, topicos, strlen(topicos), 0, (struct sockaddr *) &client_sock, slen);
        aux = aux -> proximo;
    }
}

void process_client(int client_fd) {
    lista_u lista_utilizadores = cria_u();
    lista_t lista_topico = cria_t();
    lista_n lista_noticia = cria_n();
    lista_ct lista_cliente_topicos = cria_ct();

    ler_ficheiro_utilizadores(lista_utilizadores);
    ler_ficheiro_topicos(lista_topico, lista_noticia);
    ler_ficheiro_ct(lista_cliente_topicos);

    char username[BUF_SIZE], password[BUF_SIZE];

    while (1) {
        char login_str[BUF_SIZE];
        strcpy(login_str, "Insira o nome do utilizador: ");
        write(client_fd, login_str, strlen(login_str));
        fsync(client_fd);

        read(client_fd, username, BUF_SIZE);  
        fflush(stdout);

        write(client_fd, "Insira a password: ", strlen("Insira a password: "));
        fsync(client_fd);

        read(client_fd, password, BUF_SIZE); 
        fflush(stdout);

        char inutil[BUF_SIZE];
        if (confirmar_login(lista_utilizadores, username, password) == 2) {
            write(client_fd, "Login efetuado com sucesso. Bem-vindo\n", strlen("Login efetuado com sucesso. Bem-vindo\n"));
            fsync(client_fd);

            read(client_fd, inutil, BUF_SIZE);
            fflush(stdout);

            if (e_leitor(lista_utilizadores, username)) {
                write(client_fd, "leitor", strlen("leitor"));
            } else {
                write(client_fd, "jornalista", strlen("jornalista"));
            }

            char opcao[BUF_SIZE];
            char mensagem_enviada[BUF_SIZE];

            while (1) {
                read(client_fd, opcao, BUF_SIZE);  
                if (strcmp(opcao, "1") == 0) {
                    memset(mensagem_enviada, 0, sizeof(mensagem_enviada));
                    strcpy(mensagem_enviada, lista_topicos(lista_topico));
                    write(client_fd, mensagem_enviada, strlen(mensagem_enviada));
                } else  if (strcmp(opcao, "2") == 0) {
                    write(client_fd, "Qual o topico que pretende subscrever?\n", strlen("Qual o topico que pretende subscrever?\n"));
                    char topico[BUF_SIZE];
                    memset(topico, 0, sizeof(topico));
                    read(client_fd, topico, BUF_SIZE);
                    memset(mensagem_enviada, 0, sizeof(mensagem_enviada));
                    strcpy(mensagem_enviada, subscreve_topico(username, topico, lista_cliente_topicos, lista_topico));
                    escrever_ficheiro_ct(lista_cliente_topicos);
                    write(client_fd, mensagem_enviada, strlen(mensagem_enviada));
                } else if ((strcmp(opcao, "3") == 0) && (!e_leitor(lista_utilizadores, username))) {
                    write(client_fd, "Qual o nome do topico que pretende criar?\n", strlen("Qual o nome do topico que pretende criar?\n"));
                    char novo_topico[BUF_SIZE];
                    memset(novo_topico, 0, sizeof(novo_topico));
                    read(client_fd, novo_topico, BUF_SIZE);  
                    memset(mensagem_enviada, 0, sizeof(mensagem_enviada));
                    strcpy(mensagem_enviada, cria_topico(novo_topico, lista_topico));
                    write(client_fd, mensagem_enviada, strlen(mensagem_enviada));
                    escrever_ficheiro_topicos(lista_topico, lista_noticia);
                } else if ((strcmp(opcao, "4") == 0) && (!e_leitor(lista_utilizadores, username))) {
                    write(client_fd, "Qual o topico da noticia que pretende criar?\n", strlen("Qual o topico da noticia que pretende criar?\n"));
                    char topico_noticia[BUF_SIZE];
                    memset(topico_noticia, 0, sizeof(topico_noticia));
                    read(client_fd, topico_noticia, BUF_SIZE);
                    write(client_fd, "Qual o titulo da noticia que pretende criar?\n", strlen("Qual o titulo da noticia que pretende criar?\n"));
                    char titulo_noticia[BUF_SIZE];
                    memset(titulo_noticia, 0, sizeof(titulo_noticia));
                    read(client_fd, titulo_noticia, BUF_SIZE);
                    write(client_fd, "Escreva o texto da noticia.\n", strlen("Escreva o texto da noticia.\n"));
                    char noticia[BUF_SIZE];
                    memset(noticia, 0, sizeof(noticia));
                    read(client_fd, noticia, BUF_SIZE);
                    memset(mensagem_enviada, 0, sizeof(mensagem_enviada));
                    strcpy(mensagem_enviada, cria_noticia(username, topico_noticia, titulo_noticia, noticia, lista_noticia, lista_topico));
                    write (client_fd, mensagem_enviada, strlen(mensagem_enviada));
                    escrever_ficheiro_topicos(lista_topico, lista_noticia);
                } else if (strcmp(opcao, "0") == 0){
                    write(client_fd, "Ate logo!\n", strlen("Ate logo!\n"));
                    fsync(client_fd);
                    exit(0);  
                } else {
                    write(client_fd, "Opcao invalida.\n", strlen("Opcao invalida.\n")); 
                    fsync(client_fd);
                }
                memset(opcao, 0, BUF_SIZE);
            }
        } else if (confirmar_login(lista_utilizadores, username, password) == 0){
            write(client_fd, "Cargo nao admitido. Tente outra vez.\n", strlen("Cargo nao admitido. Tente outra vez.\n"));
            fsync(client_fd);
            memset(login_str, 0, sizeof(login_str));
            memset(username, 0, BUF_SIZE);
            memset(password, 0, BUF_SIZE);

        } else {
            write(client_fd, "Dados incorretos. Tente outra vez.\n", strlen("Dados incorretos. Tente outra vez.\n"));
            fsync(client_fd);
            memset(login_str, 0, sizeof(login_str));
            memset(username, 0, BUF_SIZE);
            memset(password, 0, BUF_SIZE);
        }
        memset(inutil, 0, BUF_SIZE);
        read(client_fd, inutil, BUF_SIZE);
        fflush(stdout);
    }
    close(client_fd);
}

int procura_topico(lista_t lista_topico, char nome[TAM]){
    int count = 1;
    lista_t aux = lista_topico -> proximo;
    while(aux){
        if (strcmp(aux->t.nome, nome) == 0) {
            count = 0;
        }
        aux = aux -> proximo;
    }
    return count;
}

int e_leitor(lista_u lista_utilizadores, char username_login[BUF_SIZE]){
    lista_u aux = lista_utilizadores -> proximo;
    while(aux){
        if (strcmp(aux->u.username, username_login) == 0) {
            if (strcmp(aux->u.role, "leitor") == 0) {
                return 1;
            }
        }
        aux = aux -> proximo;
    }
    return 0;
}

char *lista_topicos(lista_t lista_topico){
    lista_t aux = lista_topico -> proximo;
    char *topicos = malloc(BUF_SIZE);
    strcpy(topicos, "Topicos:\n");
    while(aux){
        strcat(topicos, aux -> t.nome);
        strcat(topicos, "\n");
        aux = aux -> proximo;
    }
    return topicos;
}

char *subscreve_topico(char username[BUF_SIZE], char topico[BUF_SIZE], lista_ct lista_cliente_topicos, lista_t lista_topico){
    lista_t aux = lista_topico -> proximo;
    while (aux) {
        if (strcmp(aux->t.nome, topico) == 0) {
            lista_ct aux_ct = lista_cliente_topicos -> proximo;
            while (aux_ct) {
                if (strcmp(aux_ct->ct.username, username) == 0) {
                    for(int i = 0; i < TAM; i++){
                        if(strcmp(aux_ct -> ct.topicos[i], topico) == 0){
                            return "Este topico ja foi subscrito.\n";
                        }
                    }
                    strcpy(aux_ct->ct.topicos[aux_ct->ct.num_topicos], topico);
                    aux_ct->ct.num_topicos++;
                    return "Topico subscrito com sucesso.\n";
                }
                aux_ct = aux_ct->proximo;
            }
            struct cliente_topicos novo_ct;
            strcpy(novo_ct.username, username);
            strcpy(novo_ct.topicos[0], topico);
            novo_ct.num_topicos = 1;
            insere_cliente_topicos(lista_cliente_topicos, novo_ct);
            return "Topico subscrito com sucesso.\n";
        }
        aux = aux->proximo;
    }
    return "O topico nao existe.\n";
}

char *cria_topico(char novo_topico[BUF_SIZE], lista_t lista_topico){
    lista_t aux = lista_topico -> proximo;

    if (procura_topico(lista_topico, novo_topico) == 0) {
        return "Topico ja existente.\n";
    }

    struct topico nt;

    int id_atual = 0;
    while (aux) {
        char *ultimo_ponto = strrchr(aux -> t.id_topico, '.');
        int id_lista = atoi(ultimo_ponto + 1);
        if (id_lista >= id_atual) {
            id_atual = id_lista + 1;
        }
        aux = aux->proximo;
    }

    int porta_atual = 5000; // porta inicial
    aux = lista_topico;
    while (aux != NULL) {
        int porta_lista = atoi(aux->t.porta_topico);
        if (porta_lista >= porta_atual) {
            porta_atual = porta_lista + 1;
        }
        aux = aux->proximo;
    }

    char id_topico[TAM];
    strcpy(id_topico, "127.0.0.");
    char num[BUF_SIZE];
    sprintf(num, "%d", id_atual);
    strcat(id_topico, num);


    strcpy(nt.nome, novo_topico); //Guarda as 3 strings nos seus respetivos lugares na struct
    strcpy(nt.id_topico, id_topico);
    sprintf(nt.porta_topico, "%d", porta_atual);

    insere_topico(lista_topico, nt);

    return "Topico criado com sucesso\n";
}


char *cria_noticia(char username[BUF_SIZE], char topico[BUF_SIZE], char titulo[BUF_SIZE], char texto[BUF_SIZE], lista_n lista_noticia, lista_t lista_topico){
    if (procura_topico(lista_topico, topico) != 0) {
        return "Topico nao existente. Crie o topico antes de criar a noticia\n";
    }

    struct noticia n;

    strcpy(n.nome_topico, topico);
    strcpy(n.titulo, titulo);
    strcpy(n.texto, texto);
    strcpy(n.autor, username);

    insere_noticia(lista_noticia, n);

    return "Noticia criada com sucesso\n";
}

void erro(char *s){
    perror(s);
    exit(1);
}


