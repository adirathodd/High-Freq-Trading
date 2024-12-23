#include "dataframe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

typedef struct Node{
    char ticker[10];
    float price;
    int volume;
    struct Node *next;
} Node;

typedef struct {
    float cash_balance;
    float total_pnl;
} Portfolio;

Portfolio portfolio = {100000.0, 0.0};
Transaction **transactions;
Node *askHeads[3] = {NULL}, *bidHeads[3] = {NULL};
int rows, stop = 0;

pthread_mutex_t data_mutex;
pthread_mutex_t order_mutex;
pthread_mutex_t bid_mutex;
pthread_mutex_t ask_mutex;

void minQueue(Node **head, Node *add) {
    if (*head == NULL || add->price < (*head)->price) {
        add->next = *head;
        *head = add;
        return;
    }

    Node *current = *head;
    while (current->next != NULL && current->next->price < add->price) {
        current = current->next;
    }

    add->next = current->next;
    current->next = add;
}

void maxQueue(Node **head, Node *add) {
    if (*head == NULL || add->price > (*head)->price) {
        add->next = *head;
        *head = add;
        return;
    }

    Node *current = *head;
    while (current->next != NULL && current->next->price > add->price) {
        current = current->next;
    }

    add->next = current->next;
    current->next = add;
}


void *add_orders(void *arg){
    for(int i = 0; i < rows; i++){
        Node *new = (Node *)malloc(sizeof(Node));
        Transaction *curr = transactions[i];
        new->next = NULL;
        strcpy(new->ticker, curr->ticker);
        new->price = curr->last_price;
        new->volume = curr->volume;

        if(strcmp(curr->action, "BUY") == 0){
            pthread_mutex_lock(&bid_mutex);
            if(strcmp(curr->ticker, "NVDA") == 0){
                maxQueue(&bidHeads[0], new);
            } else if (strcmp(curr->ticker, "AAPL") == 0){
                maxQueue(&bidHeads[1], new);
            } else {
                maxQueue(&bidHeads[2], new);
            }
            pthread_mutex_unlock(&bid_mutex);
        } else {
            pthread_mutex_lock(&ask_mutex);
            if(strcmp(curr->ticker, "NVDA") == 0){
                minQueue(&askHeads[0], new);
            } else if (strcmp(curr->ticker, "AAPL") == 0){
                minQueue(&askHeads[1], new);
            } else {
                minQueue(&askHeads[2], new);
            }
            pthread_mutex_unlock(&ask_mutex);
        }
    }

    pthread_mutex_lock(&order_mutex);
    stop = 1;
    pthread_mutex_unlock(&order_mutex);
    printf("Add Orders Thread Completed.\n");
    return NULL;
}

void* order_execution(void* arg) {
    while (!stop) {
        pthread_mutex_lock(&bid_mutex);
        pthread_mutex_lock(&ask_mutex);

        for(int i = 0; i < 3; i++) {
            Node *bid = bidHeads[i];
            Node *ask = askHeads[i];

            while(bid != NULL && ask != NULL) {
                if(bid->price < ask->price){
                    break;
                }

                int currVol = (bid->volume < ask->volume) ? bid->volume : ask->volume;

                float total_cost = ask->price * currVol;

                pthread_mutex_lock(&data_mutex);
                if(total_cost <= portfolio.cash_balance){
                    float profit = (bid->price - ask->price) * currVol;
                    portfolio.total_pnl += profit;
                    portfolio.cash_balance += profit;

                    printf("Executed Trade: %d shares of %s at Buy %.2f / Sell %.2f | Profit: %.2f\n",
                           currVol, bid->ticker, ask->price, bid->price, profit);

                    bid->volume -= currVol;
                    ask->volume -= currVol;

                    if(bid->volume == 0){
                        bidHeads[i] = bid->next;
                        free(bid);
                        bid = bidHeads[i];
                    } else {
                        bid = bid->next;
                    }

                    if(ask->volume == 0){
                        askHeads[i] = ask->next;
                        free(ask);
                        ask = askHeads[i];
                    } else {
                        ask = ask->next;
                    }
                } else {
                    printf("Insufficient cash to execute trade: %d shares of %s at Buy %.2f / Sell %.2f | Required: %.2f, Available: %.2f\n",
                           currVol, bid->ticker, ask->price, bid->price, total_cost, portfolio.cash_balance);
                    pthread_mutex_unlock(&data_mutex);
                    break;
                }
                pthread_mutex_unlock(&data_mutex);
            }
        }

        pthread_mutex_unlock(&ask_mutex);
        pthread_mutex_unlock(&bid_mutex);

        usleep(1000);
    }
    printf("Order Execution Thread Completed.\n");
    return NULL;
}

