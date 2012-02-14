#include <booster/shared_ptr.h>
#include <booster/enable_shared_from_this.h>

#include <booster/aio/io_service.h>
#include <booster/aio/buffer.h>
#include <booster/aio/endpoint.h>
#include <booster/aio/acceptor.h>
#include <booster/aio/stream_socket.h>

#include <iostream>
#include <functional>


class echo_session : public booster::enable_shared_from_this<echo_session> {
public:
    echo_session(booster::aio::io_service &s) : socket_(s)
    {
    }

    void run()
    {
        socket_.async_read_some(
            booster::aio::buffer(buffer_,sizeof(buffer_)),
                std::bind(&echo_session::on_read,shared_from_this(),
                        std::placeholders::_1,
                        std::placeholders::_2));
                        
    }
    void on_read(booster::system::error_code const &e,size_t tr)
    {
        if(e) return;

        socket_.async_write(booster::aio::buffer(buffer_,tr),
                std::bind(&echo_session::on_written,shared_from_this(),std::placeholders::_1));
    }

    void on_written(booster::system::error_code const &e)
    {
        if(e) return;
        
        run();
    }
    
private:

    friend class echo_acceptor;

    char buffer_[1024];
    booster::aio::stream_socket socket_;
};


class echo_acceptor {
public:
    echo_acceptor(booster::aio::io_service &srv) : acceptor_(srv)
    {
        booster::aio::endpoint ep("0.0.0.0",8080);
        acceptor_.open(ep.family());
        acceptor_.set_option(booster::aio::acceptor::reuse_address,true);
        acceptor_.bind(ep);
        acceptor_.listen(10);
    }
    void run()
    {
        new_session_.reset(new echo_session(acceptor_.get_io_service()));
        acceptor_.async_accept(new_session_->socket_,
            std::bind(&echo_acceptor::on_accepted,this,std::placeholders::_1));
    }
    void on_accepted(booster::system::error_code const &e)
    {
        if(e) {
            std::cout << e.message() << std::endl;
            run();
        }
        else {
            new_session_->run();
            run();
        }
    }
private:
    booster::aio::acceptor acceptor_;
    booster::shared_ptr<echo_session> new_session_;
};


int main()
{
    try {
        booster::aio::io_service srv;

        #if !defined(_WIN32) && !defined(__CYGWIN__)
        booster::aio::basic_io_device stdin_device(srv);
        stdin_device.attach(0);
        std::cout << "Press any key to stop" << std::endl;
        stdin_device.on_readable(std::bind(&booster::aio::io_service::stop,&srv));
        #endif

        echo_acceptor acc(srv);
        acc.run();

        srv.run();
    }
    catch(std::exception const &e) {
        std::cerr<<e.what()<<std::endl;
    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
