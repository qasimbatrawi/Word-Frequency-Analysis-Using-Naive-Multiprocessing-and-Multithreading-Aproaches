#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<math.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define SHM_KEY 12345 // shared memory key
#define LOCK_KEY 54321 // shared lock key
#define SIZE 1000000 // size of (words) array
#define NUM_OF_PROCESSES 2 // number of processes we want to create

typedef struct Words Words ; // sturct to store the word and its frequency
typedef struct AllWords AllWords ; // struct to store all words from the file

struct Words{

    char word[30] ;
    int frequency ;

};

struct AllWords{

    char word[30] ;

};

Words *createSHM(int *shmID , int size){

    // shared memory creation

    size_t shmSize = size*sizeof(Words);

    int ID = shmget(SHM_KEY, shmSize, IPC_CREAT | 0666) ;

    if (ID == -1){
        printf("An error happened in creating shared memory.\n") ;
        exit(0) ;
    }

    Words *words = (Words *)shmat(ID , NULL , 0) ;
    if (words == (void *)-1) {
        printf("An error happened in creating shared memory.\n") ;
        exit(0) ;
    }

    if (*shmID != NULL){
        *shmID = ID ;
    }

   for (int i = 0 ; i < size ; i++){
        strcpy(words[i].word , "EmptyCell") ;
        words[i].frequency = 0 ;
    }

    return words ;

}

Words *createAllWords(int size){

    AllWords *allWords = NULL ;

    allWords = (AllWords*)malloc(size * sizeof(AllWords));

    if (allWords == NULL){
        printf("Out of space.\n") ;
        exit(0) ;
    }

    return allWords ;

}

Words *createWords(int size){

    Words *wordArray = NULL ;

    wordArray = (Words*)malloc(size * sizeof(Words));

    if (wordArray == NULL){
        printf("Out of space.\n") ;
        exit(0) ;
    }

    for (int i = 0 ; i < size ; i++){
        strcpy(wordArray[i].word , "EmptyCell") ;
        wordArray[i].frequency = 0 ;
    }

    return wordArray ;

}

int *createLock() {

    // shared lock creation

    int lockID = shmget(LOCK_KEY, sizeof(int), IPC_CREAT | 0666) ;

    if (lockID == -1) {
        printf("An error happened in creating shared memory for lock.\n") ;
        exit(0) ;
    }

    int *lock = (int *)shmat(lockID, NULL, 0) ;

    if (lock == (void *)-1) {
        printf("An error happened in attaching shared memory for lock.\n") ;
        exit(0) ;
    }

    *lock = 0 ;

    return lock ;
}

void toLower(char *str){

    // convert upper case to lower

    for (int i = 0 ; str[i] != '\0' ; i++){
        if (str[i] >= 'A' && str[i] <= 'Z'){
            str[i]+=('a' - 'A') ;
        }
    }

}

void fetchWordsFromFile(AllWords *allWords , int *size){

    // fetch words from file and store them in (allWords) array

    FILE *fp = fopen("dataset.txt" , "r") ;

    if (fp == NULL){
        printf("The file does not exist\n") ;
        return ;
    }

    char str[100] ;
    while (fscanf(fp , "%99s" , str) != EOF){
        toLower(str) ;

        if (str[strlen(str)-1] == '\n'){
            str[strlen(str)-1] = '\0' ;
        }

        strcpy(allWords[*size].word , str) ;
        (*size)++ ;
    }

    fclose(fp) ;
}

void acquireLock(int *lock) {

    // acquire the lock if its availabe, and wait if it is not

    while (__sync_lock_test_and_set(lock, 1)) ; // busy wait

}

void releaseLock(int *lock) {

    __sync_lock_release(lock) ; // release the lock

}

void MultiProcessing(Words *words , AllWords *allWords , int index , int size , int *lock) {

     long long start , end ; // start and end index to read from (allWords) array

     start = index * (size / NUM_OF_PROCESSES) ; // calculate the start index
     end = start + (size / NUM_OF_PROCESSES) ; // calculate the end index

     if (index == NUM_OF_PROCESSES-1){
        end = size ;
     }

    char str[100] ;

    for (int i = start ; i < end ; i++){

        strcpy(str , allWords[i].word) ;

        int found = 0 , j ;

        for (j = 0 ; strcmp(words[j].word , "EmptyCell") != 0 ; j++){

            // check if the words exist in (words) array
            if (strcmp(words[j].word , str) == 0){

                // entery section
                acquireLock(lock) ;

                // critical section
                words[j].frequency++ ;
                found=1 ;

                // exit section
                releaseLock(lock) ;

                break ;
            }
        }

        if (!found){
            // entery section
            acquireLock(lock) ;

            // critical section
            strcpy(words[j].word , str) ;
            words[j].frequency=1 ;

            // exit section
            releaseLock(lock) ;
        }

    }

}

void FindTopTen(Words *TopTen , Words *words){

    // find top 10 most frequent words

    for (int i = 0 ; i < 10 ; i++){
        int mx=0 , ind=-1 ;
        for (int j = 0 ; strcmp(words[j].word , "EmptyCell") != 0 ; j++){
            if (words[j].frequency  > mx){
                mx = words[j].frequency ;
                ind = j ;
            }
        }
        if (ind != -1){
            strcpy(TopTen[i].word , words[ind].word) ;
            TopTen[i].frequency = mx ;
            words[ind].frequency = -1 ;
        }

    }

}

void freeSHM(Words *words , int shmID) { // function to free the shared memory

    if (shmdt(words) == -1) {
        printf("An error happened in free memory.\n") ;
    }

    if (shmctl(shmID, IPC_RMID, NULL) == -1) {
        printf("An error happened in free memory.\n") ;
    }

}

int main() {

    struct timespec start, end ;
    long time_diff_ms ;

    clock_gettime(CLOCK_MONOTONIC, &start) ;

    AllWords *allWords = createAllWords(20000000) ; // struct array to store all words from file
    int size=0 ;

    fetchWordsFromFile(allWords , &size) ; // store the words from file in (allWords) array

    int *lock = createLock() ;

    int shmID ; // shared memory id
    Words *words = createSHM(&shmID , SIZE) ; // create the shared memory

    pid_t id = 1 ; // the process id

    // create children
    for (int i = 0 ; i < NUM_OF_PROCESSES ; i++){
        if (id != 0){
            id = fork() ;
        }

        if (id == -1){
            printf("An error happened in creating child process.\n") ;
            exit(0) ;
        }

        if (id == 0){
            MultiProcessing(words , allWords , i , size , lock) ;
            exit(0) ;
        }
    }

    // parent will wait for their children to terminates
    for (int i = 0 ; i < NUM_OF_PROCESSES ; i++){
        wait(NULL) ;
    }

    Words *TopTen = createWords(10) ; // struct array to store top 10 most frequent words

    FindTopTen(TopTen , words) ; // find top 10 most frequent words

    clock_gettime(CLOCK_MONOTONIC, &end) ;

    time_diff_ms = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1000000000 ;
    printf("Time taken in Multi approach: %ld sec.\n" , time_diff_ms) ;

    printf("Top 10 most frequent words:\n") ;
    for (int i = 0 ; i < 10 ; i++){
        printf("%s: %d\n" , TopTen[i].word , TopTen[i].frequency) ;
    }

    // free data
    freeSHM(words , shmID) ;
    free(allWords) ;
    free(TopTen) ;

    return 0 ;

}
