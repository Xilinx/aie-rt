#!/bin/bash
# Shared helpers for CI scripts (GitHub Actions and Jenkins).

GITHUB_ORG="${GITHUB_ORG:-AMD-AECG-AIENGINE}"

# Token for cloning private org repos. GITHUB_TOKEN in Actions only has access to
# the current repository; use CI_GITHUB_TOKEN (repo PAT secret) for aieml-tests/cpputest.
ci_git_token() {
    echo "${CI_GITHUB_TOKEN:-${GITHUB_TOKEN:-}}"
}

# Clone a repository under GITHUB_ORG (e.g. aieml-tests, cpputest).
ci_git_clone() {
    local repo_name="$1"
    local dest="$2"
    local token
    token="$(ci_git_token)"
    local url="https://github.com/${GITHUB_ORG}/${repo_name}.git"

    if [ -n "$token" ]; then
        url="https://x-access-token:${token}@github.com/${GITHUB_ORG}/${repo_name}.git"
    else
        echo "Warning: no CI_GITHUB_TOKEN or GITHUB_TOKEN set; clone may fail for private repos"
    fi

    # Avoid toolchain LD_LIBRARY_PATH breaking git-remote-https on build hosts.
    if ! LD_LIBRARY_PATH= git clone "$url" "$dest"; then
        echo "Error: failed to clone ${GITHUB_ORG}/${repo_name}"
        echo "Hint: set CI_GITHUB_TOKEN (repo secret GHB_TKN) with read access to org repos"
        return 1
    fi
}

# Clone owner/repo (e.g. AMD-AECG-AIENGINE/aie-rt).
ci_git_clone_full() {
    local full_repo="$1"
    local dest="$2"
    local token
    token="$(ci_git_token)"
    local url="https://github.com/${full_repo}.git"

    if [ -n "$token" ]; then
        url="https://x-access-token:${token}@github.com/${full_repo}.git"
    else
        echo "Warning: no CI_GITHUB_TOKEN or GITHUB_TOKEN set; clone may fail for private repos"
    fi

    if ! LD_LIBRARY_PATH= git clone "$url" "$dest"; then
        echo "Error: failed to clone ${full_repo}"
        echo "Hint: set CI_GITHUB_TOKEN (repo secret GHB_TKN) with read access to org repos"
        return 1
    fi
}

# Parse owner/repo from a GitHub or gitenterprise PR URL.
ci_parse_pr_repo() {
    echo "$GITHUB_PR_URL" | sed -E 's#https://(gitenterprise\.xilinx\.com|github\.com)/##' | sed 's#/pull/.*##'
}

# Print repo SHAs to stdout (visible in GHA / Jenkins logs).
ci_print_repo_revisions() {
    local aie_rt_dir="$1"
    local aieml_tests_dir="$2"
    echo "=== CI repo revisions ==="
    echo "aie-rt SHA:      $(git -C "$aie_rt_dir" rev-parse HEAD 2>/dev/null || echo unknown)"
    echo "aieml-tests SHA: $(git -C "$aieml_tests_dir" rev-parse HEAD 2>/dev/null || echo unknown)"
    echo "PR target:       ${GITHUB_PR_TARGET_BRANCH:-unknown}"
}

# Copy sim stdout/stderr into the GHA workspace for artifact upload.
ci_publish_sim_logs() {
    local workspace_dir="$1"
    local artifact_basename="$2"
    if [ -f sim_output.txt ]; then
        cp sim_output.txt "${workspace_dir}/${artifact_basename}.txt" 2>/dev/null || true
    fi
    if [ -f sim_stderr.txt ]; then
        cp sim_stderr.txt "${workspace_dir}/${artifact_basename}_stderr.txt" 2>/dev/null || true
    fi
}

# Verify the GEN=2/GEN=5 "all" suite ran past test_aie_generate_intr (tests 20–26).
# Menu lines like "20. test_aie_col_status" are NOT sufficient — we need execution output.
ci_sim_check_suite_complete() {
    local sim_output="$1"

    if [ ! -s "$sim_output" ]; then
        echo "Error: sim_output.txt is missing or empty"
        return 1
    fi

    if ! grep -q 'Starting aie generate intr test' "$sim_output"; then
        echo "Error: sim output missing test_aie_generate_intr start marker"
        return 1
    fi

    # Execution markers for tests 20–26 (failed or success), not the numbered menu.
    if grep -qE \
        'test_aie_col_status failed|test_aie_pm_err_injection failed|test_aie_shimdma_loopback failed|Partition initialization failed for test_aie_shimdma|test_clk_enable_disable failed|test_aie_interrupt_init failed|test_datamem_read_write failed|test_aie_shimdma_loopback_profile failed|col_status.*success|pm_err.*success|shimdma.*success|clk_enable.*success|interrupt_init.*success|datamem.*success|shimdma_loopback_profile.*success' \
        "$sim_output"; then
        return 0
    fi

    local sim_lines
    sim_lines=$(wc -l < "$sim_output")
    echo "Error: sim suite did not complete — tests after test_aie_generate_intr (20–26) did not run"
    echo "sim_output.txt has ${sim_lines} lines (partial runs often stop ~976 lines during generate_intr)"
    echo "Last 5 lines of sim_output.txt:"
    tail -5 "$sim_output"
    return 1
}

# Print sim result summary to stdout before exit.
ci_print_sim_summary() {
    local sim_ret="$1"
    local sim_output="$2"
    shift 2
    local -a failing_tests=("$@")
    local -a disallowed=()
    local test

    for test in "${failing_tests[@]}"; do
        if ! ci_is_allowed_to_fail "$test" "${CI_ALLOWED_TO_FAIL[@]}"; then
            disallowed+=("$test")
        fi
    done

    echo ""
    echo "=== CI sim summary ==="
    echo "make run_sim exit code: ${sim_ret}"
    echo "sim_output lines:       $(wc -l < "$sim_output" 2>/dev/null || echo 0)"
    echo "Failing tests:          ${failing_tests[*]:-none}"
    echo "Disallowed failures:    ${disallowed[*]:-none}"
    echo "Allowed to fail:        ${CI_ALLOWED_TO_FAIL[*]}"
}

# Return 0 if test is in the allowed_to_fail list passed as remaining arguments.
ci_is_allowed_to_fail() {
    local test="$1"
    shift
    local allowed_test
    for allowed_test in "$@"; do
        [[ $test == "$allowed_test" ]] && return 0
    done
    return 1
}
