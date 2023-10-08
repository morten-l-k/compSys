echo "Testing bonus osm_id"

echo "2202162" | ../id_query_bonus ../data/20000records.tsv > test_output.txt
  echo "Expecting to find record when searching list with multiple entries:"
if grep -q "France 1.875310 46.799535" test_output.txt; then
  echo "Passed"
else
  echo "Failed"
fi

# Test record not found
echo "45" | ../id_query_bonus ../data/20000records.tsv > test_output.txt
  echo "Expecting to not find record when searching list with multiple entries where entry is not present:"
if grep -q "45: not found" test_output.txt; then
  echo "Passed"
else
  echo "Failed"
fi

#Test invalid input
echo "ef" | ../id_query_bonus ../data/20000records.tsv > test_output.txt
  echo "Expecting to not find record when given invalid input:"
if grep -q "0: not found" test_output.txt; then
  echo "Passed"
else
  echo "Failed"
fi

echo "2202162" | ../id_query_bonus ../data/1records.tsv > test_output.txt
  echo "Expecting to find record when searching list with single entry:"
if grep -q " France 1.875310 46.799535" test_output.txt; then
  echo "Passed"
else
  echo "Failed"
fi

echo "45" | ../id_query_bonus ../data/1records.tsv > test_output.txt
  echo "Expecting not to find record when searching list with single entry where entry is not present:"
if grep -q "45: not found" test_output.txt; then
  echo "Passed"
else
  echo "Failed"
fi

echo "2202162" | ../id_query_bonus ../data/0records.tsv > test_output.txt
  echo "Expecting not to find record when searching list with no entries:"
if grep -q "2202162: not found" test_output.txt; then
  echo "Passed"
else
  echo "Failed"
fi

#Test negative input
echo "-2202162" | ../id_query_bonus ../data/20000records.tsv > test_output.txt
  echo "Expecting not to find record when searching list with negative number:"
if grep -q  -- "-2202162: not found" test_output.txt; then
  echo "Passed"
else
  echo "Failed"
fi
