#  Makefile for CRM114 library
#
# Copyright 2010 Kurt Hackenberg & William S. Yerazunis, each individually
# with full rights to relicense.
#
#   This file is part of the CRM114 Library.
#
#   The CRM114 library is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   The CRM114 Library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public License
#   along with the CRM114 Library.  If not, see <http://www.gnu.org/licenses/>.


#C flags below are for GCC 4.3.2.


#Use one of the following sets of options for generating code, debug
#information, and profiling information.

#GCC flags: no debugging, optimize, inline matrix library
#Defining DO_INLINES turns on use of a GCC extension in the matrix library,
#so don't turn it on for other compilers.
#CFLAGS += -O3 -DDO_INLINES
#
#GCC flags for debugging, no optimization
#CFLAGS += -g
#
#GCC and LD flags for debugging, no optimization, and profile for speed
#CFLAGS += -g -pg
#LDFLAGS += -pg
#
#GCC and LD flags for debugging, no optimization, and profile for coverage
#CFLAGS += -g -fprofile-arcs -ftest-coverage
#LDFLAGS += -fprofile-arcs -ftest-coverage
# LDFLAGS += -static -static-libgcc

CFLAGS += -Iinclude -fPIC

#always use this: C99, and check source code carefully
CFLAGS += -std=c99 -pedantic -Wall -Wextra -Wpointer-arith -Wstrict-prototypes
#well, pretty carefully
CFLAGS += -Wno-sign-compare -Wno-overlength-strings

#These are optional.

#warn about any type conversion that could possibly change a value
#CFLAGS += -Wconversion
#warn about variable-length arrays, which Microsoft C (C89 w/ ext) can't handle
#CFLAGS += -Wvla
#warn about undefined macro in #if value
#CFLAGS += -Wundef
#warn about structures marked packed that had no padding anyway
#CFLAGS += -Wpacked
#tell us when padding a structure
#CFLAGS += -Wpadded
#tell us when denying an inline request
#CFLAGS += -Winline

LIBHDR_FILES =					\
crm114_lib.h					\
crm114_sysincludes.h				\
crm114_config.h					\
crm114_structs.h				\
crm114_internal.h				\
crm114_regex.h					\
crm114_svm.h					\
crm114_svm_lib_fncts.h				\
crm114_pca.h					\
crm114_pca_lib_fncts.h				\
crm114_matrix.h					\
crm114_matrix_util.h				\
crm114_svm_quad_prog.h				\
crm114_datalib.h

LIBHDRS = $(foreach hdr, $(LIBHDR_FILES), include/$(hdr))

LIBOBJ_FILES =					\
crm114_base.o					\
crm114_markov.o					\
crm114_markov_microgroom.o			\
crm114_bit_entropy.o				\
crm114_svm.o					\
crm114_svm_lib_fncts.o				\
crm114_svm_quad_prog.o				\
crm114_fast_substring_compression.o		\
crm114_pca.o					\
crm114_pca_lib_fncts.o				\
crm114_matrix.o					\
crm114_matrix_util.o				\
crm114_datalib.o				\
crm114_vector_tokenize.o			\
crm114_strnhash.o				\
crm114_util.o					\
crm114_regex_tre.o

LIBC_FILES =					\
crm114_base.c					\
crm114_markov.c					\
crm114_markov_microgroom.c			\
crm114_bit_entropy.c				\
crm114_svm.c					\
crm114_svm_lib_fncts.c				\
crm114_svm_quad_prog.c				\
crm114_fast_substring_compression.c		\
crm114_pca.c					\
crm114_pca_lib_fncts.c				\
crm114_matrix.c					\
crm114_matrix_util.c				\
crm114_datalib.c				\
crm114_vector_tokenize.c			\
crm114_strnhash.c				\
crm114_util.c					\
crm114_regex_tre.c

LIBOBJS = $(foreach obj, $(LIBOBJ_FILES), lib/$(obj))

STATIC_NAME = libcrm114.a
LIB = lib/$(STATIC_NAME)
LIBLD_PATH = $(abspath lib)

all: $(LIB)

$(LIBOBJS): $(LIBHDRS) Makefile

$(LIB): $(LIBOBJS) Makefile
	ar rcs libcrm114.a $(LIBOBJS)
	
lib/%.o: lib/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean: clean_lib clean_profiling

clean_lib:
	rm -f $(LIB) $(LIBOBJS) $(STATIC_NAME)

clean_profiling:
	rm -f gmon.out *.gcov {,lib/}*.gcno {,lib/}*.gcda
