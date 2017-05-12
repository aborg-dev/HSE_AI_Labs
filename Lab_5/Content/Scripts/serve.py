import socketserver
import msgpack

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
        # self.request is the TCP socket connected to the client
        self.data = self.request.recv(100000).strip()
        print("{} wrote {} bytes".format(self.client_address[0], len(self.data)))
        unpacked = msgpack.unpackb(self.data)
        print("Unpacked data length: {}".format(len(unpacked)))
        print("Screen data length: {}".format(len(unpacked[5])))
        # print("Unpacked data: {}".format(unpacked))
        # just send back the same data, but upper-cased
        response = msgpack.packb(-1.0)
        self.request.sendall(response)

if __name__ == "__main__":
    HOST, PORT = "localhost", 6000

    # Create the server, binding to localhost on port 9999
    server = MyServer((HOST, PORT), MyTCPHandler)

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    server.serve_forever()
