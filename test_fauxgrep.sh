#!/bin/bash
mkdir -p test_dir
mkdir -p big_test
cp -r src_test/* big_test/
cp -r src_test/* big_test/
cp -r src_test/* big_test/

NUM_FILES=$(find ./big_test -type f | wc -l)
TOTAL_BYTES=$(find ./big_test -type f -exec wc -c {} + | tail -n1 | awk '{print $1}')

echo "Checking correctness..."
./fauxgrep malloc ./big_test > test_dir/single.txt
./fauxgrep-mt -n 4 malloc ./big_test > test_dir/multi.txt
sort test_dir/single.txt > test_dir/single_sorted.txt
sort test_dir/multi.txt > test_dir/multi_sorted.txt
diff test_dir/single_sorted.txt test_dir/multi_sorted.txt && echo "Outputs match!"

echo
echo "Timing..."
for CMD in "./fauxgrep malloc ./big_test" \
           "./fauxgrep-mt -n 1 malloc ./big_test" \
           "./fauxgrep-mt -n 2 malloc ./big_test" \
           "./fauxgrep-mt -n 4 malloc ./big_test" \
           "./fauxgrep-mt -n 8 malloc ./big_test"
do
    echo "--- $CMD ---"
    START=$(date +%s.%N)
    $CMD > /dev/null
    END=$(date +%s.%N)
    RUNTIME=$(echo "$END - $START" | bc)
    echo "Runtime: $RUNTIME s"
    echo "Files/sec: $(echo "$NUM_FILES / $RUNTIME" | bc -l)"
    echo "Bytes/sec: $(echo "$TOTAL_BYTES / $RUNTIME" | bc -l)"
    echo
done
