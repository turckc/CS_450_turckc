# CS 450 (Computer Graphics)
***Fall 2025***  
***Author: Caden Turck***  
***Original Author: Dr. Michael J. Reale***  
***SUNY Polytechnic Institute*** 

## Software Dependencies
You will need to install the following manually:
- C++ compilers
- CMake
- Visual Code
- Vulkan SDK

The rest of the dependencies will be automatically fetched by CMake.

### C++ Compilers
* ***For Windows:*** Download and install ONE of the following:
  * [Visual Studio 2022](https://visualstudio.microsoft.com/) (and install the Desktop C++ toolchain) ***(STRONGLY recommended if you are taking another class with me)***
  * [Build Tools for Visual Studio 2022](https://aka.ms/vs/17/release/vs_BuildTools.exe
  ) (and install the Desktop C++ toolchain)
* ***For Linux:*** Install g++ compilers if not already installed.
* ***For Mac:*** The included Clang g++ compilers should be fine.

### CMake
You will need **version 3.24** or higher.

* ***For Windows:***
  * Download the CMake installer from [here](https://cmake.org/download/).
  * If given the option, make sure to **"Add CMake to the system PATH for all users"**.
* ***For Mac:***
  * Download the latest "macOS 10.13 or later" .dmg from [here](https://cmake.org/download/)
  * Install Cmake and run it
  * Select ```"Tools" -> "How to Install for Command Line Use"```
  * Copy the second option into a terminal: ```sudo "/Applications/CMake.app/Contents/bin/cmake-gui" --install```
* ***For Linux:***
  * Install Cmake via your package manager:
```
sudo apt install cmake cmake-gui
```
  * HOWEVER, check the version:
```
cmake --version
```
  * If it is LESS than 3.24, you may have to compile and install from source.


### Visual Code
I strongly recommend Visual Code as your IDE.
* [Download](https://code.visualstudio.com/) and install Visual Code
  * I suggest enabling the context menu options.
  * ***For Mac:*** Follow [these instructions](https://code.visualstudio.com/docs/setup/mac)
* Install the following extensions:
  * **"C/C++ Extension Pack"** by Microsoft
  * **"Shader languages support for VS Code"** by slevesque
  * **"Shaderc GLSL Linter"** by Dave Frampton
  * **"Git Graph"** by mhutchie
  * **"Markdown All in One"** by Yu Zhang

### Vulkan SDK

Download and install the **Vulkan SDK** for your platform:

* ***For Windows:*** Download the appropriate installer [here](https://vulkan.lunarg.com/sdk/home) and run it.
* ***For Mac:*** 
  * Download the appropriate installer [here](https://vulkan.lunarg.com/sdk/home) and run it.
  * Open a terminal; the Vulkan SDK has almost certainly been installed to ```$HOME/VulkanSDK/<version number>```, so ```cd``` to that directory.
  * Run: ```sudo python ./install_vulkan.py```
* ***For Linux:*** Follow instructions found [here](https://vulkan.lunarg.com/doc/sdk/latest/linux/getting_started.html).

### Other Libraries
If not already installed, the following libraries will be automatically fetched into your project and compiled with your code:
- GLFW
- GLM
- Assimp
- vk-bootstrap
- VulkanMemoryAllocator (VMA)

***For Linux:*** Install the following:
```
sudo apt install libwayland-dev libxkbcommon-dev xorg-dev gdb
sudo apt install libglfw3 libglfw3-dev libglm-dev libassimp5 libassimp-dev 
```