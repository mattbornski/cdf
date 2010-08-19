#!/usr/bin/env python

def test():
    import cdf.pythonic
    import glob
    import os

    test = cdf.pythonic.archive()
    test['foo'] = [1, 2, 3, 4, 5]
    test['bar'] = ['a', 'b', 'see', 'd', 'e']
    test.attributes['Description'] = 'A test archive in Common Data Format'
    test['foo'].attributes['Creation order'] = 1
    test['bar'].attributes['Creation order'] = 'two'
    test.save('test2')

    test2 = cdf.pythonic.archive('test2')
    print test2.keys()
    print test2['foo']
    print test2.attributes
    print test2['bar'].attributes

    results = glob.glob('test*.cdf')
    for result in results:
        os.remove(result)

if __name__ == '__main__':
    test()
