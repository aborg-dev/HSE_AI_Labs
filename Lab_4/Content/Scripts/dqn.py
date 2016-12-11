import tensorflow as tf
import numpy as np


def weight_variable(shape):
    initial = tf.truncated_normal(shape, stddev=0.01)
    return tf.Variable(initial)


def bias_variable(shape):
    initial = tf.constant(0.01, shape=shape)
    return tf.Variable(initial)


def conv2d(x, W, stride):
    return tf.nn.conv2d(x, W, strides=[1, stride, stride, 1], padding="SAME")


def max_pool_2x2(x):
    return tf.nn.max_pool(x, ksize=[1, 2, 2, 1], strides=[1, 2, 2, 1], padding="SAME")


def createNetwork(action_count):
    # network weights
    W_conv1 = weight_variable([8, 8, 4, 32])
    b_conv1 = bias_variable([32])

    W_conv2 = weight_variable([4, 4, 32, 64])
    b_conv2 = bias_variable([64])

    W_conv3 = weight_variable([3, 3, 64, 64])
    b_conv3 = bias_variable([64])

    W_fc1 = weight_variable([1600, 512])
    b_fc1 = bias_variable([512])

    W_fc2 = weight_variable([512, action_count])
    b_fc2 = bias_variable([action_count])

    # input layer
    s = tf.placeholder("float", [None, 80, 80, 4])

    # hidden layers
    h_conv1 = tf.nn.relu(conv2d(s, W_conv1, 4) + b_conv1)
    h_pool1 = max_pool_2x2(h_conv1)

    h_conv2 = tf.nn.relu(conv2d(h_pool1, W_conv2, 2) + b_conv2)
    #h_pool2 = max_pool_2x2(h_conv2)

    h_conv3 = tf.nn.relu(conv2d(h_conv2, W_conv3, 1) + b_conv3)
    #h_pool3 = max_pool_2x2(h_conv3)

    #h_pool3_flat = tf.reshape(h_pool3, [-1, 256])
    h_conv3_flat = tf.reshape(h_conv3, [-1, 1600])

    h_fc1 = tf.nn.relu(tf.matmul(h_conv3_flat, W_fc1) + b_fc1)

    # readout layer
    readout = tf.matmul(h_fc1, W_fc2) + b_fc2

    return s, readout, h_fc1


class DQNAgent(object):
    def __init__(self, action_count):
        self.action_count = action_count
        self.state_placeholder, self.readout, self.h_fc1 = createNetwork(action_count)

        # define the cost function
        self.a = tf.placeholder("float", [None, action_count])
        self.y = tf.placeholder("float", [None])
        self.readout_action = tf.reduce_sum(tf.mul(self.readout, self.a), reduction_indices=1)
        self.cost = tf.reduce_mean(tf.square(self.y - self.readout_action))
        self.train_step = tf.train.AdamOptimizer(1e-6).minimize(self.cost)

    # think about putting observation as an argument here
    def act(self, session, state):
        scores = self.score_actions(session, [state])[0]
        return np.argmax(scores)

    def score_actions(self, session, state_batch):
        return session.run(self.readout, feed_dict={self.state_placeholder: state_batch})

    def train(self, session, reward_batch, action_batch, state_batch):
        # perform gradient step
        _, loss = session.run([self.train_step, self.cost], feed_dict={
            self.y: reward_batch,
            self.a: action_batch,
            self.state_placeholder: state_batch})
        return loss
