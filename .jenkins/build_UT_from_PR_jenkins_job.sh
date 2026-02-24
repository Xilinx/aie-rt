#!/bin/bash

# Jenkins already provides these variables and has merged the PR
echo "GITHUB_PR_TRIGGER_SENDER_AUTHOR: $GITHUB_PR_TRIGGER_SENDER_AUTHOR"
echo "GITHUB_PR_URL: $GITHUB_PR_URL"
echo "GITHUB_PR_NUMBER: $GITHUB_PR_NUMBER"
echo "Source Branch: $GITHUB_PR_SOURCE_BRANCH"
echo "Target Branch: $GITHUB_PR_TARGET_BRANCH"

# Current directory already contains the merged PR code
# Jenkins has already done: git clone + git checkout + git merge
echo "Current workspace already contains merged PR code at: $(pwd)"
WORKSPACE_DIR=$(pwd)

# Clone and build cpputest framework
echo "Cloning and building cpputest..."
cd .. || {
    echo "Error: Failed to change to parent directory"
    exit 1
}

rm -rf cpputest
CPPUTEST_DIR="$(pwd)/cpputest"
git clone https://gitenterprise.xilinx.com/ai-engine/cpputest.git $CPPUTEST_DIR || {
    echo "Error: Failed to clone cpputest repository"
    exit 1
}

cd $CPPUTEST_DIR || {
    echo "Error: Failed to change to cpputest directory"
    exit 1
}

echo "Configuring cpputest with CMake..."
cmake . || {
    echo "Error: CMake configuration failed"
    exit 1
}

echo "Building cpputest..."
make || {
    echo "Error: cpputest build failed"
    exit 1
}

# Return to workspace directory
cd "$WORKSPACE_DIR" || {
    echo "Error: Failed to return to workspace directory"
    exit 1
}

# Run Driver Unit Tests (skip for main-aie branch)
if [[ "$GITHUB_PR_TARGET_BRANCH" != "main-aie" ]]; then
    echo "Building and running Driver unit tests..."
    cd driver || {
        echo "Error: Failed to change to driver directory"
        exit 1
    }

    rm -rf build
    mkdir build
    cd build || {
        echo "Error: Failed to change to build directory"
        exit 1
    }

    echo "Configuring Driver tests with CMake..."
    cmake -DWITH_TESTS=ON -DDEBUG_BACKEND=on -DWITH_TESTS_EXEC=ON -DLINUX_BACKEND=off -DCPPUTEST_DIR=$CPPUTEST_DIR ../ || {
        echo "Error: Driver CMake configuration failed"
        exit 1
    }

    echo "Building Driver tests..."
    make all || {
        echo "Error: Driver build failed"
        exit 1
    }

    echo "Running Driver unit tests..."
    ./tests/utest/run-test-aie | tee >(split -b 1G -d) > /dev/null & wait

    # Find the most recently written file
    latest_file=$(find . -type f -exec ls -t1 {} + | head -1 || true)

    # Fetch the line that matches with the regex Errors
    errors_line=$(grep "Errors (.* failures,.* tests,.* ran,.* checks,.* ignored,.* filtered out,.* ms)" "$latest_file" || true)

    failures=$(echo "$errors_line" | sed -E 's/.*\(([0-9]+) failures.*/\1/')

    echo "Driver test failures: $failures"

    # Clean up split log files
    rm -rf x*

    if [[ -z "$failures" ]] || [[ "$failures" -gt 5 ]]; then
        echo "Error: Driver tests failed with $failures failures (max allowed: 5)"
        exit 1
    fi

    echo "✓ Driver tests passed (failures: $failures, max allowed: 5)"

    # Return to workspace
    cd "$WORKSPACE_DIR" || {
        echo "Error: Failed to return to workspace directory"
        exit 1
    }
else
    echo "AIE-RT Driver UT tests are not supported for main-aie branch. Skipping them!"
fi

# Run FAL Tests
echo "Building and running FAL tests..."
cd "$WORKSPACE_DIR/fal" || {
    echo "Error: Failed to change to fal directory"
    exit 1
}

rm -rf build
mkdir build
cd build || {
    echo "Error: Failed to change to FAL build directory"
    exit 1
}

echo "Configuring FAL tests with CMake..."
cmake ../ -DWITH_BUILD_XAIEDRV=on -DWITH_SHARED_LIB=off -DWITH_DOC=off -DCMAKE_BUILD_TYPE=Debug -DCODE_COVERAGE=off -DWITH_TESTS=on -DWITH_TESTS_EXEC=on -DCPPUTEST_DIR=$CPPUTEST_DIR -DWITH_EXAMPLES=off || {
    echo "Error: FAL CMake configuration failed"
    exit 1
}

echo "Building FAL tests..."
make all || {
    echo "Error: FAL build failed"
    exit 1
}

# Get the output of the make all command
make_all_output=$(make all 2>&1)

# Match the output against the regex
if [[ "$make_all_output" =~ "100% tests passed, 0 tests failed out of 1" ]]; then
    echo "✓ All FAL tests passed"
    exit 0
else
    echo "Error: Some FAL tests failed"
    echo "$make_all_output"
    exit 1
fi
