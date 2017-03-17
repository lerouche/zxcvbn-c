#!/usr/bin/env bash

set -e

cd "$(dirname "$0")"

luajitIncludesDir="$(realpath "$1")"

rm -f zxcvbn.so

if [ ! -f dict-src.h ]; then
    echo "Dictionary not found, building..."
    g++ -I. -std=c++11 -O2 -Wall -Wextra -o dictgen dict-generate.cpp
    ./dictgen -o dict-src.h words-eng_wiki.txt words-female.txt words-male.txt words-passwd.txt words-surname.txt words-tv_film.txt
fi

echo "Compiling module..."
gcc zxcvbn.c zxcvbn-luajit.c -shared -O2 -fomit-frame-pointer -fPIC -Wall -Wextra -I"$luajitIncludesDir" -lm -o zxcvbn.so

rm -f dictgen

exit 0
