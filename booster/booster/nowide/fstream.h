#ifndef BOOSTER_NOWIDE_FSTREAM_H
#define BOOSTER_NOWIDE_FSTREAM_H

#include <booster/config.h>
#include <booster/nowide/convert.h>
#include <fstream>
#include <memory>

#ifdef BOOSTER_WIN_NATIVE
#include <booster/streambuf.h>
#include <booster/nowide/cstdio.h>
#endif

namespace booster {
namespace nowide {
#ifndef BOOSTER_WIN_NATIVE

	using std::basic_ifstream;
	using std::basic_ofstream;
	using std::basic_fstream;
	using std::basic_filebuf;
	using std::filebuf;
	using std::ifstream;
	using std::ofstream;
	using std::fstream;

#else // windows crap:

	#if  defined BOOSTER_MSVC

	template<typename CharType,typename Traits = std::char_traits<CharType> >
	class basic_filebuf : public std::basic_filebuf<CharType,Traits> {
	public:
		typedef std::basic_filebuf<CharType,Traits> my_base_type;
		basic_filebuf *open(char const *s,std::ios_base::openmode mode)
		{
			try {
				if(my_base_type::open(convert(s).c_str(),mode)) {
					return this;
				}
				return 0;
			}
			catch(bad_utf const &e) {
				return 0;
			}
		}
	};

	#else  // not msvc

	namespace details {
		class stdio_iodev : public booster::io_device {
			stdio_iodev(stdio_iodev const &);
			void operator=(stdio_iodev const &);
		public:
			stdio_iodev(FILE *f) : 
				file_(f)
			{
				
			}
			size_t read(char *p,size_t n)
			{
				return fread(p,1,n,file_);
			}
			size_t write(char const *p,size_t n)
			{
				size_t res = fwrite(p,1,n,file_);
				fflush(file_);
				return res;
			}
			long long seek(long long pos,io_device::pos_type t=set) 
			{
				switch(t) {
				case cur:
					fseek(file_,pos,SEEK_CUR);
					break;
				case set:
					fseek(file_,pos,SEEK_SET);
					break;
				case end:
					fseek(file_,pos,SEEK_END);
					break;
				default:
					return -1;
				}
				return ftell(file_);
			}
			~stdio_iodev()
			{
				fclose(file_);
			}
		private:
			FILE *file_;
		};
	} // details

	template<typename CharType,typename Traits = std::char_traits<CharType> >
	class basic_filebuf;
	
	template<>
	class basic_filebuf<char> : public booster::streambuf {
	public:
		
		basic_filebuf() : opened_(false)
		{
		}
		~basic_filebuf()
		{
		}
		basic_filebuf *open(char const *s,std::ios_base::openmode mode)
		{
			reset_device();
			FILE *f = fopen(s,get_mode(mode));
			if(!f)
				return 0;
			std::auto_ptr<io_device> dev(new details::stdio_iodev(f));
			device(dev);
			opened_ = true;
			return this;
		}
		basic_filebuf *close()
		{
			
			bool res = sync();
			reset_device();
			opened_ = false;
			return res ? this : 0;
		}
		bool is_open() const
		{
			return opened_;
		}
	private:
		static char const *get_mode(std::ios_base::openmode mode)
		{
			if(mode==(std::ios_base::in))
				return "r";
			if(mode==(std::ios_base::out))
				return "w";
			if(mode==(std::ios_base::out | std::ios_base::trunc))
				return "w";
			if(mode==(std::ios_base::out | std::ios_base::app))
				return "a";
			if(mode==(std::ios_base::out | std::ios_base::in))
				return "r+";
			if(mode==(std::ios_base::out | std::ios_base::trunc | std::ios_base::in))
				return "w+";
			if(mode==(std::ios_base::out | std::ios_base::app | std::ios_base::in))
				return "a+";
			if(mode==(std::ios_base::binary | std::ios_base::in))
				return "br";
			if(mode==(std::ios_base::binary | std::ios_base::out))
				return "bw";
			if(mode==(std::ios_base::binary | std::ios_base::out | std::ios_base::trunc))
				return "bw";
			if(mode==(std::ios_base::binary | std::ios_base::out | std::ios_base::app))
				return "ba";
			if(mode==(std::ios_base::binary | std::ios_base::out | std::ios_base::in))
				return "br+";
			if(mode==(std::ios_base::binary | std::ios_base::out | std::ios_base::trunc | std::ios_base::in))
				return "bw+";
			if(mode==(std::ios_base::binary | std::ios_base::out | std::ios_base::app | std::ios_base::in))
				return "ba+";
			return "";	
		}

		bool opened_;
	};

	#endif

