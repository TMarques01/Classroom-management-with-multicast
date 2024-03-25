/**********************************************************************
 * CLIENTE liga ao servidor (definido em argv[1]) no porto especificado
 * (em argv[2]), escrevendo a palavra predefinida (em argv[3]).
 * USO: >cliente <enderecoServidor>  <porto>  <Palavra>
 **********************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define BUFF_SIZE 1024

void erro(char *msg);

int main(int argc, char *argv[]) {
  char endServer[100];
  char buffer[BUFF_SIZE];
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

  // Ler a mensagem inicial
  nread = read(fd, buffer, BUFF_SIZE-1);
  if (nread > 0){
    buffer[nread] = '\0';
    printf("%s\n", buffer);
  }
  
  while (1) { // Ciclo para o cliente enviar mais do que uma mensagem

    // Pedir um valor na consola
    fgets(buffer, BUFF_SIZE, stdin);

    buffer[strcspn(buffer, "\n")] = 0;

    // Enviar o valor para o servidor
    write(fd, buffer, strlen(buffer) + 1);

    //Receber a mensagem que o servidor envia
    nread = read(fd, buffer, BUFF_SIZE - 1);
    if (nread > 0) {
      buffer[nread] = '\0';
      printf("%s\n", buffer);
    }
    
    // Caso a mensagem recebida, seja a mensagem de despedida, o cliente fecha
    if (strcmp(buffer, "Ate logo!") == 0) {
      break;
    }
  }  
  close(fd);
  exit(0);
}


void erro(char *msg) {
  printf("Erro: %s\n", msg);
	exit(-1);
}
