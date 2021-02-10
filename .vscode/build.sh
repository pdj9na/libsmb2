#!/bin/bash

. ../libfuse/.vscode/build_util.sh

 __ACFILES_="configure.ac\
 .vscode/build.sh"
 
__AMFILES_=" Makefile.am\
 examples/Makefile.am\
 include/Makefile.am\
 lib/Makefile.am"

fun_changes "$__ACFILES_ $__AMFILES_" -f

rm -f config.status
if fun_isChangeFromMulti configure.ac;then
	type autoreconf &>/dev/null && autoreconf -vi
fi

#if fun_isChangeFromMulti configure.ac;then
if fun_isChangeFromMulti "$__ACFILES_";then
	_args="--without-libkrb5 --disable-werror --enable-examples --disable-rpath"

	if type busybox &>/dev/null && test `busybox uname -o` = Android ||
	`uname -m` = aarch64 || `uname -m` = aarch;then
		export CONFIG_SHELL=/system/bin/sh
		test `uname -m` = aarch && _args="--target=aarch-linux-android "$_args
		test `uname -m` = aarch64 && _args="--target=aarch64-linux-android "$_args
	else :
		#_args=$_args" --enable-asan"
	fi
	
	#_args=$_args" --enable-debug"
	sh ./configure $_args
fi

#fun_whereMakeClean "configure.ac $__AMFILES_"
fun_whereMakeClean "$__ACFILES_ $__AMFILES_"

make -j4
