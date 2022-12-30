#!/bin/bash

set -e

CC=gcc
CFLAGS=-Wall
LINKER=-lm
SRC=../src
BIN=bin
EXECUTABLE_NAME=sp

clean_build() {
	if [ -d "$BIN" ]; then
		rm -rf $BIN
	fi

	if [ -f "test.png" ]; then
		rm test.png
	fi	
}

run() {
	if [ -f "$BIN/$EXECUTABLE_NAME" ]; then
		$BIN/$EXECUTABLE_NAME
	else
		echo "No executable. Build first!"
	fi
}

build() {
	if [ ! -d "$BIN" ]; then
		mkdir $BIN
	fi

	pushd $BIN
	$CC $CFLAGS $SRC/main.c $LINKER -o $EXECUTABLE_NAME
	popd
}

case $1 in
	"clean")
		clean_build
		;;
	"run")
		run
		;;
	"")
		build
		;;
esac

