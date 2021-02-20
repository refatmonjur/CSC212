#!/bin/bash

[[ -x check ]] || make check
./check | csplit -s --suppress-matched --prefix=out -n1 - '/@@@/' '{*}'
if diff -q out0 out1; then
	echo -e "\033[32mSUCCESS :D\033[0m"
else
	diff -u1 out0 out1
fi
