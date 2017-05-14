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
    host = "localhost";
    port = 6000;
    connected = false;
}

FRelay::~FRelay()
{
}

void Connection::create(std::string host, int port) {
    socket.reset();
    endpoint_iterator.reset();
    query.reset();
    resolver.reset();
    io_service.reset();

    io_service = std::make_unique<asio::io_service>();
    resolver = std::make_unique<tcp::resolver>(*io_service);
    query = std::make_unique<tcp::resolver::query>(host, std::to_string(port));
    endpoint_iterator = std::make_unique<tcp::resolver::iterator>(resolver->resolve(*query));
    socket = std::make_unique<tcp::socket>(*io_service);
    asio::connect(*socket, *endpoint_iterator);
}

void FRelay::Connect()
{
    connection.create(host, port);
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
    if (!connected) {
        Connect();
        connected = true;
    }

    //UE_LOG(LogTemp, Warning, TEXT("Sending %d bytes..."), size);

    try {
        asio::error_code error;
        auto len_buf = int_to_buf(size);
        size_t out_len = connection.socket->write_some(asio::buffer(len_buf), error);
        if (out_len != 4 || error) {
            throw asio::system_error(error);
        }

        //size_t out_len = connection.socket->write_some(asio::buffer(size), error);
        out_len = connection.socket->write_some(asio::buffer(ptr, size), error);
        //UE_LOG(LogTemp, Warning, TEXT("Sent %d bytes"), out_len);

        if (error) {
            throw asio::system_error(error);
        }

        std::array<uint8_t, ACTION_SIZE> buf;
        int retries = 5;
        while (true) {
            size_t in_len = asio::read(*connection.socket, asio::buffer(buf), asio::transfer_exactly(ACTION_SIZE), error);
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
