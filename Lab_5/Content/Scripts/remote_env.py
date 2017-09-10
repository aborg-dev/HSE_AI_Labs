import abc
import socket
import struct
import logging
import msgpack
import scipy.misc

import gym
from gym import error, spaces, utils
from gym.utils import seeding
from gym.envs.classic_control import rendering

import numpy as np


def _receive_data(sock):
    # `sock` is the TCP socket connected to the client
    data_size_buf = sock.recv(4)
    data_size = struct.unpack('i', data_size_buf)[0]

    data = bytearray(data_size)
    view = memoryview(data)
    to_read = data_size
    while to_read > 0:
        nbytes = sock.recv_into(view, to_read)
        view = view[nbytes:]  # slicing views is cheap
        to_read -= nbytes

    # logging.info("{} wrote {} bytes".format(self.client_address[0], len(self.data)))
    unpacked = None
    try:
        unpacked = msgpack.unpackb(data)
        # logging.info("Unpacked data length: {}".format(len(unpacked)))
        # logging.info("Screen data length: {}".format(len(unpacked[5])))
        # print("Unpacked data: {}".format(unpacked))
        return unpacked
    except Exception as e:
        logging.info(e)


def _send_data(sock, action):
    response = struct.pack('c', action.to_bytes(1, byteorder="big"))
    # logging.info("Sending {} bytes".format(len(response)))
    sock.sendall(response)


class RemoteEnv(gym.Env):
    __metaclass__ = abc.ABCMeta

    def __init__(self, host, port):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((host, port))
        self.last_observation = None

    def _seed(self, seed=None):
        pass

    def _decode_game_state(self, message):
        # TODO: Make this method abstract.
        pass

    def _step(self, action):
        """Perform an action and simulate one step in the environment.

        Args:
            action (object): an action provided by the agent.

        Returns:
            observation (object): agent's observation of the current environment.
            reward (float): amount of reward returned after previous action.
            done (boolean): whether the episode has ended, in which case further step()
                calls will return undefined results.
            info (dict): contains auxiliary diagnostic information (helpful for debugging,
                and sometimes learning).
        """
        # print('Action: {}'.format(action))
        _send_data(self.sock, action)
        message = _receive_data(self.sock)
        observation, reward, done = self._decode_game_state(message)
        self.last_observation = observation
        return (observation, reward, done, {})

    def _render(self, mode='human', close=False):
        pass

    def _close(self):
        pass

    def _reset(self):
        """Resets the state of the environment and returns an initial observation.

        Returns:
            observation (object): the initial observation of the space.
        """
        message = _receive_data(self.sock)
        observation, reward, done = self._decode_game_state(message)
        self.last_observation = observation
        return observation


class SimplePongEnv(RemoteEnv):
    metadata = {'render.modes': ['human', 'rgb_array']}

    def __init__(self, host, port):
        # Initialize parent class.
        super().__init__(host, port)

        # The Space object corresponding to valid actions
        self.action_space = spaces.Discrete(3)
        # The Space object corresponding to valid observations
        self.observation_space = spaces.Box(low=-1000, high=1000, shape=(8,))

        # A tuple corresponding to the min and max possible rewards
        # reward_range
        reward_range = (-np.inf, np.inf)

        # Current scores of players.
        self.cpu_score = None
        self.player_score = None

        self.viewer = None

    def _step(self, action):
        return super()._step(int(action))

    def _decode_game_state(self, message):
        step, cpu_score, player_score, ball_pos, ball_vel, cpu_pos, player_pos, height, width, screen = message

        reward = 0
        if self.cpu_score and self.player_score:
            reward = (player_score - self.player_score) - (cpu_score - self.cpu_score)

        self.cpu_score = cpu_score
        self.player_score = player_score

        observation = np.array(ball_pos + ball_vel + cpu_pos + player_pos)
        # print(observation)

        return observation, reward, reward != 0

    def _reset(self):
        """Resets the state of the environment and returns an initial observation.

        Returns:
            observation (object): the initial observation of the space.
        """
        _send_data(self.sock, 0)
        return super()._reset()

    def _render(self, mode='human', close=False):
        if close:
            if self.viewer is not None:
                self.viewer.close()
                self.viewer = None
            return

        if self.viewer is None:
            self.viewer = rendering.SimpleImageViewer()

        if self.last_observation is not None:
            return self.viewer.imshow(self.last_observation)


