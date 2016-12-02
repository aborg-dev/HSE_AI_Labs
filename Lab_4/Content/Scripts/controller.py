import sys

import unreal_engine as ue

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
        return game_mode.ScreenCapturer.Screenshot

    # Called periodically during the game
    def tick(self, delta_seconds : float):
        pawn = self.uobject.GetPawn()
        ball_position = self.get_ball_position(pawn.GameMode)
        pawn_position = pawn.get_actor_location().z
        pawn.MovementDirection = sign(ball_position - pawn_position)

        screen = self.get_screen(pawn.GameMode)
        ue.log("Screen size: {}".format(len(screen)))
