#ifndef DATAFRAME_H
#define DATAFRAME_H

#include <stdio.h>
#include <stdlib.h>

typedef struct Row {
    char date[20];
    float open;
    float high;
    float low;
    float close;
    int volume;
    int openInt;
    float movAvg7;
    float movAvg30;
} Row;


typedef struct {
    char **columns;
    Row **rows;
    int numCols;
    int numRows;
} Dataframe;

Dataframe* readCSV(const char* filename, int rows, int cols);
void freeDataframe(Dataframe *df);
void printDataframe(Dataframe *df);
void printHead(Dataframe *df);

#endif