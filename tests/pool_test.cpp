///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/service.h>
#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include <cppcms/http_context.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/mount_point.h>
#include <booster/aio/deadline_timer.h>
#include <booster/posix_time.h>
#include <cppcms/json.h>
#include <booster/thread.h>
#include <iostream>
#include "client.h"
#include <sstream>
#include <booster/log.h>
#include "test.h"

int g_fail;
#define TESTNT(x) do { if(x) break;  std::cerr << "FAIL: " #x " in line: " << __LINE__ << std::endl; g_fail = 1; return; } while(0)

booster::thread_specific_ptr<int> g_thread_id;
booster::mutex g_id_lock;
int g_thread_id_counter=1000;

struct thread_submitter {
	thread_submitter(cppcms::service &srv) : srv_(&srv) {}
	void operator()() const
	{
		runner r(*srv_);
		booster::thread t(r);
		t.detach();
	}
	cppcms::service *srv_;
};

void set_thread_id(int v)
{
	g_thread_id.reset(new int(v));
}

int get_thread_id()
{
	if(g_thread_id.get()==0) {
		booster::unique_lock<booster::mutex> guard(g_id_lock);
		int new_id = ++g_thread_id_counter;
		set_thread_id(new_id);
	}
	return *g_thread_id;
}

class counter {
	typedef booster::unique_lock<booster::mutex> guard;
	counter() : 
		current(0),
		max(0),
		total(0)
	{
	}
public:
	static counter *instance(std::string const &name)
	{
		static const int max = 20;
		static int curr;
		static counter all[max];
		for(int i=0;i<curr;i++)
			if(all[i].name==name)
				return all + i;
		assert(curr < max);
		all[curr].name = name;
		return all + curr++;
	}

	void print(std::ostream &out)
	{
		guard g(lock_);
		out<<	"name="<<name<<"\n"
			"current="<<current<<"\n"
			"total="<<total<<"\n"
			"max="<<max;

	}

	std::string name;
	int current;
	int max;
	int total;

	int inc() {
		guard g(lock_);
		int r = ++total;
		current++;
		if(max < current)
			max = current;
		return r;
	}
	void dec()
	{
		guard g(lock_);
		current--;
	}

private:
	booster::mutex lock_;
};



class unit_test : public cppcms::application {
public:
	unit_test(cppcms::service &s,counter *c) : 
		cppcms::application(s),
		c_(c),
		id_(0),
		original_thread_id_(0)
	{
		id_ = c_->inc();
		original_thread_id_ = get_thread_id();
	}
	~unit_test()
	{
		c_->dec();
	}
	void main(std::string path)
	{
		double sleep_for = atof(request().get("sleep").c_str());

		BOOSTER_DEBUG("cppcms") << "--------- GOT " << path << " for " << sleep_for << " from " << id_ << " in " << get_thread_id();

		booster::ptime::sleep(booster::ptime::from_number(sleep_for));


		std::ostringstream ss;
		ss << 
		"url=" << request().script_name() << path << "\n"
		"thread_id=" << get_thread_id() << "\n"
		"original_thread_id=" << original_thread_id_ << "\n"
		"app_id="<<id_;
		
		BOOSTER_DEBUG("cppcms") << "RESPONSE " << path << " from " << id_ << "\n" << ss.str() ;
		response().out() << ss.str();

	}
private:
	counter *c_;
	int id_;
	int original_thread_id_;
};

std::map<std::string,booster::weak_ptr<cppcms::application_specific_pool> > weak_pools;

class tester : public cppcms::application {
public:
	tester(cppcms::service &srv) : cppcms::application(srv) {}
	void main(std::string name)
	{
		if(name=="/stats")
			counter::instance(request().get("id"))->print(response().out());
		else if(name=="/unmount") {
			std::string id = request().get("id");
			bool exists_before = weak_pools[id].lock();
			service().applications_pool().unmount(weak_pools[id]);
			bool exists_after = weak_pools[id].lock();
			response().out()<<"unmount=" << id << "\n"
				"before="<<exists_before <<"\n"
				"after="<<exists_after;
		}
		else if(name=="/install") {
			app_ = new unit_test(service(),counter::instance("/async/temporary"));
			service().applications_pool().mount(app_,cppcms::mount_point("/async","/temporary",0));
			response().out()<<"install=1";
		}
		else if(name=="/uninstall") {
			response().out()<<"install=0";
			app_ = 0;
		}

	}
private:
	booster::intrusive_ptr<cppcms::application> app_;
};


struct marker {
	marker(std::string const &name) : name_(name) {}
	booster::shared_ptr<cppcms::application_specific_pool> const &operator | (booster::shared_ptr<cppcms::application_specific_pool> const &in)
	{
		weak_pools[name_] = in;
		return in;
	}
	std::string name_;
};

struct pools {
	booster::weak_ptr<cppcms::application_specific_pool> sync,async;
	bool async_requested;
	pools() : async_requested(false) {}
};

class sender : public cppcms::application  {
public:

	struct src_prop {
		bool src_async;
		bool src_created;
		bool src_to_app;
	};
	sender(cppcms::service &srv,pools *p) :
		cppcms::application(srv),
		pools_(p)
	{
	}

