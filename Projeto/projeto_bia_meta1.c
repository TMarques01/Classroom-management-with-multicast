#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFLEN 512	// Tamanho do buffer
#define PORT 9876	// Porto para recepção das mensagens
#define TAM 25      // Tamanho do username/password

//login, verificadores de add_user, delete, 
struct utilizador {
    char username[TAM];
    char password[TAM];
    char role[TAM];
};

typedef struct no_lista { //Struct da lista ligada
    struct utilizador u;
    struct no_lista * proximo;
} no_lista;

typedef no_lista * lista;

void erro(char *s);
lista cria ();
void add_utilizador(lista lista_utilizadores, char username[TAM], char password[TAM], char role[TAM]);
void insere_utilizador (lista lista_utilizadores, struct utilizador u);
void remove_utilizador(lista *lista_utilizadores, char username[TAM]);
void ler_ficheiro(lista lista_utilizadores);
void escrever_ficheiro(lista lista_utilizadores);
int conf_nome(lista lista_utilizadores, char nome[TAM]);
int verifica_role(char aux[TAM]);
void list(lista lista_utilizadores);
int confirmar_login(lista lista_utilizadores ,char username_login[TAM], char password_login[TAM]);

struct sockaddr_in si_minha, si_outra;
int s,recv_len;
socklen_t slen = sizeof(si_outra);

int main(int argc, char *argv[]) {
	char buf[BUFLEN];

    if (argc != 4) {
    printf("news_server {PORTO_NOTICIAS} {PORTO_CONFIG} {ficheiro configuracao}\n"); //FEJFNEQJFNEQJDNQJQNF
    exit(-1);
  }

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
    char str[BUFLEN];

    lista lista_utilizadores = cria(); //Cria a lista ligada já com um elemento para evitar o caso de ser nulo
    ler_ficheiro(lista_utilizadores);

    int login = 1;
    int aux_login = 1;
    char username_login[TAM];
    char password_login[TAM];
    char mensagem[BUFLEN];
    int login_st = 0;

	while (1) {
        // Espera recepção de mensagem (a chamada é bloqueante)
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_outra, &slen)) == -1) {
            erro("Erro no recvfrom");  //DDJWDQWJBDQWJDBQWJDBWQJDBQWJBDQ
        }

        // Limpa o buffer buf para receber mais mensagens
        buf[recv_len] = '\0';
        
        if (login_st == 0){
            char login_str[BUFLEN];
            strcpy(login_str, "Insira o nome do utilizador: ");
            sendto(s, login_str, strlen(login_str), 0, (struct sockaddr *) &si_outra, slen);
            login_st++; //Passa a ser 1
        }

        if (strcmp(buf, "X") != 0 && strlen(buf) != 1) {
            strcpy(str, buf);
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
                    sendto(s, mensagem, strlen(mensagem), 0, (struct sockaddr *) &si_outra, slen);
                    aux_login++;
                    memset(mensagem, 0, sizeof(mensagem));
                } else if (aux_login == 2){
                    strcpy(mensagem, "Insira a password: ");
                    sendto(s, mensagem, strlen(mensagem), 0, (struct sockaddr *) &si_outra, slen);
                    aux_login++;
                    strcpy(username_login, token);
                    memset(mensagem, 0, sizeof(mensagem));
                } else if (aux_login == 3){
                    strcpy(password_login, token);
                    if (confirmar_login(lista_utilizadores, username_login, password_login) == 0){
                        login = 0;
                        char mensagem[BUFLEN];
                        strcpy(mensagem, "Login efetuado com sucesso. Bem-vindo\n");
                        sendto(s, mensagem, strlen(mensagem), 0, (struct sockaddr *) &si_outra, slen);
                        memset(mensagem, 0, sizeof(mensagem));
                    } else  if (confirmar_login(lista_utilizadores, username_login, password_login) == 2){
                        char mensagem_erro[BUFLEN];
                        strcpy(mensagem_erro, "Cargo nao admitido.\nInsira o nome de utilizador: ");
                        sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &si_outra, slen);
                        memset(mensagem_erro, 0, sizeof(mensagem_erro));
                    }else {
                        login = 1;
                        aux_login = 2;
                        char mensagem_erro[BUFLEN];
                        strcpy(mensagem_erro, "Dados incorretos. Tente outra vez.\nInsira o nome de utilizador: ");
                        sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &si_outra, slen);
                        memset(mensagem_erro, 0, sizeof(mensagem_erro));
                        memset(username_login, 0, sizeof(username_login));
                        memset(password_login, 0, sizeof(password_login));
                    }
                }
            } else {
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
                            char mensagem_erro[BUFLEN];
                            strcpy(mensagem_erro, "Username ja existente.\n");
                            sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &si_outra, slen);
                            memset(mensagem_erro, 0, sizeof(mensagem_erro));
                            aux = 0;
                            break;
                        }
                        i++; // incrementa o contador de tokens
                    }
                    if (aux == 1 && i == 4){
                        add_utilizador(lista_utilizadores, username, password, role);
                        escrever_ficheiro(lista_utilizadores);
                    } else if (i != 4){
                        char mensagem_erro[BUFLEN];
                        strcpy(mensagem_erro, "Argumentos em falta ou em demasia.\n");
                        sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &si_outra, slen);
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
                                char mensagem_erro[BUFLEN];
                                strcpy(mensagem_erro, "Utilizador nao disponivel para ser eliminado.\n");
                                sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &si_outra, slen);
                                memset(mensagem_erro, 0, sizeof(mensagem_erro));
                            }
                        }else {
                            char mensagem_erro[BUFLEN];
                            strcpy(mensagem_erro, "Username nao existente.\n");
                            sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &si_outra, slen);
                            memset(mensagem_erro, 0, sizeof(mensagem_erro));
                        }
                        i++;
                    }
                    if (aux == 1){
                        remove_utilizador(&lista_utilizadores, username);
                        escrever_ficheiro(lista_utilizadores);
                        char mensagem[BUFLEN];
                        strcpy(mensagem, "Utilizador eliminado com sucesso.\n");
                        sendto(s, mensagem, strlen(mensagem), 0, (struct sockaddr *) &si_outra, slen);
                        memset(mensagem, 0, sizeof(mensagem));
                    } else if (i != 2){
                        char mensagem_erro[BUFLEN];
                        strcpy(mensagem_erro, "Argumentos em falta ou em demasia.\n");
                        sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &si_outra, slen);
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
	// Fecha socket e termina programa
	close(s);
	return 0;
}

