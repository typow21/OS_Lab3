#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define size 100000

void initDict();
char *dict[size];
int check_dictionary(char *word);

int main(int argc, char *argv[]){
    initDict();
    // test for dictionary
    // for(int i = 0; i < 99170; i++){
    //     printf("%s\n", dict[i]);
    // }
    
    // Not working correctly
    char *line = NULL;
    size_t line_size = 0;
    while(getline(&line, &line_size, stdin) > 0){
        if(check_dictionary(line)){
            printf("Word found\n");
        }else{
            printf("Word not found");
        }
    }
}

void initDict(){
    size_t lineSize = 0;
    char* fileName = "dictionary.txt";
    FILE *file =fopen(fileName, "r");
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
}

// Returns a 1 if the word is in the dictionary
// Returns a 0 if the word is not in the dictionary
int check_dictionary(char *word){
    int indicator = 0;
    for(int i = 0; i < size; i++){
        if(strcmp(dict[i], word) >= 0){
            indicator = 1;
            // printf("Word found"); // test
            return indicator;
        }
    }
    return indicator;
}