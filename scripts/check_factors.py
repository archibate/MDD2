#!/usr/bin/env python

import pandas as pd
import numpy as np


correct = pd.read_csv('build/pred_correct.csv')
factors = pd.read_csv('build/factors.csv')

correct = correct.groupby('ts_code').last().reset_index().sort_values('ts_code').reset_index(drop=True)
factors = factors.groupby('ts_code').last().reset_index().sort_values('ts_code').reset_index(drop=True)
correct, factors = correct.merge(factors[['ts_code']], how='inner', on='ts_code').sort_values('ts_code').reset_index(drop=True), factors.merge(correct[['ts_code']], how='inner', on='ts_code').sort_values('ts_code').reset_index(drop=True)
factors = factors[['ts_code', 'timestamp'] + [c for c in factors.columns if 'momentum_h_' in c]]
correct = correct[factors.columns]

print(correct)
print(factors)
factors = factors.set_index('ts_code')
correct = correct.set_index('ts_code')
del correct['timestamp']
del factors['timestamp']

diff = (((factors - correct) / correct).replace([np.inf, -np.inf], np.nan).fillna(correct - factors) * 100).round(2)
diff[correct.isna() & factors.isna()] = 0
diff[correct.isna() & ~factors.isna()] = -100
diff[~correct.isna() & factors.isna()] = 100
diff = diff.reset_index()
print(diff)
