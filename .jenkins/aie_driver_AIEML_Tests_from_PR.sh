#!/bin/bash

# ============================================================================
# Cleanup: Kill any existing simulator processes and clean stale state
# ============================================================================
echo "=========================================="
echo "Cleaning up any existing simulator processes and state..."
echo "=========================================="

# Kill any existing mesimulator processes
pkill -f mesimulator 2>/dev/null || true
sleep 2

# Double-check and force kill if needed
pkill -9 -f mesimulator 2>/dev/null || true
sleep 1

# Clean up any stale simulator state files
rm -f /tmp/mesimulator_* 2>/dev/null || true
rm -f /tmp/.mesimulator* 2>/dev/null || true

echo "✓ Cleanup completed"
echo "=========================================="
echo ""

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

# Clone aieml-tests repository to parent directory (outside workspace)
# Using unique directory name for AIEML tests to avoid conflicts with parallel AIE2PS tests
echo "Cloning aieml-tests repository for AIEML (GEN=2)..."
cd .. || {
    echo "Error: Failed to change to parent directory"
    exit 1
}

# Remove any existing aieml-tests-gen2 directory from previous runs
if [ -d "aieml-tests-gen2" ]; then
    echo "Removing existing aieml-tests-gen2 directory..."
    rm -rf aieml-tests-gen2
fi

git clone https://gitenterprise.xilinx.com/ai-engine/aieml-tests.git aieml-tests-gen2 || {
    echo "Error: Failed to clone aieml-tests repository"
    exit 1
}

cd aieml-tests-gen2 || {
    echo "Error: Failed to change directory to aieml-tests-gen2"
    exit 1
}

# Copy the workspace contents (merged aie-rt) into aieml-tests
echo "Copying merged aie-rt code from workspace..."
cp -r "$WORKSPACE_DIR/." ./aie-rt/ || {
    echo "Error: Failed to copy aie-rt from workspace"
    exit 1
}

echo "Successfully set up aie-rt in aieml-tests directory"

# Set environment and build/test
echo "Setting up Vitis environment..."
source /proj/xbuilds/HEAD_qualified_latest/installs/lin64/HEAD/Vitis/settings64.sh || {
    echo "Error: Failed to source Vitis settings"
    exit 1
}

echo "Building with SystemC..."
make systemc JENKINS_TESTS=yes || {
    echo "Error: SystemC build failed"
    exit 1
}

echo "Setting up AIE2 architecture environment..."
source /proj/xsjsswstaff/sankarji/aie-arch/aie-arch-env-files/aie2-arch.sh || {
    echo "Error: Failed to source AIE2 architecture settings"
    exit 1
}

export LD_LIBRARY_PATH=$PWD/aie-rt/driver/src/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$PWD/../objects/:$LD_LIBRARY_PATH

echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"

# Run make run_sim and store output in a file
echo "Running simulations..."
make run_sim > sim_output.txt << EOF
all
EOF

# Define the list of tests that are allowed to fail
allowed_to_fail=("test_aie_col_status" "test_aie_dma_event")

# Use grep and awk to extract the names of the failing tests from sim_output.txt
failing_tests=($(grep ' failed' sim_output.txt 2>/dev/null | awk '{print $(NF-1)}'))

if [ ${#failing_tests[@]} -eq 0 ] && [ ! -f sim_output.txt ]; then
  echo "Error: sim_output.txt does not exist. Marking build as failed."
  exit 1
fi

# Function to check if a test is in the allowed_to_fail list
is_allowed_to_fail() {
    local test=$1
    for allowed_test in "${allowed_to_fail[@]}"; do
        [[ $test == $allowed_test ]] && return 0
    done
    return 1
}

# Check each failing test to see if it's allowed to fail
echo "Checking test results..."
if [ ${#failing_tests[@]} -eq 0 ]; then
    echo "✓ All tests passed!"
else
    echo "Found ${#failing_tests[@]} failing test(s):"
    for failing_test in "${failing_tests[@]}"; do
        echo "  - $failing_test"
    done
fi

for failing_test in "${failing_tests[@]}"; do
    if ! is_allowed_to_fail "$failing_test"; then
        echo ""
        echo "=========================================="
        echo "FAILURE: Test $failing_test is not allowed to fail"
        echo "=========================================="
        echo ""
        cat sim_output.txt
        exit 1
    else
        echo "  ℹ Test $failing_test failed but is in allowed_to_fail list"
    fi
done

echo ""
echo "✓ Test validation completed successfully"
exit 0
