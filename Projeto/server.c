//server.c

#include "server.h"

struct sockaddr_in addr, client_addr, addr_server, addr_client;
int fd, client, client_addr_size, s,recv_len;
socklen_t slen = sizeof(addr_client);

lista *lista_utilizadores;
class_list *my_class_list;

void erro(char *s){ 
	perror(s);
	exit(1);
}

// =============== Funções auxiliares ===============

//Funcao que cria uma lista com 1 elemento e retorna essa mesma lista
lista cria (){  
    lista aux;
    struct utilizador u1 = {"", "", ""};
    aux = (lista)malloc(sizeof (no_lista));
    if (aux != NULL) {
        aux->u = u1;
        aux->proximo = NULL;
        aux->anterior = NULL;
    }
    return aux;
}

//Função para destruir a lista
void destruir_lista(lista *l) {
    no_lista *atual = *l;
    while (atual != NULL) {
        no_lista *prox = atual->proximo;  // Guardar referência ao próximo nó
        free(atual);  // Liberar o nó atual
        atual = prox;  // Mover para o próximo nó
    }
    *l = NULL;  // Após destruir a lista, definir o ponteiro da lista para NULL
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
        char output[1024];
        sprintf(output,"Username: %s, Role: %s\n", atual->u.username, atual->u.role);
        sendto(s, output, strlen(output), 0, (struct sockaddr *) &addr_client, slen);
        atual = atual->proximo; // Move para o próximo nó da lista
    }
}

// Eliminar utilizadores da lista
int eliminar_utilizador(lista *l, char *buffer) {
    no_lista *atual = *l, *anterior = NULL;
    char *username = buffer + 4;
    username[strcspn(username, "\n")] = '\0';

    while (atual != NULL) {

        if (strcmp(atual->u.username, username) == 0) {
            if (anterior == NULL) {
                // O nó a ser removido é o primeiro nó da lista
                *l = atual->proximo;
                
            } else {
                // O nó a ser removido não é o primeiro
                anterior->proximo = atual->proximo;
            }
            free(atual);
            return 1;  // Retornar 1 indica que o usuário foi removido com sucesso
        }
        anterior = atual;
        atual = atual->proximo;
    }
    return 0;  // Retornar 0 indica que o usuário não foi encontrado
}

// Função para adicionar um usuário no início da lista
void adicionar_utilizador(lista *l, char *buffer) {
    no_lista **p = l;
    while (*p != NULL) {
        p = &((*p)->proximo);
    }

    // Aloca um novo nó
    *p = (no_lista *)malloc(sizeof(no_lista));
    if (*p == NULL) {
        printf("Erro ao alocar memória\n");
        return;
    }

    no_lista *novo = *p;
    novo->proximo = NULL;
    novo->u.username[0] = '\0';  // Inicializa os campos
    novo->u.password[0] = '\0';
    novo->u.role[0] = '\0';

    int aux_dados = 0;
    char *token = strtok(buffer, " ");
    while (token != NULL){
        if (aux_dados == 0){
            token = strtok(NULL, " ");
            aux_dados++;
        }
        else if (aux_dados == 1){
            strncpy(novo->u.username, token, TAM - 1);
            novo->u.username[TAM - 1] = '\0';
            token = strtok(NULL," ");
            aux_dados++;
        } else if (aux_dados == 2){
            strncpy(novo->u.password, token, TAM - 1);
            novo->u.password[TAM - 1] = '\0';
            token = strtok(NULL," ");
            aux_dados++;
        } else if (aux_dados == 3){     
            token[strcspn(token, "\n")] = '\0';
            if ((strcmp(token,"aluno") == 0) || (strcmp(token,"professor") == 0) || strcmp(token,"administrador") == 0) { // caso ao adicionar não seja um dos cargos
                strncpy(novo->u.role, token, TAM - 1);
                novo->u.role[TAM - 1] = '\0';
                aux_dados++;
            } else {
                aux_dados = 0;
            }     
            token = strtok(NULL, " ");
        }  
    }
    if (aux_dados != 4){
        sendto(s, "REJECTED\n", strlen("REJECTED\n"), 0, (struct sockaddr *) &addr_client, slen); 
        free(novo);
        *p = NULL;              
    } else {
        sendto(s, "OK\n", strlen("OK\n"), 0, (struct sockaddr *) &addr_client, slen);  
    }

}

