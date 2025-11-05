# STM32 Workspace: Real-Time-Systems

This workspace contains several STM32CubeIDE projects and example FreeRTOS projects for the Black Pill (STM32F411) board. The repository is organized as a multi-project workspace (each top-level folder is a project). This README describes the repository structure and gives brief notes about building, debugging, and git housekeeping.

## Repository layout (top-level)
- Example project folders: `Blackpill_00_memmap/`, `Blackpill_01_gpio_config/`, `Buck_converter/`, `freertos_01_blink_task/`, and others.
- Each project folder typically contains the main application code and project configuration (source files, interrupt handlers, and application logic).

Important folders you should keep tracked in the repo:
- `Core/` — source files for the MCU project.
- `Third Party` (or similarly named third-party folder) — external vendor code required by the project.

Folders we do NOT keep tracked in git (listed in `.gitignore`):
- `Drivers/` (local MCU vendor drivers and binary blobs)
- `Debug/`, `Release/`, `.build/` (build output)
- IDE settings: `.vscode/`, `.settings/`, `*.launch`, `*.project`, `*.cproject`
- Build artifacts: `*.o`, `*.elf`, `*.bin`, `*.hex`, `*.map`, etc.

See `.gitignore` for the full ignore list.



## Academic purpose
This repository is used as my notes for the Real-Time Systems course at my university (UAQ).

If you want me to do anything next, contact me at 