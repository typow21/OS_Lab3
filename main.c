#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#define size 100000
#define N_THREADS 4 // how many threads should there be? --> Just 2 for the worker and log thread
#define DEFAULT_DICTIONARY "dictionary.txt"
#define DEFAULT_PORT 8888
#define MAX 10000 //not sure if this is the correct number?

void init();
void parseArgv(int argc, char *argv[]);
void initDict();
void networkConnection();
void getWord();
int check_dictionary(char *word);

void spawn_worker_threads();
void spawn_log_thread();
void *workerThread(void *arg);
void *logThread(void *arg);
void put_connQ(int value);
int get_connQ();
void put_logQ(char* response);
char* get_logQ();

// global variables
char *dict[size];
int portNumber = -1;
char *dictName = NULL; 
int dictLen = 0;

int buffer[MAX]; 
int fill_ptr = 0;
int use_ptr = 0;
int count = 0;

char *logBuff[MAX];
int index_log = 0;
int fill_log = 0;
int use_log = 0;
// Network Variables
int socket_desc, new_socket, c;
struct sockaddr_in server, client;

// Declaring your mutex variables and cond variables
// need four cond variables (2 per queue, one each for empty, the other for full)

// Declaring Mutexes
pthread_mutex_t mutex_connQ;
pthread_mutex_t mutex_logQ; 

// Declaring condition variables
// where/what are the queue data structures??? 
pthread_cond_t fill_logQ;
pthread_cond_t empty_logQ;
pthread_cond_t fill_connQ;
pthread_cond_t empty_connQ;

int main(int argc, char *argv[]){
    init();
    // DONE NEEDS TESTING 
        // Accept command line arguments 
        // determine whether the user is inputting a port number or a dictionary file
    // if there is a new port number or dict then it will be saved in dictName and portNumber
    parseArgv(argc ,argv);  // This works
    if(dictName == NULL){
        dictName = DEFAULT_DICTIONARY;
        printf("Dict name is NULL\n");
    }else{
        printf("New dict: %s\n",dictName);
    }
    if(portNumber == -1){
        portNumber = DEFAULT_PORT;
        printf("Port number is not declared\n");
    }else{
        printf("New Port number: %d\n", portNumber);
    }
    initDict();
            // test for dictionary
            // for(int i = 0; i < 99170; i++){
            //     printf("%s\n", dict[i]);
            // }
    // initVariables(); // init variables here?
    spawn_worker_threads();
    spawn_log_thread();
    networkConnection(); //network init
    // c = sizeof(struct sockaddr_in);
    // listen(socket_desc, 3);
    // main loop 
    while(1){
        // get connection file descriptor
        puts("Waiting for incoming connections...");
        printf("Socket: %d\n", socket_desc);
        int fd = accept(socket_desc, (struct sockaddr *) &client, (socklen_t *) &c);
        if(fd < 0){
            perror("Error:");
            continue;
        }
        puts("Connection accepted");
        put_connQ(fd); // are there multiple queues?
    }
    
    // getWord(); // gets word from user that then checks dictionary

}

// WORKS
//init mutexes and cond variables
void init(){
    // Initializing condition variables / mutex
    // init mutex
    pthread_mutex_init(&mutex_logQ, NULL);
    pthread_mutex_init(&mutex_connQ, NULL);
    // init condition variable
    pthread_cond_init(&fill_connQ, NULL);
    pthread_cond_init(&fill_logQ, NULL);
    pthread_cond_init(&empty_connQ, NULL);
    pthread_cond_init(&empty_logQ, NULL);
}

// WORKS
// Parses the command line arguments
void parseArgv(int argc, char *argv[]){
    char *argument = NULL;
    for(int i = 1; i < argc; ++i){
        argument = argv[i]; 
        if(isalpha(argument[0])){
            dictName = argument; 
        }else{
            sscanf(argument, "%d", &portNumber);
        }
    }
}

// WORKS
// Loads the dictionary into the dict data structure
void initDict(){
    size_t lineSize = 0;
    FILE *file =fopen(dictName, "r");
    // char *dictTemp[size] = dict;
    char *word = NULL;
    int count = 0;
    while(getline(&word, &lineSize ,file) > 0 ){
        //removes the new line character
        char *lastChar = word + strlen(word) - 1;
        *lastChar = '\0';
        dict[count] = word;
        // printf("%s",dict[count]);
        count++;
        word = NULL;
        lineSize = 0;
    }
    dict[count+1] = "\0";
    while(dict[dictLen] != NULL){
        dictLen++;
    }
    printf("InitDict: Dict len: %d\n", dictLen);
}

// WORKS
// Returns a 1 if the word is in the dictionary
// Returns a 0 if the word is not in the dictionary
int check_dictionary(char *word){
    int indicator = 0;
    for(int i = 0; i < dictLen; i++){
        if(strcmp(dict[i], word) == 0){
            indicator = 1;
            // printf("Word found"); // test
            return indicator;
        }
    }
    return indicator;
}

// thread spool example
void spawn_worker_threads(){
    //array which holds worker threads
    pthread_t threads[N_THREADS];
    for(size_t i = 0; i < N_THREADS; ++i){
        if(pthread_create(&threads[i],
                            NULL,
                            workerThread,
                            NULL) != 0){
            printf("Error: Failed to create thread\n");
            exit(1);
        }
    }
}

