#include <stdio.h>
#include <stdlib.h>
#include "dataframe.h"
#include <string.h>
#include <ctype.h>

// Function to allocate memory for the Dataframe
Dataframe *createDataframe(int rows, int cols) {
    Dataframe *df = malloc(sizeof(Dataframe));
    for (int i = 0; i < cols; i++) {
        df->columns[i] = malloc(MAX_FIELD_SIZE * sizeof(char));
    }
    df->rows = malloc(rows * sizeof(Row *));
    for (int i = 0; i < rows; i++) {
        df->rows[i] = malloc(sizeof(Row));
    }
    df->numCols = cols;
    df->numRows = rows;
    return df;
}

// Function to free the previously allocated memory for the Dataframe
void freeDataframe(Dataframe *df) {
    for (int i = 0; i < df->numCols; i++) {
        free(df->columns[i]);
    }

    for (int i = 0; i < df->numRows; i++) {
        free(df->rows[i]);
    }

    free(df);
}


int count_rows(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("Could not open file");
        exit(EXIT_FAILURE);
    }

    size_t line_count = 0;
    char buffer[8192];

    while (1) {
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), file);
        if (bytes_read == 0) {
            break;
        }

        for (size_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                line_count++;
            }
        }
    }

    fclose(file);
    return (int)line_count;
}

// Function to get a row based on row number
// Zero indexing
Row *getRow(int rowNum, Dataframe *df){
    Row **rows = df->rows;

    if(rowNum > df->numRows){
        printf("Row number out of bounds.\n");
        return NULL;
    }

    return rows[rowNum];
}

void trimWhitespace(char *line) {
    char *end;
    while (isspace((unsigned char)*line)) line++;
    end = line + strlen(line) - 1;
    while (end > line && (isspace((unsigned char)*end) || *end == '\r' || *end == '\n')) {
        *end = '\0';
        end--;
    }
}

Dataframe *readCSV(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Unable to open file");
        return NULL;
    }

    int rows = count_rows(filename) - 1;
    char *ticker = strtok((char *)filename, "/");
    ticker = strtok(NULL, "/");
    ticker = strtok(NULL, ".");

    Dataframe *df = createDataframe(rows, 10);

    char line[1024];
    int rowCount = 0;
    float movingTotal7 = 0.0, movingTotal30 = 0.0;

    while (fgets(line, sizeof(line), file)) {
        trimWhitespace(line);
        int columnCount = 0;
        char *token = strtok(line, ",");
        
        if (rowCount == 0) {
            if (token) {
                strncpy(df->columns[columnCount], token, MAX_FIELD_SIZE - 1);
                df->columns[columnCount][MAX_FIELD_SIZE - 1] = '\0';
                columnCount++;
                token = strtok(NULL, ",");
            }

            strncpy(df->columns[columnCount], "Ticker", MAX_FIELD_SIZE - 1);
            df->columns[columnCount][MAX_FIELD_SIZE - 1] = '\0';
            columnCount++;

            while (token) {
                strncpy(df->columns[columnCount], token, MAX_FIELD_SIZE - 1);
                df->columns[columnCount++][MAX_FIELD_SIZE - 1] = '\0';
                token = strtok(NULL, ",");
            }

            strncpy(df->columns[columnCount], "MVA7", MAX_FIELD_SIZE - 1);
            df->columns[columnCount++][MAX_FIELD_SIZE - 1] = '\0';

            strncpy(df->columns[columnCount], "MVA30", MAX_FIELD_SIZE - 1);
            df->columns[columnCount++][MAX_FIELD_SIZE - 1] = '\0';

            df->numCols = columnCount;
        } else {
            Row *row = df->rows[rowCount - 1];
            strncpy(row->ticker, ticker, sizeof(row->ticker) - 1);
            if (token) {
                strncpy(row->date, token, sizeof(row->date) - 1);
                row->date[sizeof(row->date) - 1] = '\0';
                token = strtok(NULL, ",");
            }
            if (token) {
                row->open = strtof(token, NULL);
                token = strtok(NULL, ",");
            }
            if (token) {
                row->high = strtof(token, NULL);
                token = strtok(NULL, ",");
            }
            if (token) {
                row->low = strtof(token, NULL);
                token = strtok(NULL, ",");
            }
            if (token) {
                row->close = strtof(token, NULL);
                token = strtok(NULL, ",");
            }
            if (token) {
                row->volume = atoi(token);
                token = strtok(NULL, ",");
            }
            if (token) {
                row->openInt = atoi(token);
                token = strtok(NULL, ",");
            }

            if(rowCount > 7){
                Row *firstRow = getRow(rowCount - 8, df);
                row->movAvg7 = movingTotal7 / 7;
                movingTotal7 -= firstRow->close;
            } else {
                row->movAvg7 = 0;
            }

            movingTotal7 += row->close;

            if(rowCount > 30){
                Row *firstRow = getRow(rowCount - 31, df);
                row->movAvg30 = movingTotal30 / 30;
                movingTotal30 -= firstRow->close;
            } else {
                row->movAvg30 = 0;
            }
            movingTotal30 += row->close;
        }
        rowCount++;
    }
    
    fclose(file);

    int dataRows = rowCount - 1;
    df->numRows = dataRows;

    if (dataRows <= 30) {
        return df;
    }

    for (int i = 0; i < 30; i++) {
        free(df->rows[i]);
        df->rows[i] = NULL;
    }

    for (int i = 0; i < dataRows - 30; i++) {
        df->rows[i] = df->rows[i + 30];
    }

    for (int i = dataRows - 30; i < dataRows; i++) {
        df->rows[i] = NULL;
    }

    df->numRows = dataRows - 30;

    return df;
}

void printline(Row *row) {
    printf("%-15s", row->date);
    printf("%-15s", row->ticker);
    printf("%-15.4f", row->open);
    printf("%-15.4f", row->high);
    printf("%-15.4f", row->low);
    printf("%-15.4f", row->close);
    printf("%-15.10d", row->volume);
    printf("%-10d", row->openInt);
    printf("%-15.4f", row->movAvg7);
    printf("%-15.4f", row->movAvg30);
    printf("\n");
}

void printHead(Dataframe *df) {
    for (int i = 0; i < df->numCols; i++) {
        printf("%-15s", df->columns[i]);
    }
    printf("\n"); 

    for (int i = 0; i < 10; i++) printline(df->rows[i]);
}

void printDataframe(Dataframe *df) {
    for (int i = 0; i < df->numCols; i++) {
        printf("%-15s", df->columns[i]);
    }
    printf("\n");

    for (int i = 0; i < df->numRows; i++) printline(df->rows[i]);
}