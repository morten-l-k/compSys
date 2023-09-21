#!/usr/bin/env bash

# Exit immediately if any command below fails.
set -e

make


echo "Generating a test_files directory.."
mkdir -p test_files
rm -f test_files/*


echo "Generating test files.."
# Ascii
printf "Hello, World!\n" > test_files/ascii.input
printf "Hello, World!" > test_files/ascii2.input
printf "\x06 is not interval \x06" > test_files/ascii3.input
printf "a \n lot \n of \n newlines" > test_files/ascii3.input
printf " spaces     " > test_files/ascii4.input
printf "Hello there escape \x1B" > test_files/ascii5.input
printf "\x1A is not in the interval" > test_files/ascii6.input
printf "109 is a number \x1B" > test_files/ascii7.input
#ISO-8859
printf "This is \377n iso file" > test_files/iso.input
# UTF-8
printf "This is UTF because of the character: 你好" > test_files/utf.input
# Empty
printf "" > test_files/empty.input
# Data
printf "Hello,\x00World!\n" > test_files/data.input
printf "0"
### TODO: Generate more test files ###


echo "Running the tests.."
exitcode=0
for f in test_files/*.input
do
  echo ">>> Testing ${f}.."
  file    ${f} | sed -e 's/ASCII text.*/ASCII text/' \
                         -e 's/UTF-8 Unicode text.*/UTF-8 Unicode text/' \
                         -e 's/ISO-8859 text.*/ISO-8859 text/' \
                         -e 's/writable, regular file, no read permission/cannot determine (Permission denied)/' \
                         > "${f}.expected"
  ./file  "${f}" > "${f}.actual"

  if ! diff -u "${f}.expected" "${f}.actual"
  then
    echo ">>> Failed :-("
    exitcode=1
  else
    echo ">>> Success :-)"
  fi
done
exit $exitcode
