#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 8080
#define BUF_SIZE 1024


/*---------Multicast----------------*/

struct sockaddr_in * endereco_escuta;   
struct sockaddr_in * endereco_receber;
struct sockaddr_in * endereco_enviar;
int fd_socket_escuta;
char ip_destino[BUF_SIZE];


char * grupo_multicast(char username_destino[BUF_SIZE], char pacote[BUF_SIZE]);
char * conectarMulticast ();
char * recebe_mensagem(char id_fonte[BUF_SIZE], char id_destino[BUF_SIZE], char endereco[BUF_SIZE], int porto);
char * envia_mensagem(char mensagem[BUF_SIZE]);
struct sockaddr_in * mallocEndereco ();
char * recebe_mensagem();
char * imprime_mensagem(char id_fonte[BUF_SIZE], char id_destino[BUF_SIZE], char ip[BUF_SIZE], int porto, char mensagem[BUF_SIZE]);


void erro(char *msg);

int main(int argc, char *argv[]) {
    char endServer[100];
    struct sockaddr_in addr;
    struct hostent *hostPtr;
    int fd;
    if (argc != 3)
        erro("news_client {endereco do servidor} {PORTO_NOTICIAS}");

    strcpy(endServer, argv[1]);
    if ((hostPtr = gethostbyname(endServer)) == NULL)
        erro("Não consegui obter endereço.");

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ((struct in_addr *) (hostPtr->h_addr))->s_addr;
    addr.sin_port = htons((int) atoi(argv[2]));

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        erro("Socket.");
    if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        erro("Connect.");

    while(1){
        char buffer_mensagem[BUF_SIZE], buffer_pass[BUF_SIZE], buffer_resposta[BUF_SIZE];
        memset(buffer_mensagem, 0, BUF_SIZE);
        read(fd, buffer_mensagem, BUF_SIZE);
        fflush(stdout);
        printf("%s", buffer_mensagem);
        memset(buffer_mensagem, 0, BUF_SIZE);


        char username[255];
        scanf("%s", username);
        write(fd, username, strlen(username));
        memset(username, 0, BUF_SIZE);


        read(fd, buffer_pass, BUF_SIZE);
        fflush(stdout);
        printf("%s", buffer_pass);
        memset(buffer_pass, 0, BUF_SIZE);

        char password[255];
        scanf("%s", password);
        write(fd, password, strlen(password));
        memset(password, 0, BUF_SIZE);


        read(fd, buffer_resposta, BUF_SIZE);
        fflush(stdout);

        printf("%s", buffer_resposta);


        if (strcmp(buffer_resposta, "Login efetuado com sucesso. Bem-vindo\n") == 0){
            break;
        }
        memset(buffer_resposta, 0, BUF_SIZE);
        write(fd, "inutil", strlen("inutil"));
    }
    
    write(fd, "inutil", strlen("inutil"));
    
    char mensagem_enviada[BUF_SIZE];
    char mensagem_recebida[BUF_SIZE];
    char role[BUF_SIZE];
    read(fd, role, BUF_SIZE);


    while(1){
        if (strcmp(role, "leitor") == 0){
            printf("\nMenu:\n"
                   "1- LIST_TOPICS\n"
                   "2- SUBSCRIBE_TOPIC\n"
                   "0- SAIR\n"
                   "Opcao: ");
        } else {
            printf("\nMenu:\n"
                   "1- LIST_TOPICS\n"
                   "2- SUBSCRIBE_TOPIC\n"
                   "3- CREATE_TOPIC\n"
                   "4- CREATE_NEW\n"
                   "0- SAIR\n"
                   "Opcao: ");
        }

        char opcao[BUF_SIZE];
        scanf("%s", opcao);
        write(fd, opcao, strlen(opcao));
        
        if (strcmp(opcao, "1") == 0){
            read(fd, mensagem_recebida, BUF_SIZE);
            printf("%s", mensagem_recebida);
            memset(mensagem_recebida, 0, BUF_SIZE);
        } else if (strcmp(opcao, "2") == 0){
            read(fd, mensagem_recebida, BUF_SIZE);
            printf("%s", mensagem_recebida);
            memset(mensagem_recebida, 0, BUF_SIZE);
            scanf("%s", mensagem_enviada);
            write(fd, mensagem_enviada, strlen(mensagem_enviada));
            memset(mensagem_enviada, 0, BUF_SIZE);
            read(fd, mensagem_recebida, BUF_SIZE);
            printf("%s", mensagem_recebida);
            memset(mensagem_recebida, 0, BUF_SIZE);
        } else if ((strcmp(opcao, "3") == 0) && (strcmp(role, "jornalista") == 0)){
            read(fd, mensagem_recebida, BUF_SIZE);
            printf("%s", mensagem_recebida);
            memset(mensagem_recebida, 0, BUF_SIZE);
            scanf("%s", mensagem_enviada);
            write(fd, mensagem_enviada, strlen(mensagem_enviada));
            memset(mensagem_enviada, 0, BUF_SIZE);
            read(fd, mensagem_recebida, BUF_SIZE);
            printf("%s", mensagem_recebida);
            memset(mensagem_recebida, 0, BUF_SIZE);
        } else if ((strcmp(opcao, "4") == 0) && (strcmp(role, "jornalista") == 0)){
            read(fd, mensagem_recebida, BUF_SIZE);
            printf("%s", mensagem_recebida);
            memset(mensagem_recebida, 0, sizeof(mensagem_recebida));
            memset(mensagem_enviada, 0, sizeof(mensagem_enviada));
            scanf("%s", mensagem_enviada);
            write(fd, mensagem_enviada, strlen(mensagem_enviada));
            memset(mensagem_enviada, 0, sizeof(mensagem_enviada));

            read(fd, mensagem_recebida, BUF_SIZE);
            printf("%s", mensagem_recebida);
            memset(mensagem_recebida, 0, sizeof(mensagem_recebida));
            scanf("%s", mensagem_enviada);
            write(fd, mensagem_enviada, strlen(mensagem_enviada));
            memset(mensagem_enviada, 0, sizeof(mensagem_enviada));

            read(fd, mensagem_recebida, BUF_SIZE);
            printf("%s", mensagem_recebida);
            memset(mensagem_recebida, 0, sizeof(mensagem_recebida));
            scanf("%s", mensagem_enviada);
            write(fd, mensagem_enviada, strlen(mensagem_enviada));
            memset(mensagem_enviada, 0, sizeof(mensagem_enviada));

            read(fd, mensagem_recebida, BUF_SIZE);
            printf("%s", mensagem_recebida);
            memset(mensagem_recebida, 0, sizeof(mensagem_recebida));
        }else if (strcmp(opcao, "0") == 0){
            read(fd, mensagem_recebida, BUF_SIZE);
            printf("%s", mensagem_recebida);
            memset(mensagem_recebida, 0, BUF_SIZE);
            close(fd);
            exit(0);
        } else{
            read(fd, mensagem_recebida, BUF_SIZE);
            printf("%s", mensagem_recebida);
            memset(mensagem_recebida, 0, BUF_SIZE);
        }
    } 
    close(fd);
    exit(0);
}


