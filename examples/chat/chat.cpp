///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/application.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/http_request.h>
#include <cppcms/http_context.h>
#include <cppcms/urandom.h>
#include <cppcms/base64.h>
#include <cppcms/crypto.h>
#include <booster/intrusive_ptr.h>

#include <set>

#ifdef USE_STD_TR1_BIND
  #include <tr1/functional>
  using std::tr1::bind;
#elif defined USE_STD_BIND
  #include <functional>
  using std::bind;
#else
  #include <boost/bind.hpp>
  using boost::bind;
#endif


class chat : public cppcms::application {
public:
    chat(cppcms::service &srv) : cppcms::application(srv)
    {
        dispatcher().assign("/post",&chat::post,this);
        dispatcher().assign("/get/(\\d+)",&chat::get,this,1);
        dispatcher().assign(".*",&chat::redirect,this);
    }
    void redirect()
    {
        response().set_redirect_header("/the_chat.html");
    }
    std::string to_ws(std::string const &msg)
    {
        char opcode = 0x81;
        int len=msg.size();
        if(len > 125)
            len = 125;
        std::string r;
        char oplen = (unsigned char) (len);
        r += opcode;
        r += oplen;
        r.append(msg.c_str(),len);
        return r;
    }
    std::string to_ws_client(std::string const &msg)
    {
        char opcode = 0x81;
        int len=msg.size();
        if(len > 125)
            len = 125;
        char oplen = (unsigned char)0x80 | (unsigned char) (len);
        char mask[4];
        rnd_.generate(mask,4);
        std::string r;
        r.reserve(2 + 4 + len + 1);
        r += opcode;
        r += oplen;
        r.append(mask,4);
        for(int i=0;i<len;i++) {
            unsigned char oc = msg[i];
            unsigned char mc = mask[i%4];
            r+= (char)(oc ^ mc);
        }
        return r;
    }
    bool handshake()
    {
        std::string key;
        if(strcasecmp(request().http_upgrade().c_str(),"websocket")!=0 
            || strcasecmp(request().http_connection().c_str(),"upgrade")!=0
            || strstr(request().cgetenv("HTTP_SEC_WEBSOCKET_VERSION"),"13")==0)
        {
            return false;
        }
        key = request().cgetenv("HTTP_SEC_WEBSOCKET_KEY");
        std::cout << "key in " << key << std::endl; 
        key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        std::auto_ptr<cppcms::crypto::message_digest> sha1 = cppcms::crypto::message_digest::sha1();
        sha1->append(key.c_str(),key.size());
        key.resize(sha1->digest_size());
        sha1->readout(&key[0]);
        key = cppcms::b64url::encode(key);
        for(size_t i=0;i<key.length();i++) {
            if(key[i]=='-')
                key[i]='+';
            else if(key[i]=='_')
                key[i]='/';
        }
        size_t rem = key.length() % 4;
        if(rem > 0)
            key.append(4-rem,'=');
        std::cout << "Key out " << key << std::endl;
        response().status(101);
        response().set_header("Upgrade","websocket");
        response().set_header("Connection","upgrade");
        response().set_header("Sec-WebSocket-Accept",key);
        response().set_header("Sec-WebSocket-Protocol","chat");
        return true;
    }
    void post()
    {
        std::cout << "Broadcast " << std::endl;
        if(request().request_method()=="POST") {
            messages_.push_back(request().post("message"));
            broadcast();
        }
    }
    void get(std::string no)
    {
        std::cout << "get " << no << std::endl;
        unsigned pos=atoi(no.c_str());
        if(pos > messages_.size() ){
            response().status(404);
            return;
        }
        
        response().set_plain_text_header();
        if(!handshake())
            return;
        response().out();
       
        booster::shared_ptr<cppcms::http::context> context=release_context();

        context->async_on_peer_reset([=]() {
                std::cout << "peer reset" << std::endl;
                remove_context(context);
        });

        send_from_pos(pos,context);
        flush(context,messages_.size());
    }
    void flush(booster::shared_ptr<cppcms::http::context> context,size_t n)
    {
        std::cout << "Flushing after " << n << std::endl;
            context->async_flush_output([=](cppcms::http::context::completion_type c) {
                std::cout << "Flush Completed ";
                if(c!=cppcms::http::context::operation_completed) {
                    std::cout << "With error " << std::endl;
                    return;
                }
                std::cout << "ok" << std::endl;
                if(send_from_pos(n,context)) {
                    std::cout << "Flushing again " << std::endl;
                    flush(context,messages_.size());
                }
                else {
                    std::cout << "Waiting " << std::endl;
                    waiters_.insert(context);
                }
            });
    }
    bool send_from_pos(unsigned pos,booster::shared_ptr<cppcms::http::context> context)
    {
        if(pos < messages_.size()) {
            std::cout << "Sending from " << pos << " up to " << messages_.size() << std::endl;
            for(size_t i=pos;i<messages_.size();i++)
                context->response().out() << to_ws(messages_[i]);
            return true;
        }
        return false;
    }
    void remove_context(booster::shared_ptr<cppcms::http::context> context)
    {
        waiters_.erase(context);
    }
    void broadcast()
    {
        if(messages_.empty())
            return;
        std::cout << "Breadcasting" << std::endl;
        size_t pos = messages_.size()-1;
        for(waiters_type::iterator it=waiters_.begin();it!=waiters_.end();) {
            booster::shared_ptr<cppcms::http::context> waiter = *it++;
            waiters_.erase(waiter);
            send_from_pos(pos,waiter);
            flush(waiter,messages_.size());
        }
    }
private:
    cppcms::urandom_device rnd_;
    std::vector<std::string> messages_;
    typedef std::set<booster::shared_ptr<cppcms::http::context> > waiters_type;
    waiters_type waiters_;
};


int main(int argc,char **argv)
{
    try {
        cppcms::service service(argc,argv);
        booster::intrusive_ptr<chat> c=new chat(service);
        service.applications_pool().mount(c);
        service.run();
    }
    catch(std::exception const &e) {
        std::cerr<<"Catched exception: "<<e.what()<<std::endl;
        return 1;
    }
    return 0;
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
