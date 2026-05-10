# LogFlow

A custom logging plugin for Unreal Engine 5 with a dockable editor panel and automatic session log files.

![UE5 5.3](https://img.shields.io/badge/UE5-5.3-blue)
![UE5 5.4](https://img.shields.io/badge/UE5-5.4-blue)
![UE5 5.5](https://img.shields.io/badge/UE5-5.5-blue)
![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)
![Status: In Development](https://img.shields.io/badge/Status-In%20Development-orange)

## Overview

LogFlow is a focused logging system for Unreal Engine 5 that helps developers isolate their own gameplay and tool logs from engine noise. It addresses the common workflow problem where critical project messages get buried inside UE5's internal Output Log during PIE sessions.

The plugin is built around two core components: a dockable editor panel for real-time log viewing (with filtering, search, tags, and color coding) and automatic per-session `.txt` log file generation for persistent review.

## Features

- Dockable editor panel with real-time log display during PIE
- Three severity levels: Log, Warning, Error with distinct color coding
- C++ and Blueprint API (single line call)
- Automatic session `.txt` file generation per PIE session
- Configurable tags with custom colors per tag
- Real-time filtering by severity and tag
- Text search within the panel
- Configurable timestamp per entry (session time or system time)
- Copy to clipboard (single entry or full visible log)
- Auto-clear panel on PIE start (configurable)
- Message counter per severity in panel header
- Break on Error: auto-pause PIE on Error entries (configurable)
- Session history with configurable limit
- Log Viewer: dedicated editor window to browse and read past sessions

## Installation

1. Clone or download this repository into your project's `Plugins/` folder (or use a symlink during development).
2. Regenerate project files.
3. Enable the plugin in **Edit → Plugins → LogFlow**.

## Quick Start — C++

```cpp
#include "LogFlowSubsystem.h"

void UMyGameplayComponent::ReportState()
{
    if (ULogFlowSubsystem* LogFlow = GEngine->GetEngineSubsystem<ULogFlowSubsystem>())
    {
        LogFlow->LogMessage(TEXT("Player spawned"), TEXT("Gameplay"));
        LogFlow->LogWarning(TEXT("Health is below 25%"), TEXT("Combat"));
        LogFlow->LogError(TEXT("Inventory data failed validation"), TEXT("Inventory"));
    }
}
```

## Quick Start — Blueprint

`LogFlow`, `LogWarning`, and `LogError` nodes are available in any Blueprint under the **LogFlow** category, and the **Tag** parameter is optional.

```blueprint
Event BeginPlay
  → LogFlow("Session started", "Gameplay")
  → LogWarning("Low stamina", "Combat")
  → LogError("Missing save slot")
```

## Configuration

All plugin settings are available in **Edit → Editor Preferences → Plugins → LogFlow**. You can configure the log directory, session history limit, timestamp mode, break on error behavior, auto-clear on PIE start, and per-tag colors.

## Architecture

LogFlow is organized into four modules: `LogFlowCore`, `LogFlowEditor`, `LogFlowBlueprintLibrary`, and `LogFlowTests`. Runtime orchestration follows a `UEngineSubsystem` pattern to provide globally accessible logging services with clear lifecycle management. Log consumers use an Observer pattern, and session file writes are performed asynchronously on a secondary thread to keep editor interaction responsive.

## Compatibility

| UE Version | Status |
|---|---|
| 5.3 | ✅ Supported |
| 5.4 | ✅ Supported |
| 5.5 | ✅ Supported |

Platform: Windows. macOS and Linux support is planned for a future release.

## Roadmap

### v1.1 (post-launch)

- Log file per tag
- Side-by-side session comparison in Log Viewer

### v2.0 (future)

- Remote HTTP output
- Packaged build support

## Contributing

LogFlow is an open-source project, and contributions are welcome.

You can follow the development progress and find open tasks on the [LogFlow Project Board](https://github.com/users/miradorworks/projects/1).

Please read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request.

## License

LogFlow is released under the [MIT License](LICENSE).

The plugin is also available for purchase on FAB for users who prefer marketplace distribution.

## Credits

Developed by **Mirador Works**.

LogFlow was conceived during late evenings on a terrace in Ceuta, looking out at the sea.
