#!/usr/bin/env python

def test():
    import cdf
    import os

    test = cdf.archive()
    # Test the ability to set boring attributes like numbers and strings
    # on the archive.
    test.attributes['int'] = 1
    test.attributes['float'] = 1.0
    test.attributes['string'] = 'one'
    test.save('test')

    test2 = cdf.archive('test')
    print test2.attributes

    os.remove('test.cdf')

if __name__ == '__main__':
    test()
