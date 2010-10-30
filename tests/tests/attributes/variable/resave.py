#!/usr/bin/env python

def test():
    import cdf
    import os

    test = cdf.archive()
    test['foo'] = [1, 2]
    test['bar'] = [151.15, 151.30]
    test['foo'].attributes['int'] = 'true'
    test['foo'].attributes['min'] = 1
    test['bar'].attributes['int'] = 'false'
    test['bar'].attributes['max'] = 151.30
    test.save('test')

    # Test that saving it twice doesn't do anything harmful.
    test.save()

    test2 = cdf.archive('test')
    print test2['foo'].attributes
    print test2['bar'].attributes

    test2['bar'].attributes['min'] = 151.15
    test2.save()

    test3 = cdf.archive('test')
    print test3['foo'].attributes
    print test3['bar'].attributes

    os.remove('test.cdf')

if __name__ == '__main__':
    test()
