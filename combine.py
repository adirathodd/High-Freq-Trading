import pandas as pd

def process_trading_data(files, output_file):
    dataframes = []

    for file in files:
        df = pd.read_csv(file)
        dataframes.append(df)

    combined_df = pd.concat(dataframes)

    combined_df['ROLLING_VOL'] = combined_df['<VOL>'].rolling(window=5, min_periods=1).mean()
    combined_df['PRICE_DIFF'] = combined_df['<LAST>'].diff()

    combined_df['<ACTION>'] = combined_df.apply(
        lambda row: 'BUY' if row['PRICE_DIFF'] > 0 and row['<VOL>'] > row['ROLLING_VOL'] else 'SELL',
        axis=1
    )

    combined_df = combined_df.drop(columns=['ROLLING_VOL', 'PRICE_DIFF'])
    combined_df = combined_df.sort_values(by=["<TIME>", "<TICKER>"])
    combined_df.to_csv(output_file, index=False)

    print(f"Processed data saved to {output_file}")


files = ["hft_data/AAPL1.csv", "hft_data/TSLA1.csv", "hft_data/NVDA1.csv"]
output_file = "combined_data.csv"
process_trading_data(files, output_file)