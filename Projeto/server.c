//server.c

#include "server.h"

// TO DO turams terem um numero maximo (nao esta a funcionar)

struct sockaddr_in addr, client_addr, addr_server, addr_client;
int fd, client, client_addr_size, s,recv_len, classes = 1;
socklen_t slen = sizeof(addr_client);

void erro(char *s){ 
	perror(s);
	exit(1);
}

// Retornar ip de uma turma
char* get_ip_by_class_name(const char* class_name) {
    sem_wait(sem_class);

    for (int i = 0; i < MAX_CLASSES; i++) {
        if (strcmp(shared_m->classes[i].nome, "") != 0){
            if (strcmp(shared_m->classes[i].nome, class_name) == 0) {
                sem_post(sem_class);
                return shared_m->classes[i].ip;
            }
        }

    }

    sem_post(sem_class);
    return NULL;  // retorna NULL se a classe não for encontrada
}

// Incrementar tamanho da turma
int increment_class_size(const char *class_name) {
    sem_wait(sem_class);
    for (int i = 0; i < MAX_CLASSES; i++) {
        if (strcmp(shared_m->classes[i].nome, "") != 0){
            if (strcmp(shared_m->classes[i].nome, class_name) == 0) {  // Compara o nome da classe com class_name.
                if (shared_m->classes[i].tam_current < shared_m->classes[i].tam_max) {  // Verifica se há espaço disponível.
                    shared_m->classes[i].tam_current++;  // Incrementa tam_current.
                    sem_post(sem_class);
                    return 1;
                } else {
                    sem_post(sem_class);
                    return 0;
                }
            }
        }
    }
    
    sem_post(sem_class);
    return 0;
}

// Colocar um determinado user offline
int modify_online_status(const char *nome){
    for (int i = 0; i < MAX_USERS; i++){
        if (strcmp(shared_m->users[i].username, "") != 0){
            if (strcmp(shared_m->users[i].username, nome) == 0){
                shared_m->users[i].online_status = 0;
                return 1;
            }
        }
    }
    return 0;
}
// =============== Lista de Users ===============

// Função auxiliar para verificar se um user já existe na lista
int verificar_user_existe(const char *username) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(shared_m->users[i].username, username) == 0) {
            return 1; // se user ja existe
        }
    }
    return 0;  //se user ainda não exite
}

//Função para ler ficheiro e adicionar ao array
void ler_ficheiro(char *ficheiro) {

    FILE *file;
    file = fopen(ficheiro, "r");

    char *token;
    char line[BUFLEN];


    while (fgets(line, BUFLEN, file)) {
        user u;
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
        insere_utilizador(u);
    }
    fclose(file);
}

//Função para inserir utilizador no array
void insere_utilizador(user u) {

    sem_wait(sem_user);
    for (int i = 0; i < MAX_USERS; i++){
        if (strcmp(shared_m->users[i].username, "") == 0){
            strcpy(shared_m->users[i].username, u.username);
            strcpy(shared_m->users[i].password, u.password);
            strcpy(shared_m->users[i].role, u.role);
            shared_m->users[i].online_status = 0;
            break;
        } else {
            continue;
        }
    }
    sem_post(sem_user);
}

//Funcao para confirmar o login do administrador (RTRN = 0 -> admin; 2 -> aluno; 3 -> professor)
int confirmar_login(char username_login[TAM], char password_login[TAM]){

    int count = 1;
    sem_wait(sem_user);
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(shared_m->users[i].username, "") != 0){
            if (shared_m->users[i].online_status != 1){
                if (strcmp(shared_m->users[i].username, username_login) == 0 && strcmp(shared_m->users[i].password, password_login) == 0){
                    //Se for administrador
                    if (strcmp(shared_m->users[i].role, "administrador") == 0){
                        count = 0;
                    }
                    //Se tiver outro cargo
                    else if (strcmp(shared_m->users[i].role,"aluno") == 0){
                        count = 2;
                    } else if (strcmp(shared_m->users[i].role,"professor") == 0){
                        count = 3;
                    }
                    shared_m->users[i].online_status = 1;
                    break;
                }
            }
        }
    }
    sem_post(sem_user);
    return count;
}

