# Task 1: Result of the compilation
- The meaning of **text**, **data**, **bss**, **dec** are:
    - **text** : Size of the code in the Flash memory in bytes.
    - **data** : Size of the Non-Zero Initialized global and static data in bytes.
    - **bss**: Size of the Zero Initialized and Uninitialized global and static data in bytes.
    - **dec**: The sum of **text**, **data** and **bss** in bytes.
- Total amount of Flash used: **text** + **data**
- Total amount of RAM used: **data** + **bss**
- Thus, this code is using 0.13 % of Flash (1336 bytes/1 Mbytes) and 0.80 % of RAM (1544 bytes/192 kbytes)

For the point `6.3 Testing your EPuck2` in the **TPIntro_Exercise** when you tried to switch the `BODY_LED` by **uncommenting** the `TEST_NEEDED` define in `main.h`, you would have noticed that it remained switched OFF despite having changed the output state to `1`.

If you have programmed and tested your e-puck2 as explained at [Testing the EPuck2](https://github.com/EPFL-MICRO-315/TPs-Wiki/wiki/EPuck2-Testing-the-EPuck2#introduction) you must have noticed that BODY_LED could be properly controlled.

The answer is that the configuration of the output in open-drain mode does not allow to force the output to `1` but only to `0` or `tri-state` and in this last one, the external pull-down force to `0`. It would be necessary to configure the output in push-pull for this, or possibly activate the internal pull-up but it is not "nice".

But you will see all this at the next TP ;-)

> [!TIP]
> Use the EPuck Monitor test when unsure whether the state of your e-puck2 is a code problem or hardware issue. Or also simply to test features, such as measuring the sensitivity of proximity sensors or the TOF, checking the camera acquisition, etc.