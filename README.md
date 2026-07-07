# LAN WiFi P2P Chat Application

A local network peer-to-peer chat app built with C++17 and Qt. Made as an internship project to learn network programming with sockets.

## Demo

![App Screenshot](demo_screenshot.png)

## Quick Start (No Build Required)

If you just want to try the app, **double-click `LANChat.exe`** — all required DLLs are included. No need to install Qt.

To test peer-to-peer messaging, open two instances of `LANChat.exe` on the same machine. They'll find each other automatically.

## What it does
- Automatically finds other users on the same WiFi/LAN using UDP broadcast (like a heartbeat ping every 2 seconds)
- Sends actual messages over TCP with JSON payloads
- Uses QDataStream framing to handle TCP packet splitting/merging properly
- Lets you run multiple instances on one machine since it picks random ports
- You can toggle yourself as visible/invisible to other users
- Saves chat history to a text file and loads it back on startup

## Project structure
```
LANChat/
├── LANChat.pro         # qmake project file
├── include/
│   ├── peerinfo.h      # peer data struct
│   ├── chatmanager.h   # networking logic (UDP + TCP)
│   └── mainwindow.h    # UI controller
├── src/
│   ├── main.cpp
│   ├── chatmanager.cpp
│   └── mainwindow.cpp
├── ui/
│   └── mainwindow.ui   # Qt Designer layout
└── chat_history.txt    # auto-created log file
```

## Requirements (for building from source)
- Windows
- C++17 compiler (MSVC 2019+ or MinGW 8.1+)
- Qt 5.15+ or Qt 6 (needs core, gui, network, widgets modules)

## How to build

### Using Qt Creator
1. Open Qt Creator
2. File -> Open -> select `LANChat.pro`
3. Pick a kit (Desktop Qt MinGW or MSVC)
4. Hit the green Run button (Ctrl+R)

### Command line (MinGW)
Make sure `qmake` and `mingw32-make` are in your PATH.

```cmd
cd LANChat
qmake LANChat.pro
mingw32-make
```

Then run `.\debug\LANChat.exe` or `.\release\LANChat.exe`.

If you're using MSVC, use `nmake` instead of `mingw32-make`.

## Testing

The easiest way to test is to open two instances of the app on the same machine. They'll automatically discover each other within a couple seconds because each instance gets its own random TCP port. Select the other instance from the peer list and send a message - it should show up instantly on the other side.

You can also toggle the "Visible to Nearby Users" checkbox to test the offline/online broadcast. Check `chat_history.txt` to see the raw message log.
