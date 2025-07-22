#!/usr/bin/env python

import pandas as pd
import tushare as ts

today = '20250722'
market_name = 'SH'
assert today.startswith('20') and len(today) == 8, today
assert market_name in ['SH', 'SZ'], market_name

print('Today:', today)
print('Market:', market_name)

pro = ts.pro_api('e266be1f9c7345bcc6ba56649463c0c78cc69eeb9d8ecb47701a8416')

calender = pro.trade_cal(
    exchange={
        'SH': 'SSE',
        'SZ': 'SZSE',
    }[market_name],
    start_date=today,
    end_date=today,
    fields='cal_date,is_open,pretrade_date',
)

assert len(calender) == 1
if not calender['is_open'].values[0]:
    raise RuntimeError(f'today {today} is not a trade day!')
yesterday = str(calender['pretrade_date'].values[0])
print('Yesterday:', yesterday)


print('Loading factors')
csv_path = f'/data/daily_csv/{market_name.lower()}_{today}.csv'
factors = pd.read_csv(csv_path)
assert (factors['trade_date'] == int(today)).all(), factors
del factors['trade_date']

stock_codes = factors.pop('ts_code').str.removesuffix(f'.{market_name}').astype('int32')
factors = factors.astype('float64')
print(stock_codes)
print(factors)