void spawn_log_thread(){
    pthread_t threads[1];
    for(size_t i = 0; i < 1; ++i){
        if(pthread_create(&threads[i],
                            NULL,
                            logThread,
                            NULL) != 0){
            printf("Error: Failed to create thread\n");
            exit(1);
        }
    }
}
// TO_DO
// for worker thread and log thread make sure to fully error check!!

// Worker Thread
void *workerThread(void *arg){
    // THE CODE BELOW IS FROM SLIDE 10 WEEK 10 BUT IS ONLY PSEUDOCODE
    while(1){
        char* word = calloc(MAX, 1);
        int socket = get_connQ();// that function has not been yet
        while(read(socket, word, MAX) > 0){
            word[strlen(word)- 1] = '\0'; // this won't get it all
            int wasFound = check_dictionary(word);
                char *response;
                if(wasFound){
                    // strcat(word , ": OK\n");
                    // word = realloc(word, sizeof(char*)*25);
                    // strcat(word, ": OK\n");
                    response = ": OK\n";
                }else{
                    // strcat(word , ": MISPELLED\n");
                    // word = realloc(word, sizeof(char*)*25);
                    // strcat(word, ": MISPELLED\n");
                    // printf("Test%cTest", response[strlen(response)]);
                    response = ": MISPELLED\n";
                }
                put_logQ(response);
                send(socket, response, strlen(response), 0);
                // printf("REached");
                // printf("%s", response);
        }
        close(socket);
    }
}

// // Logger thread
void *logThread(void *arg){
    // THE CODE BELOW IS FROM SLIDE 11 WEEK 10 BUT IS ONLY PSEUDOCODE
    // create log file here
    printf("REACHED\n");
    FILE *logFile = fopen("LOG.txt", "w");
    if(logFile == NULL){
        printf("Error opening log file");
        exit(0);
    }
    // int fd_log =fileno(logFile);
    // if (fputs(response, logFile) >= 0)
    while(1){
        char *response = get_logQ();
        // remove string form buffer 
        if(fputs(response, logFile) > -1){
            fflush(logFile);
            printf("Successful write to file\n");
        }else{
            printf("Something went wrong writing to log file\n");
        }
        // write(fd_log, response, 0);
        // TO_DO: possibly add fflush here
        free(response); // make sure you free if you malloc
    }
}

// Works I think?
void networkConnection(){
    char *message;
    // Create socket (create active socket descriptor)
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1){ 
	puts("Error creating socket!");
	exit(1);
    }
    // prepare sockaddr_instructure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; // defaults to 127.0.0.1
    server.sin_port = htons(portNumber);
    // Bind (corrent the server's socket address to the socket descriptor)
    int bind_result = bind(socket_desc, (struct sockaddr*)&server, sizeof(server));
    if (bind_result < 0){
	puts("Error: failed to Bind.");
	exit(1);
    }
    puts("Bind done.");
    // Listen (converts active socket to a LISTENING socket which can accept connections)
    c = sizeof(struct sockaddr_in);
    if(listen(socket_desc, 3) != 0){
        printf("Fuck something went wrong");
    }
	// do something with new_socket
    //THIS CODE IS AT 12:37 OF MATT'S 3/25 LECTURE I THINK
    // char buffer[256];
    // memset(buffer, 0, 256);
    // printf("%s\n", buffer);
    // buffer[strlen(buffer) - 1] = '\0';
    // close(new_socket);
    
}

//CIRCULAR BUFFER
// (maybe chance variable names?) got from slide 15 and 16 on lab slides week 9
// Buffer variables
// add to the buffer 
void put_connQ(int value){
    printf("Put onto queue: Reached: %d\n", MAX);
    pthread_mutex_lock(&mutex_connQ);
    while(count == MAX){
        pthread_cond_wait(&empty_connQ, &mutex_connQ);
    }
    buffer[fill_ptr] = value;
    fill_ptr = (fill_ptr + 1) % MAX;
    count++;
    // printf("Buffer: %d\n", value);
    pthread_cond_signal(&fill_connQ);
    pthread_mutex_unlock(&mutex_connQ);
}

// get from the buffer (taken from chapter 30 of 3 easy pieces)
int get_connQ(){
    pthread_mutex_lock(&mutex_connQ);
    while(count == 0){
        pthread_cond_wait(&fill_connQ, &mutex_connQ);
    }
    int tmp = buffer[use_ptr];
    use_ptr = (use_ptr + 1) % MAX;
    count--;
    pthread_cond_signal(&empty_connQ);
    pthread_mutex_unlock(&mutex_connQ);
    return tmp;
}


void put_logQ(char* response){
    pthread_mutex_lock(&mutex_logQ);
    if(index_log == MAX){
        pthread_cond_wait(&fill_logQ, &mutex_logQ);
    }
    logBuff[index_log] = response;
    index_log++;
    pthread_cond_signal(&fill_logQ);
    pthread_mutex_unlock(&mutex_logQ);
}

char* get_logQ(){
    pthread_mutex_lock(&mutex_logQ);
    while(index_log == 0){
        pthread_cond_wait(&fill_logQ, &mutex_logQ);
    }
    printf("Response: %s", logBuff[index_log] );
    char* tmp = logBuff[index_log];
    index_log--;
    printf("TMP %s", tmp);
    pthread_cond_signal(&empty_logQ);
    pthread_mutex_unlock(&mutex_logQ);
    return tmp;
}