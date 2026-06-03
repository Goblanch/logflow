# LogFlow Documentation

## 1. Introduction

LogFlow is a custom logging plugin for Unreal Engine 5 developed by Mirador Works.
It provides a dockable editor panel that displays your log entries in real time 
during Play In Editor (PIE) sessions, and automatically generates a plain text
session file for each PIE run containing only the entries you explicitly register
\- no engine noise, no Output Log clutter.

LogFlow is designed for developers of all profiles: from solo indie developers
who need quick visibility into their game logic, to professional teams who need
structured, filterable and exportable logs as a part of their QA workflow. If you 
are coming from Unity, LogFlow gives you a Console window experience native to 
the UE5 editor.

### What LogFlow is not

LogFlow is not a replacement for Unreal Engine's Output Log. It does not capture
engine-generated messages, warnings or errors. It only records the entries you 
register explicitly through the LogFlow API. This is by design \- the goal is
signal, not noise.

---

## 2. Installation

### Requirements

- Unreal Engine 5.7 or later.
- Windows

### Install from source

1. Clone or download the repository.
```shell
git clone https://github.com/Goblanch/logflow.git 
```
2. Copy or symlink the repository folder into your project's `Plugins/` directory:
```
YourProject/
└── Plugins/
    └── LogFlow/   ← repository contents go here
```
3. Right-click your `.uproject` file and select **Generate Visual Studio Project files**.
4. Open the project \- UE5 will detect the plugin and prompt you to compile it.
5. Once compiled, go to **Edit → Plugins → Developer Tools → LogFlow** and enable it.
6. Restart the editor when prompted.

### Verifying the installation

Once installed, open the editor and go to **Window → LogFlow Panel**. If the
panel opens without errors, the installation is complete.

---

## 3. Quick Start

### 3.1 Logging from C++

Include the LogFlow subsystem headers in your class and call `LogMessage()`
from anywhere in your C++ code:

```c++
#include "LogFlowSubsystem.h"
// Log an informational message
ULogFlowSubsystem::LogMessage(
    TEXT("Player health dropped to zero"),
    ELogFlowSeverity::Log,
    FName("Combat"));

// Log a warning
ULogFlowSubsystem::LogMessage(
    TEXT("Inventory is almost full"),
    ELogFlowSeverity::Warning,
    FName("Inventory"));

// Log an error
ULogFlowSubsystem::LogMessage(
    TEXT("Save file could not be written"),
    ELogFlowSeverity::Error,
    FName("SaveSystem"));
```

The third parameter is the tag and is optional. You can pass `NAME_None`
to log without a tag:

```c++
ULogFlowSubsystem::LogMessage(
    TEXT("Game initialized"),
    ELogFlowSeverity::Log,
    NAME_None);
```

Make sure to add `LogFlowCore` to your module's `Build.cs` dependencies:
```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "Core", "CoreUObject", "Engine",
    "LogFlowCore"
});
```

### 3.2 Logging from Blueprint

Open any Blueprint and search for **LogFlow** in the node search. You will 
find three nodes:

- **Log Message** \- informational entry, neutral color
- **Log Warning** \- warning entry, amber color
- **Log Error** \- error entry, red color

Each node has a **Message** pin and an optional **Tag** pin. The tag pon is 
hidden by default \- click the arrow at the bottom of the node to expand 
advanced pins if you want to assign a tag.

Connect any of these nodes to an execution pin and press Play. The entries
will appear in the LogFlow panel immediately.

### 3.3 Opening the panel

Go to **Window → LogFlow Panel** to open the real-time logging panel.
Go to **Window → LogFlow Viewer** to open the session history viewer.

Both windows are dockable \- you can drag them to any position in the
editor workspace, and they will remember their position between sessions.

---

## 4. Panel reference

### 4.1 LogFlow Panel

The LogFlow Panel is the main real-time logging window. Open it from
**Window → LogFlow Panel**.

### Header controls

| Control | Description |
|---|---|
| **LOG (n)** | Toggle button. Shows or hides Log entries. The number in parentheses is the total count of Log entries in the current session, regardless of active filters. |
| **WRN (n)** | Toggle button. Shows or hides Warning entries. |
| **ERR (n)** | Toggle button. Shows or hides Error entries. |
| **Tag selector** | Dropdown. Filter entries by tag. Select **All** to show entries from all tags. |
| **Search box** | Text field. Filters entries in real time by message content. Case-insensitive. |
| **Clear** | Clears all entries from the panel. Does not affect the session file on disk. |
| **Copy All** | Copies all currently visible entries to the clipboard as plain text, respecting active filters. |

