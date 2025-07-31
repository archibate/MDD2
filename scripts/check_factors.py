#!/usr/bin/env python

import pandas as pd
import numpy as np
import warnings

warnings.filterwarnings('ignore')

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


factors = pd.read_csv('build/factors.csv')

if 1:
    correct = pd.read_csv('build/predictions_real_20250102.csv').rename(columns={'code': 'ts_code', 'limit_up_time': 'timestamp'})
    del correct['date']
    del correct['up_stat']
    del correct['limit']
    del correct['return_value']
    correct['ts_code'] = pd.to_numeric(correct['ts_code'].str.split('.').str[0])
    correct.columns = [x.replace('.', 'o') for x in correct.columns]
    correct['timestamp'] = correct['timestamp'].astype('int32')
else:
    correct = pd.read_csv('build/pred_correct.csv').reindex(columns=factors.columns)

factors = factors[factors['timestamp'] != 0]
assert isinstance(factors, pd.DataFrame)

correct = correct.groupby('ts_code').last().reset_index().sort_values('ts_code').reset_index(drop=True)
factors = factors.groupby('ts_code').last().reset_index().sort_values('ts_code').reset_index(drop=True)
correct, factors = correct.merge(factors[['ts_code']], how='inner', on='ts_code').sort_values('ts_code').reset_index(drop=True), factors.merge(correct[['ts_code']], how='inner', on='ts_code').sort_values('ts_code').reset_index(drop=True)
# factors = factors[['ts_code', 'timestamp'] + [c for c in factors.columns if 'SR' in c or 'TS' in c or 'QUA' in c]]
correct = correct[[x for x in factors.columns if x in correct.columns]]
factors = factors[correct.columns]

# print(factors)
# print(correct)

# afternoon vwap_skew_kurt incorrect
# some SR incorrect
# some filter_QUA incorrect
# some TS incorrect

# print(correct)
# print(factors)
factors = factors.set_index('ts_code')
correct = correct.set_index('ts_code')

diff = ((factors - correct) / correct).replace([np.inf, -np.inf], np.nan).fillna(correct - factors) * 100
assert isinstance(diff, pd.DataFrame)
diff[correct.isna() & factors.isna()] = 0
diff[correct.isna() & ~factors.isna()] = -9999
diff[~correct.isna() & factors.isna()] = 9999
diff['timestamp'] = correct['timestamp']
diff['time'] = ((linear_time(correct['timestamp']) * 1000) + 90) // 100 / 10 - linear_time(factors['timestamp']) # type: ignore
diff = diff.sort_values('timestamp').reset_index()
print(diff.round(2)[['ts_code', 'timestamp', 'time'] + [c for c in factors.columns if 'up' in c]])


del diff['ts_code']
del diff['timestamp']
# error = diff.abs().max()
error = diff.abs().quantile(0.8)
error = error.sort_values(ascending=False)

print()
print('===================')
print('invalid:')
print(error[error == 9999])

error = error[error != 9999]
error = error[error > 1]
assert isinstance(error, pd.Series)

print()
print('===================')
print('80% errors:')
print(error.round(3))