class PongEnv(RemoteEnv):
    metadata = {'render.modes': ['human', 'rgb_array']}

    def __init__(self, host, port):
        # Initialize parent class.
        super().__init__(host, port)

        # The Space object corresponding to valid actions
        self.action_space = spaces.Discrete(3)
        # The Space object corresponding to valid observations
        H = 120
        W = 240
        D = 3
        self.observation_space = spaces.Box(low=0, high=255, shape=(H, W, D))

        # A tuple corresponding to the min and max possible rewards
        # reward_range
        reward_range = (-np.inf, np.inf)

        # Current scores of players.
        self.cpu_score = None
        self.player_score = None

        self.viewer = None

    def _step(self, action):
        return super()._step(int(action))

    def _decode_game_state(self, message):
        step, cpu_score, player_score, height, width, screen = message
        screen = np.frombuffer(screen, dtype=np.uint8)
        screen = screen.reshape((height, width, 3), order='F').swapaxes(0, 1)

        reward = 0
        if self.cpu_score and self.player_score:
            reward = (player_score - self.player_score) - (cpu_score - self.cpu_score)

        self.cpu_score = cpu_score
        self.player_score = player_score

        return screen, reward, reward != 0

    def _reset(self):
        """Resets the state of the environment and returns an initial observation.

        Returns:
            observation (object): the initial observation of the space.
        """
        _send_data(self.sock, 0)
        return super()._reset()

    def _render(self, mode='human', close=False):
        if close:
            if self.viewer is not None:
                self.viewer.close()
                self.viewer = None
            return

        if self.viewer is None:
            self.viewer = rendering.SimpleImageViewer()

        if self.last_observation is not None:
            return self.viewer.imshow(self.last_observation)


class FilteredEnv(gym.Env):
    metadata = {'render.modes': ['human', 'rgb_array']}

    def __init__(self, env, ob_filter, rew_filter):
        self.env = env
        # copy over relevant parts of the child env
        self._spec = self.env.spec
        self.metadata = self.env.metadata
        self.action_space = self.env.action_space
        ob_space = self.env.observation_space
        shape = ob_filter.output_shape(ob_space)
        self.observation_space = spaces.Box(-np.inf, np.inf, shape)

        self.ob_filter = ob_filter
        self.rew_filter = rew_filter

    def _step(self, ac):
        ob, rew, done, info = self.env._step(ac)
        nob = self.ob_filter(ob) if self.ob_filter else ob
        nrew = self.rew_filter(rew) if self.rew_filter else rew
        info["reward_raw"] = rew
        return (nob, nrew, done, info)

    def _reset(self):
        ob = self.env.reset()
        return self.ob_filter(ob) if self.ob_filter else ob

    # def _render(self, *args, **kw):
    def _render(self, mode='human', close=False):
        self.env._render(mode=mode, close=close)


def rgb2gray(rgb):
    r, g, b = rgb[:,:,0], rgb[:,:,1], rgb[:,:,2]
    gray = 0.2989 * r + 0.5870 * g + 0.1140 * b
    return gray


class RGBImageToVector(object):
    def __init__(self, out_width=80, out_height=80):
        self.out_width = out_width
        self.out_height = out_height

    def __call__(self, obs):
        # obs is an M x N x 3 rgb image, want an (out_width x out_height,)
        # vector

        # nearly 2x faster to downsample then grayscale
        downsample = scipy.misc.imresize(obs, (self.out_width, self.out_height, 3))
        grayscale = rgb2gray(downsample)
        flatten = grayscale.reshape(self.out_width * self.out_height)
        return flatten

    def output_shape(self, x):
        return self.out_width * self.out_height

