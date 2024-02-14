# Hometask 1: Result of the compilation
- The meaning of **text**, **data**, **bss**, **dec** are:
    - **text** : Size of the code in the Flash memory in bytes.
    - **data** : Size of the Non-Zero Initialized global and static data in bytes.
    - **bss**: Size of the Zero Initialized and Uninitialized global and static data in bytes.
    - **dec**: The sum of **text**, **data** and **bss** in bytes.
- Total amount of Flash used: **text** + **data**
- Total amount of RAM used: **data** + **bss**
- Thus, this code is using 0.13 % of Flash (1336 bytes/1 Mbytes) and 0.80 % of RAM (1544 bytes/192 kbytes)
