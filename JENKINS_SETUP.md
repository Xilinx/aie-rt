# Jenkins Job Configuration Guide

This guide explains how to configure 4 separate Jenkins Multibranch Pipeline jobs to run individual test suites and report their status separately on GitHub PRs.

## Overview

The repository now contains 5 Jenkinsfiles:
- `Jenkinsfile` - Original combined pipeline (can be kept as backup or removed)
- `Jenkinsfile.aieml` - AIEML Tests
- `Jenkinsfile.aie2ps` - AIE2PS Tests
- `Jenkinsfile.unittest` - Unit Tests
- `Jenkinsfile.infer` - Infer Static Analysis

Each separate Jenkinsfile will create an individual status check on GitHub PRs.

## Repository Configuration

**Original Repository:** `https://gitenterprise.xilinx.com/ai-engine/aie-rt`

The Jenkins jobs should be configured to monitor the **original repository** (not forks). This ensures:
- PRs from any fork to the original repository will trigger the test jobs
- All contributors will see test results on their PRs
- Consistent testing across all contributions

**Note:** If you're working from a fork (e.g., `https://gitenterprise.xilinx.com/ssatpute/aie-rt`), you'll create PRs from your fork to the original repository, and the Jenkins jobs will automatically run on those PRs.

## Jenkins Job Configuration Steps

### 1. Create Job: AIE_Driver_AIEML_Tests_From_PR

1. In Jenkins, click **New Item**
2. Enter name: `AIE_Driver_AIEML_Tests_From_PR`
3. Select **Multibranch Pipeline**
4. Click **OK**

**Branch Sources Configuration:**
- Click **Add source** → **Git** (or GitHub if using GitHub plugin)
- **Project Repository:** `https://gitenterprise.xilinx.com/ai-engine/aie-rt`
- **Credentials:** Select your GitHub Enterprise credentials
- **Behaviors:** Add "Discover pull requests from origin" and "Discover branches"
  - Strategy: "Merging the pull request with the current target branch revision"
  - **Important:** Also add "Discover pull requests from forks" behavior
    - Strategy: "Merging the pull request with the current target branch revision"
    - Trust: "From users with Admin or Write permission"

**Build Configuration:**
- **Mode:** by Jenkinsfile
- **Script Path:** `Jenkinsfile.aieml`

**Scan Multibranch Pipeline Triggers:**
- ✓ Periodically if not otherwise run
- Interval: 1 minute (or as needed)

**Property Strategy:**
- Select "All branches get the same properties"

**Save** the configuration

---

### 2. Create Job: AIE_Driver_AIEML_AIE2PS_Tests_From_PR

1. In Jenkins, click **New Item**
2. Enter name: `AIE_Driver_AIEML_AIE2PS_Tests_From_PR`
3. Select **Multibranch Pipeline**
4. Click **OK**

**Branch Sources Configuration:**
- Click **Add source** → **Git** (or GitHub)
- **Project Repository:** `https://gitenterprise.xilinx.com/ai-engine/aie-rt`
- **Credentials:** Select your GitHub Enterprise credentials
- **Behaviors:** Add "Discover pull requests from origin" and "Discover branches"
  - Strategy: "Merging the pull request with the current target branch revision"
  - **Important:** Also add "Discover pull requests from forks" behavior
    - Strategy: "Merging the pull request with the current target branch revision"
    - Trust: "From users with Admin or Write permission"

**Build Configuration:**
- **Mode:** by Jenkinsfile
- **Script Path:** `Jenkinsfile.aie2ps`

**Scan Multibranch Pipeline Triggers:**
- ✓ Periodically if not otherwise run
- Interval: 1 minute (or as needed)

**Property Strategy:**
- Select "All branches get the same properties"

**Save** the configuration

---

### 3. Create Job: AIE_Driver_Build_UT_From_PR

1. In Jenkins, click **New Item**
2. Enter name: `AIE_Driver_Build_UT_From_PR`
3. Select **Multibranch Pipeline**
4. Click **OK**

**Branch Sources Configuration:**
- Click **Add source** → **Git** (or GitHub)
- **Project Repository:** `https://gitenterprise.xilinx.com/ai-engine/aie-rt`
- **Credentials:** Select your GitHub Enterprise credentials
- **Behaviors:** Add "Discover pull requests from origin" and "Discover branches"
  - Strategy: "Merging the pull request with the current target branch revision"
  - **Important:** Also add "Discover pull requests from forks" behavior
    - Strategy: "Merging the pull request with the current target branch revision"
    - Trust: "From users with Admin or Write permission"

