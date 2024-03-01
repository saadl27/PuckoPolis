# TPIntro_Exercise updated !!
There have been quite a few errors in the TPIntro_Exercise README.md file and you have certainly noticed some differences from what you had experienced.

Just as you had to experiment the use of Git therefore you can compare the README.md of the Exercise between the `reference` remote and yours in order to see which changes have been made and try to understand them.

In summary:

1. The terminal commands were rearranged in order to facilitate their copies and avoid bad use. Instead to have a mix between commands and results in the same block, they are now separated as:

    - Commands that can be copied directly (Copy button) to be then passed and executed in a terminal:

        ```shell
        python --version # prints out python version

        ```

    - Commands that can't be copied directly (No Copy button) because they need some modification:
        <div class="box"><pre>
        # Replace X by 1 and 2 successively:
        pyenv local TPIntroX-env
        </pre></div>

    - Output's result of commands:
        <div class="box">output console:<pre>
        Python 3.9.1
        </pre></div>

>[!NOTE]
> The wikis will be adapted soon with this convention.

2. PyEnv:
    - Under Windows the virtual environment can be "simulated" by duplication. It take more disk space but the end user result is the same. Then use the command `pyenv duplicate python_version env_name` for Windows instead `pyenv virtualenv python_version env_name` for MacOS/Linux.

    - Not only for Windows, the `Part 2` has been rewritten to also takes into account a test on a fresh installation that didn’t have Python installed. So it should be more in line with what you had to observe.

3. Git:
    - The `Part 4` has been rewritten and takes into account a test on a fresh installation that does not by default ignore the PyEnv configuration files `.python-version` and correct the `git merge` command.

>[!NOTE]
> ***It would therefore be wise at least to reread these 2 parts!!***

And don't forget to update the TPIntro_Exercise branch locally and on your `origin` remote group repository.

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

But you will see all this at the next TP :stuck_out_tongue_winking_eye:

> [!TIP]
> Then don’t forget to use this test if you doubt the state of your e-puck2 when it might be a code problem or simply to test features, such as measuring the sensitivity of proximity sensors or the TOF, check the camera acquisition, ...