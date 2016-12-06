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
