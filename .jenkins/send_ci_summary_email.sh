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

author_email="${author%_amdeng}@amd.com"

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

# The runner's local MTA is broken (mail sits in the queue undelivered), so send
# directly through the internal SMTP relay instead of relying on mail/sendmail.
smtp_server="${CI_SMTP_SERVER:-ATLSMTP10.amd.com}"
smtp_port="${CI_SMTP_PORT:-25}"

if command -v python3 >/dev/null 2>&1; then
    if EMAIL_BODY="$body" python3 - "$author_email" "$author_email" "$subject" "$smtp_server" "$smtp_port" <<'PYEOF'
import os
import sys
import smtplib
from email.message import EmailMessage

from_addr, to_addr, subject, server, port = sys.argv[1:6]

msg = EmailMessage()
msg["From"] = from_addr
msg["To"] = to_addr
msg["Subject"] = subject
msg.set_content(os.environ.get("EMAIL_BODY", ""))

with smtplib.SMTP(server, int(port), timeout=30) as smtp:
    smtp.send_message(msg)
PYEOF
    then
        echo "Combined summary email sent to: ${author_email} via ${smtp_server}:${smtp_port}"
    else
        echo "Warning: failed to send summary email via ${smtp_server}:${smtp_port}"
        echo "$subject"
        printf '%s\n' "$body"
        exit 0
    fi
else
    echo "Warning: python3 not found — cannot send notification via SMTP relay"
    echo "$subject"
    printf '%s\n' "$body"
    exit 0
fi