/*      -------------------MULTICAST------------------     */

char * grupo_multicast(char username_destino[BUF_SIZE], char pacote[BUF_SIZE]){
    char input_string[BUF_SIZE];
    ssize_t nRead;
    char * valor_temporario = NULL;
    char * string_temporaria = NULL;
    socklen_t endereco_tam;
    do {
        printf("Introduza o nome/ID do grupo (se nao existir sera criado um novo):\n");
        scanf("%s", input_string);
        if (input_string[0] == '\0') {
            printf("Username invalido! Tente Novamente!\n");
            input_string[0] = '\0';
        }
    } while (input_string[0] == '\0');
    snprintf(username_destino, BUF_SIZE, "%s", input_string);

    if (sendto(fd_socket_escuta, pacote, strlen(pacote) + 1, 0, (struct sockaddr *) endereco_enviar, endereco_tam) <= 0) {
        return "Erro inesperadoa enviar a mensagem";
    }

    if ((nRead = recvfrom(fd_socket_escuta, pacote, BUF_SIZE, 0, (struct sockaddr *) endereco_receber, (socklen_t *) & endereco_tam)) <= 0) {
        return "doServico", "Erro inesperado na receber a mensagem";
    }

    pacote[nRead] = '\0';
    
    snprintf(ip_destino, BUF_SIZE, "%s", valor_temporario);
    if (strcmp(conectarMulticast(), "SUCESSO") != 0) {
        return "Nao foi possivel conectar ao Multicast, conectaMulticast() == ERRO";
    }
    printf("Servico ativado com sucesso!");

}

