# Introduction
Welcome in the very first lab of MICRO-315

## Goals
- This practical work shows all the necessary steps to program the e-puck2 miniature mobile robot in C, using the standard library provided by ST.
- The main goal is to gain knowledge of the STM32F4 microcontroller and refresh some concepts about peripherals such as GPIOs and TIMERs.
- The second goal and not the least is to present the tools that will be used throughought every labs of this semester

# Part 1 - Integrated Development Environment (IDE)
Click on one the links accordingly to your computer's configuration
- 👉 [❖ Windows](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/Installing-the-IDE-%E2%9D%96-Windows)
- 👉 [🍎 MacOS](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/Installing-the-IDE-%F0%9F%8D%8E-MacOS)
- 👉 [🐧 Linux](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/Installing-the-%F0%9F%90%A7-Linux)

The IDE now freshly installed, let's start by quickly presenting IDE which we will use throuhought this whole semester.
- 👉 [🛠 Tools](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/IDE-%F0%9F%9B%A0-Tools)
- 👉 [🗔 User Interface](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/IDE-%F0%9F%97%94-User-Interface)

# Part 2 - Presenting the EPuck2 robot
- 👉 [Presenting the EPuck2](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/EPuck2-Presenting-the-EPuck2)
- 👉 [Testing the EPuck2](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/EPuck2-Testing-the-EPuck2)

# Part 3 - Git introduction
- 👉 [Click here](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/Git-Introduction-to-Git)

# Part 4 - Setting up your group TP's repository
- Click 👉 [here](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/Git-Setting-up-git-for-TPs) to set up git for the TPs
- Click 👉 [here](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/Git-Working-in-groups) to learn how to work in group using Git

# Part 5 - STM32F4 Microcontroller and GPIO configuration
## 🔌 Preparation 
- Make sure you are in the right branch (TP1_Exercise), if not you must do a **checkout TP1_Exercise**
- Open the folder TPs with VSCode Epuck2
- Run the task `Link Librairy ST to workspace`

## 🔨 Building the Project
- Building the code consists of compiling the **\*.c** and **\*.s** files to create object files, **\**.o*, and then
linking the object files to create the **blinky.elf** file. We will see in more details in TP2 the different
steps of the build process. The generated **.elf** file contains the data necessary to program the device
and additional information that lets you debug at the source code level. The additional files **.list**,
**.size** and **.mem** are generated containing the disassembly output and the memory layout table with
symbol address and size.
  1. Run the task **Make TP1**
  2. Observe the progress of the compilation in the Build Console tabular (as in Listing 1).
  3. When Done is displayed, your code is built and you are ready to program the device. If there
