#define BOOSTER_SOURCE
#include <booster/streambuf.h>


namespace booster {
	struct streambuf::_data{};

	streambuf::streambuf() : 
		buffer_size_(1024),
		device_(0)
	{

	}
	streambuf::~streambuf()
	{
	}

	int streambuf::overflow(int c)
	{
		if(pptr()==0) {
			buffer_out_.resize(buffer_size_+1,0);
			setp(&buffer_out_.front(),&buffer_out_.front()+buffer_out_.size());
		}
		
		std::streamsize n=pptr() - pbase();
		std::streamsize real_size = n;

		if(c!=EOF) {
			*pptr()=c;
			real_size++;
		}
		if(real_size > 0 && std::streamsize(device().write(pbase(),real_size))!=real_size)
			return EOF;
		pbump(-n);
		return 0;
	}
	int streambuf::sync()
	{
		if(pptr()!=pbase())
			return overflow(EOF);
		return 0;
	}

	int streambuf::underflow()
	{
		if(buffer_in_.empty())
			buffer_in_.resize(buffer_size_);
		char *buf_ptr = &buffer_in_.front();
		std::streamsize n=device().read(buf_ptr,buffer_in_.size());
		setg(buf_ptr,buf_ptr,buf_ptr+n);
		if(n==0)
			return EOF;
		return *buf_ptr;
	}

	io_device &streambuf::device()
	{
		static io_device dummy;
		if(!device_)
			return dummy;
		else
			return *device_;
	}
	void streambuf::device(io_device &d)
	{
		reset_device();
		device_ = &d;
	}
	void streambuf::reset_device()
	{
		sync();
		device_ = 0;
		device_auto_ptr_.reset();
	}
	void streambuf::device(std::auto_ptr<io_device> d)
	{
		reset_device();
		device_auto_ptr_=d;
		device_ = device_auto_ptr_.get();
	}
	void streambuf::set_buffer_size(size_t n) 
	{
		if(n == 0)
			n=1;
		buffer_size_ = n;
	}
	std::streampos streambuf::seekoff(	std::streamoff off,
						std::ios_base::seekdir seekdir,
						std::ios_base::openmode /*m*/)
	{
		if(sync())
			return -1;
		if(!buffer_in_.empty()) {
			setg(0,0,0);
		}
		if(seekdir == std::ios_base::cur)
			return device().seek(off,io_device::cur);
		if(seekdir == std::ios_base::beg)
			return device().seek(off,io_device::set);
		if(seekdir == std::ios_base::end)
			return device().seek(off,io_device::end);
		return -1;
	}
	std::streampos streambuf::seekpos(std::streampos off,std::ios_base::openmode m)
	{
		return seekoff(std::streamoff(off),std::ios_base::beg,m);
	}


}
