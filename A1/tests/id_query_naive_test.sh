

echo "2202162" | ../id_query_naive ../data/20000records.tsv > test_output.txt

if grep -q " France 1.875310 46.799535" test_output.txt; then
  echo "Test case 1:  Passed"
else
  echo "Test case 1:  Failed"
fi

# Test record not found
echo "45" | ../id_query_naive ../data/20000records.tsv > test_output.txt

if grep -q "45: not found" test_output.txt; then
  echo "Test case 2; Passed"
else
  echo "Test case 2: Failed"
fi

#Test invalid input
echo "ef" | ../id_query_naive ../data/20000records.tsv > test_output.txt

if grep -q "0: not found" test_output.txt; then
  echo "Test case 3; Passed"
else
  echo "Test case 3: Failed"
fi
#Test negative input
echo "-2202162" | ../id_query_naive ../data/20000records.tsv > test_output.txt

if grep -q  -- "-2202162: not found" test_output.txt; then
  echo "Test case 4; Passed"
else
  echo "Test case 4: Failed"
fi

