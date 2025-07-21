import pandas as pd


correct = pd.read_csv('build/pred_correct.csv')
factors = pd.read_csv('build/factors.csv')


correct = correct.groupby('ts_code').last().reset_index().sort_values('ts_code').reset_index(drop=True)
factors = factors.groupby('ts_code').last().reset_index().sort_values('ts_code').reset_index(drop=True)
correct, factors = correct.merge(factors[['ts_code']], how='inner', on='ts_code').sort_values('ts_code').reset_index(drop=True), factors.merge(correct[['ts_code']], how='inner', on='ts_code').sort_values('ts_code').reset_index(drop=True)
factors = factors[['ts_code', 'timestamp'] + [c for c in factors.columns if 'momentum_o_' in c]]
correct = correct[factors.columns]
print(correct)
print(factors)