// ================== List de class =======================

// Função para criar class
class_list cria_class (){  
    class_list aux;
    struct utilizador u1 = {"", "", ""};
    aux = (class_list)malloc(sizeof (no_class));
    if (aux != NULL) {
        aux->u.alunos= NULL;
        strcmp(aux->u.ip, "");
        strcmp(aux->u.nome, "");
        aux->u.professor = u1;
        aux->proximo = NULL;
    }
    return aux;
}

// Função para adicionar uma nova aula à lista
void adiciona_aula(class_list *lista, struct class nova_aula) {
    no_class *novo_no = (no_class *) malloc(sizeof(no_class));
    if (novo_no == NULL) {
        perror("Erro ao alocar memória para novo nó");
        exit(1);
    }
    novo_no->u = nova_aula;
    novo_no->proximo = NULL;

    if (*lista == NULL) {
        *lista = novo_no;
    } else {
        no_class *atual = *lista;
        while (atual->proximo != NULL) {
            atual = atual->proximo;
        }
        atual->proximo = novo_no;
    }
}

// Função para ler aulas de um ficheiro
void carrega_aulas_de_ficheiro(const char *nome_ficheiro, class_list *lista) {
    FILE *ficheiro = fopen(nome_ficheiro, "r");
    if (ficheiro == NULL) {
        perror("Erro ao abrir ficheiro");
        exit(1);
    }

    char linha[TAM];
    while (fgets(linha, TAM, ficheiro) != NULL) {
        struct class aula;
        aula.alunos = NULL;

        // Tokeniza a linha para extrair nome, ip, professor e alunos
        char *token = strtok(linha, "/");
        strcpy(aula.nome, token);

        token = strtok(NULL, "/");
        strcpy(aula.ip, token);

        token = strtok(NULL, "/");
        aula.tam_max = atoi(token);

        token = strtok(NULL, "/");
        struct utilizador prof;
        strcpy(prof.username, token);
        aula.professor = prof;

        // Processar os alunos
        token = strtok(NULL, "/");
        while (token != NULL) {
            no_lista *novo_aluno = (no_lista *)malloc(sizeof(no_lista));
            if (novo_aluno == NULL) {
                perror("Erro ao alocar memória para novo aluno");
                exit(1);
            }
            strcpy(novo_aluno->u.username, token);
            if (novo_aluno->u.username[strlen(novo_aluno->u.username) - 1] == '\n'){
                novo_aluno->u.username[strlen(novo_aluno->u.username) - 1] = '\0';
            }
            novo_aluno->proximo = aula.alunos;
            aula.alunos = novo_aluno;
            token = strtok(NULL, "/");
        }

        // Adiciona a nova aula à lista
        adiciona_aula(lista, aula);
    }

    fclose(ficheiro);
}

// Função para imprimir os detalhes de uma aula (DEBUG)
void imprime_aulas(const class_list lista) {
    no_class *atual = lista;
    atual = atual->proximo;
    while (atual != NULL) {
        printf("Nome da Aula: %s\n", atual->u.nome);
        printf("IP da Aula: %s\n", atual->u.ip);
        printf("Professor: %s\n", atual->u.professor.username);
        
        printf("Alunos:\n");
        no_lista *aluno_atual = atual->u.alunos;
        if (aluno_atual == NULL) {
            printf("  [Nenhum aluno registrado]\n");
        }
        while (aluno_atual != NULL) {
            printf("  %s", aluno_atual->u.username);
            aluno_atual = aluno_atual->proximo;
        }
        atual = atual->proximo;
        printf("\n"); // Espaço entre aulas
    }
}

