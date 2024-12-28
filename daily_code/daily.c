#include "dataframe.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]){
    if (argc != 2){
        printf("Usage - %s <filepath>\n", argv[0]);
        return -1;
    }

    char *filepath = argv[1];
    Dataframe *df = readCSV(filepath, 8364, 7);
    printHead(df);
    freeDataframe(df);
}