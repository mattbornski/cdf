#!/usr/bin/env python

import os

import cdf

def test():
    a = cdf.archive('a')
    a['foo'] = cdf.rVariable([1, 2, 3])
    a.save('a')

    b = cdf.archive('a')
    print b

    os.remove('a.cdf')

if __name__ == '__main__':
    test()
