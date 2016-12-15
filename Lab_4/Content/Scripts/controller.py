import sys
import time
import logging
import os

# LOG_PATH = "/Users/acid/Documents/HSE_AI_Labs/Lab_4/logs"  # path to logs
# sys.stderr = open(os.path.join(LOG_PATH, "stderr.txt"), "w")
# sys.stdout = open(os.path.join(LOG_PATH, "stdout.txt"), "w")

from collections import deque

import unreal_engine as ue
import numpy as np
import tensorflow as tf
import cv2

from scipy.ndimage.filters import maximum_filter

from memory import ExperienceMemory
from dqn import DQNAgent

# Model
GAME = "pong"
ACTIONS = 3  # number of valid actions
GAMMA = 0.99  # decay rate of past observations
OBSERVE = 10000.  # timesteps to observe before training
EXPLORE = 2000000.  # frames over which to anneal epsilon
FINAL_EPSILON = 0.0001  # final value of epsilon
INITIAL_EPSILON = 1  # starting value of epsilon
REPLAY_MEMORY = 100000  # number of previous transitions to remember
MATCH_MEMORY = 1000  # number of previous matches to remember
BATCH_SIZE = 64  # size of minibatch
FRAME_PER_ACTION = 1  # ammount of frames that are skipped before every action
MODEL_PATH = "/home/acid/Repos/HSE_AI_Labs/Lab_4/saved_networks"  # path to saved models
SNAPSHOT_PERIOD = 10000  # periodicity of saving current model

# Training
NUM_THREADS = 3  # number of threads for tensorflow session

# Logging
LOG_PERIOD = 100  # periodicity of logging
LOG_PATH = "/home/acid/Repos/HSE_AI_Labs/Lab_4/logs"  # path to logs
LOG_FILE = os.path.join(LOG_PATH, "tf.log")  # path to saved models
LOG_TIMINGS = False  # Whether to log controller speed on every tick
tf.logging.set_verbosity(tf.logging.DEBUG)

handler = logging.FileHandler(LOG_FILE)
handler.setLevel(logging.DEBUG)
handler.setFormatter(logging.Formatter(logging.BASIC_FORMAT, None))
logging.getLogger('tensorflow').addHandler(handler)


ue.log("Python version: ".format(sys.version))


c = 0


def transformImage(image):
    global c
    image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    thr, image = cv2.threshold(image, 1, 255, cv2.THRESH_BINARY)
    if c < 500:
        cv2.imwrite("/tmp/screen_full_{}.png".format(c), image)
    image = maximum_filter(image, size=(2, 4))
    if c < 500:
        cv2.imwrite("/tmp/screen_blur_{}.png".format(c), image)

    thr, image = cv2.threshold(image, 1, 255, cv2.THRESH_BINARY)

    if c < 500:
        cv2.imwrite("/tmp/screen_thr_{}.png".format(c), image)

    image = cv2.resize(image, (80, 80), cv2.INTER_AREA)
    thr, image = cv2.threshold(image, 1, 255, cv2.THRESH_BINARY)
    if c < 500:
        cv2.imwrite("/tmp/screen_small_{}.png".format(c), image)
        c += 1
    image = np.reshape(image, (80, 80, 1)).astype(np.float) / 255.0
    return image


class MatchResults(object):
    def __init__(self, reward, playtime, score):
        self.reward = reward
        self.playtime = playtime
        self.score = score


class GameHistory(object):
    def __init__(self, history_size):
        self.history_size = history_size
        self.matches = deque()
        self.total_reward = 0
        self.total_playtime = 0
        self.total_score = 0

    def add_match(self, match):
        self.matches.append(match)
        self.add_stats(match, +1)
        if len(self.matches) > self.history_size:
            oldest_match = self.matches.popleft()
            self.add_stats(oldest_match, -1)

    def add_stats(self, match, sign):
        self.total_reward += sign * match.reward
        self.total_playtime += sign * match.playtime
        self.total_score += sign * match.score

    def get_average_stats(self):
        matches_len = max(1, len(self.matches))

        return {
            "reward": self.total_reward / matches_len,
            "playtime": self.total_playtime / matches_len,
            "score": self.total_score / matches_len,
        }


class Episode(object):
    def __init__(self, loss):
        self.loss = loss