#### Entry columns

| Column | Description |
|---|---|
| **SEV** | Severity label: LOG, WRN or ERR. Color coded. |
| **TIMESTAMP** | Time of the entry. Format: HH:MM:SS.mmm. Displays session time or system time depending on your settings. |
| **TAG** | The tag assigned to the entry. Displayed in the configured tag color. A dash (-) is shown for entries without a tag. |
| **MESSAGE** | The log message text. |

#### Right-click menu

Right-clicking any entry in the panel opens a context menu with a **Copy**
option that copies that single entry to the clipboard in the format:
`[TIMESTAMP] [SEVERITY] [TAG] Message`

#### Entry colors

| Severity | Background color |
|---|---|
| Log | Transparent (default panel background) |
| Warning | Soft amber |
| Error | Soft red |

### 4.2 LogFlow Viewer

The LogFlow Viewer allows you to browse and read past session files.
Open it from **Window → LogFlow Viewer**.

#### Layout

The viewer is divided into two areas separated by a resizable splitter:

- **Left sidebar** - lists all saved sessions sorted from most recent to oldest. Each entry shows the session date, time and file size.
- **Right area** - displays the content of the selected session file.

#### Selecting a session

CLick any session in the left sidebar to load its content in the right area.
The content is color coded by severity using the same scheme the main panel.

#### Search

The search box in the content area header filters lines in real time as you 
type. Matching lines are highlighted. Use the **▲** and **▼** buttons to 
navigate between results. The counter on the right shows the current result 
position (e.g. `3 / 12`).

#### Clipboard

- Right-click any line → **Copy** to copy that line to the clipboard.
- **Copy All** button copies all loaded lines to the clipboard.

---

## 5. Configuration

All LogFlow settings are available under **Edit → Editor Preferences → Plugins → LogFlow**.

### Storage

| Setting | Default | Description |
|---|---|---|
| **Log Directory** | `Saved/LogFlow/` | Directory where session log files are saved, relative to the project root. |
| **Max Session History** | `10` | Maximum number of session files to keep on disk. When the limit is exceeded, the oldest file is deleted automatically. Accepts values between 1 and 100. |

### Display

| Setting | Default | Description |
|---|---|---|
| **Timestamp Mode** | `Session Time` | Controls how timestamps are displayed in the panel and written to session files. **Session Time** shows time elapsed since PIE started. **System Time** shows the wall clock time of your machine. |
| **Tag Colors** | _(empty)_ | Per-tag color configuration. Add an entry for each tag you want to color code. Each entry has a **Tag Name** field and a **Color** field. Changes are reflected immediately in the panel without restarting the editor. |

### Behavior

| Setting | Default | Description |
|---|---|---|
| **Auto-Clear Panel on PIE Start** | `false` | When enabled, the LogFlow panel clears automatically every time you press Play. When disabled, entries from previous sessions remain visible. |
| **Break on Error** | `false` | When enabled, the PIE session pauses automatically whenever an Error entry is registered. A notification entry is added to the panel when this happens. Resume the session normally using the editor's Resume button. This setting has no effect in packaged builds. |


### Session file format

Each session file is a plain text file named with the following pattern:

```
LogFlow_YYYYMMDD_HHMMSS.txt
```

For example: `LogFlow_20260601_143222.txt`

Each line in the file follows this format:

```
[TIMESTAMP][SEVERITY][TAG] Message
```

For example:

```
[00:00:00.012] [LOG    ] [Combat] Player health dropped to zero
[00:00:00.045] [WARNING] [Inventory] Inventory is almost full
[00:00:00.103] [ERROR  ] [SaveSystem] Save file could not be written
```

The severity field is padded to 7 characters for alignment. Entries without 
a tag show a dash in the tag field.

```
[00:00:00.200] [LOG    ] [-] Game initialized
```

---

## 6. API Reference

### 6.1 C++ API

#### ULogFlowSubsystem::LogMessage

```c++
static void LogMessage(
    const FString& Message,
    ELogFlowSeverity Severity,
    FName Tag = NAME_None);
```

Registers a log entry with the LogFlow system. This is the primary entry
point for all logging calls from C++ code.

