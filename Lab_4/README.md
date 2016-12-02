# TensorFlow + UnrealEngine + Python lab

## Lab tutorial

In this lab we are going to use TensorFlow to train a model for playing Ping Pong inside Unreal Engine! 

### Get Pong
First of all we grab a template with implementation of Ping Pong game.
Many templates are available here: https://forums.unrealengine.com/showthread.php?60150-Learning-Templates-Blueprint-Power
We are interested in this particular template: http://bit.ly/1Mp5f7d (14 - Pong : Physics Based)

The game is fully written using blueprints. We'll need to add several C++ and Python classes.

### Install UnrealEnginePython

First we need to create `ScreenCapturer` C++ class to enable compilation of the project (we will need this class later).

Afterwards go ahead and create folder Plugins in Unreal Engine project directory, and clone the plugin from this repo https://github.com/akashin/UnrealEnginePython.git to this folder:

```bash
mkdir Plugins
cd Plugins
git clone https://github.com/akashin/UnrealEnginePython.git
```

The plugin has some requirements which are described on the main page. TL;DR you need proper python installed.
We are going to use python3 because it allows to annotate function arguments with types, and that helps to get better integration with UE4 (However, python2 would be enough for this tutorial).

When you install the python you need to ensure that the paths in build file are correct and make them point to you python installation directory if they are wrong. The relevant code is located here: https://github.com/akashin/UnrealEnginePython/blob/master/Source/UnrealEnginePython/UnrealEnginePython.Build.cs#L81

Finally, you need to close UE4 editor and remove folders with current project artifacts:
```bash
rm -r Binaries
rm -r Intermediate
```

Then load project again, and it should ask you to recompile everything. Go ahead and agree to do this.
During the compilation, python plugin will also be built. If there are some compilation errors you should copy them to file and try to analyze and resolve them. If you have no luck with this - send the error to me.

When the project is successfully loaded everything should look the same. The only addition that we'll have is the ability to create blueprints based on python classes.

To test this, create new blueprint class based on PyController and call it PyController_BP.

Now we are going to implement this class in python

All python scripts must be located in Content/Scripts directory.

Lets create new file Content/Scripts/controller.py with the following content:

```python
import sys

import unreal_engine as ue

ue.log("Python version: ".format(sys.version))

class PythonAIController(object):

    # Called at the started of the game
    def begin_play(self):
        ue.log("Begin Play on PythonAIController class")

    # Called periodically during the game
    def tick(self, delta_seconds : float):
        ue.log("Tick on PythonAIController class")
```

Note that the basic methods of the python controller class are the same as for C++ class.

To connect this class with our blueprint go to its settings and set `Python Module` to `controller.py` and `Python Class` to `PythonAIController`.

The final step would be to use this controller to manipulate one of the pawns in the game. To do this go to Player_Paddle and set it's controller to `PyController_BP` and disable auto posess player.

Go ahead and run the game and keep tracking the output log. You should see messages from our python controller prefixed with "LogPython:".

### Implement actual strategy

To make out controller useful we will need to implement the strategy that don't just look writes messages to log, but actually does something :)

The first step towards this would be to obtain game information, for example current position of the ball.
One simple way to do this is to add be able to access GameMode class from python code. To achieve this we do the following:

* Go to Paddle_BP and add a new blueprint variable of type reference to Pong Game Mode.
* Set this variable during BeginPlay event (this is a simple blueprint exercise, you can see the example of getting the game mode in lower branch of Paddle blueprint)
* Go to python code and add the following to the `PythonAIController` class:
```python
def get_ball_position(self, game_mode):
    if not game_mode:
        return 0
    return game_mode.Ball_Ref.get_actor_location().z

def tick(self, delta_seconds : float):
    pawn = self.uobject.GetPawn()
    ball_position = self.get_ball_position(pawn.GameMode)
    ue.log("Ball position: {}".format(ball_position))
```
* Run the game and check that log contains current position of the ball

Then we will need to communicate decisions of our controller to game logic.

## Additional materials

DQN networks implemented in TensorFlow for
* Flappy Bird: https://github.com/yenchenlin/DeepLearningFlappyBird
* Pong and tetris: https://github.com/asrivat1/DeepLearningVideoGames

Awesome RL resources: https://github.com/aikorea/awesome-rl

OpenCV for OSX: http://seeb0h.github.io/howto/howto-install-homebrew-python-opencv-osx-el-capitan/

DeepRL for TensorFlow: https://github.com/carpedm20/deep-rl-tensorflow