class EpisodeHistory(object):
    def __init__(self, history_size):
        self.history_size = history_size
        self.episodes = deque()
        self.total_loss = 0

    def add_episode(self, episode):
        self.episodes.append(episode)
        self.add_stats(episode, +1)
        if len(self.episodes) > self.history_size:
            oldest_episode = self.episodes.popleft()
            self.add_stats(oldest_episode, -1)

    def add_stats(self, episode, sign):
        self.total_loss += sign * episode.loss

    def get_average_stats(self):
        episodes_len = max(1, len(self.episodes))

        return {
            "loss": self.total_loss / episodes_len,
        }


class AgentTrainer(object):
    def __init__(self):
        # Create session to store trained parameters
        self.session = tf.Session(
            config=tf.ConfigProto(intra_op_parallelism_threads=NUM_THREADS))

        # Create agent for training
        self.agent = DQNAgent(ACTIONS)

        # Create memory to store observations
        self.memory = ExperienceMemory(REPLAY_MEMORY)

        # Tools for saving and loading networks
        self.saver = tf.train.Saver()

        # Last action that agent performed
        self.last_action_index = None

        # Deque to keep track of average reward and play time
        self.game_history = GameHistory(MATCH_MEMORY)

        # Deque to store losses
        self.episode_history = EpisodeHistory(REPLAY_MEMORY)

    def init_training(self):
        # Initialize training parameters
        self.session.run(tf.global_variables_initializer())
        self.epsilon = INITIAL_EPSILON
        self.t = 0
        self.last_action_index = 0

    def load_model(self):
        checkpoint = tf.train.get_checkpoint_state(MODEL_PATH)
        if checkpoint and checkpoint.model_checkpoint_path:
            self.saver.restore(self.session, checkpoint.model_checkpoint_path)
            ue.log("Successfully loaded: {}".format(checkpoint.model_checkpoint_path))
        else:
            ue.log("Could not find old network weights")

    def save_model(self, step):
        self.saver.save(self.session, MODEL_PATH + "/" + GAME + "-dqn", global_step=step)

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
                action_index = self.agent.act(self.session, self.s_t)
        else:
            action_index = self.last_action_index  # do the same thing as before
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
        x_t1 = transformImage(x_t1)
        s_t1 = np.append(x_t1, self.s_t[:, :, :3], axis=2)

        # store the transition in memory
        self.memory.add_experience((self.s_t, a_t, r_t, s_t1, terminal))

        # only train if done observing
        if self.t > OBSERVE:
            loss = self.make_train_step()
            self.episode_history.add_episode(Episode(loss))

        # update the old values
        self.s_t = s_t1
        self.t += 1

        # save progress every SNAPSHOT_PERIOD iterations
        if self.t % SNAPSHOT_PERIOD == 0:
            self.save_model(self.t)

        # print info
        if self.t % LOG_PERIOD == 0:
            ue.log("TIMESTEP {}, EPSILON {}, EPISODE_STATS {}, MATCH_STATS {}".format(
                self.t,
                self.epsilon,
                self.episode_history.get_average_stats(),
                self.game_history.get_average_stats()))

        self.match_reward += r_t * self.gamma_pow
        self.match_playtime += 1
        self.gamma_pow *= GAMMA

        if terminal:
            self.game_history.add_match(MatchResults(
                self.match_reward,
                self.match_playtime,
                reward))
            self.reset_state(screen)

    def make_train_step(self):
        # sample a minibatch to train on
        minibatch = self.memory.sample(BATCH_SIZE)

        # get the batch variables
        s_j_batch = [d[0] for d in minibatch]
        a_batch = [d[1] for d in minibatch]
        r_batch = [d[2] for d in minibatch]
        s_j1_batch = [d[3] for d in minibatch]

        # get the batch variables
        # s_j_batch, a_batch, r_batch, s_j1_batch, terminal_batch = zip(*minibatch)
        action_scores_batch = np.array(self.agent.score_actions(self.session, s_j1_batch))
        y_batch = []
        for i in range(0, len(minibatch)):
            # if terminal only equals reward
            if minibatch[i][4]:
                y_batch.append(r_batch[i])
            else:
                y_batch.append(r_batch[i] + GAMMA * np.max(action_scores_batch[i]))
        # r_future = GAMMA * (1 - np.array(terminal_batch)) * np.max(action_scores_batch, axis=1)
        # y_batch = r_batch + r_future

        return self.agent.train(self.session, y_batch, a_batch, s_j_batch)


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
        self.trainer.load_model()

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

        # Log elapsed time
        finish_time = time.clock()
        elapsed = finish_time - start_time
        if LOG_TIMINGS:
            ue.log("Delta seconds: {}, time elapsed: {}".format(delta_seconds, elapsed))
