#!/usr/bin/env bash

set -e

cd "$(dirname "$0")"

luajitIncludesDir="$(realpath "$1")"

rm -f zxcvbn.so

g++ -I. -std=c++11 -O2 -Wall -Wextra -o dictgen dict-generate.cpp
./dictgen -o dict-src.h words-eng_wiki.txt words-female.txt words-male.txt words-passwd.txt words-surname.txt words-tv_film.txt

gcc zxcvbn.c zxcvbn-luajit.c -shared -O2 -fomit-frame-pointer -fPIC -Wall -Wextra -Wdeclaration-after-statement -I"$luajitIncludesDir" -o zxcvbn.so

rm -f dictgen
rm -f dict-src.h

exit 0