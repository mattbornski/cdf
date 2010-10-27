#!/usr/bin/env python

import os

import cdf

def test():
    test = cdf.archive('test')
    test['foo'] = cdf.zVariable([1, 2, 3])
    test['bar'] = cdf.zVariable(['a', 'b', 'see'])
    test.save()

    test2 = cdf.archive('test')
    print test2['foo']
    print test2['bar']

    os.remove('test.cdf')

if __name__ == '__main__':
    test()