//Função para debug (listar todos os utlizadores do ficheiro)
void listar_utilizadores() {

    for (int i = 0; i < MAX_USERS; i++) {
        char output[1024];
        if (strcmp(shared_m->users[i].username, "") != 0){
        sprintf(output,"Username: %s, Role: %s\n", shared_m->users[i].username, shared_m->users[i].role);
        printf("%s", output);
        sendto(s, output, strlen(output), 0, (struct sockaddr *) &addr_client, slen);
        }
    }
}

// Eliminar utilizadores do array
int eliminar_utilizador(char *buffer) {
    char *username = buffer + 4;
    username[strcspn(username, "\n")] = '\0';

    sem_wait(sem_user);
    for (int i = 0; i < MAX_USERS; i++){

        if (strcmp(shared_m->users[i].username, username) == 0) {

            strcpy(shared_m->users[i].username, "");
            strcpy(shared_m->users[i].password, "");
            strcpy(shared_m->users[i].role, "");
            sem_post(sem_user);
            return 1;  // Retornar 1 indica que o usuário foi removido com sucesso
        }
    }

    sem_post(sem_user);
    return 0;  // Retornar 0 indica que o usuário não foi encontrado
}

// Função para adicionar um usuário ao array
void adicionar_utilizador(char *buffer) {

    user novo; 
    
    novo.username[0] = '\0';  // Inicializa os campos
    novo.password[0] = '\0';
    novo.role[0] = '\0';

    int aux_dados = 0;
    char *token = strtok(buffer, " ");
    while (token != NULL){ // dividir os dados e guardar dentro da struct
        if (aux_dados == 0){
            token = strtok(NULL, " ");
            aux_dados++;
        }
        else if (aux_dados == 1){
            if (verificar_user_existe(token)){
                sendto(s, "USER ALREADY IN THE LIST\n", strlen("USER ALREADY IN THE LIST\n"), 0, (struct sockaddr *) &addr_client, slen); 
                return;
            }
            strncpy(novo.username, token, TAM - 1);
            novo.username[TAM - 1] = '\0';
            token = strtok(NULL," ");
            aux_dados++;
        } else if (aux_dados == 2){
            strncpy(novo.password, token, TAM - 1);
            novo.password[TAM - 1] = '\0';
            token = strtok(NULL," ");
            aux_dados++;
        } else if (aux_dados == 3){     
            token[strcspn(token, "\n")] = '\0';
            if ((strcmp(token,"aluno") == 0) || (strcmp(token,"professor") == 0) || strcmp(token,"administrador") == 0) { // caso ao adicionar não seja um dos cargos
                strncpy(novo.role, token, TAM - 1);
                novo.role[TAM - 1] = '\0';
                aux_dados++;
            } else {
                aux_dados = 0;
            }     
            token = strtok(NULL, " ");
        }  
    }
    if (aux_dados != 4){ // caso não tenha todos os parametros
        sendto(s, "REJECTED\n", strlen("REJECTED\n"), 0, (struct sockaddr *) &addr_client, slen); 
    } else {

        int flag = 0;
        sem_wait(sem_user);
        for (int i = 0; i < MAX_USERS; i++){ // adicionar à lista de users
            if (strcmp(shared_m->users[i].username, "") == 0){
                strcpy(shared_m->users[i].username, novo.username);
                strcpy(shared_m->users[i].password, novo.password);
                strcpy(shared_m->users[i].role, novo.role);
                shared_m->users[i].online_status = 0;
                flag = 1; // certificar que há espaço na lista de users
                break;
            } else {
                continue;
            }
        }
        sem_post(sem_user);

        if (flag == 1){
            sendto(s, "OK\n", strlen("OK\n"), 0, (struct sockaddr *) &addr_client, slen);  
        } else {
            sendto(s, "USERS LIST FULL\n", strlen("USERS LIST FULL\n"), 0, (struct sockaddr *) &addr_client, slen);  
        }
    }
}

