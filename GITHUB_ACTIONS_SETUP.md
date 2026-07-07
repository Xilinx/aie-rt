# GitHub Actions CI Setup Guide

This guide describes GitHub Actions CI for `https://github.com/AMD-AECG-AIENGINE/aie-rt`,
migrated from Jenkins on gitenterprise (SSW-16665).

## Overview

| Component | Location |
|-----------|----------|
| PR CI workflow | [`.github/workflows/pr-ci.yml`](.github/workflows/pr-ci.yml) |
| Patch email workflow | [`.github/workflows/send-email.yml`](.github/workflows/send-email.yml) |
| Test scripts | [`.jenkins/`](.jenkins/) |
| Shared git helpers | [`.jenkins/ci_common.sh`](.jenkins/ci_common.sh) |

### PR CI jobs (4 parallel + notify)

| Job | Status context | Script |
|-----|----------------|--------|
| AIEML Tests | `AIE_Driver_AIEML_Tests_From_PR` | `.jenkins/aie_driver_AIEML_Tests_from_PR.sh` |
| AIE2PS Tests | `AIE_Driver_AIEML_AIE2PS_Tests_From_PR` | `.jenkins/aie_driver_AIEML_AIE2PS_Tests_from_PR.sh` |
| Unit Tests | `AIE_Driver_Build_UT_From_PR` | `.jenkins/build_UT_from_PR_jenkins_job.sh` |
| Infer | `AIE_Driver_Infer_From_PR` | `.jenkins/infer_from_PR_jenkins_job.sh` |
| CI Summary Notification | (no blocking check) | `.jenkins/send_ci_summary_email.sh` |

**Triggers:** pull requests targeting `main`, `main-aie`, `main-next`, or `main-aie-next`.

## Infrastructure

| Role | Host |
|------|------|
| Self-hosted GHA runner (jobs execute here) | **xsjsswsip50** |
| Developer workspace (edit, commit, push) | xsjaiedev50 / xsjaie2dev50 |

The runner checks out code into its own work directory (e.g.
`/scratch2/staff/ssatpute/actions-runner/_work/aie-rt/aie-rt`). Your dev
workspace is not used at CI runtime.

## Runner setup (xsjsswsip50)

1. Repo → **Settings → Actions → Runners → New self-hosted runner** (Linux x64).
2. Install under a dedicated directory (e.g. `/scratch2/staff/ssatpute/actions-runner`).
3. Install as a service (do not use interactive `run.sh`):

   ```bash
   sudo ./svc.sh install
   sudo ./svc.sh start
   sudo ./svc.sh status
   ```

4. Confirm the runner shows **Idle** in repo Settings → Runners.

### Host prerequisites

On **xsjsswsip50** (not only dev machines):

```bash
for cmd in git cmake make jq curl wget; do command -v "$cmd" || echo "MISSING: $cmd"; done
test -f /proj/xbuilds/HEAD_qualified_latest/installs/lin64/HEAD/Vitis/settings64.sh && echo OK vitis
test -f /proj/xsjsswstaff/sankarji/aie-arch/aie-arch-env-files/aie2-arch.sh && echo OK aie2-arch
```

Clone smoke test:

```bash
export CI_GITHUB_TOKEN="<PAT-with-repo-read>"
source .jenkins/ci_common.sh
ci_git_clone cpputest /tmp/cpputest-test && echo OK
```

## Repository secrets

Configure under **Settings → Secrets and variables → Actions → Secrets**:

| Secret | Purpose |
|--------|---------|
| `GHB_TKN` | PAT with `repo` read access to `AMD-AECG-AIENGINE/*` (clones `aieml-tests`, `cpputest`, patch-email workflow) |
| `GITHUB_TOKEN` | Provided automatically by GitHub Actions (checkout, status API) |

`GITHUB_TOKEN` alone cannot clone other private org repos; `GHB_TKN` is required for
AIEML, AIE2PS, and Unit Test jobs.

## Viewing CI logs

### GitHub UI

1. Open the PR → **Checks** tab.
2. Click **AIE-RT PR CI**.
3. Click the failed job (e.g. **AIE_Driver_AIEML_Tests_From_PR**).
4. Expand **Run AIEML tests** (or the matching step).

Direct: `https://github.com/AMD-AECG-AIENGINE/aie-rt/actions`

### On the runner

```bash
# Simulation output (after a failed AIEML run)
cat /scratch2/staff/ssatpute/actions-runner/_work/aie-rt/aieml-tests-gen2/sim_output.txt

# AIE2PS
cat /scratch2/staff/ssatpute/actions-runner/_work/aie-rt/aieml-tests-gen5/sim_output.txt
```

### Artifacts

Failed AIEML/AIE2PS jobs upload `aieml_sim_output.txt` / `aie2ps_sim_output.txt`
as workflow artifacts (workflow run page → **Artifacts**).

## Re-running jobs

- **Single failed job:** PR → Checks → failed run → **Re-run failed jobs**
- **All jobs:** **Re-run all jobs**
- Push a new commit to the PR branch to trigger a fresh run

## Patch email workflow

[`.github/workflows/send-email.yml`](.github/workflows/send-email.yml) sends the approved
PR patch to `git_aiengine_dev@xilinx.com` when:

- PR targets `main`, `main-aie`, `main-next`, or `main-aie-next`
- At least one developer approval exists
- All legacy commit statuses are `success` (the four `AIE_Driver_*_From_PR` checks)
- PR is mergeable (not behind/dirty)

Trigger: PR review approval or comment `approved`.

Requires `GHB_TKN` for `gh` API calls and cloning the repo from GitHub.

## Simulator job behavior

AIEML and AIE2PS share a concurrency group (`aieml-simulator-<PR#>`) so they do not
run at the same time (avoids mesimulator port conflicts).

Allowed simulator test failures (same as Jenkins):

- `test_aie_col_status`
- `test_aie_dma_event`

All other failing tests fail the job.

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|--------------|-----|
| Clone fails in &lt;10s | Missing/invalid `GHB_TKN` | Add PAT secret with org repo read |
| `libgnutls` / `git-remote-https` error | `LD_LIBRARY_PATH` from Vitis | Scripts clear it for `git clone`; clone before sourcing Vitis |
| AIEML/AIE2PS fail after ~5 min | Real sim test failures | See `sim_output.txt`; not a GHA config issue |
| Duplicate checks on PR | Legacy status + GHA job check | Expected; both names appear until Jenkins is disabled |
| `send-email` waits forever | CI statuses pending/failing | Ensure all 4 `AIE_Driver_*` checks pass |
| Secrets not available | PR from fork | Use branches on `AMD-AECG-AIENGINE/aie-rt` |

## Jenkins decommission

After GitHub Actions is stable (1–2 weeks):

1. Disable Jenkins multibranch jobs on gitenterprise.
2. Optionally remove or archive `Jenkinsfile.*` (see [JENKINS_SETUP.md](JENKINS_SETUP.md)).
3. Update branch protection to require the four GitHub Actions status contexts.

## Reference

- Confluence: [Automated Github EMU Test (Github Actions)](https://amd.atlassian.net/wiki/spaces/~7120208ea78f4ee4d44906b987149f97a2521e/pages/1755505583/Automated+Github+EMU+Test+Github+Actions)
- JIRA: SSW-16665