**Build Configuration:**
- **Mode:** by Jenkinsfile
- **Script Path:** `Jenkinsfile.unittest`

**Scan Multibranch Pipeline Triggers:**
- ✓ Periodically if not otherwise run
- Interval: 1 minute (or as needed)

**Property Strategy:**
- Select "All branches get the same properties"

**Save** the configuration

---

### 4. Create Job: AIE_Driver_Infer_From_PR

1. In Jenkins, click **New Item**
2. Enter name: `AIE_Driver_Infer_From_PR`
3. Select **Multibranch Pipeline**
4. Click **OK**

**Branch Sources Configuration:**
- Click **Add source** → **Git** (or GitHub)
- **Project Repository:** `https://gitenterprise.xilinx.com/ai-engine/aie-rt`
- **Credentials:** Select your GitHub Enterprise credentials
- **Behaviors:** Add "Discover pull requests from origin" and "Discover branches"
  - Strategy: "Merging the pull request with the current target branch revision"
  - **Important:** Also add "Discover pull requests from forks" behavior
    - Strategy: "Merging the pull request with the current target branch revision"
    - Trust: "From users with Admin or Write permission"

**Build Configuration:**
- **Mode:** by Jenkinsfile
- **Script Path:** `Jenkinsfile.infer`

**Scan Multibranch Pipeline Triggers:**
- ✓ Periodically if not otherwise run
- Interval: 1 minute (or as needed)

**Property Strategy:**
- Select "All branches get the same properties"

**Save** the configuration

---

## GitHub Status Check Configuration

Each job will report status to GitHub using these context names:
- `AIE_Driver_AIEML_Tests_From_PR` - AIEML Tests
- `AIE_Driver_AIEML_AIE2PS_Tests_From_PR` - AIE2PS Tests
- `AIE_Driver_Build_UT_From_PR` - Unit Tests
- `AIE_Driver_Infer_From_PR` - Infer Analysis

These will appear as separate status checks on your GitHub PRs, matching the expected checks shown in your screenshot.

## Required Jenkins Plugins

Ensure these plugins are installed:
- **Git Plugin** - For Git repository integration
- **GitHub Plugin** (optional) - For better GitHub integration
- **Pipeline: Multibranch Plugin** - For multibranch pipeline support
- **GitHub Commit Status Setter** - For updating GitHub commit statuses

## Credentials Setup

Make sure the credential `github-enterprise-token` exists in Jenkins:
1. Go to **Manage Jenkins** → **Manage Credentials**
2. Select appropriate domain (usually "Global")
3. Verify credential ID `github-enterprise-token` exists
4. This credential should have permissions to:
   - Read repository
   - Write commit statuses

## Testing the Setup

1. Create a test PR targeting `main` or `main-aie` branch
2. All 4 jobs should automatically trigger
3. Check the PR on GitHub - you should see 4 separate status checks:
   - AIE_Driver_AIEML_Tests_From_PR
   - AIE_Driver_AIEML_AIE2PS_Tests_From_PR
   - AIE_Driver_Build_UT_From_PR
   - AIE_Driver_Infer_From_PR

## Troubleshooting

### Jobs not triggering on PRs
- Verify "Discover pull requests from origin" is enabled in Branch Sources
- Check that the scan interval is set appropriately
- Manually trigger a scan: Go to job → Click "Scan Multibranch Pipeline Now"

### GitHub status not updating
- Verify the GitHub Commit Status Setter plugin is installed
- Check Jenkins logs for GitHub API errors
- Ensure the credential has "Commit statuses: Read and write" permission

### Jobs running but not showing on GitHub
- Verify the context names match exactly (case-sensitive)
- Check that the jobs are running on PR branches (not just regular branches)
- Ensure the GitHub repository webhook is configured correctly

## Migration from Original Jenkinsfile

Once you've verified all 4 separate jobs are working correctly:
1. You can disable or delete the original combined pipeline job
2. Optionally rename or remove the original `Jenkinsfile`
3. Update any documentation referencing the old job structure

## Benefits of This Setup

✓ **Individual Status Checks** - Each test suite appears separately on GitHub PRs
✓ **Independent Execution** - Tests can be re-run individually without running all tests
✓ **Better Visibility** - Clear indication of which specific test suite failed
✓ **Parallel Execution** - All 4 jobs run in parallel automatically
✓ **Flexible Management** - Can disable/enable individual test suites as needed
