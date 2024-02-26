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
    >The wikis will be adapted soon with this convention.

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