| Parameter | Type | Description |
|---|---|---|
| `Message` | `FString` | The log message text to display in the panel and write to the session file. |
| `Severity` | `ELogFlowSeverity` | The severity level of the entry. See ELogFlowSeverity below. |
| `Tag` | `FName` | Optional category tag. Pass `NAME_None` or omit to log without a tag. Defaults to `NAME_None`. |

This method is thread-safe and returns immediately. All file I/O and panel updates happen asynchronously on 
background threads.

#### ELogFlowSeverity

```c++
UENUM(BlueprintType)
enum class ELogFlowSeverity : uint8
{
    Log,      // Informational message. Neutral color in the panel.
    Warning,  // Non-critical anomaly. Amber color in the panel.
    Error     // Critical failure. Red color in the panel.
              // Triggers PIE pause if Break on Error is active.
};
```

#### FLogFlowEntry

```c++
USTRUCT(BlueprintType)
struct FLogFlowEntry
{
    FString          Message;     // The log message text.
    ELogFlowSeverity Severity;    // The severity level.
    FName            Tag;         // The category tag. NAME_None if not set.
    FTimespan        Timestamp;   // Time elapsed since PIE session start.
    FDateTime        SystemTime;  // Wall clock time at entry creation.

    static FLogFlowEntry Create(
        const FString& InMessage,
        ELogFlowSeverity InSeverity,
        FName InTag,
        FTimespan InTimestamp);
};
```

#### ILogFlowConsumer

```c++
class ILogFlowConsumer
{
public:
    virtual void OnLogEntryReceived(const FLogFlowEntry& Entry) = 0;
};
```

Interface for custom log consumers. Implement this interface and register
your consumer with `FLogFlowDispatcher` via `ULogFlowSubsystem::GetDispatcher()`
to receive all log entries as they arrive.

Implementation must not block the calling thread. If you need to perform
heavy processing, enqueue the entry and process it on the appropiate thread.

### 6.2 Blueprint API

All three Blueprint nodes are available in any Blueprint under the
**LogFlow** category. They are also findable by searching: `log`, `debug`,
`print`, `warning`, `error`.

#### Log Message

Registers an informational log entry. Displayed with neutral color.

| Pin | Type | Description |
|---|---|---|
| `Message` | String | The log message text. |
| `Tag` | Name | Optional category tag. Hidden by default — expand advanced pins to access. |

#### Log Warning

Registers a warning log entry. Displayed with amber color.

| Pin | Type | Description |
|---|---|---|
| `Message` | String | The warning message text. |
| `Tag` | Name | Optional category tag. Hidden by default — expand advanced pins to access. |

#### Log Error

Registers an error log entry. Displayed with red color. If **Break on Error**
is active in settings, the PIE session pauses automatically when this node
executes.

| Pin | Type | Description |
|---|---|---|
| `Message` | String | The error message text. |
| `Tag` | Name | Optional category tag. Hidden by default — expand advanced pins to access. |

---

## 7. Architecture Overview

This section provides a brief technical overview of how LogFlow is structured 
internally. It is intended for developers who want to extend the plugin or 
understand its design decisions.

### 7.1 Module structure

LogFlow is composed of four modules:

| Module | Type | Description |
|---|---|---|
| **LogFlowCore** | Runtime | Core system. Contains all business logic: entry registration, dispatch, async file writing and session management. Has no dependencies on other LogFlow modules. |
| **LogFlowEditor** | Editor | Editor UI. Contains the dockable panel, the Log Viewer and the Editor Preferences integration. Only compiled in editor builds. Depends on LogFlowCore. |
| **LogFlowBlueprintLibrary** | Runtime | Blueprint API. Exposes the three Blueprint nodes as a thin wrapper over LogFlowCore. Depends on LogFlowCore. |
| **LogFlowTests** | Editor | Automated tests using the UE5 Automation Framework. Not included in the distributed build. |

### 7.2 Key design decisions

**UEngineSubsystem**
`ULogFlowSubsystem` is implemented as a `UEngineSubsystem`. The engine manages its
lifecycle automatically - it is  created when the engine starts and destroyed when 
it shuts down. No manual initialization is required from user code.

**Observer Pattern**
`FLogFlowDispatcher` maintains a list of `ILogFlowConsumer` implementations
and notifies all of them when a new entry arrives. This decouples
`LogFlowCore` from its consumers - the core never references `SLogFlowPanel`
or `FLogFlowFileWriter` directly. Adding a new consumer (such as a remote
HTTP output in a future version) requires no changes to the core.

