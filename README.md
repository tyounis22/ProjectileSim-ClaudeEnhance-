# Projectile Simulator

![Start screen](images/start_screen.png)

## How to install

Pick the section for your operating system. The same `make` command builds the game on MacOS and Windows.

### macOS

These steps use [Homebrew](https://brew.sh), a package manager for macOS. If you don't already have it, install it by pasting this into your terminal and following the prompts:

```/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"```

**1. Get the code.** If you don't have git, install it first with `brew install git`. Then download the code and move into the project folder:

```
git clone https://github.com/ehochw01/Projectile-Sim.git
cd Projectile-Sim
```

**2. Install raylib:**

```brew install raylib```

**3. Compile and run:**

```make && ./sim```

That's it — Ready to Play! 🎉

> **Linux:** the Makefile builds on Linux too — install raylib via your package manager (e.g. `sudo apt install libraylib-dev`), then `make && ./sim`.

### Windows

The easiest setup on Windows is [w64devkit](https://github.com/skeeto/w64devkit/releases), which bundles `g++`, `make`, and git in one download.

**1. Install w64devkit.** Download the latest release, unzip it, and run `w64devkit.exe` to open its terminal. Run all the commands below inside that terminal.

**2. Get the code:**

```
git clone https://github.com/ehochw01/Projectile-Sim.git
cd Projectile-Sim
```

**3. Install raylib.** Download the raylib Windows release from the [raylib releases page](https://github.com/raysan5/raylib/releases) (pick the `mingw-w64` build) and unzip it, for example to `C:\raylib\raylib`.

**4. Compile and run.** The Makefile expects raylib at `C:/raylib/raylib`. If you put it there, just run:

```make && sim.exe```

If you unzipped it somewhere else, point the build at it:

```make RAYLIB_PATH=C:/your/path/to/raylib && sim.exe```

That's it — Ready to Play! 🎉

## How to play

Use your computer arrow keys (UP, DOWN, LEFT, RIGHT) to change the aim direction of the cannon.

Hold the space bar to begin firing the cannon. The longer you hold the space bar, the more powerful the shot will be. Release the space bar to fire. 

Aim for the bullseye targets on the screen. Each target is a set of concentric rings, and the more centered your hit, the more points you score: the outer ring is worth 1 point, climbing up to 5 points for a dead-center bullseye. After being struck, a target briefly disappears before reappearing at a new spot, and every 3rd hit, it also shrinks and changes color, making it progressively harder to hit.

You start with a limited number of misses (shown as "Misses Left" at the bottom of the screen). A shot counts as a miss if it sails past the target or is too weak to ever reach it. When you run out of misses, the game ends and a Game Over screen displays your final score.

Make sure to consider wind when aiming your shot. 

## Live reload during development

> **Note:** `make watch` is macOS/Linux only, since it relies on `entr` and a Unix shell. On Windows, just rerun `make` manually after each change.

To automatically rebuild and rerun the simulation every time you save a source or header file, use the `watch` target.

First install [entr](https://eradman.com/entrproject/) (a lightweight file watcher):

```brew install entr```

Then run:

```make watch```

This watches all files in `src/` and `include/`. Whenever you save a change, the running sim closes, only the changed files recompile, and the sim relaunches. Press `Ctrl-C` to stop watching.

Notes:
- If a build fails, the old sim won't relaunch. The compiler error shows in the terminal and the watcher keeps waiting for your next save.
- If you *add* a brand-new source file, restart `make watch` so it picks up the new file (and remember to add it to `SRCS` in the Makefile).

## Folder structure

```
include/   header files (.h)
src/       implementation files (.cpp)
Makefile
README.md
```

Headers live in ```include/``` and are compiled with the ```-Iinclude``` flag, so source files include them by bare filename (e.g. ```#include "Cannon.h"```) without needing a relative path.

## Inheritance structure of classes

The root class is ```Entity``` which has a position.
It also has a ```virtual``` destructor method.
It has a ```virtual``` update method which takes in a framerate. 
It has a ```virtual``` draw method which renders the entity to the screen. 

Class ```PhysicsBody``` inherits from entity and implements the update class with all of the relevant physics math. Entites that move with gravity / wind resistance need to inherit from ```PhysicsBody```. This includes bounce mechanics and randomly generated wind. ```PhysicsBody``` defines the gravitational force, wind, and bounce mechanics of a given set of rounds. Wind changes every three round. 

The ```Projectile``` class implements the draw method of entity. It represents the ball being launched, and uses raylib's ```DrawSphere()``` API method. It takes a position, radius and color. 

The ```Debris``` class also inherits from ```PhysicsBody```, so it reuses the same gravity and bounce physics for free (it only implements ```Draw()```). Debris are the colored fragments that erupt from a target when it is struck.

The ```Target``` class inherits directly from ```Entity``` (it is static, so it does not need ```PhysicsBody```). It is drawn as a bullseye of concentric vertical rings facing the cannon using raylib's ```DrawCylinderEx()``` method. Its ```CheckHit()``` method tests whether the ball crossed the disk's plane within the disk's face on a given frame, firing exactly once per pass-through instead of repeatedly while the ball overlaps a solid volume. It returns which ring was struck (0 for a miss, 1 for the outer ring, up to 5 for a dead-center bullseye) so the caller can score the hit. Its ```Missed()``` method reports when a shot can no longer score, either because it sailed past the target's plane or is too weak to ever reach it.

Then there is the ```Cannon``` class which does NOT inherit from the ```Entity``` class as it is not subject to gravity or wind for it's movement. Only user input can move the cannon, changing its aim direction. The ```Cannon.Fire()``` method takes a ```Projectile``` object by reference, and then affects it's physics metrics. 

main.cpp contains ```DrawWorld()``` which renders the the game environment appearance. It also contains ```DrawWindHUD()``` which will render the wind compass in the top right corner of the screen for the user to consider when shooting a ball. 


