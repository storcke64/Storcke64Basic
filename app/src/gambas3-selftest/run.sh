#! /bin/sh

MYDIR=$(cd `dirname $0` && pwd)

# run Production without JIT
gbx3 -j -T "@Production" $MYDIR

if [ $? -ne 0 ] 
then
	echo Test without JIT failed
	return 1
fi

# run Production with JIT
GB_NO_JIT=01 GB_JIT_CFLAGS=-O0 gbx3 -T "@Production" $MYDIR

if [ $? -ne 0 ] 
then
	echo Test with JIT failed
	return 1
fi


