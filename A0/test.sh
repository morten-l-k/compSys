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
printf "\x07 is in interval \x07" > test_files/ascii3.input
printf "\x0D is too \x0D" > test_files/ascii4.input
printf "\x1B is in the interval \x1B" > test_files/ascii5.input
printf "\x20 is in interval \x20" > test_files/ascii6.input
printf "\x7E is in interval \x7E" > test_files/ascii7.input
# # ISO-8859
printf "This is \377n iso file" > test_files/iso.input
printf "\xFF\xFF\xFF\xFF" > test_files/iso.input
printf "\x1B\x1B\x1B\x1B\x1B\xA1\xFD" > test_files/iso2.input
printf "\x20\x7E\xA0\xFF" > test_files/iso3.input
printf "\xA0 is in the interval \xA0" > test_files/iso4.input 
printf "\xFF is in the interval \xFF" > test_files/iso5.input 
printf "\xC8 is in the interval \xCF" > test_files/iso6.input 
# # # UTF-8
printf "This is UTF because of the character: 你好" > test_files/utf1.input
printf "🤣🤣🤣🤣🤣🤣" > test_files/utf2.input
printf "你aaaaaa" > test_files/utf3.input
# # # Empty
printf "" > test_files/empty.input
# # Data
printf "Hello,\x00World!\n" > test_files/data.input
printf "0"
# I/O errors 
printf "THIS IS A SECRET FILE" > test_files/secret.input
chmod -r test_files/secret.input


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