are errors in your code, they will be displayed in this console.
  ```
  > Compiling gpio.c
  > Compiling main.c   
  ar> Compiling timer.c
  m-none-eabi> Compiling stm32f4xx_ll_rcc.c
  -gcc > Compiling system_clock_config.c
  -c -mcpu=cortex-m4 -O0 -ggdb -fomit-frame-pointer -falign-functions=16 -ffunction-sections -fdata-sections -fno-common -Wall -Wextra -Wundef -Wstrict-prototypes -DSTM32F4 -DSTM32F407xx -mthumb -mno-thumb-interwork -MD -MP -I. -I../ST gpio.c -o gpio.o
  > Compiling system_stm32f4xx.c   
  > Compiling startup_stm32f407xx.s
  arm-none-eabi-gcc -c -mcpu=cortex-m4 -O0 -ggdb -fomit-frame-pointer -falign-functions=16 -ffunction-sections -fdata-sections -fno-common -Wall -Wextra -Wundef -Wstrict-prototypes -DSTM32F4 -DSTM32F407xx -mthumb -mno-thumb-interwork -MD -MP -I. -I../ST main.c -o main.o
  arm-none-eabi-gcc -c -mcpu=cortex-m4 -O0 -ggdb -fomit-frame-pointer -falign-functions=16 -ffunction-sections -fdata-sections -fno-common -Wall -Wextra -Wundef -Wstrict-prototypes -DSTM32F4 -DSTM32F407xx -mthumb -mno-thumb-interwork -MD -MP -I. -I../ST timer.c -o timer.o
  arm-none-eabi-gcc -c -mcpu=cortex-m4 -O0 -ggdb -fomit-frame-pointer -falign-functions=16 -ffunction-sections -fdata-sections -fno-common -Wall -Wextra -Wundef -Wstrict-prototypes -DSTM32F4 -DSTM32F407xx -mthumb -mno-thumb-interwork -MD -MP -I. -I../ST ../ST/system_clock_config.c -o ../ST/system_clock_config.o
  arm-none-eabi-gcc -c -mcpu=cortex-m4 -O0 -ggdb -fomit-frame-pointer -falign-functions=16 -ffunction-sections -fdata-sections -fno-common -Wall -Wextra -Wundef -Wstrict-prototypes -DSTM32F4 -DSTM32F407xx -mthumb -mno-thumb-interwork -MD -MP -I. -I../ST ../ST/stm32f4xx_ll_rcc.c -o ../ST/stm32f4xx_ll_rcc.o
  arm-none-eabi-gcc -c -mcpu=cortex-m4 -O0 -ggdb -fomit-frame-pointer -falign-functions=16 -ffunction-sections -fdata-sections -fno-common -Wall -Wextra -Wundef -Wstrict-prototypes -DSTM32F4 -DSTM32F407xx -mthumb -mno-thumb-interwork -MD -MP -I. -I../ST ../ST/system_stm32f4xx.c -o ../ST/system_stm32f4xx.o
  arm-none-eabi-gcc -x assembler-with-cpp -c -mcpu=cortex-m4 -mthumb -mno-thumb-interwork -I. -I../ST ../ST/startup_stm32f407xx.s -o ../ST/startup_stm32f407xx.o
  > Linking blinky.elf
  arm-none-eabi-gcc main.o gpio.o timer.o ../ST/system_clock_config.o ../ST/stm32f4xx_ll_rcc.o ../ST/system_stm32f4xx.o ../ST/startup_stm32f407xx.o   -mcpu=cortex-m4 -O0 -ggdb -fomit-frame-pointer -falign-functions=16 -ffunction-sections -fdata-sections -fno-common -nostartfiles -L./ -mthumb -mno-thumb-interwork -Wl,--no-warn-mismatch,--gc-sections,--script=../ST/STM32F407VGTx_FLASH.ld  -o blinky.elf
  > Creating blinky.mem
  > Creating blinky.list
  > Creating blinky.size
  arm-none-eabi-nm --numeric-sort --print-size blinky.elf > blinky.mem
  arm-none-eabi-objdump -d blinky.elf > blinky.list
  arm-none-eabi-nm --size-sort --print-size blinky.elf > blinky.size
  arm-none-eabi-size blinky.elf
     text    data     bss     dec     hex filename
     1432       4    1540    2976     ba0 blinky.elf
  > Done
  ```
  > `Home Task 1`<br>
  > Try to find out the meaning of the words text, data, bss, dec.<br>
  > Estimate the size of your code in percentage of total Flash and RAM available with this microcontroller.

## 🐞 Programming the EPuck2 robot
- Now that you have built your code and created an **.elf** file that can be loaded on the microcontroller, you can program the device. To program the robot e-puck2, plug the USB cable (there is no need to turn on the e-puck2 using the dedicated button since the programmer MCU will automatically power the main MCU) 
- First you must specify the port to which the gdb-server is connected, click 👉 [here](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/EPuck2-Presenting-the-EPuck2#identify-the-ports) for more info

## 🐛 Debugging the EPuck2 robot
- click 👉 [here](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/IDE-%F0%9F%90%9B-Debugging) for more info
