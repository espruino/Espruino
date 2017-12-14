# How to GIT for Espruino(ESP32)
Target of this is to be a help working with Git for ESP32 branch of Espruino.
Lets assume, that you already worked with Git for your own project.
If you are a total beginner, please go to Google and search for documents.
To be honest, I did this some years ago, and don’t know for sure how this worked ;-)

Lets assume, we already have forked a repository from `https://github.com/espruino/Espruino` into our repository.
Lets give it the name OurRepository and open a cmd for our git folder in documents. Usually I take Windows Power Shell
for this.
BTW, don’t forget to replace `OurRepository` by the name of your repository.
There is no way, that I know, to sync a fork directly. Only way is to sync our local copy first and then sync forket
repository from your local computer.
First step is to clone sources to your local computer.

```
git clone https://github.com/OurRepository/Espruino.git
```

Thats easy, depending on your connection, this will take more or less time. During this process you will see
something like this.

```
Cloning into ‘Espruino’…
remote: Counting objects: 33631, done.
remote: Compressing objects: 100% (111/111), done.
remote: Total 33631 (delta 41), reused 0 (delta 0), pack-reused 33517
Receiving objects: 100% (33631/33631), 57.69 MiB | 5.21 MiB/s, done.
Resolving deltas: 100% (22537/22537), done.
Checking out files: 100% (2268/2268), done.
```

Next we switch to the new directory, named `Espruino`.

```
$ cd Espruino
```

Now its time to switch to the branch for ESP32.

```
git checkout ESP32
```

Well its time for a more complex action. The original repository is usually some steps ahead to your fork.
Means Gordon made some changes and these changes are not copied to your fork automatically.
Before you can synchronize your fork from the original repository, an upstream needs to be created.
This step has to be done only the first time.

```
git remote add upstream https://github.com/espruino/Espruino.git
```

Before using this, its a good idea to check actual status.

```
git remote -v
```

should see something like this

```
origin  https://github.com/OurRepository/Espruino.git (fetch)
origin  https://github.com/OurRepository/Espruino.git (push)
upstream https://github.com/espruino/Espruino.git (fetch)
upstream https://github.com/espruino/Espruino.git (push)
```

Upstream is available, so lets use it. Let’s sync our local copy from the ESP32 branch in original repository
for Espruino.

```
git pull upstream ESP32
```

Of course, there is a log from Git where you can see what happened. There will be some lines like this.

```
remote: Counting objects: 71, done.
remote: Compressing objects: 100% (71/71), done.
remote: Total 71 (delta 31), reused 0 (delta 0), pack-reused 0
Unpacking objects: 100% (71/71), done.
From https://github.com/espruino/Espruino
branch ESP32 -> FETCH_HEAD
[new branch] ESP32 -> upstream/ESP32
Updating 1f7a760..9e2b4db
Fast-forward
Makefile |6 +-
boards/ESP32.py |2 +-
...
...
targets/esp32/tests/simple_web_server.js |14 +
targets/esp32/tests/timer1.js |3 +
16 files changed, 1271 insertions(+), 46 deletions(-)
create mode 100644 libs/network/esp32/network_esp32.c
create mode 100644 libs/network/esp32/network_esp32.h
...
```

If you already have some changes for the source, its a good idea to do them now. Obviously, you can also save all the
new things you got from Espruino(ESP32) directly to your fork.

```
git push origin ESP32
```

Once again you should get a log which looks like this

```
Counting objects: 71, done.
Delta compression using up to 4 threads.
Compressing objects: 100% (71/71), done.
Writing objects: 100% (71/71), 18.27 KiB | 0 bytes/s, done.
Total 71 (delta 45), reused 0 (delta 0)
remote: Resolving deltas: 100% (45/45), completed with 14 local objects.
To https://github.com/OurRepository/Espruino.git
1f7a760..9e2b4db  ESP32 -> ESP32
```

Well we are done with, our fork is in sync with original Espruino. At least for the ESP32 branch ;-) Git is a very complex
tool and sometimes it looks too complex. Like others I would prefer a nice UI where only a button needs to be pushed and
everything runs like magic. If there is a tool like this, please send me a link ;-)<br>If you want to get some more helpful
info around Git or other it fields, give the guys at SitePoint a try. They have been a big help in creating this tutorial.</p>