	void main(std::string url)
	{
		if(url!="/sender") {
			src_prop *p = context().get_specific<src_prop>();
			TESTNT(p);
			response().out() << "async=" << is_asynchronous() << "\n"
			"src_async="  << p->src_async   <<"\n"
			"src_created="<< p->src_created <<"\n"
			"src_to_app=" << p->src_to_app  <<"\n"
			"path="       << url<<"\n"
			;
			return;
		}

		src_prop *p=new src_prop();
		context().reset_specific<src_prop>(p);
		
		bool to_app = request().get("to_app")=="1";
		p->src_to_app = to_app;
		p->src_async = is_asynchronous();
		p->src_created = false;
		booster::shared_ptr<cppcms::application_specific_pool> tgt;
		if(request().get("to")=="sync") 
			tgt = pools_->sync.lock();
		else 
			tgt = pools_->async.lock();

		booster::shared_ptr<cppcms::http::context> ctx = release_context();
		TESTNT(tgt);
		
		if(!to_app) {
			ctx->submit_to_pool(tgt,"/pool");
			return; 
		}
		
		booster::intrusive_ptr<cppcms::application> app;
		app =tgt->asynchronous_application_by_io_service(service().get_io_service());
		if(!app) {
			app = tgt->asynchronous_application_by_io_service(service().get_io_service(),service());
			p->src_created=true;
		}
		TESTNT(app);
		ctx->submit_to_asynchronous_application(app,"/app");
	}
private:
	pools *pools_;
};

int main(int argc,char **argv)
{
	try {
		{
			using cppcms::mount_point;
			cppcms::service srv(argc,argv);

			set_thread_id(1);
			
			srv.applications_pool().mount(
				marker("/sync") | cppcms::create_pool<unit_test>(counter::instance("/sync")),
				mount_point("/sync","",0),
				cppcms::app::synchronous);
			
			srv.applications_pool().mount(
				cppcms::create_pool<unit_test>(counter::instance("/sync/work")),
				mount_point("/sync","/work",0),
				cppcms::app::synchronous);

			srv.applications_pool().mount(
				marker("/sync/prepopulated") | cppcms::create_pool<unit_test>(counter::instance("/sync/prepopulated")),
				mount_point("/sync","/prepopulated",0),
				cppcms::app::synchronous | cppcms::app::prepopulated);
			
			srv.applications_pool().mount(
				marker("/sync/tss") | cppcms::create_pool<unit_test>(counter::instance("/sync/tss")),
				mount_point("/sync","/tss",0),
				cppcms::app::synchronous | cppcms::app::thread_specific);

			srv.applications_pool().mount(
				cppcms::applications_factory<unit_test>(counter::instance("/sync/legacy")),
				mount_point("/sync","/legacy",0));
			
			srv.applications_pool().mount(
				marker("/async") | cppcms::create_pool<unit_test>(counter::instance("/async")),
				mount_point("/async","",0),
				cppcms::app::asynchronous);

			srv.applications_pool().mount(
				marker("/async/prepopulated") | cppcms::create_pool<unit_test>(counter::instance("/async/prepopulated")),
				mount_point("/async","/prepopulated",0),
				cppcms::app::asynchronous | cppcms::app::prepopulated);

			booster::intrusive_ptr<cppcms::application> app = new unit_test(srv,counter::instance("/async/legacy"));
			srv.applications_pool().mount(
				app,
				mount_point("/async","/legacy",0));

			counter::instance("/async/temporary");

			srv.applications_pool().mount(cppcms::create_pool<tester>(),mount_point("/test"),cppcms::app::asynchronous);

			pools p;
			{
				booster::shared_ptr<cppcms::application_specific_pool> tmp;
				srv.applications_pool().mount(
					(tmp=cppcms::create_pool<sender>(&p)),
					mount_point("/async","/sender",0),
					cppcms::app::asynchronous);
				p.async = tmp;
			}
			
			{
				booster::shared_ptr<cppcms::application_specific_pool> tmp;
				srv.applications_pool().mount(
					(tmp=cppcms::create_pool<sender>(&p)),
					mount_point("/sync","/sender",0),
					cppcms::app::synchronous);
				p.sync = tmp;
			}

			srv.after_fork(thread_submitter(srv));
			srv.run();

			weak_pools.clear();
		}

		std::cout << "Test all deleted" << std::endl;
		TEST(counter::instance("/sync")->current == 0);
		TEST(counter::instance("/sync/prepopulated")->current == 0);
		TEST(counter::instance("/sync/tss")->current == 0);
		TEST(counter::instance("/sync/legacy")->current == 0);
		TEST(counter::instance("/async")->current == 0);
		TEST(counter::instance("/async/prepopulated")->current == 0);
		TEST(counter::instance("/async/legacy")->current == 0);
		std::cout << "Done" << std::endl;
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	if(run_ok && !g_fail) {
		std::cout << "Full Test: Ok" << std::endl;
		return EXIT_SUCCESS;
	}
	else {
		std::cout << "FAILED" << std::endl;
		return EXIT_FAILURE;
	}
}
