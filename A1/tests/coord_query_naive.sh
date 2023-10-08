echo "Testing coord"

echo "1.8 46.7" | ../coord_query_naive ../data/20000records.tsv > test_output.txt
  echo "Expecting to find record when searching list with multiple entries:"
if grep -q "(1.800000,46.700000): France (1.875310,46.799535)" test_output.txt; then
  echo "Passed"
else
  echo "Failed"
fi

echo "1.8 46.7" | ../coord_query_naive ../data/1records.tsv > test_output.txt
  echo "Expecting to find record when searching list with single entry:"
if grep -q "(1.800000,46.700000): France (1.875310,46.799535)" test_output.txt; then
  echo "Passed"
else
  echo "Failed"
fi

echo "1" | ../coord_query_naive ../data/0records.tsv > test_output.txt
  echo "Expecting not to find record when searching list with no entries:"
if grep -q "(1.000000,0.000000): (null) (0.000000,0.000000)" test_output.txt; then
  echo "Passed"
else
  echo "Failed"
fi
