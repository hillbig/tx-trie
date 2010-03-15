lib_LTLIBRARIES = libtx.la
libtx_la_CPPFLAGS = -D_REENTRANT
libtx_la_LIBADD = -lz
libtx_la_SOURCES = \
  ssv.cpp ssv.hpp \
  tx.cpp tx.hpp

pkginclude_HEADERS = tx.hpp ssv.hpp

bin_PROGRAMS = txsearch txbuild txsearch_mmap s2sbuild s2ssearch txlist

txsearch_CPPFLAGS = -D_REENTRANT
txsearch_LDADD = libtx.la
txsearch_SOURCES = txsearch.cpp

txbuild_CPPFLAGS = -D_REENTRANT
txbuild_LDADD = libtx.la
txbuild_SOURCES = txbuild.cpp

txsearch_mmap_CPPFLAGS = -D_REENTRANT
txsearch_mmap_LDADD = libtx.la
txsearch_mmap_SOURCES = txsearch_mmap.cpp

s2sbuild_CPPFLAGS = -D_REENTRANT
s2sbuild_LDADD = libtx.la
s2sbuild_SOURCES = s2sbuild.cpp

s2ssearch_CPPFLAGS = -D_REENTRANT
s2ssearch_LDADD = libtx.la
s2ssearch_SOURCES = s2ssearch.cpp

txlist_CPPFLAGS = -D_REENTRANT
txlist_LDADD = libtx.la
txlist_SOURCES = txlist.cpp

EXTRA_DIST = tx.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = tx.pc