// Função para salvar aulas em um ficheiro
void salva_aulas_em_ficheiro(const char *nome_ficheiro, const class_list lista) {
    FILE *ficheiro = fopen(nome_ficheiro, "w");
    if (ficheiro == NULL) {
        perror("Erro ao abrir ficheiro para escrita");
        exit(1);
    }

    no_class *atual = lista;
    atual = atual ->proximo;
    while (atual != NULL) {
        fprintf(ficheiro, "%s/%s/%d/%s", atual->u.nome, atual->u.ip, atual->u.tam_max ,atual->u.professor.username);

        no_lista *aluno_atual = atual->u.alunos;
        while (aluno_atual != NULL) {
            fprintf(ficheiro, "/%s", aluno_atual->u.username);
            aluno_atual = aluno_atual->proximo;
        }

        fprintf(ficheiro, "\n");  // Nova linha após cada aula
        atual = atual->proximo;
    }

    fclose(ficheiro);
}

// Função para listar apenas o nome das turmas
void lista_nomes_turmas(const class_list lista, int client_fd) {
    no_class *atual = lista;
    char aux[TAM];
    strcat(aux, "CLASS ");
    while (atual != NULL) {
        strcat(aux, atual->u.nome);
        strcat(aux, " ");
        atual = atual->proximo;
    }
    write(client_fd, aux, strlen(aux)); 
}

// Função auxiliar para verificar se um aluno está na lista de alunos de uma turma
int aluno_esta_na_turma(no_lista *lista, char *nome_aluno) {
    while (lista != NULL) {
        if (strcmp(lista->u.username, nome_aluno) == 0) {
            return 1; // Aluno encontrado
        }
        lista = lista->proximo;
    }
    return 0; // Aluno não encontrado
}

// Função para encontrar em quais turmas um aluno está inscrito
void buscar_turmas_do_aluno(const class_list turmas, char *nome_aluno, int client_fd) {
    no_class *atual = turmas;
    char turma[256];
    char aux_[124] ={0};
    strcat(turma, "CLASS ");
    while (atual != NULL) {
        if (aluno_esta_na_turma(atual->u.alunos, nome_aluno)) {
            strcat(aux_, atual->u.nome);
            strcat(aux_,"/");
            strcat(aux_, atual->u.ip);
            strcat(turma, aux_);
            strcat(turma, " ");   
            memset(aux_, 0, sizeof(aux_));
        }
        atual = atual->proximo;
    }
    write(client_fd, turma, strlen(turma));
}

// =============================================


