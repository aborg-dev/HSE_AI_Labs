import abc
import socket
import struct
import logging
import msgpack

import gym
from gym import error, spaces, utils
from gym.utils import seeding

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


def _send_data(sock, direction):
    def get_dir_enc(value):
        return value + 1

    enc_direction = get_dir_enc(direction)
    response = struct.pack('c', enc_direction.to_bytes(1, byteorder="big"))
    # logging.info("Sending {} bytes".format(len(response)))
    sock.sendall(response)


class RemoteEnv(gym.Env):
    __metaclass__ = abc.ABCMeta

    def __init__(self, host, port):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((host, port))

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
        _send_data(self.sock, action)
        message = _receive_data(self.sock)
        observation, reward, done = self._decode_game_state(message)
        logging.info("Observation len: %s", len(observation))
        return (observation, reward, done, None)

    def _render(self, mode='human', close=False):
        pass

    def _close(self):
        pass

    def _reset(self):
        """Resets the state of the environment and returns an initial observation.

        Returns:
            observation (object): the initial observation of the space.
        """
        observation = _receive_data(self.sock)
        return observation


class PongEnv(RemoteEnv):

    def __init__(self, host, port):
        # Initialize parent class.
        super().__init__(host, port)

        # The Space object corresponding to valid actions
        self.action_space = spaces.Discrete([-1, 0, 1])
        # The Space object corresponding to valid observations
        H = 80
        W = 80
        D = 1
        self.observation_space = spaces.Box(low=0, high=255, shape=(H, W, D))
        # A tuple corresponding to the min and max possible rewards
        # reward_range
        reward_range = (-np.inf, np.inf)

    def _decode_game_state(self, message):
        step, cpu_score, player_score, height, width, screen = message
        screen = np.frombuffer(screen, dtype=np.uint8)
        screen = screen.reshape((height, width, 3), order='F').swapaxes(0, 1)
        return screen, 0, 0
