#!/bin/bash

# Jenkins already provides these variables and has merged the PR
echo "GITHUB_PR_TRIGGER_SENDER_AUTHOR: $GITHUB_PR_TRIGGER_SENDER_AUTHOR"
echo "GITHUB_PR_URL: $GITHUB_PR_URL"
echo "GITHUB_PR_NUMBER: $GITHUB_PR_NUMBER"
echo "Source Branch: $GITHUB_PR_SOURCE_BRANCH"
echo "Target Branch: $GITHUB_PR_TARGET_BRANCH"

# Current directory already contains the merged PR code
echo "Current workspace already contains merged PR code at: $(pwd)"
WORKSPACE_DIR=$(pwd)

# Parse owner and repo from PR URL
REPO_URL=$(echo $GITHUB_PR_URL | sed 's/https:\/\/gitenterprise.xilinx.com\///' | sed 's/\/pull.*//')
OWNER=$(echo $REPO_URL | cut -d'/' -f1)
REPO=$(echo $REPO_URL | cut -d'/' -f2)
DEST_REPO="${OWNER}/${REPO}"
DEST_BRANCH=${GITHUB_PR_TARGET_BRANCH}

echo "Destination Repository: $DEST_REPO"
echo "Destination Branch: $DEST_BRANCH"

# Move to parent directory for infer operations
cd .. || {
    echo "Error: Failed to change to parent directory"
    exit 1
}

# Download and extract Infer tool
echo "Downloading Facebook Infer static analysis tool..."
wget -q "https://github.com/facebook/infer/releases/download/v1.1.0/infer-linux64-v1.1.0.tar.xz" || {
    echo "Error: Failed to download Infer"
    exit 1
}

echo "Extracting Infer..."
tar xf infer-linux64-v1.1.0.tar.xz || {
    echo "Error: Failed to extract Infer"
    exit 1
}

INFER_BIN="$(pwd)/infer-linux64-v1.1.0/bin/infer"

# Extract specific issue counts from a given file
function extract_issue_count {
    local file=$1
    local issue=$2
    grep "$issue" "$file" | awk -F': ' '{print $2}'
}

# Run Infer on SOURCE (merged PR code in Jenkins workspace)
echo "Running Infer analysis on merged PR code..."
rm -rf infer-source
mkdir -p infer-source
cp -r "$WORKSPACE_DIR/." infer-source/ || {
    echo "Error: Failed to copy workspace to infer-source"
    exit 1
}

cd infer-source/driver/src || {
    echo "Error: Failed to change to infer-source/driver/src"
    exit 1
}

echo "Analyzing source code with Infer..."
CC=gcc CXX=g++ $INFER_BIN -- make -f Makefile.Linux > ../../../infer_source.txt 2>&1

cd ../../..

# Run Infer on DESTINATION (target branch)
echo "Cloning destination branch for comparison..."
rm -rf infer-dest
git clone "https://gitenterprise.xilinx.com/${DEST_REPO}.git" infer-dest || {
    echo "Error: Failed to clone destination repository"
    exit 1
}

cd infer-dest || {
    echo "Error: Failed to change to infer-dest directory"
    exit 1
}

git checkout $DEST_BRANCH || {
    echo "Error: Failed to checkout destination branch"
    exit 1
}

cd driver/src || {
    echo "Error: Failed to change to infer-dest/driver/src"
    exit 1
}

echo "Analyzing destination code with Infer..."
CC=gcc CXX=g++ $INFER_BIN -- make -f Makefile.Linux > ../../../infer_dest.txt 2>&1

cd ../../..

# Display Infer reports
echo ""
echo "=========================================="
echo "SOURCE (Merged PR) Infer Report:"
echo "=========================================="
cd infer-source/driver/src
cat infer-out/report.txt || echo "No report.txt found for source"
cd ../../..

echo ""
echo "=========================================="
echo "DESTINATION (Target Branch) Infer Report:"
echo "=========================================="
cd infer-dest/driver/src
cat infer-out/report.txt || echo "No report.txt found for destination"
cd ../../..

# Compare Infer results
echo ""
echo "=========================================="
echo "Comparing Infer Results..."
echo "=========================================="
diff infer_dest.txt infer_source.txt > infer_diff.txt || true

# Extract counts from infer_source.txt
source_uninitialized=$(extract_issue_count infer_source.txt "Uninitialized Value(UNINITIALIZED_VALUE)" || echo "0")
source_dead_store=$(extract_issue_count infer_source.txt "Dead Store(DEAD_STORE)" || echo "0")
source_null_deref=$(extract_issue_count infer_source.txt "Null Dereference(NULL_DEREFERENCE)" || echo "0")

# Extract counts from infer_dest.txt
dest_uninitialized=$(extract_issue_count infer_dest.txt "Uninitialized Value(UNINITIALIZED_VALUE)" || echo "0")
dest_dead_store=$(extract_issue_count infer_dest.txt "Dead Store(DEAD_STORE)" || echo "0")
dest_null_deref=$(extract_issue_count infer_dest.txt "Null Dereference(NULL_DEREFERENCE)" || echo "0")

# Default to 0 if empty
source_uninitialized=${source_uninitialized:-0}
source_dead_store=${source_dead_store:-0}
source_null_deref=${source_null_deref:-0}
dest_uninitialized=${dest_uninitialized:-0}
dest_dead_store=${dest_dead_store:-0}
dest_null_deref=${dest_null_deref:-0}

echo "Source - Uninitialized: $source_uninitialized, Dead Store: $source_dead_store, Null Deref: $source_null_deref"
echo "Dest   - Uninitialized: $dest_uninitialized, Dead Store: $dest_dead_store, Null Deref: $dest_null_deref"

# Compare counts and decide if the test should fail
if (( source_uninitialized > dest_uninitialized )) ||
   (( source_dead_store > dest_dead_store )) ||
   (( source_null_deref > dest_null_deref )); then
    echo ""
    echo "=========================================="
    echo "FAILURE: New issues detected in PR"
    echo "=========================================="
    echo "Diff between destination and source:"
    cat infer_diff.txt
    exit 1
else
    echo ""
    echo "✓ No new significant issues detected"
    if [ -s infer_diff.txt ]; then
        echo "Differences found (but no increase in critical issues):"
        cat infer_diff.txt
    fi
    exit 0
fi
