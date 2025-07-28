#!/usr/bin/env python

import pandas as pd
import numpy as np

def linear_time(time: pd.Series):
    time = (pd.to_datetime(time, format='%H%M%S%f') - pd.to_datetime('09:30:00', format='%H:%M:%S')).dt.total_seconds()

    market_open = pd.to_datetime('09:30:00', format='%H:%M:%S')
    open_duration = (market_open - pd.to_datetime('9:25:00', format='%H:%M:%S')).total_seconds()
    break_start = (pd.to_datetime('11:30:00', format='%H:%M:%S') - market_open).total_seconds()
    break_end = (pd.to_datetime('13:00:00', format='%H:%M:%S') - market_open).total_seconds()
    break_duration = break_end - break_start

    time.loc[(time > break_start) & (time < break_end)] = break_start
    time.loc[time >= break_end] -= break_duration
    time.loc[(time > -open_duration) & (time < 0)] = -open_duration
    time.loc[time <= -open_duration] += open_duration

    return time


correct = pd.read_csv('build/predictions_real_20250102.csv').rename(columns={'code': 'ts_code', 'limit_up_time': 'timestamp'})
del correct['date']
del correct['up_stat']
del correct['limit']
del correct['return_value']
correct['ts_code'] = pd.to_numeric(correct['ts_code'].str.split('.').str[0])
# correct = pd.read_csv('build/pred_correct.csv')

factors = pd.read_csv('build/factors.csv')
factors = factors[factors['timestamp'] != 0]
assert isinstance(factors, pd.DataFrame)

correct = correct.groupby('ts_code').last().reset_index().sort_values('ts_code').reset_index(drop=True)
factors = factors.groupby('ts_code').last().reset_index().sort_values('ts_code').reset_index(drop=True)
correct, factors = correct.merge(factors[['ts_code']], how='inner', on='ts_code').sort_values('ts_code').reset_index(drop=True), factors.merge(correct[['ts_code']], how='inner', on='ts_code').sort_values('ts_code').reset_index(drop=True)
# factors = factors[['ts_code', 'timestamp'] + [c for c in factors.columns if c.startswith('vol_') and 'vwap' not in c]]
factors = factors[['ts_code', 'timestamp'] + [c for c in factors.columns if c.startswith('vol_') and 'turnover' in c]]
correct = correct[factors.columns]

print(correct)
print(factors)
factors = factors.set_index('ts_code')
correct = correct.set_index('ts_code')
# del correct['timestamp']
# del factors['timestamp']

diff = (((factors - correct) / correct).replace([np.inf, -np.inf], np.nan).fillna(correct - factors) * 100).round(2)
diff[correct.isna() & factors.isna()] = 0
diff[correct.isna() & ~factors.isna()] = -9999
diff[~correct.isna() & factors.isna()] = 9999
if 'timestamp' in correct.columns and 'timestamp' in factors.columns:
    diff['timestamp'] = ((linear_time(correct['timestamp']) * 1000) + 90) // 100 / 10 - linear_time(factors['timestamp']) # type: ignore
diff = diff.reset_index()
print(diff)
