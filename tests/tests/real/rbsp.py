#!/usr/bin/env python

import cdf
import os.path

def test():
    a = cdf.archive(os.path.splitext(__file__)[0])

if __name__ == '__main__':
    test()
