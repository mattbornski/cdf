#!/usr/bin/env python

def test():
    import cdf
    import glob
    import os
    # Implicit naming of file on disk.
    a = cdf.archive('a')
    a.save()
    # Explicit naming of file on disk.
    b = cdf.archive()
    b.save('b')
    # Combo, same name.
    c = cdf.archive('c')
    c.save('c')
    # Combo, different names.
    d = cdf.archive('d')
    d.save('e')

    results = glob.glob('[a-z].cdf')
    print sorted(results)
    for result in results:
        os.remove(result)

if __name__ == '__main__':
    test()
