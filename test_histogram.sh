#!/bin/bash
set -e

mkdir -p test_dir
mkdir -p big_test

cp -r src_test big_test/run1
cp -r src_test big_test/run2
cp -r src_test big_test/run3

touch big_test/empty.txt
dd if=/dev/urandom of=big_test/random.bin bs=1K count=10 status=none
echo "abc123" > big_test/small.txt

NUM_FILES=$(find ./big_test -type f | wc -l)
TOTAL_BYTES=$(find ./big_test -type f -exec wc -c {} + | tail -n1 | awk '{print $1}')

echo "Checking correctness..."
./fhistogram ./big_test > test_dir/single.txt
./fhistogram-mt -n 4 ./big_test > test_dir/multi.txt

sort test_dir/single.txt > test_dir/single_sorted.txt
sort test_dir/multi.txt > test_dir/multi_sorted.txt

if diff test_dir/single_sorted.txt test_dir/multi_sorted.txt > /dev/null; then
    echo "Outputs match — correctness OK"
else
    echo "Outputs differ!"
    diff test_dir/single_sorted.txt test_dir/multi_sorted.txt | head -n 20
fi

echo
echo "Timing..."
for CMD in "./fhistogram ./big_test" \
           "./fhistogram-mt -n 1 ./big_test" \
           "./fhistogram-mt -n 2 ./big_test" \
           "./fhistogram-mt -n 4 ./big_test" \
           "./fhistogram-mt -n 8 ./big_test"
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

echo
echo "Edge case tests..."
mkdir -p empty_dir
./fhistogram-mt -n 4 empty_dir > test_dir/empty_output.txt
./fhistogram-mt -n 2 big_test/random.bin > test_dir/bin_output.txt
./fhistogram-mt -n 2 big_test/small.txt > test_dir/small_output.txt
echo "Edge case tests done"
