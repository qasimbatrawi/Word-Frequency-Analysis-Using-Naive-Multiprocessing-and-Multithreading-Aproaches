#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<math.h>
#include <time.h>
#include<pthread.h>

#define SIZE 1000000 // size of (words) array
#define NUM_OF_THREADS 2 // number of threads we want to create

pthread_mutex_t mutex ; // mutex lock to avoid race conditions, so only one thread will access the critical section at a time

typedef struct Words Words ; // struct to store the word and its frequency
typedef struct AllWords AllWords ; // struct to store all words from the file
typedef struct ThreadArgs ThreadArgs ; // struct to store the top 10 frequent words

struct Words{

    char word[30] ;
    int frequency ;

};

struct AllWords{

    char word[30] ;

};

typedef struct ThreadArgs{

    Words *words ;

    AllWords *allWords ;

    int index ;

    int size ;

};

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

Words *createAllWords(int size){

    AllWords *allWords = NULL ;

    allWords = (AllWords*)malloc(size * sizeof(AllWords));

    if (allWords == NULL){
        printf("Out of space.\n") ;
        exit(0) ;
    }

    return allWords ;

}


ThreadArgs *createThreadArgs(){

    ThreadArgs *args = malloc(sizeof(ThreadArgs)) ;

    if (!args) {
        printf("Out of space.\n") ;
        exit(0) ;
    }

    return args ;

}

void toLower(char *str){

    // convert upper case characters to lower

    for (int i = 0 ; str[i] != '\0' ; i++){
        if (str[i] >= 'A' && str[i] <= 'Z'){
            str[i]+=('a' - 'A') ;
        }
    }

}

void fetchWordsFromFile(AllWords *allWords , int *size){

    // open the file and store all their words in a struct array (allWords)

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

void *MultiThreading(void *args) {

    ThreadArgs *temp = (ThreadArgs*)args ;

    // distribute the argument
    Words *words = temp->words ;
    AllWords *allWords = temp->allWords ;
    int index = temp->index ;
    int size = temp->size ;

    long long start , end ; // stat and end index to read from (allWords) array

    start = index * (size / NUM_OF_THREADS) ; // calculate the start index
    end = start + (size / NUM_OF_THREADS) ; // calculate the end index

    if (index == NUM_OF_THREADS-1){
        end = size ;
    }

    char str[100] ;

    for (int i = start ; i < end ; i++){

        strcpy(str , allWords[i].word) ;

        int found = 0 , j ;

        for (j = 0 ; strcmp(words[j].word , "EmptyCell") != 0 ; j++){

            // check if str exists in (words) array
            if (strcmp(words[j].word , str) == 0){

                // entery section
                pthread_mutex_lock(&mutex) ;

                // critical section
                words[j].frequency++ ;
                found=1 ;

                // exit section
                pthread_mutex_unlock(&mutex) ;

                break ;
            }

        }

        // check if str does not exist in (words) array
        if (!found){

            // entery section
            pthread_mutex_lock(&mutex) ;

            // critical section
            strcpy(words[j].word , str) ;
            words[j].frequency=1 ;

            // exit section
            pthread_mutex_unlock(&mutex) ;
        }

    }


}

void FindTopTen(Words *TopTen , Words *words){

    // find the top 10 frequencies
    for (int i = 0 ; i < 10 ; i++){
        int mx=0 , ind=-1 ;
        for (int j = 0 ; strcmp(words[j].word , "EmptyCell") != 0 ; j++){
            if (words[j].frequency > mx){
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

int main() {

    struct timespec start, end ;
    long time_diff_ms ;

    clock_gettime(CLOCK_MONOTONIC, &start) ;

    AllWords *allWords = createAllWords(20000000) ; // struct to store all words from the file
    int size=0 ;

    fetchWordsFromFile(allWords , &size) ; // store the words from the file in the struct array

    Words *words = createWords(SIZE) ; // struct array to store each unique word with its frequency

    pthread_mutex_init(&mutex , NULL) ; // initialzie the mutex

    pthread_t threads[NUM_OF_THREADS] ; // array of threads

    int *threadNumber = malloc(sizeof(int)) ; // one of the arguments that will be passed to the thread function

    for (int i = 0 ; i < NUM_OF_THREADS ; i++){

        *threadNumber = i ;

        ThreadArgs *args = createThreadArgs() ; // declare the argument that will conatin all arguments

        args->words = words ;
        args->allWords = allWords ;
        args->index = i ;
        args->size = size ;

        // create thread and call thread function
        if (pthread_create(&threads[i] , NULL , &MultiThreading , args) != 0){
            printf("An error happened in creating thread.\n") ;
            exit(0) ;
        }

    }

    // parent will wait for its threads
    for (int i = 0 ; i < NUM_OF_THREADS ; i++){
        if (pthread_join(threads[i] , NULL) != 0){
            printf("An error happened in joining threads.\n") ;
            exit(0) ;
        }
    }

    Words *TopTen = createWords(10) ; // struct array that will store the top 10 most frequent words with their frequency

    FindTopTen(TopTen , words) ; // find the top 10 most frequent words

    clock_gettime(CLOCK_MONOTONIC, &end) ;

    time_diff_ms = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1000000000 ;
    printf("Time taken in Multi approach: %ld sec.\n" , time_diff_ms) ;

    printf("Top 10 most frequent words:\n") ;
    for (int i = 0 ; i < 10 ; i++){
        printf("%s: %d\n" , TopTen[i].word , TopTen[i].frequency) ;
    }

    pthread_mutex_destroy(&mutex) ; // clean the mutex

    // free data
    free(allWords) ;
    free(TopTen) ;
    free(words) ;

    return 0 ;

}
