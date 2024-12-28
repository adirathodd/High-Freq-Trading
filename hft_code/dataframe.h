#ifndef DATAFRAME_H
#define DATAFRAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char ticker[10];      // Stock ticker symbol
    char date[10];        // Date in format YYMMDD
    char time[10];        // Time in HHMMSS
    float last_price;     // Last transaction price
    int volume;           // Transaction volume
    char action[5];       // Action type (BUY or SELL)
} Transaction;

typedef struct {
    char **columns;
    Transaction **rows;
    int numCols;
    int numRows;
} Dataframe;

Dataframe* readCSV(const char* filename, int rows, int cols);
void freeDataFrame(Dataframe *df);
void printDataFrame(Dataframe *df);
void printHead(Dataframe *df);

#endif