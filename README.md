# IOT with esp-idf framework
## Overview

This repository provides a **modular, production-grade software foundation** built on the ESP-IDF and FreeRTOS kernel. It utilizes dedicated components and explicit initialization for precise resource management and system robustness. The architecture ensures **non-blocking, event-driven state** transitions for reliable real-time operation.

## Development Environment

This project requires a specific environment setup based on the Espressif IoT Development Framework (ESP-IDF) to ensure proper compilation and flashing.

### Prerequisites

| Requirement | Version / Specification | Notes |
| :--- | :--- | :--- |
| **Toolchain** | ESP-IDF **v5.x** | Recommended to use the official installer for system path setup. |
| **Operating System** | Linux, macOS, or Windows (via ESP-IDF PowerShell Environment). | |
| **CMake** | Minimum version 3.16 | Included with the standard ESP-IDF installation. |
| **Hardware** | ESP32-S3 Target | Project is configured for the ESP32-S3 series. |

### Setup Instructions

1.  **Obtain ESP-IDF:** Follow the official **ESP-IDF Getting Started Guide** for your operating system.
2.  **Set Environment Variables:** Ensure the `IDF_PATH` variable is set correctly to the root of your ESP-IDF installation.
3.  **Run `idf.py menuconfig`:** This command must be executed first to configure project-specific variables defined in the `Kconfig` file and generate the build system.
4.  **Build Command:** Use the `idf.py build` command to compile all source files and link the executable.