void* same_price_matching(void* arg) {
    while (!stop) {
        pthread_mutex_lock(&bid_mutex);
        pthread_mutex_lock(&ask_mutex);

        for(int i = 0; i < 3; i++) {
            Node *bid = bidHeads[i];
            Node *ask = askHeads[i];

            if(bid != NULL && ask != NULL && bid->price == ask->price) {
                int currVol = (bid->volume < ask->volume) ? bid->volume : ask->volume;

                float total_cost = ask->price * currVol;

                pthread_mutex_lock(&data_mutex);
                if(total_cost <= portfolio.cash_balance){
                    float profit = (bid->price - ask->price) * currVol;
                    portfolio.total_pnl += profit;
                    portfolio.cash_balance += profit;

                    printf("Same Price Trade Executed: %d shares of %s at Price %.2f | Profit: %.2f\n",
                           currVol, bid->ticker, bid->price, profit);

                    bid->volume -= currVol;
                    ask->volume -= currVol;

                    if(bid->volume == 0){
                        bidHeads[i] = bid->next;
                        free(bid);
                        bid = bidHeads[i];
                    }

                    if(ask->volume == 0){
                        askHeads[i] = ask->next;
                        free(ask);
                        ask = askHeads[i];
                    }
                } else {
                    printf("Insufficient cash to execute same price trade: %d shares of %s at Price %.2f | Required: %.2f, Available: %.2f\n",
                           currVol, bid->ticker, bid->price, total_cost, portfolio.cash_balance);
                }
                pthread_mutex_unlock(&data_mutex);
            }
        }

        pthread_mutex_unlock(&ask_mutex);
        pthread_mutex_unlock(&bid_mutex);

        usleep(1000);
    }
    printf("Same Price Matching Thread Completed.\n");
    return NULL;
}

void* portfolio_management(void* arg) {
    while (!stop) {
        pthread_mutex_lock(&data_mutex);
        printf("Portfolio: Cash = %.2f, PnL = %.2f\n",
               portfolio.cash_balance, portfolio.total_pnl);
        pthread_mutex_unlock(&data_mutex);
        sleep(5);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc != 2){
        printf("Usage - %s <csv filepath>\n", argv[0]);
        return -1;
    }

    char* filename = argv[1];
    Dataframe *df = readCSV(filename, 78938, 6);
    transactions = df->rows;
    rows = df->numRows;
    
    pthread_t threads[4];

    pthread_mutex_init(&data_mutex, NULL);
    pthread_mutex_init(&order_mutex, NULL);
    pthread_mutex_init(&ask_mutex, NULL);
    pthread_mutex_init(&bid_mutex, NULL);

    pthread_create(&threads[0], NULL, add_orders, NULL);
    pthread_create(&threads[1], NULL, order_execution, NULL);
    pthread_create(&threads[2], NULL, portfolio_management, NULL);
    pthread_create(&threads[3], NULL, same_price_matching, NULL);



    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&data_mutex);
    pthread_mutex_destroy(&order_mutex);
    pthread_mutex_destroy(&ask_mutex);
    pthread_mutex_destroy(&bid_mutex);

    freeDataFrame(df);

     printf("Portfolio: Cash = %.2f, PnL = %.2f\n",
               portfolio.cash_balance, portfolio.total_pnl);

    return 0;
}