#!/bin/bash
mkdir -p test_dir

echo "Checking correctness..."
./fauxgrep malloc ./src_test > test_dir/single.txt
./fauxgrep-mt -n 4 malloc ./src_test > test_dir/multi.txt
sort test_dir/single.txt > test_dir/single_sorted.txt
sort test_dir/multi.txt > test_dir/multi_sorted.txt
diff test_dir/single_sorted.txt test_dir/multi_sorted.txt && echo "Outputs match!"

echo "Timing..."
time ./fauxgrep malloc ./src_test > /dev/null
time ./fauxgrep-mt -n 1 malloc ./src_test > /dev/null
time ./fauxgrep-mt -n 2 malloc ./src_test > /dev/null
time ./fauxgrep-mt -n 4 malloc ./src_test > /dev/null
time ./fauxgrep-mt -n 8 malloc ./src_test > /dev/null