char * conectarMulticast () {
    struct hostent * ptr_ip = NULL ;
    struct ip_mreq endereco_multicast;
    int multicast_TTL = 3 ;
    int soReuseAddr = 1 ;
    int ip_multicast_loop = 0 ;

    if ((ptr_ip = gethostbyname (ip_destino)) == 0 ) {
        return "Impossível obter endereco ";
    }

    endereco_escuta-> sin_family = AF_INET;
    endereco_escuta-> sin_addr . s_addr = htonl (0);
    endereco_escuta-> sin_port = htons (5000);

    endereco_receber-> sin_family = AF_INET;
    endereco_receber-> sin_addr . s_addr = htonl (0);
    endereco_receber-> sin_port = htons (5000);

    endereco_enviar-> sin_family = AF_INET;
    endereco_enviar-> sin_addr . s_addr = (( struct in_addr *)(ptr_ip-> h_addr ))-> s_addr ;
    endereco_enviar-> sin_port = htons (5000);



    if ((fd_socket_escuta = socket (AF_INET, SOCK_DGRAM, 0 )) < 0 ) {
        return " Socket";
    }

    if ( setsockopt (fd_socket_escuta, IPPROTO_IP, SO_REUSEADDR, ( void *) &soReuseAddr, sizeof (soReuseAddr)) < 0 ) {
        return " setsockopt";
    }

    if ( bind (fd_socket_escuta, ( struct sockaddr*) endereco_escuta, sizeof ( struct sockaddr_in)) < 0 ) {
        return " bind";
    }

    endereco_multicast. imr_multiaddr . s_addr = inet_addr (ip_destino);
    endereco_multicast. imr_interface . s_addr = htonl (0);

    if (setsockopt (fd_socket_escuta, IPPROTO_IP, IP_ADD_MEMBERSHIP, &endereco_multicast, sizeof (endereco_multicast)) < 0 ) {
        return " setsockopt, IP_ADD_MEMBERSHIP ";
    }

    if (setsockopt (fd_socket_escuta, IPPROTO_IP, IP_MULTICAST_TTL, ( void *) &multicast_TTL, sizeof (multicast_TTL)) < 0 ) {
        return " setsockopt, IP_MULTICAST_TTL ";
    }

    if (setsockopt (fd_socket_escuta, IPPROTO_IP, IP_MULTICAST_LOOP, ( void *) &ip_multicast_loop, sizeof (ip_multicast_loop)) < 0 ) {
        return " setsockopt, IP_MULTICAST_LOOP ";
    }

    return "SUCESSO";
}

char * recebe_mensagem(char id_fonte[BUF_SIZE], char id_destino[BUF_SIZE], char endereco[BUF_SIZE], int porto) {
    int endereco_tamanho = sizeof(struct sockaddr_in);
    char mensagem_enviada[BUF_SIZE];
    ssize_t nRead;


    while (1) {              // Fica a espera de mensagem
        if ((nRead = recvfrom(fd_socket_escuta, mensagem_enviada, BUF_SIZE, 0, (struct sockaddr *) endereco_receber, (socklen_t *) & endereco_tamanho)) <= 0) {
            return "Erro inesperado na receber a mensagem";
        }
        mensagem_enviada[nRead] = '\0';
    }
    imprime_mensagem(id_fonte, id_destino, endereco, porto, mensagem_enviada);
}

char * envia_mensagem(char mensagem[BUF_SIZE]) {
    int endereco_tam = sizeof(struct sockaddr_in);
    char pacote[BUF_SIZE];

    while (1) {// Fica a espera do input do utilizador
        scanf ("%s", mensagem);
        if (mensagem[0] != '\0') {                                              // se estiver vazia, ignora
            if (sendto(fd_socket_escuta, pacote, strlen(pacote) + 1, 0, (struct sockaddr *) endereco_enviar, endereco_tam) <= 0) {    // Enviar a mensagem
                return "Erro inesperado a enviar a mensagem";
            }
        }
    }
}



struct sockaddr_in * mallocEndereco () {
    struct sockaddr_in * endereco_temporario = ( struct sockaddr_in *) malloc (sizeof ( struct sockaddr_in));

    if (endereco_temporario ) {
        printf(" endereco == NULL ");
    } else {
        bzero (( void *) endereco_temporario, sizeof ( struct sockaddr_in));
    }

    return endereco_temporario;
}

char * imprime_mensagem(char id_fonte[BUF_SIZE], char id_destino[BUF_SIZE], char ip[BUF_SIZE], int porto, char mensagem[BUF_SIZE]){
    if (id_fonte == NULL)
        id_fonte = "?";
    if (id_destino == NULL)
        id_destino = "?";
    if (ip == NULL)
        ip = "?";
    if (mensagem == NULL)
        mensagem = "?";

    printf("DE: \"%s\",  PARA: \"%s\" (%s:%d),  MSG: \"%s\"\n", id_fonte, id_destino, ip, porto, mensagem);
    fflush(stdout);
}


void erro(char *msg) {
    printf("Erro: %s\n", msg);
    exit(-1);
}