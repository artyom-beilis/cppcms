#ifndef BOOSTER_NOWIDE_FSTREAM_H
#define BOOSTER_NOWIDE_FSTREAM_H

#include <booster/config.h>
#include <booster/nowide/convert.h>
#include <fstream>
#include <memory>

#if defined BOOSTER_WIN_NATIVE || defined BOOSTER_WORKAROUND_BROKEN_GCC_ON_DARWIN
#include <booster/streambuf.h>
#include <booster/nowide/cstdio.h>
#endif

namespace booster {
///
/// \brief This namespace includes implementation of basic STL's / STDLIb's functions
/// such that they accept UTF-8 strings. on Windows. Otherwise it is just an alias
/// of std namespace (i.e. not on Windows)
///
namespace nowide {
#if !defined BOOSTER_WIN_NATIVE && !defined(BOOSTER_WORKAROUND_BROKEN_GCC_ON_DARWIN)

	using std::basic_ifstream;
	using std::basic_ofstream;
	using std::basic_fstream;
	using std::basic_filebuf;
	using std::filebuf;
	using std::ifstream;
	using std::ofstream;
	using std::fstream;

#endif
#if defined(BOOSTER_WIN_NATIVE) || defined(BOOSTER_DOXYGEN_DOCS) || defined(BOOSTER_WORKAROUND_BROKEN_GCC_ON_DARWIN)

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
	/// \cont INTERNAL
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

	/// \endcond

	template<typename CharType,typename Traits = std::char_traits<CharType> >
	class basic_filebuf;

	///
	/// Same as std::basic_filebuf<char> but accepts UTF-8 strings under Windows
	///

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
			wchar_t const *smode = get_mode(mode);
			if(!smode)
				return 0;
			std::wstring name;
			try {
				name = convert(s);
			}
			catch(bad_utf const &) {
				return 0;
			}
			FILE *f = _wfopen(name.c_str(),smode);
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
		static wchar_t const *get_mode(std::ios_base::openmode mode)
		{
			//
			// done according to n2914 table 106 27.9.1.4
			//

			// note can't use switch case as overload operator can't be used
			// in constant expression
			if(mode == (std::ios_base::out))
				return L"w";
			if(mode == (std::ios_base::out | std::ios_base::app))
				return L"a";
			if(mode == (std::ios_base::app))
				return L"a";
			if(mode == (std::ios_base::out | std::ios_base::trunc))
				return L"w";
			if(mode == (std::ios_base::in))
				return L"r";
			if(mode == (std::ios_base::in | std::ios_base::out))
				return L"r+";
			if(mode == (std::ios_base::in | std::ios_base::out | std::ios_base::trunc))
				return L"w+";
			if(mode == (std::ios_base::in | std::ios_base::out | std::ios_base::app))
				return L"a+";
			if(mode == (std::ios_base::in | std::ios_base::app))
				return L"a+";
			if(mode == (std::ios_base::binary | std::ios_base::out))
				return L"wb";
			if(mode == (std::ios_base::binary | std::ios_base::out | std::ios_base::app))
				return L"ab";
			if(mode == (std::ios_base::binary | std::ios_base::app))
				return L"ab";
			if(mode == (std::ios_base::binary | std::ios_base::out | std::ios_base::trunc))
				return L"wb";
			if(mode == (std::ios_base::binary | std::ios_base::in))
				return L"rb";
			if(mode == (std::ios_base::binary | std::ios_base::in | std::ios_base::out))
				return L"r+b";
			if(mode == (std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::trunc))
				return L"w+b";
			if(mode == (std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::app))
				return L"a+b";
			if(mode == (std::ios_base::binary | std::ios_base::in | std::ios_base::app))
				return L"a+b";
			return 0;	
		}

		bool opened_;
	};

	#endif

	///
	/// Same as std::basic_ifstream<char> but accepts UTF-8 strings under Windows
	///
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
			if(!buf_->open(file_name,mode | std::ios_base::in)) {
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

	///
	/// Same as std::basic_ofstream<char> but accepts UTF-8 strings under Windows
	///

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
			if(!buf_->open(file_name,mode | std::ios_base::out)) {
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

	///
	/// Same as std::basic_fstream<char> but accepts UTF-8 strings under Windows
	///

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


	///
	/// Same as std::filebuf but accepts UTF-8 strings under Windows
	///
	typedef basic_filebuf<char> filebuf;
	///
	/// Same as std::ifstream but accepts UTF-8 strings under Windows
	///
	typedef basic_ifstream<char> ifstream;
	///
	/// Same as std::ofstream but accepts UTF-8 strings under Windows
	///
	typedef basic_ofstream<char> ofstream;
	///
	/// Same as std::fstream but accepts UTF-8 strings under Windows
	///
	typedef basic_fstream<char> fstream;

#endif
} // nowide
} // booster



#endif
