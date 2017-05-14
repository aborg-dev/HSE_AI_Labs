// Fill out your copyright notice in the Description page of Project Settings.

#include "PongGame.h"
#include "Relay.h"

#include <iostream>
#include <array>
#include <asio.hpp>

using asio::ip::tcp;

FRelay::FRelay()
{
}

FRelay::~FRelay()
{
}

std::string FRelay::Act(char* ptr, size_t size)
{
    UE_LOG(LogTemp, Warning, TEXT("Sending %d bytes..."), size);

    host = "localhost";
    asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host, "6000");

    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    tcp::socket socket(io_service);
    asio::connect(socket, endpoint_iterator);

    for (;;) {
        asio::error_code error;

        size_t out_len = socket.write_some(asio::buffer(ptr, size), error);
        UE_LOG(LogTemp, Warning, TEXT("Sent %d bytes"), out_len);

        if (error) {
            throw asio::system_error(error);
        }

        std::array<char, 128> buf;
        size_t in_len = socket.read_some(asio::buffer(buf), error);
        UE_LOG(LogTemp, Warning, TEXT("Read %d bytes"), in_len);

        if (error == asio::error::eof) {
            break;
        } else if (error) {
            throw asio::system_error(error);
        }

        return std::string(buf.begin(), buf.begin() + in_len);
    }

    return "";
}
