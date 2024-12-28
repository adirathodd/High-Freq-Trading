#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FIELD_SIZE 256

typedef struct {
    char ticker[10];      // Stock ticker symbol
    char date[10];        // Date in format YYMMDD
    char time[10];        // Time in HHMMSS
    float last_price;     // Last transaction price
    int volume;           // Transaction volume
    char action[5];       // Action type (BUY or SELL)
} Transaction;

typedef struct {
    char **columnNames;
    Transaction **rows;
    int numColumns;
    int numRows;
} DataFrame;

// Function to allocate memory for the DataFrame
DataFrame *createDataFrame(int rows, int cols) {
    DataFrame *df = malloc(sizeof(DataFrame));
    df->columnNames = malloc(cols * sizeof(char *));
    df->rows = (Transaction **)malloc(rows * sizeof(Transaction *));

    for(int i = 0; i < rows; i++) df->rows[i] = (Transaction *)malloc(sizeof(Transaction));

    df->numColumns = cols;
    df->numRows = rows;
    return df;
}

void freeDataFrame(DataFrame *df) {
    for (int i = 0; i < df->numColumns; i++) {
        free(df->columnNames[i]);
    }
    free(df->rows);
    free(df->columnNames);
    free(df);
}

DataFrame *readCSV(const char *filename, int rows, int cols) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Unable to open file");
        return NULL;
    }

    DataFrame *df = createDataFrame(rows, cols);

    char line[1024];
    int rowCount = 0;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        char *token = strtok(line, ",");
        int columnCount = 0;

        if (rowCount == 0) {
            while (token) {
                df->columnNames[columnCount] = strdup(token);
                token = strtok(NULL, ",");
                columnCount++;
            }
            df->numColumns = columnCount;
        } else {
            Transaction *transaction = df->rows[rowCount - 1];
            if (token) {
                strncpy(transaction->ticker, token, sizeof(transaction->ticker) - 1);
                transaction->ticker[sizeof(transaction->ticker) - 1] = '\0';
                token = strtok(NULL, ",");
            }
            if (token) {
                strncpy(transaction->date, token, sizeof(transaction->date) - 1);
                transaction->date[sizeof(transaction->date) - 1] = '\0';
                token = strtok(NULL, ",");
            }
            if (token) {
                strncpy(transaction->time, token, sizeof(transaction->time) - 1);
                transaction->time[sizeof(transaction->time) - 1] = '\0';
                token = strtok(NULL, ",");
            }
            if (token) {
                transaction->last_price = strtof(token, NULL);
                token = strtok(NULL, ",");
            }
            if (token) {
                transaction->volume = atoi(token);
                token = strtok(NULL, ",");
            }
            if (token) {
                strncpy(transaction->action, token, sizeof(transaction->action) - 1);
                transaction->action[sizeof(transaction->action) - 1] = '\0';
            }
        }
        rowCount++;
    }

    df->numRows = rowCount - 1;
    fclose(file);
    return df;
}

void printline(Transaction *row) {
    printf("%s\t", row->ticker);
    printf("%s\t", row->date);
    printf("%s\t", row->time);
    printf("%.2f\t", row->last_price);
    printf("%d\t", row->volume);
    printf("%s\t", row->action);
}

void printHead(DataFrame *df) {
    for (int i = 0; i < df->numColumns; i++) {
        printf("%s\t", df->columnNames[i]);
    }
    printf("\n");

    for (int i = 0; i < 10; i++) {
        printline(df->rows[i]);
        printf("\n");
    }
}
void printDataFrame(DataFrame *df) {
    for (int i = 0; i < df->numColumns; i++) {
        printf("%s\t", df->columnNames[i]);
    }
    printf("\n");

    for (int i = 0; i < df->numRows; i++) {
        printline(df->rows[i]);
        printf("\n");
    }
}