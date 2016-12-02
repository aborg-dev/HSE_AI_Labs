import sys

import unreal_engine as ue

ue.log("Python version: ".format(sys.version))

class PythonAIController(object):

    # Called at the started of the game
    def begin_play(self):
        ue.log("Begin Play on PythonAIController class")

    def get_ball_position(self, game_mode):
        if not game_mode:
            return 0
        return game_mode.Ball_Ref.get_actor_location().z

    # Called periodically during the game
    def tick(self, delta_seconds : float):
        pawn = self.uobject.GetPawn()
        ball_position = self.get_ball_position(pawn.GameMode)
        ue.log("Ball position: {}".format(ball_position))
