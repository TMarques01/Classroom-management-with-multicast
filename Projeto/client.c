#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

#define BUFF_SIZE 1024
#define MAX_GROUPS 1000

#define MULTICAST_PORT 8888

int sockets[MAX_GROUPS];
int num_groups = 0;
char ips[MAX_GROUPS][16]; // array de ips
char name_classes[MAX_GROUPS][BUFF_SIZE]; // array de nomes de turmas
struct ip_mreq mreqs[MAX_GROUPS];
char username[BUFF_SIZE];

void erro(char *msg);
void join_group(char group[], int port, char name_class[]);
void* listen_group(void* arg);
void close_connections();
void sigint_handler(int sgn);
void print_subscribed_classes();

int main(int argc, char *argv[]) {
    char endServer[100];
    char buffer_write[BUFF_SIZE];
    char buffer_read[BUFF_SIZE];
    int fd, nread = 0;
    
    struct sockaddr_in addr;
    struct hostent *hostPtr;

    if (argc != 3) {
        printf("class_client {endereço servidor} {PORTO_TURMAS}\n");
        exit(-1);
    }

    strcpy(endServer, argv[1]);
    if ((hostPtr = gethostbyname(endServer)) == 0)
        erro("Não consegui obter endereço");

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
    addr.sin_port = htons((short) atoi(argv[2]));

    if ((fd = socket(AF_INET,SOCK_STREAM,0)) == -1) erro("socket");
    if (connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0) erro("Connect");

    while (1) { // Ciclo para o cliente enviar mais do que uma mensagem

        signal(SIGINT, sigint_handler);

        // Pedir um valor na consola
        fgets(buffer_write, BUFF_SIZE, stdin);

        buffer_write[strcspn(buffer_write, "\n")] = 0;

        if ((strcmp(buffer_write, "QUIT_CLIENT") == 0) || strlen(buffer_write) == 0){
            char buffer_final[BUFF_SIZE] = "QUIT_CLIENT ";
            strcat(buffer_final, username);

            write(fd, buffer_final, strlen(buffer_final) + 1);

            close_connections();
            break;
        } 

        // Enviar o valor para o servidor
        write(fd, buffer_write, strlen(buffer_write) + 1);

        //Receber a mensagem que o servidor envia
        nread = read(fd, buffer_read, BUFF_SIZE - 1);
        if (nread > 0) {
            buffer_read[nread] = '\0';
            printf("%s\n", buffer_read);

            if (strncmp(buffer_write, "LOGIN", 5) == 0 && strcmp(buffer_read, "REJECTED") != 0) {

                strcpy(username, buffer_write);
                char *token = strtok(username, " ");
                token = strtok(NULL, " ");
                strcpy(username, token);

            } else if (strncmp(buffer_write, "LIST_SUBSCRIBED", strlen("LIST_SUBSCRIBED")) == 0 && strcmp(buffer_read, "REJECTED") != 0) {
                print_subscribed_classes();            // Listar as turmas inscritas
            } else if (strncmp(buffer_write, "SUBSCRIBE_CLASS", strlen("SUBSCRIBE_CLASS")) == 0 && strcmp(buffer_read, "REJECTED") != 0) {
                join_group(buffer_read, atoi(argv[2]), buffer_write);
            } 

       
        } 

        memset(buffer_write, 0, sizeof(buffer_write));        
        memset(buffer_read, 0, sizeof(buffer_read));  
        fflush(stdout);      
    }  

    close(fd);
    exit(0);
}

void erro(char *msg) {
    printf("Erro: %s\n", msg);
    exit(-1);
}

void join_group(char group[], int port, char name[]){
    
    struct sockaddr_in addr;
    struct ip_mreq mreq;

    // Criar socket Mutlicast dentro do array que contem todos os sockets para multicast
    if ((sockets[num_groups] = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    //reuse port
    int optval = 1;
    setsockopt(sockets[num_groups], SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    //bind to all local interfaces
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(MULTICAST_PORT);

    if (bind(sockets[num_groups], (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    // get the ip of the class
    char *ip_multicast = strtok(group , " ");
    ip_multicast = strtok(NULL, " ");

    // get the name of the class
    char *name_class = strtok(name, " ");
    name_class = strtok(NULL, " ");

    //join multicast group
    mreq.imr_multiaddr.s_addr = inet_addr(ip_multicast);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sockets[num_groups], IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
    }

    //add to mreq struct array
    mreqs[num_groups-0] = mreq;

    //start listening thread
    pthread_t thread;
    pthread_create(&thread, NULL, listen_group, &sockets[num_groups]);

    //add to ips array and classes_name
    strncpy(ips[num_groups], ip_multicast, sizeof(ips[num_groups]));
    strncpy(name_classes[num_groups], name_class, sizeof(name_classes[num_groups]));
    num_groups++;
}

void* listen_group(void* arg) {
    int socket = *(int*)arg;

    while (1) {
        char buf[BUFF_SIZE];
        struct sockaddr_in addr;
        int addrlen = sizeof(addr);
        int n = recvfrom(socket, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &addrlen);

        if (n < 0) {
            perror("recvfrom");
            exit(1);
        }

        buf[n] = '\0';
        printf("RECEIVED FROM CLASS: %s\n", buf);
        fflush(stdout);
    }

    return NULL;
}

void sigint_handler(int sgn){

    printf("SIGINT received. Closing multicast sockets...\n");
    close_connections();
    exit(0);
}

void close_connections(){
    for(int i=0; i<num_groups; i++){
        if (setsockopt(sockets[i], IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreqs[i], sizeof(mreqs[i])) < 0) {
            perror("setsockopt");
            exit(1);
        }
        close(sockets[i]);
    }
}

void print_subscribed_classes(){
    int first = 1;  // Flag para controlar a adição de vírgulas.
    int found = 0;  // Flag para verificar se algum grupo foi adicionado.

    char output_buffer[BUFF_SIZE];

    strcpy(output_buffer, "CLASS ");  // Inicia a string de saída com "CLASS ".
    for (int i = 0; i < MAX_GROUPS; i++) {
        if (name_classes[i][0] != '\0' && ips[i][0] != '\0') {  // Verifica se o nome e o IP não estão vazios.
            char temp_buffer[BUFF_SIZE + 64];  // Buffer temporário para formatar a saída de cada grupo.
            if (!first) {
                strcat(output_buffer, ", ");  // Adiciona vírgula antes dos elementos subsequentes.
            }
            snprintf(temp_buffer, sizeof(temp_buffer), "%s/%s", name_classes[i], ips[i]);  // Formata a informação de cada classe.
            strcat(output_buffer, temp_buffer);  // Concatena ao buffer de saída.
            first = 0;  // Reseta a flag após o primeiro elemento.
            found = 1;  // Seta a flag de encontrado.
        }
    }

    if (!found) {  // Se nenhum grupo válido foi encontrado.
        strcpy(output_buffer, "Os arrays estão vazios.");  // Mensagem indicando que não há dados.
        return;
    } else {
        printf("%s\n", output_buffer);
        return;
    }
}