CDF - a quick tutorial
======================

>>> import cdf
>>> test = cdf.archive()
>>> test['foo'] = [1, 2, 3, 4, 5]
>>> test['bar'] = ['a', 'b', 'see', 'd', 'e']
>>> test.attributes['Description'] = 'A test archive in Common Data Format'
>>> test['foo'].attributes['Creation order'] = 1
>>> test['bar'].attributes['Creation order'] = 'two'
>>> test.save('test2')
>>> test2 = cdf.archive('test2')
>>> print test2.keys()
['foo', 'bar']
>>> print test2['foo']
[record(1), record(2), record(3), record(4), record(5)]
>>> print test2.attributes
{'Description': ['A test archive in Common Data Format']}
>>> print test2['bar'].attributes
{'Creation order': 'two'}