// ================== Lista de Class =======================

// Função para imprimir os detalhes de uma aula (DEBUG)
void imprime_aulas() {
    sem_wait(sem_class);
    for (int i = 0; i < MAX_CLASSES; i++) {
        if (strcmp(shared_m->classes[i].nome, "") != 0){
            printf("Nome da Aula: %s\n", shared_m->classes[i].nome);
            printf("IP da Aula: %s\n", shared_m->classes[i].ip);
        
            printf("\n"); // Espaço entre aulas
        }
    }
    sem_post(sem_class);
}

// Função para listar apenas o nome das turmas
void lista_nomes_turmas(int client_fd) {
    char aux[TAM] = "CLASS ";

    sem_wait(sem_class);
    for (int i = 0; i < MAX_CLASSES; i++){
        if (strcmp(shared_m->classes[i].nome, "") != 0){
            strcat(aux, shared_m->classes[i].nome);
            strcat(aux, " ");
        }
    }
    sem_post(sem_class);
    write(client_fd, aux, strlen(aux)); 
}

// Função para subscrever uma turma
void subscribe_class(int client_fd, char *buffer){
    char *nome_turma = strtok(buffer, " ");
    nome_turma = strtok(NULL, " ");
    if (nome_turma == NULL){
        write(client_fd, "REJECTED", strlen("REJECTED"));
    } else {

        char *ip = get_ip_by_class_name(nome_turma);

        if (ip == NULL){
            write(client_fd, "CLASS DOES NOT EXIST", strlen("CLASS DOES NOT EXIST"));
        } else {
            if (increment_class_size(nome_turma) == 1){
                char aux[124];
                sprintf(aux, "ACCEPTED %s", ip);
                write(client_fd, aux, strlen(aux));
            } else {
                write(client_fd, "REJECTED", strlen("REJECTED")); 
            }
        }
    }
}

// Função para enviar mensagem para as respectivas turmas
void enviar_mensagem_turma(int client_fd, struct sockaddr_in multicast_addr , char *buffer, int multicast_socket){
    char *token = strtok(buffer, " ");
    token = strtok(NULL, " "); // getting the name of the class

    if (token == NULL){
        write(client_fd, "REJECTED", strlen("REJECTED"));
    } else {

        char *ip = get_ip_by_class_name(token);
        if (ip != NULL){

            //multicast_addr.sin_addr = inet_aton(ip);
            if (inet_aton(ip, &multicast_addr.sin_addr)){
                token = strtok(NULL, " ");

                if (sendto(multicast_socket, token, strlen(token), 0, (struct sockaddr*)&multicast_addr, sizeof(multicast_addr)) < 0)
                    perror("Error sending news to multicast group");
                
                write(client_fd, "MENSAGE SENT", strlen("MENSAGE SENT"));
            } else {

                write(client_fd, "INVALID IP", strlen("INVALID IP"));
            }
        } else {
            write(client_fd, "REJECTED", strlen("REJECTED"));
        }
    }
}

// Função para criar uma turma
void creat_class(int client_fd, char *buffer){
    char *token = strtok(buffer, " ");
    token = strtok(NULL, " ");
    
    sem_wait(sem_class);

    if (token != NULL){
        for (int i = 0; i < MAX_CLASSES; i++){
            if (strcmp(shared_m->classes[i].nome, "") == 0){

                strcpy(shared_m->classes[i].nome, token);

                token = strtok(NULL, " ");

                if (token == NULL){
                    strcpy(shared_m->classes[i].nome, "");
                    write(client_fd, "REJECTED", strlen("REJECTED"));
                    break;
                }

                shared_m->classes[i].tam_max = atoi(token); // atribui valor máximo à turma

                unsigned long ip_num = htonl(0xEF000000 + classes);
                struct in_addr ip_addr;
                ip_addr.s_addr = ip_num;
                
                char* ip_str = inet_ntoa(ip_addr);
                strcpy(shared_m->classes[i].ip, ip_str);
                shared_m->classes[i].tam_current = 0;

                char write_buffer[64 + TAM];
                sprintf(write_buffer, "OK %s", shared_m->classes[i].ip);

                write(client_fd, write_buffer, strlen(write_buffer));
                classes++;
                break;
            }
        }
    } else {
        write(client_fd, "REJECTED", strlen("REJECTED"));
    }

    sem_post(sem_class);
}

