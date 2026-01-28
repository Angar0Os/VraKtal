# VraKtal Engine

> VraKtal is a C++ Vulkan-based rendering engine developed as a master’s
final-year technical project. Its goal is to serve as a technical
showcase for advanced real-time rendering techniques, including ray
tracing, modern GPU programming, and flexible rendering architecture
design.

The project is developed by: Noukho, TheGeekFire, HideNknow, and Your
Name.

## How to install

### Requirements

**Vulkan**

* Vulkan SDK version 1.4 or **higher is mandatory**
* **Only ONE Vulkan SDK installation** must be present on the machine

**Additional Requirements**

* C++20 compatible compiler
* GPU compatible with Vulkan Ray Tracing extensions
* Windows
* CMake

## Project Installation \& Setup

1. Clone the repository

   git clone https://github.com/YourRepo/VraKtal.git
   cd VraKtal

2. Install Vulkan SDK (1.4+)

   Ensure only one Vulkan SDK version is installed and correctly
   configured.

3. Install CMake (3.5+)
4. In the terminal do : 
   	*mkdir build
   	cd build
   	cmake .. -G "Visual Studio 17 2022"
   	cmake --build .*
5. Generate project files
6. Build the Engine FIRST
7. Set the Editor as Startup Project

   > \*\*Build the engine, then set VraKtalEditor as startup project and run.\*\*

   ## TODO List

   ### Rendering

* ☐ Ray Tracing
* ☐ TAA
* ☐ Denoising
* ☐ Stratified sampling, blue noise, importance sampling
* ☐ Restir DI
* ☐ Restir GI
* ☐ Render Graph system

  ### Editor

* ☐ Editor Core
* ☐ Light Tool
* ☐ Inspector
* ☐ Material Tool
* ☐ Scene Editor

  ### Engine \& Tools

* ☐ Serialization system

  ## License

  This project is under MIT license.

