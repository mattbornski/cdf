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
    a['foo'] = [1, 2, 3]
    a.attributes['varname'] = 'foo'
    a.save('a')

    os.chdir(last)

    b = cdf.archive(os.path.abspath(os.path.join(dir, 'a')))
    if 'varname' in b.attributes:
        name = b.attributes['varname'][0]
        if name in b:
            if len(b[name]) == 3:
                if b[name][0] == 1:
                    if b[name][1] == 2:
                        if b[name][2] == 3:
                            return True
                print 'Incorrect value.'
            else:
                print 'Incorrect length.'
        else:
            print 'Incorrect variable name.'
    else:
        print 'Attribute missing.'
    print 'Test failed.'
    return False

if __name__ == '__main__':
    sys.exit(0 if test() else 1)
