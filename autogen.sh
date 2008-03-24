#!/bin/sh
aclocal
libtoolize --force
automake --add-missing
autoconf
