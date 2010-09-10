#!/usr/bin/env python

def test():
    import cdf
    import os

    test = cdf.archive()
    # Test the ability to set lists of attributes on the archive.
    test.attributes['int'] = [1, 2]
    test.attributes['float'] = [1.0, 2.0]
    test.attributes['string'] = ['one', 'two']
    test.save('test')

    test2 = cdf.archive('test')
    print test2.attributes

    os.remove('test.cdf')

if __name__ == '__main__':
    test()
