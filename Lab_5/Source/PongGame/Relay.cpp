// Fill out your copyright notice in the Description page of Project Settings.

#include "PongGame.h"
#include "Relay.h"

#include <iostream>
#include <boost/array.hpp>
#include <asio.hpp>

using asio::ip::tcp;

FRelay::FRelay()
{
}

void FRelay::Tick()
{
    try {
        host = "localhost:6000";
        asio::io_service io_service;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(host, "daytime");

        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        tcp::socket socket(io_service);
        asio::connect(socket, endpoint_iterator);

        for (;;) {
            boost::array<char, 128> buf;
            asio::error_code error;

            size_t len = socket.read_some(asio::buffer(buf), error);

            if (error == asio::error::eof) {
                break;
            } else if (error) {
                throw asio::system_error(error);
            }


            UE_LOG(LogTemp, Warning, TEXT("Received %d bytes"), len);
        }
    } catch (std::exception& e) {
        FString message(e.what());
        UE_LOG(LogTemp, Warning, TEXT("Caught an exception: %s"), *message);
    }
}

FRelay::~FRelay()
{
}

TArray<uint8> FRelay::Act(const TArray<uint8>& state)
{
    UE_LOG(LogTemp, Warning, TEXT("Sending %d bytes..."), state.Num());
    return state;
}