	template<typename CharType,typename Traits = std::char_traits<CharType> >
	class basic_ifstream : public std::basic_istream<CharType,Traits>
	{
	public:
		typedef basic_filebuf<CharType,Traits> internal_buffer_type;
		typedef std::basic_istream<CharType,Traits> internal_stream_type;

		basic_ifstream() : 
			internal_stream_type(new internal_buffer_type())
		{
			buf_.reset(static_cast<internal_buffer_type *>(internal_stream_type::rdbuf()));
		}
		explicit basic_ifstream(char const *file_name,std::ios_base::openmode mode = std::ios_base::in) :
			internal_stream_type(new internal_buffer_type())
		{
			buf_.reset(static_cast<internal_buffer_type *>(internal_stream_type::rdbuf()));
			open(file_name,mode);
		}
		void open(char const *file_name,std::ios_base::openmode mode = std::ios_base::in)
		{
			if(!buf_->open(file_name,mode)) {
				this->setstate(std::ios_base::failbit);
			}
			else {
				this->clear();
			}
		}
		bool is_open()
		{
			return buf_->is_open();
		}
		bool is_open() const
		{
			return buf_->is_open();
		}
		void close()
		{
			if(!buf_->close())
				this->setstate(std::ios_base::failbit);
			else
				this->clear();
		}

		internal_buffer_type *rdbuf() const
		{
			return buf_.get();
		}
		~basic_ifstream()
		{
			buf_->close();
		}
			
	private:
		std::auto_ptr<internal_buffer_type> buf_;
	};


	template<typename CharType,typename Traits = std::char_traits<CharType> >
	class basic_ofstream : public std::basic_ostream<CharType,Traits>
	{
	public:
		typedef basic_filebuf<CharType,Traits> internal_buffer_type;
		typedef std::basic_ostream<CharType,Traits> internal_stream_type;

		basic_ofstream() : 
			internal_stream_type(new internal_buffer_type())
		{
			buf_.reset(static_cast<internal_buffer_type *>(internal_stream_type::rdbuf()));
		}
		explicit basic_ofstream(char const *file_name,std::ios_base::openmode mode = std::ios_base::out) :
			internal_stream_type(new internal_buffer_type())
		{
			buf_.reset(static_cast<internal_buffer_type *>(internal_stream_type::rdbuf()));
			open(file_name,mode);
		}
		void open(char const *file_name,std::ios_base::openmode mode = std::ios_base::out)
		{
			if(!buf_->open(file_name,mode)) {
				this->setstate(std::ios_base::failbit);
			}
			else {
				this->clear();
			}
		}
		bool is_open()
		{
			return buf_->is_open();
		}
		bool is_open() const
		{
			return buf_->is_open();
		}
		void close()
		{
			if(!buf_->close())
				this->setstate(std::ios_base::failbit);
			else
				this->clear();
		}

		internal_buffer_type *rdbuf() const
		{
			return buf_.get();
		}
		~basic_ofstream()
		{
			buf_->close();
		}
			
	private:
		std::auto_ptr<internal_buffer_type> buf_;
	};

	template<typename CharType,typename Traits = std::char_traits<CharType> >
	class basic_fstream : public std::basic_iostream<CharType,Traits>
	{
	public:
		typedef basic_filebuf<CharType,Traits> internal_buffer_type;
		typedef std::basic_iostream<CharType,Traits> internal_stream_type;

		basic_fstream() : 
			internal_stream_type(new internal_buffer_type())
		{
			buf_.reset(static_cast<internal_buffer_type *>(internal_stream_type::rdbuf()));
		}
		explicit basic_fstream(char const *file_name,std::ios_base::openmode mode = std::ios_base::out | std::ios_base::in) :
			internal_stream_type(new internal_buffer_type())
		{
			buf_.reset(static_cast<internal_buffer_type *>(internal_stream_type::rdbuf()));
			open(file_name,mode);
		}
		void open(char const *file_name,std::ios_base::openmode mode = std::ios_base::out | std::ios_base::out)
		{
			if(!buf_->open(file_name,mode)) {
				this->setstate(std::ios_base::failbit);
			}
			else {
				this->clear();
			}
		}
		bool is_open()
		{
			return buf_->is_open();
		}
		bool is_open() const
		{
			return buf_->is_open();
		}
		void close()
		{
			if(!buf_->close())
				this->setstate(std::ios_base::failbit);
			else
				this->clear();
		}

		internal_buffer_type *rdbuf() const
		{
			return buf_.get();
		}
		~basic_fstream()
		{
			buf_->close();
		}
			
	private:
		std::auto_ptr<internal_buffer_type> buf_;
	};


	typedef basic_filebuf<char> filebuf;
	typedef basic_ifstream<char> ifstream;
	typedef basic_ofstream<char> ofstream;
	typedef basic_fstream<char> fstream;

#endif
} // nowide
} // booster



#endif
