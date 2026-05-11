# 🏦 Banker's Algorithm — Deadlock Avoidance Simulator

> A complete Operating Systems project featuring **two implementations** of Dijkstra's Banker's Algorithm:
> a classic **C command-line tool** and a modern **Qt Widgets GUI simulator** with randomized multi-demo scenarios, step-by-step visualization, and educational logging.

---

## 📋 Table of Contents

- [Overview](#-overview)
- [Project Structure](#-project-structure)
- [Part 1 — C CLI Implementation](#-part-1--c-cli-implementation)
  - [Features](#features-cli)
  - [Build & Run (CLI)](#build--run-cli)
- [Part 2 — Qt GUI Simulator](#-part-2--qt-gui-simulator)
  - [Features](#features-gui)
  - [Prerequisites](#prerequisites)
  - [Build & Run (GUI)](#build--run-gui)
  - [Using the Application](#using-the-application)
  - [Load Demo — Randomized Scenarios](#-load-demo--randomized-scenarios)
- [Algorithm Explained](#-algorithm-explained)
- [Screenshots / Demo](#-screenshots--demo)
- [Troubleshooting](#-troubleshooting)

---

## 🔍 Overview

The **Banker's Algorithm** is a deadlock-avoidance algorithm developed by Edsger W. Dijkstra. It treats each process like a bank customer that declares its maximum resource needs upfront. The OS (the "banker") only grants a resource request if doing so keeps the system in a **safe state** — i.e., there exists at least one ordering of processes that allows every process to eventually finish.

This project implements and visualizes the full algorithm including:

- **Safety Algorithm** — determines whether a given allocation state is safe
- **Resource-Request Algorithm** — tests whether a new request can be granted
- **Need Matrix** — computed as `Need[i][j] = Maximum[i][j] − Allocation[i][j]`

---

## 📁 Project Structure

```
OS project/
├── Bankers-Algorithm-For-Deadlock-Prevention/   ← You are here
│   ├── solution.c          # C CLI implementation
│   ├── solution            # Compiled binary (Linux)
│   ├── banker_log.txt      # Sample execution log
│   └── README.md           # This file
│
└── BankersGUI/             # Qt C++ GUI simulator
    ├── BankersGUI.pro      # Qt project file
    ├── build.sh            # One-command build script
    ├── build/              # Build output (BankersGUI binary)
    └── src/
        ├── main.cpp
        ├── banker.h / banker.cpp       # Core algorithm + 10 demo scenarios
        ├── mainwindow.h / mainwindow.cpp
        ├── mainwindow_slots.cpp        # All UI slot logic
        └── visualizer.h / visualizer.cpp
```

---

## ⌨ Part 1 — C CLI Implementation

### Features (CLI)

- Interactive terminal prompts for process/resource counts
- Reads Allocation, Maximum, and Available matrices from stdin
- Computes the Need matrix automatically
- Runs the Safety Algorithm and prints the safe sequence
- Simulates individual resource requests with rollback on unsafe results
- Saves execution log to `banker_log.txt`

### Build & Run (CLI)

**Requirements:** GCC (any modern version), Linux/macOS/WSL

```bash
# Navigate to the CLI directory
cd "Bankers-Algorithm-For-Deadlock-Prevention"

# Compile
gcc -o solution solution.c -Wall

# Run
./solution
```

**Example session:**

```
Enter number of processes: 5
Enter number of resource types: 3
Enter Allocation Matrix:
  P0: 0 1 0
  P1: 2 0 0
  P2: 3 0 2
  P3: 2 1 1
  P4: 0 0 2
Enter Maximum Matrix:
  P0: 7 5 3
  ...
Enter Available resources: 3 3 2

✔  System is in a SAFE state.
Safe Sequence: P1 → P3 → P4 → P0 → P2
```

---

## 🖥 Part 2 — Qt GUI Simulator

### Features (GUI)

| Feature | Description |
|---|---|
| **Dynamic Matrix Editor** | Editable Allocation & Maximum tables — any size up to 20×10 |
| **Live Need Calculation** | Need matrix updates instantly as you type |
| **Full Safety Algorithm** | One-click run with colour-coded step trace |
| **Step-by-Step Mode** | Walk through each algorithm iteration with `⏭ Step Execution` |
| **Resource Request Simulation** | Test arbitrary requests; safe ones are applied, unsafe ones roll back |
| **🎲 Randomized Load Demo** | 10 pre-built scenarios (5 safe + 5 unsafe) selected randomly each click |
| **Safe/Unsafe Banners** | Green gradient banner for safe, red gradient for unsafe — auto-shown after demo load |
| **Process Visualizer** | Animated node graph showing the safe sequence or blocked processes |
| **Educational Console** | Timestamped log with colour-coded reasoning for every algorithm step |
| **Save / Load Scenarios** | Persist your custom scenarios to `.bkr` files |
| **Dark / Light Theme** | Toggle with one button |

---

### Prerequisites

#### Ubuntu / Debian / Linux Mint

```bash
sudo apt update
sudo apt install -y qtbase5-dev qttools5-dev-tools g++ make
```

#### Fedora / RHEL

```bash
sudo dnf install -y qt5-qtbase-devel gcc-c++ make
```

#### Arch Linux

```bash
sudo pacman -S qt5-base gcc make
```

#### macOS (Homebrew)

```bash
brew install qt@5
export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"
```

#### Windows (MSYS2 / MinGW)

```bash
pacman -S mingw-w64-x86_64-qt5 mingw-w64-x86_64-gcc make
```

> **Verify Qt is installed:**
> ```bash
> qmake --version
> # Should print: QMake version 3.x, Using Qt version 5.x.x
> ```

---

### Build & Run (GUI)

#### Option A — One-command build script (recommended)

```bash
cd "BankersGUI"
chmod +x build.sh
./build.sh
```

The binary will be at `BankersGUI/build/BankersGUI`. The script auto-runs `qmake` and `make`.

#### Option B — Manual build

```bash
cd "BankersGUI"
mkdir -p build && cd build
qmake ../BankersGUI.pro -spec linux-g++ CONFIG+=release
make -j$(nproc)
./BankersGUI
```

#### Option C — Qt Creator IDE

1. Open Qt Creator
2. **File → Open File or Project** → select `BankersGUI/BankersGUI.pro`
3. Configure the kit (Qt 5 + GCC/Clang)
4. Press **▶ Run** (Ctrl+R)

---

### Using the Application

#### Quick Start — 3 clicks to see the algorithm

1. **Click `📂 Load Demo`** — loads a random scenario, runs the safety check, and shows the result instantly
2. **Click `▶ Run Algorithm`** — re-runs the algorithm with full step logging
3. **Click `⏭ Step Execution`** — replay the algorithm one step at a time with highlights

#### Manual Data Entry

1. Set **Processes** and **Resources** count in the left panel
2. Click **`⚡ Generate Tables`**
3. Fill in the **Allocation** and **Maximum** matrices by clicking cells
4. The **Need** matrix updates automatically as you type
5. Edit the **Available Resources** row
6. Click **`▶ Run Algorithm`**

#### Simulate a Resource Request

1. Load or enter a scenario
2. In the right panel, select a process from the **Process** dropdown
3. Enter the request vector in the **Request Vector** table
4. Click **`📨 Simulate Request`**
   - ✅ **Granted** — system stays safe, allocation updated in-place
   - ⏳ **Wait** — resources not currently available
   - ❌ **Denied** — would cause unsafe state, automatically rolled back

---

### 🎲 Load Demo — Randomized Scenarios

Every click of **`📂 Load Demo`** randomly selects one of **10 built-in scenarios**:

| # | Scenario Name | Processes | Resources | State |
|---|---|:-:|:-:|:-:|
| 1 | Silberschatz Classic | 5 | 3 | ✅ Safe |
| 2 | Small System | 3 | 2 | ✅ Safe |
| 3 | Heterogeneous Resources | 4 | 4 | ✅ Safe |
| 4 | Medium Scale | 6 | 3 | ✅ Safe |
| 5 | Resource-Rich | 4 | 3 | ✅ Safe |
| 6 | Circular Deadlock | 4 | 3 | 🚫 Unsafe |
| 7 | Over-Allocated R0 | 3 | 2 | 🚫 Unsafe |
| 8 | One Blocker | 5 | 3 | 🚫 Unsafe |
| 9 | All-Held Deadlock | 4 | 4 | 🚫 Unsafe |
| 10 | Starved Process | 5 | 2 | 🚫 Unsafe |

After loading, the simulator automatically:
- Populates all matrices with a **flash animation**
- Runs the Banker's Safety Algorithm
- Shows a **green banner** (safe) or **red banner** (unsafe)
- Highlights safe rows in **green**, blocked rows in **red**, partial executions in **yellow**
- Logs the full Available vector, Need matrix, and safety reasoning in the console

---

## 📐 Algorithm Explained

### Key Matrices

| Matrix | Size | Meaning |
|---|---|---|
| `Allocation[i][j]` | n × m | Resources of type j currently held by process i |
| `Maximum[i][j]` | n × m | Maximum resources of type j that process i may ever need |
| `Need[i][j]` | n × m | Still-needed: `Maximum[i][j] − Allocation[i][j]` |
| `Available[j]` | 1 × m | Number of free instances of resource type j |

### Safety Algorithm (pseudo-code)

```
Work  ← Available
Finish[i] ← false for all i

loop until no progress:
    find i such that: Finish[i] == false AND Need[i] ≤ Work
    if found:
        Work  ← Work + Allocation[i]
        Finish[i] ← true
        append i to safe sequence

if all Finish[i] == true → SAFE STATE
else                      → UNSAFE STATE
```

### Resource-Request Algorithm (pseudo-code)

```
if Request[i] > Need[i]       → ERROR (exceeds declared max)
if Request[i] > Available     → WAIT  (resources not free)

tentatively:
    Available  ← Available  − Request[i]
    Allocation ← Allocation + Request[i]
    Need       ← Need       − Request[i]

run Safety Algorithm:
    if SAFE  → GRANT  (keep new allocation)
    if UNSAFE → DENY  (rollback to previous state)
```

---

## 🖼 Screenshots / Demo

Run the application and click **`📂 Load Demo`** several times to cycle through all 10 scenarios and see both safe and unsafe states with full visual feedback.

---

## 🔧 Troubleshooting

| Problem | Fix |
|---|---|
| `qmake: command not found` | Install Qt5 dev tools: `sudo apt install qtbase5-dev` |
| `cannot find -lGL` | Install: `sudo apt install libgl1-mesa-dev` |
| App won't start: `libQt5Widgets.so not found` | Install runtime: `sudo apt install libqt5widgets5` |
| Build fails on macOS | Run: `export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"` before building |
| Blank window on first launch | Resize the window — Qt splitter may need a manual resize on first paint |
| `make: Nothing to be done` | Run `make clean` then `make` again, or delete the `build/` directory |

---

## 👨‍💻 Tech Stack

| Component | Technology |
|---|---|
| GUI Framework | Qt 5 Widgets (C++17) |
| Build System | qmake |
| CLI Implementation | C (C99), GCC |
| Randomization | `std::mt19937` + `std::uniform_int_distribution` |
| Algorithm Source | Silberschatz, Galvin & Gagne — *Operating System Concepts* |

---

## 📄 License

This project is developed for educational purposes as part of an Operating Systems course.
