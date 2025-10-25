#!/bin/bash
set -e

# Prepare directories
mkdir -p test_dir2
mkdir -p big_test2

# Populate big_test2 with multiple copies of src_test
cp -r src_test big_test2/run1
cp -r src_test big_test2/run2
cp -r src_test big_test2/run3

# Add edge-case files
touch big_test2/empty.txt
dd if=/dev/urandom of=big_test2/random.bin bs=1K count=10 status=none
echo "abc123" > big_test2/small.txt

NUM_FILES=$(find ./big_test2 -type f | wc -l)
TOTAL_BYTES=$(find ./big_test2 -type f -exec wc -c {} + | tail -n1 | awk '{print $1}')

echo "Checking correctness (final histogram only)..."

# Run single-threaded, capture only the last histogram
./fhistogram ./big_test2 | tail -n 10 > test_dir2/single_final.txt

# Run multi-threaded, capture only the last histogram
./fhistogram-mt -n 4 ./big_test2 | tail -n 10 > test_dir2/multi_final.txt

# Compare final outputs
if diff test_dir2/single_final.txt test_dir2/multi_final.txt > /dev/null; then
    echo "Final outputs match — correctness OK"
else
    echo "Final outputs differ!"
    diff test_dir2/single_final.txt test_dir2/multi_final.txt
fi

echo
echo "Timing..."
for CMD in "./fhistogram ./big_test2" \
           "./fhistogram-mt -n 1 ./big_test2" \
           "./fhistogram-mt -n 2 ./big_test2" \
           "./fhistogram-mt -n 4 ./big_test2" \
           "./fhistogram-mt -n 8 ./big_test2"
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
./fhistogram-mt -n 4 empty_dir > test_dir2/empty_output.txt
./fhistogram-mt -n 2 big_test2/random.bin > test_dir2/bin_output.txt
./fhistogram-mt -n 2 big_test2/small.txt > test_dir2/small_output.txt
echo "Edge case tests done"
