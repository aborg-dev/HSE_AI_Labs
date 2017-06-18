// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>
#include <asio.hpp>
#include <memory>
#include <array>

struct Connection
{
    void create(std::string host, int port);

    std::unique_ptr<asio::io_service> io_service;
    std::unique_ptr<asio::ip::tcp::resolver> resolver;
    std::unique_ptr<asio::ip::tcp::resolver::query> query;
    std::unique_ptr<asio::ip::tcp::resolver::iterator> endpoint_iterator;
    std::unique_ptr<asio::ip::tcp::socket> socket;
};

struct Server
{
    void create(int port);

    std::unique_ptr<asio::io_service> io_service;
    std::unique_ptr<asio::ip::tcp::acceptor> acceptor;
    std::unique_ptr<asio::ip::tcp::socket> socket;
};

const int ACTION_SIZE = 1;
using ActionType = std::array<uint8_t, ACTION_SIZE>;

/**
 * 
 */
class PONGGAME_API FRelay
{
public:
	FRelay();
	~FRelay();

    ActionType Act(char* ptr, size_t size);

private:

    void Connect();

    //Connection connection;
    Server server;
    bool connected;

    std::string host;
    int port;
};
