#!/usr/bin/env python

def test():
    import cdf
    import os

    test = cdf.archive()
    test.attributes['int'] = 1
    test.attributes['float'] = 1.0
    test.attributes['string'] = 'one'
    test.save('test')

    # Test that saving it twice doesn't do anything harmful.
    test.save()

    test2 = cdf.archive('test')
    print test2.attributes

    test2.attributes['int'] = 2
    test2.save()

    test3 = cdf.archive('test')
    print test3.attributes

    os.remove('test.cdf')

if __name__ == '__main__':
    test()
