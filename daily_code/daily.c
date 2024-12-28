#include "dataframe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_FILES 10
#define NUM_THREADS 5

Dataframe **dataframes;
char *filepaths[MAX_FILES];
int file_count = 0;

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

    dataframes = (Dataframe **)malloc(sizeof(Dataframe *) * MAX_FILES);
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
    
    for(int i = 0; i < file_count; i++){
        Dataframe *df = readCSV(filepaths[i]);
        if (df == NULL) {
            printf("Failed to read file: %s\n", filepaths[i]);
            continue;
        }
        printHead(df);
        dataframes[i] = df;
    }

    return 0;
}