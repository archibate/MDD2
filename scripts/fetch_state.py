#!/usr/bin/env python

import pandas as pd
import rqdatac as rq
import datetime as dt
import numpy as np
import warnings
import sys

warnings.filterwarnings('ignore')
rq.init()

today = sys.argv[2]
market_name = sys.argv[1]
assert today.startswith('20') and len(today) == 8, today
assert market_name in ['SH', 'SZ'], market_name

print('Today:', today)
print('Market:', market_name)

today_date = dt.datetime.strptime(today, '%Y%m%d')
calendar = rq.get_trading_dates(start_date=today_date - dt.timedelta(days=20), end_date=today_date)

if not calendar[-1] != today_date:
    raise RuntimeError(f'today {today} is not a trade day!')
yesterday = calendar[-2].strftime('%Y%m%d')
print('Yesterday:', yesterday)


print('Loading factors')
csv_path = f'/data/daily_csv/{market_name.lower()}_{today}.csv'
factors = pd.read_csv(csv_path)
assert (factors['trade_date'] == int(today)).all(), factors
del factors['trade_date']

stocks = factors.pop('ts_code').str.split('.').str[0].astype('int32')
factors = factors.astype('float64')

print('Loading feature order')
feature_order = []
with open(f'src/FactorEnum.h', 'r') as f:
    for line in f.readlines():
        line = line.strip()
        if line.endswith(','):
            feature_order.append(line.removesuffix(',').strip())
factors = factors.reindex(columns=feature_order)
assert len(stocks) == len(factors)


print('Loading prev limit up')
stock_prev_limits = pd.read_json(f'/data/daily_csv/previous_trading_day_limit_up_data_{today}.json')['ts_code'].str.split('.').str[0].astype('int32')
print('Done!')
print()


config = pd.DataFrame([
    {'fileVersion': 250722, 'today': today, 'marketID': {'SH': 1, 'SZ': 2}[market_name], 'stockCount': len(stocks), 'prevLimitUpCount': len(stock_prev_limits), 'factorCount': factors.shape[1], 'factorDtypeSize': 8},
]).astype('int32')

print('Stocks:')
print(stocks)
print('Factors:')
print(factors)
print('Prev limit up stocks:')
print(stock_prev_limits)
print('Config:')
print(config)


with open(f'/data/daily_csv/mdd2_factors_{market_name.lower()}_{today}.bin', 'wb') as f:
    f.write(config.to_numpy().tobytes())
    f.write(stocks.to_numpy().tobytes())
    f.write(stock_prev_limits.to_numpy().tobytes())
    f.write(factors.to_numpy().tobytes())
