# High-Frequency Trading Data Preprocessing and Analysis

This project is designed to preprocess and analyze high-frequency trading data. It combines Python and C to achieve efficient preprocessing and analytical computation of trading transactions. The workflow includes adding missing information to the data and then analyzing it using a custom-built C program.

---

## Project Structure

### Files Overview

- **`dataframe.c`**  
  Contains the implementation of functions for managing and analyzing trading data, such as filtering transactions or calculating statistics.

- **`dataframe.h`**  
  Header file for `dataframe.c`. It defines the necessary structures and declares the functions used in the C program.

- **`main.c`**  
  The main entry point for the C program. This file integrates the functionality from `dataframe.c` and provides a way to run and test the program.

- **`combine.py`**  
  A Python script for preprocessing the transaction data:
  - Reads raw CSV transaction data.
  - Processes and determines whether each transaction is a `BUY` or `SELL`.
  - Saves the processed data into a new CSV file.

- **Transaction CSV File**  
  The dataset of high-frequency trading transactions. Initially, the dataset lacked a column indicating transaction type (`BUY` or `SELL`). This is addressed by the `combine.py` script.

---

## Features

1. **Data Preprocessing**  
   - Adds a new column to the dataset to indicate the type of transaction (`BUY` or `SELL`).
   - Handles large CSV files efficiently using Python's `pandas` library.

2. **Data Analysis**  
   - Processes the preprocessed data in C for high performance.
   - Analyzes trading statistics such as transaction volume, price trends, or other custom metrics.

3. **Combined Use of Python and C**  
   - Leverages Python for flexible data preprocessing.
   - Utilizes C for high-speed analytical computation.

---

## Workflow

1. Place the raw CSV file (e.g., `transactions.csv`) in the project folder.
2. Compile the code:

   ```bash
   gcc dataframe.c main.c -o main
   ```
3. Run the code:

    ```bash
    ./main transactions.csv
    ```