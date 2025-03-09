#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<math.h>
#include <time.h>

#define SIZE 1000000 // size of (words) array

typedef struct Words Words ; // struct to store the word and its frequency
typedef struct AllWords AllWords ; // struct to store all words from file

struct Words{

    char word[30] ; // array of words
    int frequency ; // array for frequencies

};

struct AllWords{

    char word[30] ;

};

Words *createWords(int size){

    Words *words = NULL ;

    words = (Words*)malloc(size * sizeof(Words));

    if (words == NULL){
        printf("Out of space.\n") ;
        exit(0) ;
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

void Naive(Words *words , AllWords *allWords , int size){

    char str[100] ;

    for (int i = 0 ; i < size ; i++){

        strcpy(str , allWords[i].word) ;

        int found = 0 , j ;

        for (j = 0 ; strcmp(words[j].word , "EmptyCell") != 0 ; j++){

            if (strcmp(words[j].word , str) == 0){
                words[j].frequency++ ;
                found=1 ;
                break ;
            }

        }

        if (!found){
            strcpy(words[j].word , str) ;
            words[j].frequency=1 ;
        }
    }

}

void FindTopTen(Words *TopTen , Words *words){

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

    AllWords *allWords = createAllWords(20000000) ; // struct array to store all words from file
    int size=0 ;

    fetchWordsFromFile(allWords , &size) ; // store the words from file in (allWords) array

    Words *words = createWords(SIZE) ; // struct array to store the word with its frequent

    Naive(words , allWords , size) ; // find each unique word with its frequent

    Words *TopTen = createWords(10) ; // struct array to store top 10 most frequent words

    FindTopTen(TopTen , words) ; // find top 10 most frequent words

    clock_gettime(CLOCK_MONOTONIC, &end) ;

    time_diff_ms = (end.tv_sec - start.tv_sec)+ (end.tv_nsec - start.tv_nsec)/1000000000 ;
    printf("%Time taken in Naive approach: %ld sec.\n" , time_diff_ms) ;

    printf("Top 10 most frequent words:\n") ;
    for (int i = 0 ; i < 10 ; i++){
        printf("%s: %d\n" , TopTen[i].word , TopTen[i].frequency) ;
    }

    // free data
    free(words) ;
    free(allWords) ;
    free(TopTen) ;

    return 0 ;

}
