import sys
import time

import unreal_engine as ue
import numpy as np
import tensorflow as tf

from agent_trainer import AgentTrainer

# Model
GAME = "pong"
ACTIONS = 3  # number of valid actions
MODEL_PATH = "/home/acid/Repos/HSE_AI_Labs/Lab_4/saved_networks"  # path to saved models
SNAPSHOT_PERIOD = 10000  # periodicity of saving current model
SEED = 42

# Logging
LOG_PATH = "/home/acid/Repos/HSE_AI_Labs/Lab_4/logs"  # path to logs
LOG_TIMINGS = False  # Whether to log controller speed on every tick

config = {
    "action_count": ACTIONS,
    "gamma": 0.99,  # decay rate of past observations
    "observe_step_count": 100,  # timesteps to observe before training
    "explore_step_count": 2000000,  # frames over which to anneal epsilon
    "initial_epsilon": 1.0,  # starting value of epsilon
    "final_epsilon": 0.0001,  # final value of epsilon
    "replay_memory_size": 100000,  # number of previous transitions to remember
    "match_memory_size": 1000,  # number of previous matches to remember
    "batch_size": 64,  # size of minibatch
    "frame_per_action": 1,  # ammount of frames that are skipped before every action
    "log_period": 100,  # periodicity of logging
}

class UnrealEngineOutput:
    def __init__(self, logger):
        self.logger = logger

    def write(self, buf):
        self.logger(buf)

    def flush(self):
        return


sys.stdout = UnrealEngineOutput(ue.log)
sys.stderr = UnrealEngineOutput(ue.log_error)

ue.log("Python version: ".format(sys.version))


class Score(object):
    def __init__(self, cpu_score, player_score):
        self.score = (cpu_score, player_score)

    def update(self, new_score):
        if self.score != new_score:
            reward = (new_score[1] - new_score[0]) - (self.score[1] - self.score[0])
            self.score = new_score
            return reward
        else:
            return 0


def get_action_direction(action):
    if action == 0:
        return 0
    if action == 1:
        return 1
    return -1


class PythonAIController(object):

    # Called at the started of the game
    def begin_play(self):
        ue.log("Begin Play on PythonAIController class")
        np.random.seed(SEED)
        tf.set_random_seed(SEED)
        self.current_score = Score(0, 0)
        self.step_count = 0
        self.trainer = AgentTrainer(config)
        self.trainer.init_training()
        self.trainer.load_model(MODEL_PATH)

    def get_ball_position(self, game_mode):
        if not game_mode:
            return 0
        return game_mode.Ball_Ref.get_actor_location().z

    def get_screen(self, game_mode):
        if not game_mode:
            return None

        screen_capturer = game_mode.ScreenCapturer
        screenshot = np.array(screen_capturer.Screenshot, dtype=np.uint8)
        H = screen_capturer.Height
        W = screen_capturer.Width

        if len(screenshot) == 0:
            return None

        return screenshot.reshape((H, W, 3), order='F').swapaxes(0, 1)

    def get_score(self, game_mode):
        if not game_mode:
            return (0, 0)
        return (game_mode.Cpu_Score, game_mode.Player_Score)

    # Called periodically during the game
    def tick(self, delta_seconds: float):
        start_time = time.clock()

        pawn = self.uobject.GetPawn()
        game_mode = pawn.GameMode
        score = self.get_score(game_mode)

        # Attribute this to previous action
        reward = self.current_score.update(score)
        screen = self.get_screen(pawn.GameMode)

        # Skip frames when no screen is available
        if screen is None or len(screen) == 0:
            return

        self.trainer.process_frame(screen, reward, reward != 0)

        # Make new action
        action = self.trainer.act()
        pawn.MovementDirection = get_action_direction(action)

        self.step_count += 1
        if self.step_count % SNAPSHOT_PERIOD == 0:
            self.trainer.save_model(MODEL_PATH)

        # Log elapsed time
        finish_time = time.clock()
        elapsed = finish_time - start_time
        if LOG_TIMINGS:
            ue.log("Delta seconds: {}, time elapsed: {}".format(delta_seconds, elapsed))
