# Contributing to LogFlow

Thank you for your interest in contributing to LogFlow. This document explains how to report bugs, propose features, and submit code contributions.

---

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Reporting Bugs](#reporting-bugs)
- [Proposing Features](#proposing-features)
- [Development Setup](#development-setup)
- [Branch Conventions](#branch-conventions)
- [Commit Conventions](#commit-conventions)
- [Pull Request Guidelines](#pull-request-guidelines)
- [Code Style](#code-style)

---

## Code of Conduct

This project follows the [Contributor Covenant Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this standard. Please report unacceptable behavior to the maintainers via GitHub Issues.

---

## Reporting Bugs

Use the **Bug Report** issue template on GitHub. Before opening a new issue, please search existing issues to avoid duplicates.

A good bug report includes:

- The version of LogFlow you are using
- The version of Unreal Engine (5.3, 5.4 or 5.5)
- Your operating system
- Clear steps to reproduce the issue
- What you expected to happen and what actually happened
- Relevant log output or screenshots

---

## Proposing Features

Use **GitHub Discussions** to propose new features before opening an issue or submitting a pull request. This allows the community and maintainers to discuss the idea before any implementation work begins.

Feature requests that align with the project roadmap defined in the README are more likely to be accepted.

---

## Development Setup

LogFlow is a plugin-only repository. It does not contain a full Unreal Engine project.

**Requirements:**

- Unreal Engine 5.3, 5.4 or 5.5
- JetBrains Rider or Visual Studio 2022
- Git

**Recommended local setup:**

1. Clone the repository:
```bash
git clone https://github.com/miradorworks/logflow.git
```

2. Create a symlink from your UE5 project's `Plugins/` folder to the cloned repository:

**Windows (run as Administrator):**
```bash
mklink /D "C:\Path\To\YourProject\Plugins\LogFlow" "C:\Path\To\logflow"
```

**macOS / Linux:**
```bash
ln -s /path/to/logflow /path/to/YourProject/Plugins/LogFlow
```

3. Regenerate project files from the `.uproject` file and open the project in your IDE.

4. The plugin should appear in **Edit → Plugins → LogFlow**.

---

## Branch Conventions

| Branch | Purpose |
|---|---|
| `main` | Stable, release-ready code. Never commit directly. |
| `develop` | Integration branch. Base for all feature work. |
| `feature/LOGF-XX-short-description` | One branch per Jira ticket. |
| `bugfix/LOGF-XX-short-description` | One branch per bug fix. |
| `release/vX.Y` | Release preparation branch. |

Always branch from `develop`, never from `main`.

```bash
git checkout develop
git pull origin develop
git checkout -b feature/LOGF-XX-short-description
```

---

## Commit Conventions

Every commit message must reference the Jira ticket it belongs to:

```
LOGF-XX: Short imperative description of the change
```

**Examples:**

```
LOGF-09: Implement FLogFlowDispatcher with thread-safe queue
LOGF-16: Add severity color coding to SLogFlowEntryRow
LOGF-37: Fix crash when closing Log Viewer during active PIE session
```

Rules:

- Use the imperative mood: "Add", "Fix", "Implement", not "Added" or "Fixes"
- Keep the first line under 72 characters
- Reference the Jira ticket ID at the start, always
- If the change needs more context, add a blank line after the first and write a short paragraph

---

## Pull Request Guidelines

All contributions must go through a pull request into `develop`. Direct pushes to `main` and `develop` are blocked.

Before opening a PR:

- The plugin compiles without errors or warnings in the target UE5 version
- All existing automated tests pass
- If your change adds new functionality, add the corresponding tests in `LogFlowTests`
- The PR description clearly explains what was changed and why
- The PR references the Jira ticket: include `Closes LOGF-XX` or `Relates to LOGF-XX` in the description

PR title format:

```
LOGF-XX: Short description of the change
```

---

## Code Style

LogFlow follows the [Unreal Engine Coding Standard](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine).

Key rules:

- Class and struct names follow UE5 prefix conventions: `U`, `F`, `S`, `I`, `E`
- All public classes and methods must have documentation comments in UE5 format
- No business logic in `LogFlowEditor` or `LogFlowBlueprintLibrary` — all logic belongs in `LogFlowCore`
- No I/O operations on the game thread — all file writes go through `FLogFlowFileWriter`
- No direct references from `LogFlowCore` to `LogFlowEditor` or `LogFlowBlueprintLibrary`

---

## Questions

If you have questions about the codebase or are unsure where to start, open a thread in **GitHub Discussions**. We are happy to help.
