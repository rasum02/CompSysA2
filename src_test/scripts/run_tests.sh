#!/bin/bash

NO_PROGRAMS=()
ID_PROGRAMS=("id_query_naive" "id_query_indexed" "id_query_binsort")
COORD_PROGRAMS=("coord_query_naive" "coord_query_kdtree")
coord_query_naive=("coord_query_naive")
coord_query_kdtree=("coord_query_kdtree")
QUERY_DIR="../queries"
EXPECTED_DIR="../expected"

for prog in "${ID_PROGRAMS[@]}"; do
  echo ""
  echo "Testing $prog..."
  
  # =================================================================== ID_QUERY_NAIVE, ID_QUERY_INDEXED, ID_QUERY_BINSORT ===================================================================

  # ====== Basic Functionality and Program Logic Tests ======

  # Test basic queries on large dataset
  echo "- Basic queries"
  ./../$prog ../records/25000records.tsv < $QUERY_DIR/basic_queries.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  diff output.txt $EXPECTED_DIR/basic_expected.txt && echo "PASS" || echo "FAIL"

  # Test edge queries on large dataset
  echo "- Edge queries"
  ./../$prog ../records/25000records.tsv < $QUERY_DIR/edge_queries.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  diff output.txt $EXPECTED_DIR/edge_expected.txt && echo "PASS" || echo "FAIL"

	# Test repeated queries on large dataset
	echo "- Repeated queries"
	./../$prog ../records/25000records.tsv < $QUERY_DIR/repeat_queries.txt \
			| grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
	diff output.txt $EXPECTED_DIR/repeat_expected.txt && echo "PASS" || echo "FAIL"

  # Test empty query file on large dataset
	echo "- Empty query file"
	./../$prog ../records/25000records.tsv < $QUERY_DIR/empty.txt \
		| grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
	if [ ! -s output.txt ]; then
		echo "PASS"
	else
		echo "FAIL"
	fi

   # Test non-existing queries on large dataset
  echo "- Non-existing queries"
  ./../$prog ../records/25000records.tsv < $QUERY_DIR/non_existing.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  if grep -q "not found" output.txt; then
    echo "PASS"
  else
    echo "FAIL"
  fi

  # Test random queries on large dataset
  echo "- Random queries"
  ./../$prog ../records/25000records.tsv < $QUERY_DIR/random_queries.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  diff output.txt $EXPECTED_DIR/random_expected.txt && echo "PASS" || echo "FAIL"










  # ====== Parsing Tests ======

	# Test only header in records file
	echo "- Only header"
	./../$prog ../records/only_header.tsv < $QUERY_DIR/basic_queries.txt \
		| grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
	if grep -q "not found" output.txt; then
		echo "PASS"
	else
		echo "FAIL"
	fi

  # Test invalid input (wrong header file) 
  echo "- Wrong header file"
  if ./../$prog ../records/wrong_header.tsv < $QUERY_DIR/small_queries.txt 2> err.txt; then
    echo "FAIL"
  else
    grep -q "Failed to read input" err.txt && echo "PASS" || echo "FAIL"
  fi

  # Test empty records file
  echo "- Empty records file"
  ./../$prog ../records/empty.tsv < $QUERY_DIR/basic_queries.txt \
		| grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
	if grep -q "not found" output.txt; then
		echo "PASS"
	else
		echo "FAIL"
	fi





  # ===== Small Dataset Tests ======

  # Test small dataset with known outputs
  echo "- Small dataset queries"
  ./../$prog ../records/small.tsv < $QUERY_DIR/small_queries.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  diff output.txt $EXPECTED_DIR/small_expected.txt && echo "PASS" || echo "FAIL"


  # Test small dataset, with queries that are not found
  echo "- Small dataset, queries not found"
  ./../$prog ../records/small.tsv < $QUERY_DIR/small_notfound.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  diff output.txt $EXPECTED_DIR/small_notfound_expected.txt && echo "PASS" || echo "FAIL"

  # Test small - different order (should behave same as the other small test)
  echo "- Small with different order"
  ./../$prog ../records/small_different_order.tsv < $QUERY_DIR/small_queries.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  diff output.txt $EXPECTED_DIR/small_expected.txt && echo "PASS" || echo "FAIL"

  # Query 100 IDs in small.tsv
  echo "- Query 100 IDs in small dataset"
  seq 1 100 > $QUERY_DIR/all_ids.txt
  ./../$prog ../records/small.tsv < $QUERY_DIR/all_ids.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  # Count "not found" occurrences
  if grep -q "not found" output.txt; then
    echo "PASS"
  else
    echo "FAIL"
  fi

  # Test lookup of negative and huge IDs
  echo "- Extreme ID queries in small dataset"
  echo -e "-1\n9223372036854775807" > $QUERY_DIR/extreme_ids.txt
  ./../$prog ../records/small.tsv < $QUERY_DIR/extreme_ids.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  if grep -q "not found" output.txt; then
    echo "PASS"
  else
    echo "FAIL"
  fi






  # ===== Performance and Memory ======

  # Valgrind memory check
  echo ""
  echo "- Memory check for basic queries"
  valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./../$prog ../records/25000records.tsv < $QUERY_DIR/basic_queries.txt
  echo ""
  echo "- Memory check for edge queries"
  valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./../$prog ../records/25000records.tsv < $QUERY_DIR/edge_queries.txt
  echo ""
  echo "- Memory check for repeated queries"
  valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./../$prog ../records/25000records.tsv < $QUERY_DIR/repeat_queries.txt
  echo ""
  echo "- Memory check for random queries"
  valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./../$prog ../records/25000records.tsv < $QUERY_DIR/random_queries.txt
