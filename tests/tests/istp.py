#!/usr/bin/env python

def test():
    import cdf
    import cdf.istp
    import os
    import os.path

    test = cdf.istp.archive('test', skeleton = 'istp.skt')
    test['foo'] = [1, 2, 3, 4, 5]
    test['bar'] = ['a', 'b', 'see', 'd', 'e']
    test.save()

    readback = cdf.archive('test')
    print readback.keys()
    print readback['foo']
    print readback.attributes
    print readback['bar'].attributes

    os.remove('test.cdf')

if __name__ == '__main__':
    test()
