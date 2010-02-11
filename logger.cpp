#define CPPCMS_SOURCE
#include "logger.h"
#include "cppcms_error.h"
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/format.hpp>
#   include <boost/thread.hpp>
#   include <boost/shared_ptr.hpp>
#else // Internal Boost
#   include <cppcms_boost/format.hpp>
#   include <cppcms_boost/thread.hpp>
#   include <cppcms_boost/shared_ptr.hpp>
    namespace boost = cppcms_boost;
#endif

#include <sstream>
#include <fstream>


namespace cppcms {
	namespace impl {
		class log_device : public util::noncopyable {
		public:
			virtual void write(std::string const &msg) = 0;
			virtual ~log_device() {}
		};
		class file_log_device : public log_device {
		public:
			file_log_device(std::string const &file_name,logger::open_mode_type how) 
			{
				switch(how) {
				case logger::overwrite:
					output_.open(file_name.c_str());
					break;
				case logger::append:
				default:
					output_.open(file_name.c_str(),std::fstream::app);
				}
				if(!output_) 
					throw cppcms_error("Failed to open log file:" + file_name);
			}
			virtual void write(std::string const &msg) 
			{
				output_ << msg <<std::endl;
			}
		private:
			std::ofstream output_;
		};

		class stream_log_device : public log_device {
		public:
			stream_log_device(std::ostream &out) : output_(out) 
			{
			}
			virtual void write(std::string const &msg) 
			{
				output_ << msg <<std::endl;
			}
		private:
			std::ostream &output_;
		};

	} /// impl

	struct logger::data {
		boost::shared_mutex lock;
		logger::level_type global_level;
		bool empty_map;
		typedef std::map<std::string,logger::level_type> module_level_type;
		module_level_type module_level;
		std::auto_ptr<std::fstream> output;
		std::vector<boost::shared_ptr<impl::log_device> > devices;
	};

	logger::level_type logger::module_level(char const *module)
	{
		if(d->empty_map) {
			return d->global_level;
		}
		else {
			boost::shared_lock<boost::shared_mutex> guard(d->lock);
			data::module_level_type::const_iterator p;
			if((p=d->module_level.find(module)) != d->module_level.end())
				return p->second;
			return d->global_level;
		}
	}
	void logger::module_level(char const *module,level_type l)
	{
		boost::shared_lock<boost::shared_mutex> guard(d->lock);
		d->module_level[module] = l;
		d->empty_map = false;
	}

	void logger::reset_module_level(char const *module)
	{
		boost::shared_lock<boost::shared_mutex> guard(d->lock);
		d->module_level.erase(module);
		d->empty_map = d->module_level.empty();
	}

	bool logger::level(level_type l,char const *module)
	{
		return l <= module_level(module);
	}

	void logger::default_level(level_type l)
	{
		d->global_level = l;
	}
	logger::level_type logger::default_level()
	{
		return d->global_level;
	}

	logger::ostream_proxy logger::proxy(level_type l,char const *module,char const *file,int line)
	{
		ostream_proxy proxy(l,module,file,line,this);
		return proxy;
	}

	logger &logger::instance()
	{
		static std::auto_ptr<logger> instance_ptr;
		static boost::once_flag flag;
		boost::call_once(flag,boost::bind(init,boost::ref(instance_ptr)));
	}

	void logger::init(std::auto_ptr<logger> &logger_ref)
	{
		logger_ref.reset(new logger());
	}
	logger::ostream_proxy::ostream_proxy() :
		level_(all),
		line_(0),
		file_(""),
		module_(""),
		log_(&logger::instance()),
		output_(new std::ostringstream)
	{
	}

	logger::ostream_proxy::ostream_proxy(level_type lv,char const *m,char const *f,int line,logger *log) :
		level_(lv),
		line_(line),
		file_(f),
		module_(m),
		log_(log),
		output_(new std::ostringstream)
	{
	}
	
	logger::ostream_proxy::ostream_proxy(logger::ostream_proxy &other) :
		level_(other.level_),
		line_(other.line_),
		file_(other.file_),
		module_(other.module_),
		log_(other.log_)
	{
		output_ = other.output_;
	}

	logger::ostream_proxy &logger::ostream_proxy::operator=(logger::ostream_proxy &other)
	{
		if(&other!=this) {
			level_ = other.level_;
			line_ = other.line_;
			file_ = other.file_;
			module_ = other.module_;
			log_ = other.log_;
			output_ = other.output_;
		}
		return *this;
	}

	std::ostream &logger::ostream_proxy::out()
	{
		return *output_;
	}
	logger::ostream_proxy::~ostream_proxy()
	{
		try {
			log_->write_to_log(level_,module_,file_,line_,output_->str());
		}
		catch(...)
		{
		}
	}

	void logger::write_to_log(level_type l,char const *module,char const *file,int line,std::string const &msg)
	{
		switch(l) {
		case fatal: level = "fatal"; break;
		case critical: level = "critical"; break;
		case error: level = "error"; break;
		case warning: level = "warning"; break;
		case message: level = "message"; break;
		case info: level = "info"; break;
		case debug: level = "debug"; break;
		default:
			level="unknown";
		}
		std::string result = (boost::format("%1%, %2%:%3% %4%:%5%") 
			<<module << file << line << level << msg).str();
		for(unsigned i=0;i<d->devices.size();i++)
			d->devices[i]->write(result);
	}
	
} // cppcms
