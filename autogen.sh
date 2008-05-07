#!/bin/sh

aclocal \
  && autoheader \
  && libtoolize --force --copy \
  && automake --add-missing --foreign --copy \
  && autoconf

