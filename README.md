# Koten

[![License](https://img.shields.io/github/license/MayronDAV/Koten.svg)](https://github.com/MayronDAV/Koten/blob/master/LICENSE)
[![image](https://github.com/MayronDAV/Koten/workflows/linux/badge.svg)](https://github.com/MayronDAV/Koten/actions?query=workflow%3Alinux)
[![image](https://github.com/MayronDAV/Koten/workflows/windows/badge.svg)](https://github.com/MayronDAV/Koten/actions?query=workflow%3Awindows)

A cross-platform C++ game engine, currently only supports OpenGL

## 🚀 Getting Started

### Prerequisites
- Git
- Mono
- OpenGL 4.5+
- C++17 compatible compiler
  - **Windows**: Visual Studio 2022
  - **Linux**: GCC 11+ or Clang

---

### 📥 Installation

#### Clone the Repository
```bash
git clone --recursive https://github.com/MayronDAV/Koten
```

If the repository was cloned non-recursively previously, use:
```bash
git submodule update --recursive --init
```
to clone the necessary submodules

---

### 🔧 Dependencies

- Windows: 
  - Run the [GenVS22.bat](https://github.com/MayronDAV/Koten/blob/main/Scripts/Windows/GenVS22.bat) file found in `Scripts\Windows` folder.
    
- Linux:
  - Install dependencies:
    ```bash
    sudo apt update && sudo apt upgrade -y
    ```
    ```bash
    sudo apt install -y build-essential g++-11 gcc-11 libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxxf86vm-dev mesa-common-dev libgtk-3-dev mono-complete
    ```

---

### 🛠️ Building

- Windows:
  - Option 1: Run the [Build.bat](https://github.com/MayronDAV/Koten/blob/main/Build.bat) file found in the root dir.
  - Option 2: Open the `Koten.sln` in Visual Studio.

- Linux:
     ```bash
    chmod u+r+x Build-Linux.sh && ./Build-Linux.sh
     ```

---

### 🤝 Contributing

  1. Fork the project

  2. Create your feature branch (git checkout -b feature/AmazingFeature)

  3. Commit your changes (git commit -m 'Add some amazing feature')

  4. Push to the branch (git push origin feature/AmazingFeature)

  5. Open a Pull Request

---

### 📜 License

Distributed under the [Apache-2.0 License](https://github.com/MayronDAV/Koten/blob/master/LICENSE). See LICENSE for more information.

