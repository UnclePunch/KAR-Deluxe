# Build Instructions

This project is written in **C** and uses `make` to manage the build process. It outputs `.bin` (code) and `.dat` (asset) files and can be built on **Windows**, **macOS**, and **Linux**.

---

## Prerequisites

### 1. **[devkitPPC](https://devkitpro.org/wiki/Getting_Started)** 

This contains the required PowerPC compiler. After installing, make sure the DEVKITPPC environment variable is set. You can verify this by typing `echo %DEVKITPPC%` in the Windows command prompt.

### 2. **Python 3**

This project depends on a script written in Python to pack the compiled `.o` file into a `.bin` file to be used by the game. Make sure Python 3 is installed and available in your PATH.

* **Windows**: Download from [python.org](https://www.python.org/downloads/) and check the "Add to PATH" option during installation.
* **Linux/macOS**: Use your package manager (e.g., `sudo apt install python3` or `brew install python`).

---

## Building the Project

Navigate to the root folder of the project and run:

```bash
make all
```

This will:

* Compile and link the source files.
* Pack each mod into a hoshi compatible `.bin` file using the Python script.
* Output `.bin` and asset `.dat` files to the `out/` directory.

---

## Output

Built files are placed in the `/out` directory. These files should be placed in your hoshi iso.

To automatically install the files to your iso filesystem after building the project, run:

```bash
make all install INSTALL_DIR=/path/to/your/iso/filesystem
```
