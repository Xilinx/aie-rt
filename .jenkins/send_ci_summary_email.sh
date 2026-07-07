#!/bin/bash
# Send combined CI summary email (GitHub Actions replacement for Jenkins sendNotification).

format_result() {
    local result="$1"
    case "$result" in
        success)  echo "SUCCESS" ;;
        failure)  echo "FAILURE" ;;
        cancelled) echo "ABORTED" ;;
        *)        echo "${result^^}" ;;
    esac
}

result_icon() {
    local status="$1"
    case "$status" in
        SUCCESS) echo "✓" ;;
        ABORTED) echo "⊘" ;;
        *)       echo "✗" ;;
    esac
}

author="${GITHUB_PR_TRIGGER_SENDER_AUTHOR:-}"
if [ -z "$author" ]; then
    echo "Warning: GITHUB_PR_TRIGGER_SENDER_AUTHOR not set — skipping email"
    exit 0
fi

author_email="${author}@amd.com"

aieml_status=$(format_result "${RESULT_AIEML:-unknown}")
aie2ps_status=$(format_result "${RESULT_AIE2PS:-unknown}")
unittest_status=$(format_result "${RESULT_UNITTEST:-unknown}")
infer_status=$(format_result "${RESULT_INFER:-unknown}")

all_passed=true
for status in "$aieml_status" "$aie2ps_status" "$unittest_status" "$infer_status"; do
    if [ "$status" != "SUCCESS" ]; then
        all_passed=false
        break
    fi
done

if $all_passed; then
    overall_icon="✓"
    overall_summary="ALL PASSED"
else
    overall_icon="✗"
    overall_summary="SOME FAILED"
fi

run_url="${GITHUB_RUN_URL:-N/A}"
pr_number="${GITHUB_PR_NUMBER:-unknown}"

body=$(cat <<EOF
PR #${pr_number} Combined CI Test Results
--------------------------------------------------
Overall Status : ${overall_icon} ${overall_summary}

Pull Request:
  Author  : ${author}
  Branch  : ${GITHUB_PR_SOURCE_BRANCH:-N/A} -> ${GITHUB_PR_TARGET_BRANCH:-N/A}
  PR URL  : ${GITHUB_PR_URL:-N/A}

Test Results:
  $(result_icon "$aieml_status") AIEML Tests                      : ${aieml_status}
  $(result_icon "$aie2ps_status") AIE2PS Tests                     : ${aie2ps_status}
  $(result_icon "$unittest_status") Unit Tests                       : ${unittest_status}
  $(result_icon "$infer_status") Infer Analysis                    : ${infer_status}

Build URL:
  ${run_url}

This is an automated notification from GitHub Actions CI/CD pipeline.
EOF
)

subject="${overall_icon} PR #${pr_number} CI Results: ${overall_summary}"

if command -v mail >/dev/null 2>&1; then
    printf '%s\n' "$body" | mail -s "$subject" "$author_email"
    echo "Combined summary email sent to: ${author_email}"
elif command -v sendmail >/dev/null 2>&1; then
    {
        printf 'To: %s\n' "$author_email"
        printf 'Subject: %s\n\n' "$subject"
        printf '%s\n' "$body"
    } | sendmail -t
    echo "Combined summary email sent via sendmail to: ${author_email}"
else
    echo "Warning: neither mail nor sendmail found — cannot send notification"
    echo "$subject"
    printf '%s\n' "$body"
    exit 0
fi
