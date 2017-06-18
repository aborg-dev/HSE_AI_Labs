import socket
import msgpack
import time
import struct
import logging
import sys
import random

import controller

root = logging.getLogger()
root.setLevel(logging.DEBUG)

ch = logging.StreamHandler(sys.stdout)
ch.setLevel(logging.DEBUG)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
ch.setFormatter(formatter)
root.addHandler(ch)

start = time.time()
handled = 0

def talk(host, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))

    while True:
        # `sock` is the TCP socket connected to the client
        data_size_buf = sock.recv(4)
        data_size = struct.unpack('i', data_size_buf)[0]

        data = bytearray(data_size)
        view = memoryview(data)
        to_read = data_size
        while to_read > 0:
            nbytes = sock.recv_into(view, to_read)
            view = view[nbytes:] # slicing views is cheap
            to_read -= nbytes

        # logging.info("{} wrote {} bytes".format(self.client_address[0], len(self.data)))
        unpacked = None
        try:
            unpacked = msgpack.unpackb(data)
            # logging.info("Unpacked data length: {}".format(len(unpacked)))
            # logging.info("Screen data length: {}".format(len(unpacked[5])))
            # print("Unpacked data: {}".format(unpacked))
        except Exception as e:
            logging.info(e)

        def get_dir_enc(value):
            return value + 1

        direction = 0
        if unpacked:
            direction = random.choice([-1, 1])
            # direction = py_controller.tick(
                    # unpacked[0],
                    # unpacked[1],
                    # unpacked[2],
                    # unpacked[3],
                    # unpacked[4],
                    # unpacked[5])

        enc_direction = get_dir_enc(direction)
        # response = msgpack.packb(direction)
        response = struct.pack('c', enc_direction.to_bytes(1, byteorder="big"))
        # logging.info("Sending {} bytes".format(len(response)))
        sock.sendall(response)

        global start
        global handled

        handled += 1
        if handled % 1000 == 0:
            logging.info(handled / (time.time() - start))


if __name__ == "__main__":
    HOST, PORT = "localhost", 6000
    talk(HOST, PORT)
