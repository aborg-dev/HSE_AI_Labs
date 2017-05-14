import socketserver
import msgpack
import time
import struct
import logging
import sys

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

class MyServer(socketserver.TCPServer):
    allow_reuse_address = True

class MyTCPHandler(socketserver.BaseRequestHandler):
    """
    The RequestHandler class for our server.

    It is instantiated once per connection to the server, and must
    override the handle() method to implement communication to the
    client.
    """

    def handle(self):
        py_controller = controller.PythonAIController()
        py_controller.begin_play()

        while True:
            # self.request is the TCP socket connected to the client
            data_size_buf = self.request.recv(4).strip()
            data_size = struct.unpack('i', data_size_buf)[0]

            self.data = bytearray()
            to_read = data_size
            while to_read > 0:
                data = self.request.recv(to_read).strip()
                self.data.extend(data)
                to_read -= len(data)

            # print(len(self.data), data_size)
            # logging.info("{} wrote {} bytes".format(self.client_address[0], len(self.data)))
            unpacked = None
            try:
                unpacked = msgpack.unpackb(self.data)
                # logging.info("Unpacked data length: {}".format(len(unpacked)))
                # logging.info("Screen data length: {}".format(len(unpacked[5])))
                # print("Unpacked data: {}".format(unpacked))
            except Exception as e:
                logging.info(e)

            def get_dir_enc(value):
                return value + 1

            direction = 0
            if unpacked:
                direction = py_controller.tick(
                        unpacked[0],
                        unpacked[1],
                        unpacked[2],
                        unpacked[3],
                        unpacked[4],
                        unpacked[5])

            enc_direction = get_dir_enc(direction)
            # response = msgpack.packb(direction)
            response = struct.pack('c', enc_direction.to_bytes(1, byteorder="big"))
            # logging.info("Sending {} bytes".format(len(response)))
            self.request.sendall(response)

            global start
            global handled

            handled += 1
            if handled % 1000 == 0:
                logging.info(handled / (time.time() - start))

if __name__ == "__main__":
    HOST, PORT = "localhost", 6000

    # Create the server, binding to localhost on port 9999
    server = MyServer((HOST, PORT), MyTCPHandler)

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    server.serve_forever()