**Asynchronous file writing**
`FLogFLowFileWriter` runs on a dedicated background thread as an `FRunnable`.
Log entries are enqueued in a thread-safe `TQueue` by the game thread and
dequeued by the writer thread. This ensures that file I/O never occurs on
the game thread, keeping the overhead of `LogMessage()` calls within the 
0.1ms budget defined in the plugins requirements.

**Slate UI**
The editor panel and LogViewer are built with Slate, that native UI framework
of the UE5 editor. This allows them to be registered as dockable tabs in the 
editor workspace and ensures they match the native look and feel of the editor.

### 7.3 Data flow

The following diagram illustrates the flow of a log entry from the moment
it is registered by user code to the moment it appears in the panel and the 
session file:

```
User code (C++ or Blueprint)
│
▼
ULogFlowSubsystem::LogMessage()
│
▼
FLogFlowDispatcher::Dispatch()
│
├──────────────────────────┐
▼                          ▼
FLogFlowFileWriter         SLogFlowPanel
(background thread)        (editor main thread
writes to .txt file)       updates list view)
```

### 7.4 Extending LogFlow

To receive log entries in your own code, implement `ILogFlowConsumer` and 
registers with the dispatcher.

```c++
class FMyConsumer : public ILogFlowConsumer
{
public:
    virtual void OnLogEntryReceived(const FLogFlowEntry& Entry) override
    {
        // Handle the entry
    }
};

// Register
FMyConsumer* MyConsumer = new FMyConsumer();
ULogFlowSubsystem::Get()->GetDispatcher()->RegisterConsumer(MyConsumer);

// Unregister when done
ULogFlowSubsystem::Get()->GetDispatcher()->UnregisterConsumer(MyConsumer);
```

---

## 8. Troubleshooting

### The LogFlow panel does not appear in the Window menu

- Verify the plugin is enabled in **Edit → Plugins → Developer Tools → LogFlow**.
- If the plugin is enabled but the menu entry is missing, try restarting the editor.
- If the plugin fails to compile, check the Output Log for error messages and
  verify that your Unreal Engine version is supported (5.3 or later).

### No entries appear in the panel during PIE

- Verify you are calling `ULogFlowSubsystem::LogMessage()` from code that
  actually executes during PIE — add a breakpoint or a `UE_LOG` call to
  confirm the code is being reached.
- Verify that the LogFlow panel is open before pressing Play. The panel
  registers as a consumer when it is constructed — if it was never opened
  it will not receive entries.
- Verify the severity toggle buttons in the panel header are all active
  (LOG, WRN and ERR should all be highlighted).

### The session file is not being generated

- Check the configured **Log Directory** in Editor Preferences → LogFlow.
  The default is `Saved/LogFlow/` relative to the project root.
- Verify the directory is writable. If your project is on a network drive
  or a OneDrive-synced folder, file locking may prevent the writer from
  creating files.
- The session file is only written during active PIE sessions. Calling
  `LogMessage()` outside of PIE will update the panel but will not write
  to disk.

### The session file exists but is empty

- This can happen if the editor crashes or is force-closed during a PIE
  session before the file writer has flushed its queue. The file writer
  flushes all pending entries when `Stop()` is called at PIE end — a
  clean Stop is required for the file to be complete.
- If this happens consistently, check whether your project is stored in
  a OneDrive or cloud-synced folder. Cloud sync can interfere with file
  handles held open by the writer thread.

### The panel clears automatically even though Auto-Clear is disabled

- Verify the **Auto-Clear Panel on PIE Start** setting in Editor Preferences
  → LogFlow is set to `false`.
- If the setting appears correct but the panel still clears, try changing
  the setting, saving preferences and restarting the editor.

### The PIE session pauses unexpectedly

- **Break on Error** is active in Editor Preferences → LogFlow. Disable it
  if you do not want PIE to pause on Error entries.

### Tag colors are not showing in the panel

- Verify the tag name in **Tag Colors** matches exactly the tag you are
  passing to `LogMessage()` — tag names are case-sensitive.
- Color changes are applied immediately. If colors are not showing after
  adding a tag color, try pressing Play to start a new session.

### The Log Viewer session list is empty

- The session list is populated from the session index file
  `LogFlow_Index.json` stored in the log directory. If no sessions have
  been completed yet (i.e. no PIE session has been started and stopped),
  the list will be empty.
- If you have session files on disk but the list is empty, the index file
  may have been deleted. Starting and completing a new PIE session will
  rebuild the index.

## 9. Changelog

See [CHANGELOG.md](CHANGELOG.md)