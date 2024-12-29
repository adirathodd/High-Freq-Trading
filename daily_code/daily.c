#include "dataframe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_FILES 10000
#define NUM_THREADS 5

Dataframe *dataframes[MAX_FILES];
char *filepaths[MAX_FILES];
int file_count = 0;

// Function to compare 2 rows for qsort
int compareRows(const void *a, const void *b) {
    const Row *ra = *(const Row **)a;
    const Row *rb = *(const Row **)b;
    
    int dateCmp = strcmp(ra->date, rb->date);
    if (dateCmp != 0) {
        return dateCmp;
    }
    
    return strcmp(ra->ticker, rb->ticker);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <folderpath>\n", argv[0]);
        return -1;
    }

    char *folderpath = argv[1];
    
    struct dirent *entry;
    DIR *dp = opendir(folderpath);

    if (dp == NULL) {
        perror("Error opening folder");
        return -1;
    }

    for (int i = 0; i < MAX_FILES; i++) {
        dataframes[i] = NULL;
    }

    while ((entry = readdir(dp)) != NULL) {
        if (file_count >= MAX_FILES) {
            break;
        }

        char filepath[2048];
        snprintf(filepath, sizeof(filepath), "%s/%s", folderpath, entry->d_name);

        struct stat st;
        if (stat(filepath, &st) != 0 || !S_ISREG(st.st_mode)) {
            continue;
        }

        if (strstr(entry->d_name, ".txt") != NULL) {
            filepaths[file_count] = strdup(filepath);
            file_count++;
        }
    }

    
    closedir(dp);

    int totalRows = 0;
    
    for(int i = 0; i < file_count; i++){
        
        Dataframe *df = readCSV(filepaths[i]);
        if (df == NULL) {
            printf("Failed to read file: %s\n", filepaths[i]);
            continue;
        }
        
        dataframes[i] = df;
        totalRows += df->numRows + 1;
       
    }

    Dataframe *master = createDataframe(totalRows, 10);
    if (!master) {
        fprintf(stderr, "Failed to create master Dataframe\n");
        return -1;
    }

    for (int c = 0; c < 10; c++) {
        strncpy(master->columns[c], dataframes[0]->columns[c], MAX_FIELD_SIZE - 1);
        master->columns[c][MAX_FIELD_SIZE - 1] = '\0';
    }

    int masterIndex = 0;
    for (int i = 0; i < file_count; i++) {
        if (!dataframes[i]) {
            continue;
        }
        
        Dataframe *df = dataframes[i];

        for (int r = 0; r < df->numRows; r++) {
            Row *sourceRow = df->rows[r];
            Row *destRow   = master->rows[masterIndex];

            strcpy(destRow->date, sourceRow->date);
            strcpy(destRow->ticker, sourceRow->ticker);
            destRow->open   = sourceRow->open;
            destRow->high   = sourceRow->high;
            destRow->low    = sourceRow->low;
            destRow->close  = sourceRow->close;
            destRow->volume = sourceRow->volume;
            destRow->openInt    = sourceRow->openInt;
            destRow->movAvg7  = sourceRow->movAvg7;
            destRow->movAvg30 = sourceRow->movAvg30;
            masterIndex++;
        }
        freeDataframe(df);
    }

    master->numRows = totalRows;
    
    qsort(
        master->rows,
        master->numRows,
        sizeof(Row *),
        compareRows
    );

    freeDataframe(master);
    return 0;
}