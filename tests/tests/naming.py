#!/usr/bin/env python

def test():
    import cdf.pythonic
    import glob
    import os
    # Implicit naming of file on disk.
# TODO make this work
#    a = cdf.pythonic.archive('a')
#    a.save()
    # Explicit naming of file on disk.
    b = cdf.pythonic.archive()
    b.save('b')
    # Combo, same name.
    c = cdf.pythonic.archive('c')
    c.save('c')
    # Combo, different names.
# TODO make this respect the passed name
#    d = cdf.pythonic.archive('d')
#    d.save('e')

    results = glob.glob('[a-z].cdf')
    print sorted(results)
    for result in results:
        os.remove(result)

if __name__ == '__main__':
    test()
