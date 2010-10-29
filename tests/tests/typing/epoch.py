#!/usr/bin/env python

import cdf
import datetime
import os

def test():
    unix_epoch = datetime.datetime(1970, 1, 1, 0, 0, 0)
    decade_1910 = [
      datetime.datetime(1910, 1, 1, 0, 0, 0, 0),
      datetime.datetime(1919, 12, 31, 23, 59, 59),
    ]
    decade_2010 = [
      datetime.datetime(2010, 1, 1, 0, 0, 0, 0),
      datetime.datetime(2019, 12, 31, 23, 59, 59),
    ]

    test = cdf.archive()
    test['epoch'] = [unix_epoch]
    test['epoch'].append(test['epoch'][0])
    test['decades'] = [decade_1910, decade_2010]
    print test
    test.save('test')

    test2 = cdf.archive('test')
    print test2

    if test2['epoch'][0] < datetime.datetime.utcnow():
        print 'We are definitely existing after the start of the unix epoch.'

    os.remove('test.cdf')

if __name__ == '__main__':
    test()
