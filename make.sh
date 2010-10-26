#!/bin/bash

BASE=`readlink -f \`dirname $0\``

pushd $BASE

# Documentation generation.
#pushd docs/package
#make html
#cd _build/html
#zip -r sphinx .
#mv sphinx.zip $BASE
#popd

# Package building.
# python setup.py build

# Package testing.
# diffval .

# Upload package and documentation.
python setup.py sdist bdist upload build_sphinx upload_sphinx

popd
