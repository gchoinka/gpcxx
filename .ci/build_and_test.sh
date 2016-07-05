set -x
set -e

cd build
export CMAKE_OPTIONS="-DGPCXX_BUILD_DOCS=OFF -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DGPCXX_CHECK_HEADER_COMPILE=ON"

if [ "$COVERALLS_BUILD" == "on" ];
then
    export CMAKE_OPTIONS="$CMAKE_OPTIONS -DGPCXX_TEST_COVERAGE=ON "
fi


cmake .. $CMAKE_OPTIONS
make

if [ "$RUN_VALGRIND" == "on" ];
then 
    VALGRIND_CMD="valgrind --leak-check=full --show-reachable=yes --error-exitcode=1 "
    $VALGRIND_CMD test/util/util_tests
    $VALGRIND_CMD test/tree/tree_tests
    $VALGRIND_CMD test/io/io_tests
    $VALGRIND_CMD test/operator/operator_tests
    $VALGRIND_CMD test/generate/generate_tests
    $VALGRIND_CMD test/stat/stat_tests
    $VALGRIND_CMD test/eval/eval_tests
fi