// =============================================

//Função para clientes TCP
void process_client(int client_fd, char *ficheiro){

    int multicast_socket; //multicast socket
    struct sockaddr_in multicast_addr;
    memset(&multicast_addr, 0, sizeof(multicast_addr));

    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_port = htons(MULTICAST_PORT);

    if ((multicast_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        perror("Error creating socket for multicast group");

    int ttl_value = 128; //increase packet life time
    if (setsockopt(multicast_socket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl_value, sizeof(ttl_value)) < 0)
        perror("Error enabling multicast on socket");

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

        printf("Client: %s\n", buffer);

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
                int confirmacao_login = confirmar_login(username, password);
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

            if (strncmp (buffer,"LIST_CLASSES", 12) == 0) { 
                lista_nomes_turmas(client_fd);
            } else if (strncmp(buffer, "LIST_SUBSCRIBED", 15) == 0) { 
                write(client_fd, "OK", strlen("OK"));
            } else if (strncmp(buffer,"SUBSCRIBE_CLASS", 15) == 0) {
                subscribe_class(client_fd, buffer);
            } else if (strncmp(buffer, "QUIT_CLIENT", 11) == 0) {
                char *token = strtok(buffer, " ");
                token = strtok(NULL, " ");
                if (!modify_online_status(token)) printf("ERROR IN MODIFY ONLINE STATUS\n");
            } else {
                if (login == 1){ // Caso seja aluno
                    write(client_fd, "REJECTED", strlen("REJECTED"));
                }
                else if (login == 2){ // Caso seja professor

                    if (strncmp (buffer,"CREATE_CLASS", 12) == 0){
                        creat_class(client_fd, buffer);
                    } else if (strncmp(buffer, "SEND", 4) == 0){
                        enviar_mensagem_turma(client_fd, multicast_addr, buffer, multicast_socket);
                    } else {
                        write(client_fd, "REJECTED", strlen("REJECTED"));
                    }       
                } 
            }
        }
    }

    printf("A fechar cliente...\n");
}

//Funcao para lidar com o sinal (TCP)
void treat_signal(int sig){

    close(fd);
    while (wait(NULL)> 0);
    printf("A fechar servidor TCP\n");
    exit(0);
}

