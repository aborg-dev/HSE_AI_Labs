import tensorflow as tf
import numpy as np
import cv2
import sys


from scipy.ndimage.filters import maximum_filter

from dqn import DQNAgent

from memory import ExperienceMemory
from memory import MatchResults, GameHistory
from memory import Episode, EpisodeHistory


# TODO: Think where this should be located
def transformImage(image):
    image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    _, image = cv2.threshold(image, 1, 255, cv2.THRESH_BINARY)
    image = maximum_filter(image, size=(2, 4))
    _, image = cv2.threshold(image, 1, 255, cv2.THRESH_BINARY)
    image = cv2.resize(image, (80, 80), cv2.INTER_AREA)
    _, image = cv2.threshold(image, 1, 255, cv2.THRESH_BINARY)
    return np.reshape(image, (80, 80, 1)).astype(np.uint8)


class AgentTrainer(object):
    def __init__(self, config):
        # Create session to store trained parameters
        self.session = tf.Session()

        self.action_count = config["action_count"]

        # Create agent for training
        self.agent = DQNAgent(self.action_count)

        # Create memory to store observations
        self.memory = ExperienceMemory(config["replay_memory_size"])

        # Tools for saving and loading networks
        self.saver = tf.train.Saver()

        # Last action that agent performed
        self.last_action_index = None

        # Deque to keep track of average reward and play time
        self.game_history = GameHistory(config["match_memory_size"])

        # Deque to store losses
        self.episode_history = EpisodeHistory(config["replay_memory_size"])

        self.INITIAL_EPSILON = config["initial_epsilon"]
        self.FINAL_EPSILON = config["final_epsilon"]
        self.OBSERVE = config["observe_step_count"]
        self.EXPLORE = config["explore_step_count"]
        self.FRAME_PER_ACTION = config["frame_per_action"]
        self.GAMMA = config["gamma"]
        self.LOG_PERIOD = config["log_period"]
        self.BATCH_SIZE = config["batch_size"]

    def init_training(self):
        # Initialize training parameters
        self.session.run(tf.global_variables_initializer())
        self.epsilon = self.INITIAL_EPSILON
        self.t = 0
        self.last_action_index = None

    def load_model(self, path):
        checkpoint = tf.train.get_checkpoint_state(path)
        if checkpoint and checkpoint.model_checkpoint_path:
            self.saver.restore(self.session, checkpoint.model_checkpoint_path)
            print("Successfully loaded: {}".format(checkpoint.model_checkpoint_path))
        else:
            print("Could not find old network weights")

    def save_model(self, path):
        # Replace with os.path.join
        self.saver.save(self.session, path + "/dqn", global_step=self.t)

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
        if self.t % self.FRAME_PER_ACTION == 0:
            if np.random.random() <= self.epsilon:
                action_index = np.random.randint(0, self.action_count)
            else:
                action_index = self.agent.act(self.session, self.s_t)
        else:
            action_index = self.last_action_index  # do the same thing as before
        self.last_action_index = action_index
        return action_index

    def process_frame(self, screen, reward, terminal):
        if self.last_action_index is None:
            self.reset_state(screen)
            return

        a_t = np.zeros([self.action_count])
        a_t[self.last_action_index] = 1

        # scale down epsilon
        if self.epsilon > self.FINAL_EPSILON and self.t > self.OBSERVE:
            self.epsilon -= (self.INITIAL_EPSILON - self.FINAL_EPSILON) / self.EXPLORE

        # run the selected action and observe next state and reward
        x_t1, r_t = screen, reward
        x_t1 = transformImage(x_t1)
        s_t1 = np.append(x_t1, self.s_t[:, :, :3], axis=2)

        # store the transition in memory
        self.memory.add_experience((self.s_t, a_t, r_t, s_t1, terminal))

        # only train if done observing
        if self.t > self.OBSERVE:
            loss = self.make_train_step()
            self.episode_history.add_episode(Episode(loss))

        # update the old values
        self.s_t = s_t1
        self.t += 1

        # print info
        if self.t % self.LOG_PERIOD == 0:
            print("TIMESTEP {}, EPSILON {}, EPISODE_STATS {}, MATCH_STATS {}".format(
                self.t,
                self.epsilon,
                self.episode_history.get_average_stats(),
                self.game_history.get_average_stats()))
            sys.stdout.flush()

        self.match_reward += r_t * self.gamma_pow
        self.match_playtime += 1
        self.gamma_pow *= self.GAMMA

        if terminal:
            self.game_history.add_match(MatchResults(
                self.match_reward,
                self.match_playtime,
                reward))
            self.reset_state(screen)

    def make_train_step(self):
        # sample a minibatch to train on
        minibatch = self.memory.sample(self.BATCH_SIZE)

        # get the batch variables
        s_j_batch = [d[0] for d in minibatch]
        a_batch = [d[1] for d in minibatch]
        r_batch = [d[2] for d in minibatch]
        s_j1_batch = [d[3] for d in minibatch]

        # get the batch variables
        # s_j_batch, a_batch, r_batch, s_j1_batch, terminal_batch = zip(*minibatch)
        action_scores_batch = np.array(self.agent.score_actions(self.session, s_j1_batch))
        # r_future = GAMMA * (1 - np.array(terminal_batch)) * np.max(action_scores_batch, axis=1)
        # y_batch = r_batch + r_future

        y_batch = []
        for i in range(0, len(minibatch)):
            # if terminal only equals reward
            if minibatch[i][4]:
                y_batch.append(r_batch[i])
            else:
                y_batch.append(r_batch[i] + self.GAMMA * np.max(action_scores_batch[i]))

        return self.agent.train(self.session, y_batch, a_batch, s_j_batch)
