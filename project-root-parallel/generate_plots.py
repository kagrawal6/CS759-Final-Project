#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt

def main():
    # 1) Load the CSV with skipinitialspace to handle stray spaces
    df = pd.read_csv('benchmark_results.csv', skipinitialspace=True)
    
    # 2) Debug: print columns and sample values
    print("Columns found in CSV:")
    print(df.columns.tolist())
    print("\nSample of threads, profit_usd_per_s, and trade_count:")
    print(df[['threads', 'profit_usd_per_s', 'trade_count']].head(), "\n")
    
    # 3) Strip whitespace from column names just in case
    df.rename(columns=lambda s: s.strip(), inplace=True)
    
    # Re-verify columns
    print("Columns after stripping whitespace:")
    print(df.columns.tolist())
    
    # 4) Plotting
    
    # Bar chart: Mean Wall-Clock Time vs. Threads
    plt.figure()
    plt.bar(df['threads'], df['mean_wall_ms'], yerr=df['stddev_wall_ms'], capsize=5)
    plt.xlabel('Threads')
    plt.ylabel('Mean Wall-Clock Time (ms)')
    plt.title('Mean Wall-Clock Time vs. Threads')
    plt.tight_layout()
    plt.savefig('wall_clock_time.pdf')
    plt.close()

    # Speedup vs. Threads
    plt.figure()
    plt.plot(df['threads'], df['speedup'], '-o')
    plt.xlabel('Threads')
    plt.ylabel('Speedup')
    plt.title('Speedup vs. Threads')
    if 8 in df['threads'].values:
        y8 = df.loc[df['threads'] == 8, 'speedup'].iloc[0]
        plt.annotate('Diminishing returns',
                     xy=(8, y8), xytext=(8, y8 * 0.8),
                     arrowprops=dict(arrowstyle='->'))
    plt.tight_layout()
    plt.savefig('speedup.pdf')
    plt.close()

    # Efficiency vs. Threads
    plt.figure()
    plt.plot(df['threads'], df['efficiency'], '-o')
    plt.xlabel('Threads')
    plt.ylabel('Efficiency')
    plt.title('Efficiency vs. Threads')
    plt.tight_layout()
    plt.savefig('efficiency.pdf')
    plt.close()

    # Profit per Second vs. Threads
    plt.figure()
    plt.plot(df['threads'], df['profit_usd_per_s'], '-o')
    plt.xlabel('Threads')
    plt.ylabel('Profit (USD) per Second')
    plt.title('Profit per Second vs. Threads')
    idx = df['profit_usd_per_s'].idxmax()
    xt = df.at[idx, 'threads']
    yt = df.at[idx, 'profit_usd_per_s']
    plt.annotate('Peak profit/s',
                 xy=(xt, yt), xytext=(xt, yt * 0.9),
                 arrowprops=dict(arrowstyle='->'))
    plt.tight_layout()
    plt.savefig('profit_per_second.pdf')
    plt.close()

    print("PDFs generated: wall_clock_time.pdf, speedup.pdf, efficiency.pdf, profit_per_second.pdf")

if __name__ == '__main__':
    main()
