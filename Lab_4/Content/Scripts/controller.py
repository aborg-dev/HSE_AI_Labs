import sys
import time

from collections import deque

import unreal_engine as ue
import numpy as np
import cv2
import tensorflow

from memory import ExperienceMemory
from dqn import DQNAgent

GAME = "pong"
ACTIONS = 3 # number of valid actions
GAMMA = 0.99 # decay rate of past observations
OBSERVE = 1000. # timesteps to observe before training
EXPLORE = 200000. # frames over which to anneal epsilon
FINAL_EPSILON = 0.0001 # final value of epsilon
INITIAL_EPSILON = 1 # starting value of epsilon
REPLAY_MEMORY = 50000 # number of previous transitions to remember
BATCH_SIZE = 16 # size of minibatch
FRAME_PER_ACTION = 1

def transformImage(image):
    image = cv2.cvtColor(cv2.resize(image, (80, 80)), cv2.COLOR_BGR2GRAY)
    image = np.reshape(image, (80, 80, 1))
    return image

class MatchResults(object):
    def __init__(self, reward, playtime):
        self.reward = reward
        self.playtime = playtime

class GameHistory(object):
    def __init__(self, history_size):
        self.history_size = history_size
        self.matches = deque()
        self.total_reward = 0
        self.total_playtime = 0

    def add_match(self, match):
        self.matches.append(match)
        self.add_stats(match, +1)
        if len(self.matches) > self.history_size:
            oldest_match = self.matches.popleft()
            self.add_stats(oldest_match, -1)

    def add_stats(self, match, sign):
        self.total_reward += sign * match.reward
        self.total_playtime += sign * match.playtime

    def get_average_stats(self):
        matches_len = max(1, len(self.matches))

        return {
            "reward": self.total_reward / matches_len,
            "playtime": self.total_playtime / matches_len,
        }

class AgentTrainer(object):
    def __init__(self):
        # Create session to store trained parameters
        self.session = tensorflow.InteractiveSession()

        # Create agent for training
        self.agent = DQNAgent(ACTIONS)

        # Create memory to store observations
        self.memory = ExperienceMemory(REPLAY_MEMORY)

        # Tools for saving and loading networks
        self.saver = tensorflow.train.Saver()

        # Last action that agent performed
        self.last_action_index = None

        # Deque to keep track of average reward and play time
        self.game_history = GameHistory(REPLAY_MEMORY)

    def load_model(self):
        self.session.run(tensorflow.initialize_all_variables())
        checkpoint = tensorflow.train.get_checkpoint_state("saved_networks")
        if checkpoint and checkpoint.model_checkpoint_path:
            self.saver.restore(self.session, checkpoint.model_checkpoint_path)
            print("Successfully loaded:", checkpoint.model_checkpoint_path)
        else:
            print("Could not find old network weights")

    def save_model(self, step):
        self.saver.save(sess, 'saved_networks/' + GAME + '-dqn', global_step = step)

    def init_training(self):
        # Initialize training parameters
        self.session.run(tensorflow.initialize_all_variables())
        self.epsilon = INITIAL_EPSILON
        self.t = 0
        self.episode_count = 1000000
        self.max_steps = 1000000
        self.last_action_index = 0

    def reset_state(self, initial_state):
        # Get the first state by doing nothing and preprocess the image to 80x80x4
        x_t = initial_state
        x_t = transformImage(x_t)
        self.s_t = np.concatenate((x_t, x_t, x_t, x_t), axis=2)
        self.match_reward = 0
        self.match_playtime = 0
        self.gamma_pow = 1

    def act(self):
        # Choose an action epsilon greedily
        action_index = 0
        if self.t % FRAME_PER_ACTION == 0:
            if np.random.random() <= self.epsilon:
                action_index = np.random.randint(0, ACTIONS)
            else:
                action_index = self.agent.act(self.s_t)
        else:
            action_index = 0 # do nothing
        self.last_action_index = action_index
        return action_index

    def process_frame(self, screen, reward, terminal):
        if not self.last_action_index:
            self.reset_state(screen)
            return

        a_t = np.zeros([ACTIONS])
        a_t[self.last_action_index] = 1
        # scale down epsilon
        if self.epsilon > FINAL_EPSILON and self.t > OBSERVE:
            self.epsilon -= (INITIAL_EPSILON - FINAL_EPSILON) / EXPLORE

        # run the selected action and observe next state and reward
        x_t1, r_t = screen, reward
        if r_t == 0:
            r_t = 0.1
        x_t1 = transformImage(x_t1)
        s_t1 = np.append(x_t1, self.s_t[:, :, :3], axis=2)

        # store the transition in memory
        self.memory.add_experience((self.s_t, a_t, r_t, s_t1, terminal))

        # only train if done observing
        if self.t > OBSERVE:
            self.make_train_step()

        # update the old values
        self.s_t = s_t1
        self.t += 1

        # save progress every 10000 iterations
        if self.t % 10000 == 0:
            self.save_model(self.t)

        # print info
        if self.t % 100 == 0:
            state = ""
            if self.t <= OBSERVE:
                state = "observe"
            elif self.t > OBSERVE and self.t <= OBSERVE + EXPLORE:
                state = "explore"
            else:
                state = "train"

            ue.log("TIMESTEP {}, EPSILON {}, GAME_STATS {}".format(
                self.t, self.epsilon, self.game_history.get_average_stats()))

        if terminal:
            self.game_history.add_match(MatchResults(self.match_reward, self.match_playtime))
            self.reset_state(screen)
        else:
            self.match_reward += r_t * self.gamma_pow
            self.match_playtime += 1
            self.gamma_pow *= GAMMA

    def make_train_step(self):
        # sample a minibatch to train on
        minibatch = self.memory.sample(BATCH_SIZE)

        # get the batch variables
        s_j_batch, a_batch, r_batch, s_j1_batch, terminal_batch = zip(*minibatch)
        action_scores_batch = np.array(self.agent.score_actions(s_j1_batch))
        y_batch = r_batch + GAMMA * (1 - np.array(terminal_batch)) * np.max(action_scores_batch, axis=1)

        self.agent.train(y_batch, a_batch, s_j_batch)



ue.log("Python version: ".format(sys.version))

def sign(x):
    if x > 0:
        return 1.0
    if x < 0:
        return -1.0
    return 0.0

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
        self.current_score = Score(0, 0)
        self.trainer = AgentTrainer()
        self.trainer.init_training()

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
    def tick(self, delta_seconds : float):
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

        # if not screen is None:
        #     ue.log("Screen shape: {}".format(screen.shape))
        #     cv2.imwrite("/tmp/screen.png", screen)
        # else:
        #     ue.log("Screen is not available")

        finish_time = time.clock()
        elapsed = finish_time - start_time
        # ue.log("Delta seconds: {}, time elapsed: {}".format(delta_seconds, elapsed))
