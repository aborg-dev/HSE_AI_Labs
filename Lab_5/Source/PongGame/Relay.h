// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>
#include <asio.hpp>
#include <memory>
#include <array>
#include <functional>

class TcpConnection
: public std::enable_shared_from_this<TcpConnection>
{
public:
    typedef std::shared_ptr<TcpConnection> pointer;

    static pointer create(asio::io_service& io_service)
    {
        return pointer(new TcpConnection(io_service));
    }

    asio::ip::tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        // TODO: Wait for messages from Relay here.
        //message_ = make_daytime_string();

        //asio::async_write(socket_, asio::buffer(message_),
        //std::bind(&TcpConnection::handle_write, shared_from_this(),
        //std::placeholders::_1,
        //std::placeholders::_2));
    }

private:
    TcpConnection(asio::io_service& io_service)
        : socket_(io_service)
    {
    }

    //void handle_write(const asio::error_code& [>error<],
            //size_t [>bytes_transferred<])
    //{
    //}

    asio::ip::tcp::socket socket_;
    std::string message_;
};

class TcpServer
{
public:
    TcpServer(asio::io_service& io_service, int port)
        : acceptor_(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    { }

private:
    void start_accept()
    {
        TcpConnection::pointer current_connection_ =
            TcpConnection::create(acceptor_.get_io_service());

        acceptor_.async_accept(current_connection_->socket(),
                std::bind(&TcpServer::handle_accept, this, new_connection,
                    std::placeholders::_1));
    }

    void handle_accept(TcpConnection::pointer new_connection, const asio::error_code& error)
    {
        if (!error) {
            new_connection->start();
            connected = true;
        } else {
            connected = false;
            start_accept();
        }
    }

    bool connected = false;
    TcpConnection::pointer current_connection_;
    asio::ip::tcp::acceptor acceptor_;
};

struct Server
{
    void create(int port);

    std::unique_ptr<asio::io_service> io_service;
    std::unique_ptr<TcpServer> tcp_server;
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

    void TryConnect();

    Server server;
    bool connected;
    bool serverCreated;

    std::string host;
    int port;
};
