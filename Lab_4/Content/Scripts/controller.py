import sys

import unreal_engine as ue
import numpy as np
import cv2

ue.log("Python version: ".format(sys.version))

def sign(x):
    if x > 0:
        return 1.0
    if x < 0:
        return -1.0
    return 0.0

class PythonAIController(object):

    # Called at the started of the game
    def begin_play(self):
        ue.log("Begin Play on PythonAIController class")

    def get_ball_position(self, game_mode):
        if not game_mode:
            return 0
        return game_mode.Ball_Ref.get_actor_location().z

    def get_screen(self, game_mode):
        if not game_mode:
            return None

        screen_capturer = game_mode.ScreenCapturer
        screenshot = np.array(screen_capturer.Screenshot)
        H = screen_capturer.Height
        W = screen_capturer.Width

        if len(screenshot) == 0:
            return None

        return screenshot.reshape((H, W, 3), order='F').swapaxes(0, 1)

    # Called periodically during the game
    def tick(self, delta_seconds : float):
        pawn = self.uobject.GetPawn()
        ball_position = self.get_ball_position(pawn.GameMode)
        pawn_position = pawn.get_actor_location().z
        pawn.MovementDirection = sign(ball_position - pawn_position)

        ue.log("Delta seconds: {}".format(delta_seconds))
        screen = self.get_screen(pawn.GameMode)
        if not screen is None:
            ue.log("Screen shape: {}".format(screen.shape))
            cv2.imwrite("/tmp/screen.png", 255.0 * screen)
        else:
            ue.log("Screen is not available")