done


for prog in "${COORD_PROGRAMS[@]}"; do
  echo ""
  echo "Testing $prog..."

# ========================================================================= COORD_QUERY_NAIVE, COORD_QUERY_KDTREE =========================================================================

# ====== Basic Functionality and Program Logic Tests ======

  # Test basic queries on large dataset
  echo "- Basic queries"
  ./../$prog ../records/25000records.tsv < $QUERY_DIR/coord_basic_queries.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  diff output.txt $EXPECTED_DIR/coord_basic_expected.txt && echo "PASS" || echo "FAIL"

  # Test edge queries on large dataset
  echo "- Edge queries"
  ./../$prog ../records/25000records.tsv < $QUERY_DIR/coord_edge_queries.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  diff output.txt $EXPECTED_DIR/coord_edge_expected.txt && echo "PASS" || echo "FAIL"

	# Test repeated queries on large dataset
	echo "- Repeated queries"
	./../$prog ../records/25000records.tsv < $QUERY_DIR/coord_repeat_queries.txt \
			| grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
	diff output.txt $EXPECTED_DIR/coord_repeat_expected.txt && echo "PASS" || echo "FAIL"

  # Test lookup of negative and huge IDs
  echo "- Extreme ID queries"
  echo -e "-115333333333 -13166666666666\n92233720368 9223372036" > $QUERY_DIR/extreme_ids.txt
  ./../$prog ../records/25000records.tsv < $QUERY_DIR/extreme_ids.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  diff output.txt $EXPECTED_DIR/coord_extreme_expected.txt && echo "PASS" || echo "FAIL"








  # ====== Parsing Tests ======

	# Test only header in records file
	echo "- Only header"
	./../$prog ../records/only_header.tsv < $QUERY_DIR/coord_basic_queries.txt \
		| grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
	if grep -q "not found" output.txt; then
		echo "PASS"
	else
		echo "FAIL"
	fi

  # Test invalid input (wrong header file) 
  echo "- Wrong header file"
  if ./../$prog ../records/wrong_header.tsv < $QUERY_DIR/coord_basic_queries.txt 2> err.txt; then
    echo "FAIL"
  else
    grep -q "Failed to read input" err.txt && echo "PASS" || echo "FAIL"
  fi

  # Test empty records file
  echo "- Empty records file"
  ./../$prog ../records/empty.tsv < $QUERY_DIR/coord_basic_queries.txt \
		| grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
	if grep -q "not found" output.txt; then
		echo "PASS"
	else
		echo "FAIL"
	fi





  # ===== Small Dataset Tests ======

  # Test small dataset with known outputs
  echo "- Small dataset queries"
  ./../$prog ../records/small.tsv < $QUERY_DIR/coord_small_queries.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  diff output.txt $EXPECTED_DIR/coord_small_expected.txt && echo "PASS" || echo "FAIL"

  # Test small dataset with basic queries (should return differently compared to coord_basic_expected.txt)
  echo "- Small dataset with basic queries"
  ./../$prog ../records/small.tsv < $QUERY_DIR/coord_basic_queries.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  diff output.txt $EXPECTED_DIR/coord_small_basic_queries_expected.txt && echo "PASS" || echo "FAIL"

  # Test small - different order (should behave same as the other small test)
  echo "- Small with different order"
  ./../$prog ../records/small_different_order.tsv < $QUERY_DIR/coord_small_queries.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  diff output.txt $EXPECTED_DIR/coord_small_expected.txt && echo "PASS" || echo "FAIL"







  # ===== Performance and Memory ======

  # Valgrind memory check
  echo ""
  echo "- Memory check for basic queries"
  valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./../$prog ../records/25000records.tsv < $QUERY_DIR/coord_basic_queries.txt
  echo ""
  echo "- Memory check for edge queries"
  valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./../$prog ../records/25000records.tsv < $QUERY_DIR/coord_edge_queries.txt
  echo ""
  echo "- Memory check for repeated queries"
  valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./../$prog ../records/25000records.tsv < $QUERY_DIR/coord_repeat_queries.txt
  echo ""
  echo "- Memory check for small dataset queries"
  valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./../$prog ../records/small.tsv < $QUERY_DIR/coord_small_queries.txt

done



  # ================================================================ Special Case Tests =============================================

for prog in "${coord_query_naive[@]}"; do
  echo ""
  echo "Testing $prog... (Special Cases)"

  # Test Japan (Should return Japan)
  echo "- Japan Test"
  ./../$prog ../records/25000records.tsv < $QUERY_DIR/coord_japan.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  diff output.txt $EXPECTED_DIR/coord_naive_japan_expected.txt && echo "PASS" || echo "FAIL"

  echo ""
  echo "- Memory check for basic queries"
  valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./../$prog ../records/25000records.tsv < $QUERY_DIR/coord_japan.txt
done

for prog in "${coord_query_kdtree[@]}"; do
  echo ""
  echo "Testing $prog... (Special Cases)"

  # Test Japan (Should return Hokkaido)
  echo "- Japan Test"
  ./../$prog ../records/25000records.tsv < $QUERY_DIR/coord_japan.txt \
    | grep -Ev "Reading records|Building index|Query time|Total query runtime" > output.txt
  diff output.txt $EXPECTED_DIR/coord_kdtree_japan_expected.txt && echo "PASS" || echo "FAIL"

  echo ""
  echo "- Memory check for basic queries"
  valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./../$prog ../records/25000records.tsv < $QUERY_DIR/coord_japan.txt
done

# Cleanup
rm -f output.txt err.txt
echo "Testing complete."