// Função para escrever a lista para um arquivo
void escrever_para_arquivo() {
    FILE *arquivo = fopen("ficheiro.txt", "w");  // Abrir o arquivo para escrita
    if (arquivo == NULL) {
        perror("Falha ao abrir arquivo");
        return;
    }

    for (int i = 0; i < MAX_USERS; i++){
        if (strcmp(shared_m->users[i].username, "") != 0){
            fprintf(arquivo, "%s;%s;%s\n", shared_m->users[i].username, shared_m->users[i].password, shared_m->users[i].role);
        } 
    }

    fclose(arquivo);
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
    
    int shm_size = sizeof(shm) + (sizeof(user) * MAX_USERS) + (sizeof(class) * MAX_CLASSES * MAX_USERS_P_CLASS);
    shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | 0777);
    if (shm_id == -1){
        perror("ERROR CREATING SHARED MEMORY");
        return 1;
    } 

    if ((shared_m = (shm *)shmat(shm_id, NULL, 0)) == (shm *)-1){
		printf("Erro shmat\n");
		exit(0);
	}

    shared_m->users = (user *)(shared_m + 1);
    shared_m->classes = (class *)(shared_m->users + MAX_USERS);

    for (int i = 0; i < MAX_USERS; i++){
        shared_m->users[i] = (user){.username = "",
                                    .password = "",
                                    .role = "",
                                    .online_status = 0};
    }

    for (int i = 0; i < MAX_CLASSES; i++){
        shared_m->classes[i] = (class){ .nome = "",
                                        .ip = "",
                                        .tam_max = -999,
                                        .tam_current = -999};
    }
    

    // Criar semaforos
    sem_unlink(USER_SEM);
    sem_user = sem_open(USER_SEM, O_CREAT| O_EXCL, 0777, 1);

    sem_unlink(CLASS_SEM);
    sem_class = sem_open(CLASS_SEM, O_CREAT | O_EXCL, 0777, 1);

    ler_ficheiro(argv[3]); // Ler ficheiro dos alunos
    listar_utilizadores();

    signal(SIGINT, treat_signal);

    //SERVIDOR TCP
    pid_t TCP_process = fork();
    if (TCP_process == 0){

        signal(SIGTSTP, SIG_IGN);
        
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
                    process_client(client, argv[3]);
                    close(client);
                    exit(0);
                }
            }
        }
        
        close(fd);

    } else { //SERVIDOR UDP
        
        char buf[BUFLEN];

        // Cria um socket para recepção de pacotes UDP
        if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
            erro("Erro na criação do socket");
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

            signal(SIGTSTP, SIG_IGN);

            //Mensagem recebida
            if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&addr_client, &slen)) == -1) {
                erro("Erro no recvfrom"); 
            }

            // Limpa o buffer buf para receber mais mensagens
            buf[recv_len] = '\0';

            if (strcmp(buf, "X") != 0) printf("Administrador: %s", buf);

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
                            if (strcmp(buf, "X") != 0){
                                sendto(s, "LOGIN {username} {password}\n", strlen("LOGIN {username} {password}\n"), 0, (struct sockaddr *) &addr_client, slen);
                            }
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
                    int confirmacao_login = confirmar_login(username, password);
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
                // Adicionar utilizadores
                if (strncmp (buf,"ADD_USER", 8) == 0){ 
                    adicionar_utilizador(buf);

                } else if (strncmp(buf, "DEL", 3) == 0){ 
                    // Eliminar utilizadores
                    if (eliminar_utilizador(buf) == 0){
                        sendto(s, "NOT FOUND\n", strlen("NOT FOUND\n"), 0, (struct sockaddr *) &addr_client, slen);
                    } else {
                        sendto(s, "OK\n", strlen("OK\n"), 0, (struct sockaddr *) &addr_client, slen);
                    }
                // Listar utilizadores
                } else if (strncmp(buf,"LIST", 4) == 0){ listar_utilizadores();

                //SAIR DO SERVIDOR
                } else if (strncmp(buf,"QUIT_SERVER",11) == 0) { 
          
                    // Guardar nos ficheiros as listas ligadas;
                    escrever_lista_para_arquivo();

                    sendto(s, "CLOSING\n", strlen("CLOSING\n"), 0, (struct sockaddr *) &addr_client, slen);

                    kill(TCP_process, SIGTERM);
                    waitpid(TCP_process, NULL, 0); // Espera o filho terminar (TCP Process)

                    // Fechar semaforos
                    sem_close(sem_class);
                    sem_close(sem_user);
                    sem_unlink(USER_SEM);
                    sem_unlink(CLASS_SEM);

                    // Fechar memoria partilhada
                    if(shmdt(shared_m)== -1) printf ("ERROR IN shmdt");
	                if(shmctl(shm_id, IPC_RMID, NULL) == -1) printf("ERROR IN shmctl");  
                    break;

                } else {
                    sendto(s, "REJECTED\n", strlen("REJECTED\n"), 0, (struct sockaddr *) &addr_client, slen);
                }   
            }
        }

        //Fechar socket UDP
        printf("A fechar servidor UDP\n");
        close (s);  
    }

    return 0;
}