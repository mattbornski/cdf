#!/usr/bin/env python

def test():
    import cdf
    import os

    test = cdf.archive('test')
    test['foo'] = [[1, 1], [2, 2], [3, 3], [4, 4], [5, 5]]
    for record in test['foo']:
        record[1] += 1
    test['bar'] = ['a', 'b', 'see', 'd', 'e']
    test.save()

    test2 = cdf.archive('test')
    print test2['foo']
    print test2['bar']

    os.remove('test.cdf')

if __name__ == '__main__':
    test()