//Função para clientes TCP
void process_client(int client_fd, char *ficheiro){

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
                sem_wait(sem_user);
                int confirmacao_login = confirmar_login(*lista_utilizadores, username, password);
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

                sem_post(sem_user);
            }
        // Login efetuado
        } else if (login == 1 || login == 2){

            if (strncmp (buffer,"LIST_CLASSES", 12) == 0){
                sem_wait(sem_class);
                lista_nomes_turmas(*my_class_list, client_fd);
                sem_post(sem_class);    
            } else if (strncmp(buffer, "LIST_SUBSCRIBED", 15) == 0){
                sem_wait(sem_class);
                buscar_turmas_do_aluno(*my_class_list, username, client_fd);
                sem_post(sem_class);
            } else {
                if (login == 1){ // Caso seja aluno

                    if (strncmp(buffer,"SUBSCRIBE_CLASS", 15) == 0){
                        write(client_fd, "SUBSCRIBE_CLASS entrou", strlen("SUBSCRIBE_CLASS entrou"));
                    } else {
                        write(client_fd, "REJECTED", strlen("REJECTED"));
                    } 
                }
                else if (login == 2){ // Caso seja professor

                    if (strncmp (buffer,"CREAT_CLASS", 11) == 0){
                        write(client_fd,"CREAT_CLASS entrou",strlen("CREAT_CLASS entrou"));
                    } else if (strncmp(buffer, "SEND", 4) == 0){
                        write(client_fd,"SEND entrou", strlen("SEND entrou"));
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
void escrever_lista_para_arquivo(lista l) {
    FILE *arquivo = fopen("ficheiro.txt", "w");  // Abrir o arquivo para escrita
    if (arquivo == NULL) {
        perror("Falha ao abrir arquivo");
        return;
    }

    no_lista *atual = l->proximo;
    while (atual != NULL) {
        fprintf(arquivo, "%s;%s;%s\n", atual->u.username, atual->u.password, atual->u.role);
        atual = atual->proximo;
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

    // Criar shared_memory
    shm_user = shmget(IPC_PRIVATE, sizeof(lista), IPC_CREAT | 0777);
    if (shm_user == -1) {
        perror("Erro ao criar memória compartilhada");
        return 1;
    }
    lista_utilizadores = (lista*) shmat(shm_user, NULL, 0);

    shm_class = shmget(IPC_PRIVATE, sizeof(class_list), IPC_CREAT | 0777);
    if (shm_class == -1) {
        perror("Erro ao criar memória compartilhada");
        return 1;
    }
    my_class_list = (class_list*) shmat(shm_class, NULL, 0);

    // Criar semaforos
    sem_unlink(USER_SEM);
    sem_user = sem_open(USER_SEM, O_CREAT| O_EXCL, 0777, 1);

    sem_unlink(CLASS_SEM);
    sem_class = sem_open(CLASS_SEM, O_CREAT | O_EXCL, 0777, 1);

    // Criar lista ligada das aulas
    *my_class_list = cria_class();
    carrega_aulas_de_ficheiro("aulas.txt", my_class_list);
    //imprime_aulas(*my_class_list);

    //Cria a lista ligada já com um elemento para evitar o caso de ser nulo
    *lista_utilizadores = cria(); 
    ler_ficheiro(*lista_utilizadores, argv[3]); // Ler ficheiro dos alunos

    signal(SIGINT, treat_signal);

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
                    sem_wait(sem_user);
                    int confirmacao_login = confirmar_login(*lista_utilizadores, username, password);
                    if (confirmacao_login == 0){
                        login = 1;
                        sendto(s, "OK\n", strlen("OK\n"), 0, (struct sockaddr *) &addr_client, slen);
                    } else {
                        login = 0;
                        sendto(s, "REJECTED\n", strlen("REJECTED\n"), 0, (struct sockaddr *) &addr_client, slen);
                        memset(username, 0, sizeof(username));
                        memset(password, 0, sizeof(password));
                    }
                    sem_post(sem_user);
                }

                
            // SE JÁ ESTIVER LOGADO
            } else if (login == 1){
                if (strncmp (buf,"ADD_USER", 8) == 0){ // Adcionar utilizadores
                    sem_wait(sem_user);
                    adicionar_utilizador(lista_utilizadores, buf);
                    sem_post(sem_user);

                } else if (strncmp(buf, "DEL", 3) == 0){ // Eliminar utilizadores
                    sem_wait(sem_user);
                    if (eliminar_utilizador(lista_utilizadores, buf) == 0){
                        sendto(s, "NOT FOUND\n", strlen("NOT FOUND\n"), 0, (struct sockaddr *) &addr_client, slen);
                    } else {
                        sendto(s, "OK\n", strlen("OK\n"), 0, (struct sockaddr *) &addr_client, slen);
                    }
                    sem_post(sem_user);

                } else if (strncmp(buf,"LIST", 4) == 0){ // Listar utilizadores

                    sem_wait(sem_user);
                    listar_utilizadores(*lista_utilizadores);
                    sem_post(sem_user);

                //SAIR DO SERVIDOR
                } else if (strncmp(buf,"QUIT_SERVER",11) == 0) { 

                    sem_wait(sem_user);     
                    sem_wait(sem_class);            
                    // Salvar nos ficheiros as listas ligadas;

                    escrever_lista_para_arquivo(*lista_utilizadores);
                    salva_aulas_em_ficheiro("aulas.txt", *my_class_list);

                    sem_post(sem_user);
                    sem_post(sem_class);

                    sendto(s, "CLOSING\n", strlen("CLOSING\n"), 0, (struct sockaddr *) &addr_client, slen);

                    kill(TCP_process, SIGTERM);
                    waitpid(TCP_process, NULL, 0); // Espera o filho terminar

                    // Fechar semaforos
                    sem_close(sem_class);
                    sem_close(sem_user);
                    sem_unlink(USER_SEM);
                    sem_unlink(CLASS_SEM);

                    // Fechar memorias partilhadas
                    shmctl(shm_class, IPC_RMID, NULL);
                    shmctl(shm_user, IPC_RMID, NULL);

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


