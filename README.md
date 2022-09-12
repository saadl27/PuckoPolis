# Introduction
Welcome in the very first lab of MICRO-315

# Part 1 - Integrated Development Environment (IDE)
Click on one the links accordingly to you computer's configuration
- 👉 [❖ Windows](https://github.com/epfl-mobots/MICRO-315-TPs-Student/wiki/Installing-the-IDE-%7C-%E2%9D%96-Windows)
- 👉 [🍎 MacOS]()
- 👉 [🐧 Linux]()

The IDE now freshly installed, let's start by quickly presenting IDE which we will use throuhought this whole semester.
- 👉 [🛠 Tools](https://github.com/epfl-mobots/MICRO-315-TPs-Student/wiki/IDE-%7C%F0%9F%9B%A0-Tools)
- 👉 [🗔 User Interface](https://github.com/epfl-mobots/MICRO-315-TPs-Student/wiki/IDE-%7C-%F0%9F%97%94-User-Interface)

# Part 2 - Presenting the EPuck2
- 👉 [Click here](https://github.com/epfl-mobots/MICRO-315-TPs-Student/wiki/EPuck2-%7C-Presenting-the-EPuck2)
## Demo program 1
- open in VSCode EPuck2 the folder under Workplace_EPuck2/Lib
- run the task `Make ST`
- go in the **Run and Debug** and click on `Start Debugging` OR press `F5`
- run the task `Run ST`

## Demo program 2
- still in the folder Workplace_EPuck2/Lib
- run the task `Make e-puck2_main-processor`
- go in the **Run and Debug** and click on `Start Debugging` OR press `F5`
- run the task `Run EPuckMonitor`

# Part 3 - Presenting Git
- 👉 [Click here](https://github.com/epfl-mobots/MICRO-315-TPs-Student/wiki/Git-Presenting-Git)
- take inspiration [here]https://rogerdudler.github.io/git-guide/)
## Exercise with git
Let's 

# Part 4 - Setting up your group TP's repository
- This part explains how to set up the github repository that you will BOTH be using during the labs
- It is very important to use the git tool
- Only ONE person needs to perform the initialization of the github repository, this person will have to follow the steps in Computer1's section
- The other person will have to follow the steps in Computer2's section
- ⚠ The two members of the group must create github account if they do not own one
## Computer 
- Launch a terminal in the Workplace_EPuck2 directory
- Clone the repository in the Workplace
  ```
  git clone https://github.com/epfl-mobots/MICRO-315-TPs-Student TPs
  cd TPs
  ```
- Let's check the already existing remotes and branches
  ```
  $ git remote -v
  origin        https://github.com/epfl-mobots/MICRO-315-TPs-Student (fetch)
  origin        https://github.com/epfl-mobots/MICRO-315-TPs-Student (push)
  ```
  - As we can see, there is one remote named **origin** pointing to the url of the cloned repository
  - in git, the remote named **origin** is the remote used by default when doing push/pull
  - However, you have no right to push (write) anything on this github repository
  - The goal is to have a repository you own in which you are administrators
  - 💡 A **remote** is a version of repository stored elsewhere, usually on a server 
- First rename the remote **origin** to **upstream**, 
- Rename the **origin** remote into **upstream** -> https://github.com/epfl-mobots/MICRO-315-TPs-Student is no longer the default remote
  ```
  $ git remote rename origin upstream
  $ git remote -v
  upstream      https://github.com/epfl-mobots/MICRO-315-TPs-Student (fetch)
  upstream      https://github.com/epfl-mobots/MICRO-315-TPs-Student (push)
  ```
- Now create the github repository which will store your work's group during the labs
  - ⚠ Create ONLY ONE github repository per group
  - In fact there is no point having two repositories per group, one repository per group will makes sharing of your work much easier
  - Click on `New Repository`
    <p float="left">
      <img src="https://github.com/epfl-mobots/MICRO-315-TPs-Student/blob/TP1_Exercise/pictures/new_repo_1.PNG" alt="drawing" width="500"/>
    </p>
  - Name it as you wish, for instance **MICRO-315-TPs**
    <p float="left">
      <img src="https://github.com/epfl-mobots/MICRO-315-TPs-Student/blob/TP1_Exercise/pictures/new_repo_2.PNG" alt="drawing" width="500"/>
    </p>
  - Add your workmate in the collaborator list
    <p float="left">
      <img src="https://github.com/epfl-mobots/MICRO-315-TPs-Student/blob/TP1_Exercise/pictures/collaborator.PNG" alt="drawing" width="500"/>
    </p>
    
  - your workmate will receive an invitation by e-mail that he must accept
  - That's it !
  - 💡 Ignore github's tips on quick setup
- Specify a new remote origin repository, replace the github link with the one of your repo
  ```
  $ git remote add origin https://github.com/username/MICRO-315-TPs
  ```
- Verify the new remote
  $ git remote -v
  origin          https://github.com/username/TPs (fetch)
  origin          https://github.com/username/TPs (push)
  upstream        https://github.com/epfl-mobots/MICRO-315-TPs-Student (fetch)
  upstream        https://github.com/epfl-mobots/MICRO-315-TPs-Student (push)
  ```
- Checkout to TP1_Exercise branch and push the changes to your repo
  ```
  $ git checkout TP1_Exercise
  Switched to a new branch 'TP1_Exercise'
  branch 'TP1_Exercise' set up to track 'upstream/TP1_Exercise'.
  $ git push -u origin
  Enumerating objects: 38, done.
  Counting objects: 100% (38/38), done.
  Delta compression using up to 4 threads
  Compressing objects: 100% (27/27), done.
  Writing objects: 100% (35/35), 315.13 KiB | 78.78 MiB/s, done.
  Total 35 (delta 5), reused 34 (delta 5), pack-reused 0
  remote: Resolving deltas: 100% (5/5), done.
  remote:
  remote: Create a pull request for 'TP1_Exercise' on GitHub by visiting:
  remote:      https://github.com/username/TPs/pull/new/TP1_Exercise
  remote:
  To https://github.com/username/TPs
   * [new branch]      TP1_Exercise -> TP1_Exercise
  branch 'TP1_Exercise' set up to track 'origin/TP1_Exercise'. <----- The branch now tracks origin as specified by git push -u origin !!
  ```
- As a final check, verify the content of the file .git/config
  ```
  [core]
          repositoryformatversion = 0
          filemode = false
          bare = false
          logallrefupdates = true
          symlinks = false
          ignorecase = true
  [remote "upstream"]
          url = https://github.com/epfl-mobots/MICRO-315-TPs-Student
          fetch = +refs/heads/*:refs/remotes/upstream/*
  [branch "main"]
          remote = origin
          merge = refs/heads/main
  [remote "origin"]
          url = https://github.com/username/TPs
          fetch = +refs/heads/*:refs/remotes/origin/*
  [branch "TP1_Exercise"]
          remote = origin
          merge = refs/heads/TP1_Exercise
  ```
  - This config file is one of the main git config files,
  - It is local to your copy of the repository, thus a clone of https://github.com/username/TPs won't contain the content .git/config
  - This file tells us many things
  - In this case, the branches **main** and **TP1_Exercise**'s are tracking **origin**
  - This means you don't have to specify the remote when performing a `git push main` or `git push TP1_Exercise` 
## Computer 2
- Execute this steps only once the work on computer 1 is over
- Launch a terminal in the Workplace_EPuck2 directory
- Clone your workmate repository
  ```
  & git clone https://github.com/username/TPs
  ```
- Add the remote pointing to https://github.com/epfl-mobots/MICRO-315-TPs-Student
  ```
  & git remote add upstream https://github.com/epfl-mobots/MICRO-315-TPs-Student
  ```
- Checkout to TP1_Exercise branch
  ```
  $ git checkout TP1_Exercise
  ```

# Part 5 - STM32F4 Microcontroller and GPIO configuration
  
