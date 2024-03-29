import setuptools
import glob
import os.path

if __name__ == '__main__':
    # List the source files from the CDF v3.3 distribution.
    # CDF stands for Common Data Format.  The source code
    # included below is part of the NASA Goddard Space Flight
    # Center (GSFC)'s package.  To the best of my knowledge it
    # is unmodified from the original sources.  The copyright
    # statement is reproduced herein:
    #
    #################################
    # Copyright 2009
    # Space Physics Data Facility
    # NASA/Goddard Space Flight Center
    # 
    # This software may be copied or redistributed as long as it is not sold
    # for profit, but it can be incorporated into any other substantive
    # product with or without modifications for profit or non-profit.  If the 
    # software is modified, it must include the following notices:
    # 
    #   - The software is not the original (for protectiion of the original
    #     author's reputations from any problems introduced by others)
    # 
    #   - Change history (e.g. date, functionality, etc.)
    # 
    # This copyright notice must be reproduced on each copy made. This
    # software is provided as is without any express or implied warranties
    # whatsoever. 
    #################################
    #
    # The NASA GSFC CDF software is referred to be a variety of names
    # throughout the documentation, but in the context of this Python
    # package it includes any C source or header file in the "cdf33-dist"
    # directory.  
    #
    # Note that the Python package you are examining right now is
    # not the CDF package released by Goddard but does incorporate
    # substantial portions of the Goddard software.  To use the
    # Python package in any meaningful way is to use the Goddard
    # CDF library.  Likewise, to distribute or incorporate the
    # Python package is to distribute of incorporate the CDF
    # library.  The copyright applied to the Python package may
    # be different than the copyright applied to the CDF library.
    # It is your responsibility to ensure that you are in compliance
    # with copyright law.
    cdf33_distribution_sources = glob.glob(
      os.path.join('cdf33-dist',
        os.path.join('src', os.path.join('lib', '*.c'))))
    cdf33_distribution_sources.remove(
      os.path.join('cdf33-dist',
        os.path.join('src', os.path.join('lib', 'libmain.c'))))
    # End of NASA GSFC CDF library source files.

    # Define a C extension.
    internal = [setuptools.Extension(
      'cdf.internal',
      sources = [
        # The core code for the Python extension
        'internal.c'] \
        # The unadulterated source code from the CDF distribution.
        # While the Python extension itself is not part of the CDF
        # distribution, it incorporates significant portions of the
        # code from the distribution.  Any use you make of the Python
        # extensions is likely to be subject to the licensing and
        # copyright restrictions of both the Python extension and the
        # CDF distribution.
        + cdf33_distribution_sources,
      include_dirs = [
        'cdf33-dist/src/include',
      ],
      extra_compile_args = [
        '-D_FILE_OFFSET_BITS=64',
        '-D_LARGEFILE64_SOURCE',
        '-D_LARGEFILE_SOURCE',
        '-DMALLOC_CHECK_=0',
      ],
      libraries = ['m'],
    )]

    # Define a Python extension module.  This is a lot easier to
    # do than a C extension module because Python has a lot more
    # insight into the workings of Python than it does C.
    pythonic = [
      'cdf.interface',
      'cdf.framework',
      'cdf.attribute',
      'cdf.entry',
      'cdf.typing',
    ]

    # Define another Python extension.  This represents the "standard"
    # interface as described in the docs.
    standard = ['cdf.standard']

    # Define another Python extension.  This extension validates ISTP
    # compliance and fills in ISTP attributes based on "skeleton files".
    # Presently, my "skeleton files" are just Python dictionaries.  It
    # would be a great TODO to make this operate from existing CDF
    # skeletons, too.
    istp = ['cdf.istp.interface']

    # Invoke the setup code, which will (depending on the command line
    # arguments) build, install, or otherwise tinker with this package
    # on this system.
    setuptools.setup(
      name = 'CDF',
      version = '0.32',
      description = 'This package handles files in NASA Common Data Format',
      author = 'Matt Bornski',
      author_email = 'matt@bornski.com',
      url = 'https://github.com/mattbornski/cdf',
      classifiers = [
        'Development Status :: 4 - Beta', 
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: Apache Software License',
        'Topic :: Scientific/Engineering :: Astronomy',
      ],
      ext_modules = internal,
      py_modules = standard + pythonic + istp,
      requires = ['numpy (>=1.4)'],
    )
