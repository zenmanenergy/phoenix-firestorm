<img align="left" width="100" height="100" src="doc/firestorm_256.png" alt="Logo of Firestorm viewer"/>

**[Firestorm](https://www.firestormviewer.org) is a free client for 3D virtual worlds such as Second Life and various OpenSim worlds where users can create, connect and chat with others from around the world.**

This repository contains the official source code for the Firestorm viewer.

## Changes in this fork

This fork ([zenmanenergy/phoenix-firestorm](https://github.com/zenmanenergy/phoenix-firestorm), branch multi-monitor-world-view) adds the features and fixes below to Firestorm. Code changes are marked with FS:Steve comments. Settings named here can be changed in Advanced > Show Debug Settings.

### Multiple monitors

- Stretch Window Across All Monitors (Avatar > Multiple Monitors, setting FSSpanAllMonitors): makes the viewer one borderless window covering every monitor. Turning it off puts the window back where it was. It takes priority over the FullScreen setting.
- Keep World View on One Monitor (setting FSLockWorldViewToMonitor): draws the 3D world on one monitor only, so the other monitors become space for floaters. FSWorldViewMonitor picks the monitor: 0 is the primary, 1 and up are the others from left to right.
- The menu bar, status bar, navigation bar, toolbars, chat bar, nearby chat text, login screen and progress screens stay on the world monitor. New floaters open and center there.
- Floaters can be dragged to any monitor and are remembered there.
- Single-monitor and multi-monitor layouts keep separate floater positions, so floaters placed on another monitor don't end up off screen when you go back to one monitor.
- Clicking, hovering or right-clicking on empty space on the other monitors does nothing in-world.
- On Windows, the UI scale follows the world monitor's display scaling.
- HUDs and the edit tools (move, rotate, stretch, grab) work correctly when the world view is offset.
- Alerts appear centered on the world monitor instead of the leftmost monitor.
- The login screen stays clickable in multi-monitor mode.
- Works on Windows and Linux. Not supported on Mac.

### People > Nearby (radar)

- Gender column: M, F or ? (not known yet), read from the avatar's shape. Sorting by gender groups by gender, then by how long they've been seen, so the list doesn't jump around.
- Gender filter at the bottom of the list (setting FSRadarGenderFilter): show everyone, hide male, or hide female. Avatars whose gender isn't known yet are always shown.
- Looks column: counts how many times each person has looked at you or your attachments (from their look-at target). It is bold while they are looking at you right now, and resets when you change regions. Sorting puts the most looks first.
- Idle column: how long since each person last moved, turned, sat, stood, typed, chatted or moved their camera. Shows "Away" with a time when their own viewer marks them away. Counting starts when you first see them.
- Notes after the name: if you have written notes about someone, the notes appear in parentheses after their name, on one line and clipped if long.
- Name colors: green if you have notes about them, red if they have been idle 5 minutes or more (red wins). Settings: FSRadarNotesColor, FSRadarIdleColor and FSRadarIdleColorMinutes. Set a color's alpha to 0 to turn it off.
- Zoom / Cam on someone outside your draw distance now works: the draw distance is raised for the moment (up to 1024 m) until they load, then goes back when you reset your camera (Esc) or teleport.
- Teleport To works for people whose height isn't known: the viewer asks the LSL bridge for their height first, then teleports.
- The seen-time sort is numeric, so 10 minutes sorts after 9 minutes.
- Columns can be turned on and off from the radar options menu.

### Groups

- Region group tags: in a group's profile, check "Wear this group's tag in [REGION]" to make that group active whenever you arrive in the current region. When you leave for a region without a group set, your active group goes back to None. Stored per account (FSRegionGroupTags) and can be turned off with FSRegionGroupTagsEnabled.

### Derender and asset blacklist

- Remove Selected in the asset blacklist now brings people and objects back straight away, instead of leaving them invisible until you teleport or relog. Objects come back from the object cache; people (and anything not cached) are requested again from the server. This covers derenders from the in-world menu, area search and the People list, and items blacklisted in earlier sessions.

### Alerts

- Escape closes alert dialogs. An alert with one button is dismissed; otherwise Escape presses the Cancel, No, Close, Ignore, Not Now, Later or Dismiss button if there is one.

### Fixes

- A duplicate wearable no longer crashes debug (RelWithDebInfo) builds; it is logged as a warning instead.
- The draw distance raised by the radar is never saved as your normal draw distance.

### Building this fork on Windows

Set up the build tools as described in [doc/building_windows.md](doc/building_windows.md), then from a Command Prompt in this folder:

    build-env.bat
    autobuild configure -A 64 -c ReleaseFS_open -- --chan MultiMonitor -DLL_TESTS:BOOL=FALSE
    autobuild build -A 64 -c ReleaseFS_open --no-configure > build-release.txt 2>&1

Configure only needs to be run once, or again when a CMakeLists.txt file changes. Don't click inside the Command Prompt window while it builds, or Windows pauses the build. The viewer is build-vc170-64\newview\Release\firestorm-bin.exe and has to be started with indra\newview as its working folder.

## Open Source

Firestorm is a third party viewer derived from the official [Second Life](https://github.com/secondlife/viewer) client. The client codebase has been open source since 2007 and is available under the LGPL license.

## Download

Pre-built versions of the viewer releases for Windows, Mac and Linux can be downloaded from the [official website](https://www.firestormviewer.org/choose-your-platform/).

## Build Instructions

Build instructions for each operating system can be found using the links below and in the official [wiki](https://wiki.firestormviewer.org).

- [Windows](doc/building_windows.md)
- [Mac](doc/building_macos.md)
- [Linux](doc/building_linux.md)

> [!NOTE]
> We do not provide support for compiling the viewer or issues resulting from using a self-compiled viewer. However, there is a self-compilers group within Second Life that can be joined to ask questions related to compiling the viewer: [Firestorm Self Compilers](https://tinyurl.com/firestorm-self-compilers)

## Contribute

Help make Firestorm better! You can get involved with improvements by filing bugs and suggesting enhancements via [JIRA](https://jira.firestormviewer.org) or [creating pull requests](CONTRIBUTING.md).

## Community respect

This section is guided by the [TPV Policy](https://secondlife.com/corporate/third-party-viewers) and the [Second Life Code of Conduct](https://github.com/secondlife/viewer?tab=coc-ov-file).

Firestorm code is made available during ongoing development, with the **master** branch representing the current nightly build. Developers and self-compilers are encouraged to work on their own forks and contribute back via pull requests, as detailed in the [contributing guide](CONTRIBUTING.md).

If you intend to use our code for your own viewer beyond personal use, please only use code from official release branches (for example, `Firestorm_7.1.13`), rather than from pre-release/preview or nightly builds.
