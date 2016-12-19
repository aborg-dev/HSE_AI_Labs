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

### Control the paddle

The next thing is to learn how to communicate decisions of our controller to game logic.
To do this we will change the paddle blueprint to read the actions of the controller from internal variable and then change this variable in python code.

Create a new float variable MovementDirection with value range [-1.0, 1.0] and use it instead GetVerticalMovementAxis blueprint node.
Now go to python code and set this variable to the value that will make paddle to follow the ball:

```python
def sign(x):
    if x > 0:
        return 1.0
    if x < 0:
        return -1.0
    return 0.0

class PythonAIController(object):
    def tick(self, delta_seconds : float):
        pawn = self.uobject.GetPawn()
        ball_position = self.get_ball_position(pawn.GameMode)
        pawn_position = pawn.get_actor_location().z
        pawn.MovementDirection = sign(ball_position - pawn_position)
```

Now run the code and enjoy the infinite ping pong round between bots!

### Implement ScreenCapturer class to take screenshots

We're almost done! To build really mighty strategy we need to also capture the position of the enemy and feed this to the controller to be able to make intelligent decisions.
For this we will build a ScreenCapturer class that will take screenshots of current game field and provide them to python code. This will allow us to use them to train Deep Reinforcement Learning model.

This class will make a screenshot every ScreenshotPeriod seconds and store it to variable Screenshot that will be available for access in python.
The implementation of ScreenCapturer is available here: https://github.com/akashin/HSE_AI_Labs/commit/8cb8c41fa5115b8ae532977a6d694293798925b2

Now let's compile everything and create appropriate blueprint ScreenCapturer_BP.

The final step would be to spawn new instance in PongGameMode blueprint and set it to a variable ScreenshotCapturer of GameMode. Let's do this during BeginPlay event.

Finally, let's test this in python class by printing the shape of current screenshot:

```python
def get_screen(self, game_mode):
    if not game_mode:
        return None
    return game_mode.ScreenCapturer.Screenshot

# Called periodically during the game
def tick(self, delta_seconds : float):
    pawn = self.uobject.GetPawn()
    ball_position = self.get_ball_position(pawn.GameMode)
    pawn_position = pawn.get_actor_location().z
    pawn.MovementDirection = sign(ball_position - pawn_position)

    screen = self.get_screen(pawn.GameMode)
    ue.log("Screen size: {}".format(len(screen)))
```

You should see the size 888864 which is 752 * 394 * 3 (Y * X * Color).

### Process screenshots using python

Next reasonable step would be to verify that screenshots are indeed the right representation of the game field. To do this we will need OpenCV library to work with images.
OpenCV 3 is a C++ library but there are python2 and python3 bindings available.
I found following links useful for installing it on OSX:

* http://tsaith.github.io/install-opencv-3-for-python-3-on-osx.html
* http://seeb0h.github.io/howto/howto-install-homebrew-python-opencv-osx-el-capitan/

For windows, I believe this links might be relevant, though I haven't checked them:

* https://solarianprogrammer.com/2016/09/17/install-opencv-3-with-python-3-on-windows/
* https://www.scivision.co/install-opencv-3-0-x-for-python-on-windows/
* http://stackoverflow.com/a/21212023

We'll also use NumPy library for working with multidimensional arrays.

When you've have opencv3 and numpy installed we can do the following to write the current screen to a file.

```python
import numpy as np
import cv2

H = 394
W = 752

class PythonAIController(object):

    def get_screen(self, game_mode):
        if not game_mode:
            return None
        screenshot = np.array(game_mode.ScreenCapturer.Screenshot)

        if len(screenshot) == 0:
            return None

        return screenshot.reshape((W, H, 3), order='F').swapaxes(0, 1)

    # Called periodically during the game
    def tick(self, delta_seconds : float):
        pawn = self.uobject.GetPawn()
        ball_position = self.get_ball_position(pawn.GameMode)
        pawn_position = pawn.get_actor_location().z
        pawn.MovementDirection = sign(ball_position - pawn_position)

        screen = self.get_screen(pawn.GameMode)
        if not screen is None:
            ue.log("Screen shape: {}".format(screen.shape))
            cv2.imwrite("/tmp/screen.png", 255.0 * screen)
        else:
            ue.log("Screen is not available")
```

This code will transform the image to the proper shape and then write it to the file "/tmp/screen.png". Note that we need to multiply the colors by 255 because they have been normalized in our ScreenshotCapturer.

### Use TensorFlow to train RL model

Out final step in this lab would be to train a DQN model using tensorflow.

#### Set project FPS

We are going to set FPS to 32 (approx. 31ms between frames) to ensure that time distance between consecutive frames is fixed.
This is essential for training reinforcement learning model because it needs to predict the reward in the next state, but if there are several possible next states for the current state (introduced by different delays between frames) then model will have hard times predicting the rewards.

To do this we need to go to *Project settings* and in section *Engine - General settings* go to *Framerate subsection* and first check the checkbox *Use fixed frame rate*, then set *Fixed frame rate* to 32.


#### Change game resolution

Second performance optimization that we need to make in order to achive reasonable rendering speed is to decrease the resolution of the image to 240x240.

To do this go to editor *Preferences* and in section *Editor - Play* change the resolution for game in window to 240x240.
Afterwards, try to run the game using *Run in Window* option. You should see much smaller then usual standalone window with the game.

One problem that you may encounter is that the HUD is broken after resolution change. This may be due to the fact that it wasn't intended to be used on such scale or it's just not resizing automatically. You can fix this either by changing the widget dimensions so that it looks normal again or by trying to setup automatic resize.

#### Patch ScreenCapturer to support different width and height

When we changed the resolution of the game, all our hardcoded resolution values in controller.py won't work any more. We need to make our system more flexible.
First, we start storing Width and Height of the current viewport inside the ScreenCapturer and make this fields accessible to our python controller.
Then we read their values to do proper reshape using numpy.

Check out this commit for details: https://github.com/akashin/HSE_AI_Labs/commit/3cc8f4cc19bf9eafe792ec837f0043e5fdd6e787

#### Take screenshot every tick

Now that we've fixed FPS, we can freely make screenshots every tick and we'll have the guarantee that the time distance between subsequent screenshots is fixed.

## Additional materials

DQN networks implemented in TensorFlow for
* Flappy Bird: https://github.com/yenchenlin/DeepLearningFlappyBird
* Pong and tetris: https://github.com/asrivat1/DeepLearningVideoGames

Awesome RL resources: https://github.com/aikorea/awesome-rl

DeepRL for TensorFlow: https://github.com/carpedm20/deep-rl-tensorflow
