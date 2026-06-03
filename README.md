# LogFlow

A custom logging plugin for Unreal Engine 5 — clean, focused, real-time logs without the engine noise.

![UE5 5.7](https://img.shields.io/badge/UE5-5.7-blue)
![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)
![Status: Released](https://img.shields.io/badge/Status-Released-brightgreen)

## Overview

If you have ever lost a critical gameplay message buried inside UE5's Output Log, LogFlow is for you.

LogFlow gives you a dedicated dockable panel in the editor that shows only the logs you write — no engine messages, no noise. Every PIE session automatically generates a plain text file with your entries, formatted and ready to review. Filter by severity, search by text, colour-code by tag, and browse past sessions from the built-in Log Viewer. If you are coming from Unity, think of it as the Console window you have always wanted inside UE5.

LogFlow works with a single line of code from C++ or Blueprint and requires no configuration to get started.

---

## Documentation

| Document | Description |
|---|---|
| [DOCUMENTATION.md](DOCUMENTATION.md) | Full user documentation: installation, quick start, panel reference, API reference, configuration and troubleshooting. |
| [CHANGELOG.md](CHANGELOG.md) | Version history and release notes. |
| [CONTRIBUTING.md](CONTRIBUTING.md) | How to contribute: branch conventions, commit format, pull request guidelines and code style. |
| [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) | Community standards and expected behaviour for contributors. |

---

## Features

- Dockable editor panel with real-time log display during PIE
- Three severity levels: Log, Warning, Error with distinct color coding
- C++ and Blueprint API — single line call, no setup required
- Automatic session `.txt` file generation per PIE session
- Configurable tags with custom colors per tag
- Real-time filtering by severity and tag
- Text search within the panel
- Configurable timestamp per entry (session time or system time)
- Copy to clipboard — individual entries or full visible log
- Auto-clear panel on PIE start (configurable)
- Message counter per severity in panel header
- Break on Error — auto-pause PIE on Error entries (configurable)
- Session history with configurable limit
- Log Viewer — browse and read past sessions with search and clipboard support

---

## Screenshots

![LogFlow Panel](https://github.com/user-attachments/assets/9c9b13ab-d146-401c-b7c2-0423d4709dc1)
*LogFlow Panel — real-time log display during PIE with severity filters, tag selector and text search.*

![LogFlow Viewer](https://github.com/user-attachments/assets/d815dab2-a680-47c1-b329-6525b4f2d358)
*LogFlow Viewer — browse and read past session files with search and clipboard support.*

## Installation

Full installation instructions are available in [DOCUMENTATION.md](DOCUMENTATION.md).

**Quick install from source:**

1. Clone or download this repository into your project's `Plugins/` folder:
   ```
   YourProject/Plugins/LogFlow/
   ```
2. Right-click your `.uproject` file and select **Generate Visual Studio project files**.
3. Open the project — UE5 will detect the plugin and prompt you to compile it.
4. Enable the plugin in **Edit → Plugins → Developer Tools → LogFlow**.
5. Restart the editor when prompted.
6. Open **Window → LogFlow Panel** to verify the installation.

---

## Quick Start — C++

```cpp
#include "LogFlowSubsystem.h"

ULogFlowSubsystem::LogMessage(
    TEXT("Player spawned"),
    ELogFlowSeverity::Log,
    FName("Gameplay"));

ULogFlowSubsystem::LogMessage(
    TEXT("Health below 25%"),
    ELogFlowSeverity::Warning,
    FName("Combat"));

ULogFlowSubsystem::LogMessage(
    TEXT("Save file could not be written"),
    ELogFlowSeverity::Error,
    FName("SaveSystem"));
```

Add `LogFlowCore` to your module's `Build.cs`:

```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "Core", "CoreUObject", "Engine",
    "LogFlowCore"
});
```

---

## Quick Start — Blueprint

Search for **LogFlow** in the Blueprint node search. Three nodes are available
under the LogFlow category: **Log Message**, **Log Warning** and **Log Error**.
The **Tag** parameter is optional and hidden by default — expand advanced pins
to access it.

---

## Configuration

All settings are available in **Edit → Editor Preferences → Plugins → LogFlow**:

- Log directory and session history limit
- Timestamp mode (session time or system time)
- Per-tag custom colors
- Auto-clear panel on PIE start
- Break on Error

---

## Architecture

LogFlow is organized into four modules: `LogFlowCore` (runtime, all business logic),
`LogFlowEditor` (editor UI, Slate panel and Log Viewer), `LogFlowBlueprintLibrary`
(Blueprint API wrapper) and `LogFlowTests` (automation tests, not shipped).

The system uses a `UEngineSubsystem` for lifecycle management, an Observer pattern
for log consumers, and asynchronous file writing on a dedicated background thread
to keep the game thread overhead under 0.1ms per call.

---

## Compatibility

| UE Version | Status |
|---|---|
| 5.7 | ✅ Supported |

---

## Contributing

LogFlow is open source and contributions are welcome.

To report a bug, open an Issue using the **Bug Report** template. Include your UE5 version, plugin version, steps to reproduce and relevant log output.

To propose a new feature, open a Discussion before submitting a pull request so the idea can be evaluated before any implementation work begins.

To contribute code, read [CONTRIBUTING.md](CONTRIBUTING.md) for branch conventions, commit format and pull request guidelines. All contributions go through a pull request into `develop`.

---

## License

LogFlow is released under the [MIT License](LICENSE).

---

## Credits

Developed by **Gonzalo Blanch** (Mirador Works).