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
printf "\x06 is not in interval \x06" > test_files/ascii3.input
printf "\x07 is in interval \x07" > test_files/ascii4.input
printf "\x0D is too \x0D" > test_files/ascii5.input
printf "\x0E is not in interval \x0E" > test_files/ascii6.input
printf "\x1A is not in interval \x1A" > test_files/ascii7.input
printf "\x1B is in the interval \x1B" > test_files/ascii8.input
printf "\x1C is not in interval \x1C" > test_files/ascii9.input
printf "\x1F is not in interval \x1F" > test_files/ascii10.input
printf "\x20 is in interval \x20" > test_files/ascii11.input
printf "\x7E is in interval \x7E" > test_files/ascii12.input
printf "\x7F is not in interval \x7F" > test_files/ascii13.input
################
printf "a \n lot \n of \n newlines" > test_files/ascii3.input
printf " spaces     " > test_files/ascii4.input
printf "Hello there escape \x1B" > test_files/ascii5.input
printf "\x1A is not in the interval" > test_files/ascii6.input
printf "109 is a number \x1B" > test_files/ascii7.input
# ISO-8859
printf "This is \377n iso file" > test_files/iso.input
# # UTF-8
#printf "This is UTF because of the character: 你好" > test_files/utf.input
# # Empty
printf "" > test_files/empty.input
# # Data
printf "Hello,\x00World!\n" > test_files/data.input
# printf "0"


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
