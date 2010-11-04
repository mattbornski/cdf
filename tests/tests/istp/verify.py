#!/usr/bin/env python

def test():
    import cdf
    import cdf.istp
    import os
    import os.path

    test = cdf.istp.archive('test',
      skeleton = os.path.splitext(__file__)[0] + '.skt')

    # Invent some variables
    test['onedim'] = [i for i in xrange(0, 5)]
    test['twodim'] = [[i, i + 1] for i in xrange(10, 15)]
    test['threedim'] = [[[i, 2 * i], [i + 1, 2 * i + 1]] for i in xrange(20, 25)]
    # Set their attributes.  In particular, make them dependent on things.
    test['twodim'].attributes['DEPEND_1'] = 'onedim'
    test['threedim'].attributes['DEPEND_1'] = 'onedim'

    # There should be an error message, since there are mismatches in the
    # dimensions and their dependencies.
#    try:
#        test.save()
#    except cdf.istp.interface.InferenceError:
#        pass
#    else:
#        print 'no inference error'

    # Correct the inference error.
#    test['two'] = [1, 2]
    test['threedim'].attributes['DEPEND_2'] = 'two'

    # This time, there should be no errors.
    test.save()

    os.remove('test.cdf')

if __name__ == '__main__':
    test()