int confirmar_login(lista lista_utilizadores ,char username_login[TAM], char password_login[TAM]){
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

int conf_nome(lista lista_utilizadores, char username[TAM]){
    lista aux = lista_utilizadores -> proximo;
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

lista cria (){  //Funcao que cria uma lista com 1 elemento e retorna essa mesma lista
    lista aux;
    struct utilizador u1 = {"", "", ""};
    aux = (lista)malloc(sizeof (no_lista));
    if (aux != NULL) {
        aux->u = u1;
        aux->proximo = NULL;
    }
    return aux;
}

void add_utilizador(lista lista_utilizadores, char username[TAM], char password[TAM], char role[TAM]){ //ver isto porque o verifica aparece ja em cima
    struct utilizador u;
    char aux[TAM];
    strcpy(aux, role);

    if (verifica_role(&aux[0]) == 0) { //Verifica se o 3 parametros é leitor, admin ou jornalista
        strcpy(u.username, username); //Guarda as 3 strings nos seus respetivos lugares na struct
        strcpy(u.password, password);
        strcpy(u.role, role);

        insere_utilizador(lista_utilizadores, u); //Adiciona-se essa struct à lista

        char mensagem[BUFLEN];
        strcpy(mensagem, "Utilizador adicionado com sucesso.\n");
        sendto(s, mensagem, strlen(mensagem), 0, (struct sockaddr *) &si_outra, slen);
        memset(mensagem, 0, sizeof(mensagem));
    } else {
        char mensagem_erro[BUFLEN];
        strcpy(mensagem_erro, "Cargo nao existente.\n");
        sendto(s, mensagem_erro, strlen(mensagem_erro), 0, (struct sockaddr *) &si_outra, slen);
        memset(mensagem_erro, 0, sizeof(mensagem_erro));
    }
}

void insere_utilizador(lista lista_utilizadores, struct utilizador u) {
    lista no = (lista) malloc(sizeof(no_lista)); // aloca espaço para um novo nó
    no->u = u; // armazena a struct utilizador no campo 'u' do novo nó
    no->proximo = lista_utilizadores->proximo; // define o próximo ponteiro do novo nó para o primeiro nó da lista 
    lista_utilizadores->proximo = no; // define o próximo ponteiro do nó anterior para o novo nó, adicionando-o na lista
}

void remove_utilizador(lista *lista_utilizadores, char username[TAM]) {
    lista aux = *lista_utilizadores;
   
    lista ant = aux;
    aux = aux->proximo;
    int count = 0;

    while (aux != NULL) {
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

void list(lista lista_utilizadores){
    lista aux = lista_utilizadores -> proximo;

    while(aux){
        char buf[BUFLEN];
        strcpy(buf, aux -> u.username);
        strcat(buf, "\n");
        sendto(s, buf, strlen(buf), 0, (struct sockaddr *) &si_outra, slen);
        aux = aux -> proximo;
    }
}

void ler_ficheiro(lista lista_utilizadores) {
    char *token;
    char line[TAM];

    FILE *file = fopen("ficheiro.txt", "r");

    if (file == NULL) {
        printf("Erro ao abrir o ficheiro.\n");   //DJFBEQFJQEBFJEBFJQFQE
    }

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

void escrever_ficheiro(lista lista_utilizadores) {
    FILE *file;

    file = fopen("ficheiro.txt", "w");

    if (file == NULL) {
        printf("Ocorreu um erro ao abrir o ficheiro de texto.\n"); //DEFWEFEWNFJEWFNEWJFEJFBWE
        exit(1);   //A execução do programa termina com erros.
    }

    lista aux = lista_utilizadores -> proximo;

    while(aux) {
        fprintf(file, "%s;%s;%s\n", aux -> u.username, aux -> u.password, aux -> u.role);

        aux = aux -> proximo;
    }

    fclose(file);
}

void erro(char *s){ 
	perror(s);
	exit(1);
}