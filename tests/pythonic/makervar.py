#!/usr/bin/env python

import os
import sys
import tempfile

import cdf

def test():
    dir = tempfile.mkdtemp()
    last = os.getcwd()
    os.chdir(dir)

    a = cdf.archive('a')
    a['foo'] = cdf.rVariable([1, 2, 3])
    a.save('a')

    os.chdir(last)

    b = cdf.archive(os.path.abspath(os.path.join(dir, 'a')))
    if 'foo' in b:
        if len(b['foo']) == 3:
            if b['foo'][0] == 1:
                if b['foo'][1] == 2:
                    if b['foo'][2] == 3:
                        return True
            print 'Incorrect value.'
        else:
            print 'Incorrect length.'
    else:
        print 'Variable missing.'
    print 'Test failed.'
    return False

if __name__ == '__main__':
    sys.exit(0 if test() else 1)
