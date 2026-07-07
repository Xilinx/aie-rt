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

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=ci_common.sh
source "${SCRIPT_DIR}/ci_common.sh"

# CI provides these variables; the workspace already contains the merged PR code.
echo "GITHUB_PR_TRIGGER_SENDER_AUTHOR: $GITHUB_PR_TRIGGER_SENDER_AUTHOR"
echo "GITHUB_PR_URL: $GITHUB_PR_URL"
echo "GITHUB_PR_NUMBER: $GITHUB_PR_NUMBER"
echo "Source Branch: $GITHUB_PR_SOURCE_BRANCH"
echo "Target Branch: $GITHUB_PR_TARGET_BRANCH"

echo "Current workspace already contains merged PR code at: $(pwd)"
WORKSPACE_DIR=$(pwd)

# Clone aieml-tests repository to parent directory (outside workspace)
# Using unique directory name for AIE2PS tests to avoid conflicts with parallel AIEML tests
echo "Cloning aieml-tests repository for AIE2PS (GEN=5)..."
cd .. || {
    echo "Error: Failed to change to parent directory"
    exit 1
}

# Remove any existing aieml-tests-gen5 directory from previous runs
if [ -d "aieml-tests-gen5" ]; then
    echo "Removing existing aieml-tests-gen5 directory..."
    rm -rf aieml-tests-gen5
fi

ci_git_clone aieml-tests aieml-tests-gen5 || {
    echo "Error: Failed to clone aieml-tests repository"
    exit 1
}

cd aieml-tests-gen5 || {
    echo "Error: Failed to change directory to aieml-tests-gen5"
    exit 1
}

# Copy the workspace contents (merged aie-rt) into aieml-tests
echo "Copying merged aie-rt code from workspace..."
cp -r "$WORKSPACE_DIR/." ./aie-rt/ || {
    echo "Error: Failed to copy aie-rt from workspace"
    exit 1
}

echo "Successfully set up aie-rt in aieml-tests directory"
ci_print_repo_revisions "$WORKSPACE_DIR" "$(pwd)"

# Set environment and build/test
echo "Setting up Vitis environment..."
source /proj/xbuilds/HEAD_qualified_latest/installs/lin64/HEAD/Vitis/settings64.sh || {
    echo "Error: Failed to source Vitis settings"
    exit 1
}

echo "Building with SystemC for AIE2PS (GEN=5)..."
make systemc JENKINS_TESTS=yes GEN=5 || {
    echo "Error: SystemC build failed for AIE2PS"
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

# Run make run_sim; capture stdout and stderr separately
echo "Running simulations for AIE2PS..."
set +e
make run_sim > sim_output.txt 2> sim_stderr.txt << EOF
all
EOF
sim_ret=$?
set -e

ci_publish_sim_logs "$WORKSPACE_DIR" "aie2ps_sim_output"

if [ ! -f sim_output.txt ]; then
    echo "Error: sim_output.txt does not exist (make run_sim exit code: $sim_ret)"
    exit 1
fi

if ! ci_sim_check_suite_complete sim_output.txt; then
    ci_publish_sim_logs "$WORKSPACE_DIR" "aie2ps_sim_output"
    if [ -f sim_stderr.txt ]; then
        echo ""
        echo "=== Last 20 lines of sim_stderr.txt ==="
        tail -20 sim_stderr.txt
    fi
    exit 1
fi

# Define the list of tests that are allowed to fail
CI_ALLOWED_TO_FAIL=("test_aie_col_status" "test_aie_dma_event" "test_aie_pm_err_injection" "test_aie_shimdma_loopback" "test_aie_shimdma_loopback_profile")

# Extract test names from lines containing " failed"
mapfile -t failing_tests < <(grep ' failed' sim_output.txt | grep -oE 'test_[a-zA-Z0-9_]+' | sort -u)

ci_print_sim_summary "$sim_ret" sim_output.txt "${failing_tests[@]}"

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
    if ! ci_is_allowed_to_fail "$failing_test" "${CI_ALLOWED_TO_FAIL[@]}"; then
        echo ""
        echo "=========================================="
        echo "FAILURE: Test $failing_test is not allowed to fail"
        echo "=========================================="
        echo ""
        cat sim_output.txt
        ci_publish_sim_logs "$WORKSPACE_DIR" "aie2ps_sim_output"
        exit 1
    else
        echo "  ℹ Test $failing_test failed but is in allowed_to_fail list"
    fi
done

if [ "$sim_ret" -ne 0 ] && [ ${#failing_tests[@]} -eq 0 ]; then
    echo "Error: make run_sim failed with exit code $sim_ret but no failing tests were parsed"
    cat sim_output.txt
    exit 1
fi

echo ""
echo "✓ Test validation completed successfully"
exit 0
