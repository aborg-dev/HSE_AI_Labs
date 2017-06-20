// Fill out your copyright notice in the Description page of Project Settings.

#include "PongGame.h"
#include "Relay.h"

#include <iostream>
#include <array>
#include <asio.hpp>
#include <unistd.h>
#include <exception>

using asio::ip::tcp;

FRelay::FRelay()
{
    //host = "localhost";
    port = 6000;
    connected = false;
    serverCreated = false;
}

FRelay::~FRelay()
{
}

void Server::create(int port) {
    tcp_server.reset();
    io_service.reset();

    io_service = std::make_unique<asio::io_service>();
    tcp_server = std::make_unique<TcpServer>(*io_service, port);
}

void FRelay::TryConnect()
{
    server.io_service->poll();
}

std::array<char, 4> int_to_buf(int v) {
    std::array<char, 4> buf;
    int base = 1 << 8;
    for (int i = 0; i < 4; ++i) {
        buf[i] = v % base;
        v /= base;
    }
    return buf;
}

ActionType FRelay::Act(char* ptr, size_t size)
{
    if (!serverCreated) {
        // We can't do this in constructor because it's evaluated even before
        // the start of the game.
        server.create(port);
        serverCreated = true;
        server.tcp_server->start_accept();
    }

    if (!connected) {
        TryConnect();
        if (!server.tcp_server->connected) {
            throw std::logic_error("No clients accepted.");
        }
        connected = true;
    }

    auto& socket = server.tcp_server->current_connection_->socket_;

    //UE_LOG(LogTemp, Warning, TEXT("Sending %d bytes..."), size);

    try {
        asio::error_code error;
        auto len_buf = int_to_buf(size);
        size_t out_len = socket.write_some(asio::buffer(len_buf), error);
        if (out_len != 4 || error) {
            throw asio::system_error(error);
        }

        //size_t out_len = socket.write_some(asio::buffer(size), error);
        out_len = socket.write_some(asio::buffer(ptr, size), error);
        //UE_LOG(LogTemp, Warning, TEXT("Sent %d bytes"), out_len);

        if (error) {
            throw asio::system_error(error);
        }

        std::array<uint8_t, ACTION_SIZE> buf;
        int retries = 5;
        while (true) {
            size_t in_len = asio::read(*socket, asio::buffer(buf), asio::transfer_exactly(ACTION_SIZE), error);
            //UE_LOG(LogTemp, Warning, TEXT("Read %d bytes"), in_len);

            if (in_len == ACTION_SIZE) {
                break;
            }
            if (in_len != 0) {
                throw std::logic_error("Wrong length " + std::to_string(in_len));
            }

            if (error == asio::error::eof) {
                if (!(--retries)) {
                    throw std::logic_error("Timeout");
                    break;
                }
                UE_LOG(LogTemp, Warning, TEXT("EOF, retrying: %d"), retries);
                sleep(1);
                continue;
            } else if (error) {
                throw asio::system_error(error);
            }
        }
        return buf;

    } catch (std::exception& e) {
        connected = false;
        FString error_message(e.what());
        UE_LOG(LogTemp, Warning, TEXT("Caught an exception: %s"), *error_message);

        throw e;
    }
}
