///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/copy_filter.h>

namespace cppcms {
	class copy_filter::tee_device : public booster::io_device{
	public:
		tee_device(std::ostream &output,std::list<std::string> &output_buffer) :
			output_(output),
			output_buffer_(&output_buffer)
		{
		}
		size_t write(char const *pos,size_t len)
		{
			output_.write(pos,len);
			if(!output_)
				return 0;
			output_buffer_->push_back(std::string());
			output_buffer_->back().assign(pos,len);
			return len;
		}
	private:
		std::ostream &output_;
		std::list<std::string> *output_buffer_;
	};

	struct copy_filter::data {};

	copy_filter::copy_filter(std::ostream &output) :
		output_(output),
		real_output_stream_(output.rdbuf(&copy_buffer_)),
		detached_(false)
	{
		std::auto_ptr<booster::io_device> device(new tee_device(real_output_stream_,data_));
		copy_buffer_.device(device);
	}

	std::string copy_filter::detach()
	{
		output_ << std::flush;
		copy_buffer_.reset_device();
		detached_ = true;
		output_.rdbuf(real_output_stream_.rdbuf(0));
		size_t len = 0;
		std::list<std::string>::iterator p;
		for(p=data_.begin();p!=data_.end();p++)
			len+=p->size();
		std::string result;
		result.reserve(len);
		for(p=data_.begin();p!=data_.end();p++)
			result+=*p;
		return result;
	}

	copy_filter::~copy_filter()
	{
		if(!detached_) {
			output_.rdbuf(real_output_stream_.rdbuf(0));
		}
	}

}
