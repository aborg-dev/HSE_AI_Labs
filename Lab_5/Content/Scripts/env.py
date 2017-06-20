import abc

import gym
from gym import error, spaces, utils
from gym.utils import seeding

class RemoteEnv(gym.Env):

    def __init__(self):
        # The Space object corresponding to valid actions
        # action_space
        # The Space object corresponding to valid observations
        # observation_space
        # A tuple corresponding to the min and max possible rewards
        # reward_range
        pass

    def _seed(self, seed=None):
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
        pass

    def _render(self, mode='human', close=False):
        pass

    def _close(self):
        pass

    def _reset(self):
        pass
