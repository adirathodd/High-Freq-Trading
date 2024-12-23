# Multithreaded Trading System

This project simulates a **multithreaded trading system** for high-frequency financial transactions. It processes market data, matches orders between buyers and sellers, and manages a portfolio of assets. The system demonstrates efficient concurrent processing using threads, mutexes for synchronization, and queues for managing order books.

---

## Key Components

### 1. Order Book Management
The program maintains **bid (buy)** and **ask (sell)** queues for three stocks (`NVDA`, `AAPL`, and a third unnamed stock). These queues are:

- **Bid Queue**: A max-priority queue (highest price first) to prioritize buyers willing to pay the most.
- **Ask Queue**: A min-priority queue (lowest price first) to prioritize sellers offering the lowest price.

Order queues are implemented using linked lists, with new orders being inserted into the appropriate position based on their price.

---

### 2. Portfolio Management
The program simulates a trading portfolio that tracks:

- **Cash Balance**: The amount of available cash for trading.
- **PnL (Profit and Loss)**: Tracks the cumulative profit or loss from executed trades.

Initial portfolio values:
- Cash Balance: `$100,000`
- PnL: `0.0`

---

### 3. Threads and Synchronization
The system uses multiple threads to handle concurrent tasks:

#### Thread 1: Order Addition (`add_orders`)
- Processes the input transaction data (read from a CSV file).
- Adds each transaction to the appropriate bid or ask queue, depending on the action (`BUY` or `SELL`).

#### Thread 2: Order Execution (`order_execution`)
- Matches orders between buyers (bids) and sellers (asks).
- Executes trades if:
  - The buyer’s price is greater than or equal to the seller’s price.
  - The portfolio has sufficient cash to complete the transaction.
- Updates the portfolio's cash balance and PnL after each trade.

#### Thread 3: Same Price Matching (`same_price_matching`)
- Specifically matches orders where the bid and ask prices are identical.
- Executes trades under the same conditions as the `order_execution` thread.

#### Thread 4: Portfolio Management (`portfolio_management`)
- Periodically prints the current state of the portfolio, including:
  - Cash Balance
  - Total PnL

---

## Data Handling: `dataframe.c`

The file **`dataframe.c`** is responsible for:
1. **CSV Parsing**:  
   It reads the input CSV file containing transaction data into a structured format.
2. **Data Representation**:  
   - Data is stored in a `Dataframe` structure, which holds the parsed rows and metadata about the dataset (e.g., number of rows and columns).
   - Each row is stored as a `Transaction` structure with fields like:
     - `Ticker`
     - `Last Price`
     - `Volume`
     - `Action` (`BUY` or `SELL`).

3. **Memory Management**:  
   - Allocates memory for the parsed data.
   - Frees allocated memory when the program completes.

Example Functions:
- `readCSV()`:  
  Reads the CSV file and parses it into a `Dataframe` object.

- `freeDataFrame()`:  
  Frees memory used by the `Dataframe`.

---

## Workflow

### Step 1: Input Data
- The program takes a CSV file as input. Each row in the file represents a transaction with the following fields:
  - `Ticker`: Stock symbol (e.g., `NVDA`, `AAPL`).
  - `Last Price`: The price of the transaction.
  - `Volume`: The number of shares.
  - `Action`: `BUY` or `SELL`.

### Step 2: Multithreading
- The program spawns four threads:
  1. **Order Addition**: Processes the input data and populates the bid/ask queues.
  2. **Order Execution**: Matches and executes trades.
  3. **Same Price Matching**: Handles trades where bid and ask prices are equal.
  4. **Portfolio Management**: Displays portfolio updates every 5 seconds.

### Step 3: Trade Matching
- The system compares orders in the bid and ask queues.
- Executes trades based on:
  1. **Price Matching**: A bid price must meet or exceed the ask price.
  2. **Volume Matching**: Trades are executed for the smaller volume between the bid and ask.
  3. **Cash Availability**: Ensures there is enough cash in the portfolio to complete the trade.

---

## Usage

1. Compile the code:

   ```bash
   gcc dataframe.c main.c -o main
   ```
2. Run the code:

    ```bash
    ./main transactions.csv
    ```