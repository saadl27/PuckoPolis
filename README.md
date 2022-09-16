# Introduction
Welcome in the very first lab of MICRO-315

# Part 1 - Integrated Development Environment (IDE)
Click on one the links accordingly to you computer's configuration
- 👉 [❖ Windows](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/Installing-the-IDE-%E2%9D%96-Windows)
- 👉 [🍎 MacOS](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/Installing-the-IDE-%F0%9F%8D%8E-MacOS)
- 👉 [🐧 Linux](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/Installing-the-%F0%9F%90%A7-Linux)

The IDE now freshly installed, let's start by quickly presenting IDE which we will use throuhought this whole semester.
- 👉 [🛠 Tools](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/IDE-%F0%9F%9B%A0-Tools)
- 👉 [🗔 User Interface](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/IDE-%F0%9F%97%94-User-Interface)

# Part 2 - Presenting the EPuck2
- 👉 [Presenting the EPuck2](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/EPuck2-Presenting-the-EPuck2)
- 👉 [Testing the EPuck2](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/EPuck2-Testing-the-EPuck2)

# Part 3 - Introduction Git
- 👉 [Click here](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/Git-Introduction-to-Git)

# Part 4 - Setting up your group TP's repository
- Click 👉 [here](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/Git-Setting-up-git-for-TPs) to set up git for the TPs
- Click 👉 [here](https://github.com/EPFL-MICRO-315/TPs-Student/wiki/Git-Working-in-groups) to learn how to work in group using Git

# Part 5 - STM32F4 Microcontroller and GPIO configuration

## `Task1`
>Use a simple delay function to make **LED7** blink in a while loop at 1 Hz

```c
// LED7 - GPIOD 11
#define LED7_PORT GPIOD
#define LED7_PIN 11
// Enable GPIOD peripheral clock
RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
// Output mode : MODERy = 01
LED7_PORT->MODER = (LED7_PORT->MODER & ~(3 << (LED7_PIN * 2))) | (1 << (LED7_PIN * 2));
// Output type open-drain : OTy = 1
LED7_PORT->OTYPER |= (1 << LED7_PIN);
// Output data low : ODRy = 0
LED7_PORT->ODR &= ~(1 << LED7_PIN);
// Floating, no pull-up/down : PUPDRy = 00
LED7_PORT->PUPDR &= ~(3 << (LED7_PIN * 2));
// Output highest speed : OSPEEDRy = 11
LED7_PORT->OSPEEDR |= (3 << (LED7_PIN * 2));
```

