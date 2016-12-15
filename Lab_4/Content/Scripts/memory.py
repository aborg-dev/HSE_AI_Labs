import random

from collections import deque


class ExperienceMemory(object):
    def __init__(self, max_size):
        self.max_size = max_size
        self.memory = deque()

    def add_experience(self, experience):
        self.memory.append(experience)
        if len(self.memory) > self.max_size:
            self.memory.popleft()

    def sample(self, batch_size):
        return random.sample(self.memory, batch_size)


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
