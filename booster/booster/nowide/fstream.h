#ifndef BOOSTER_NOWIDE_FSTREAM_H
#define BOOSTER_NOWIDE_FSTREAM_H

#include <booster/config.h>
#include <booster/nowide/convert.h>
#include <fstream>
#include <memory>

#if defined BOOSTER_WIN_NATIVE
#  if defined BOOSTER_MSVC
#    define BOOSTER_NOWIDE_WIDE_BASIC_FILEBUF
#  elif defined __GLIBCXX__
#    include <ext/stdio_filebuf.h>
#    define BOOSTER_NOWIDE_STDIO_FILEBUF
#  else
#    define BOOSTER_NOWIDE_NATIVE
#  endif
#else
#  define BOOSTER_NOWIDE_NATIVE
#endif


namespace booster {
	namespace nowide {
		#ifdef BOOSTER_NOWIDE_NATIVE

		using std::basic_ifstream;
		using std::basic_ofstream;
		using std::basic_fstream;
		using std::basic_filebuf;
		using std::filebuf;
		using std::ifstream;
		using std::ofstream;
		using std::fstream;

		#elif defined BOOSTER_NOWIDE_WIDE_BASIC_FILEBUF

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

		#elif defined(BOOSTER_NOWIDE_STDIO_FILEBUF) 

		template<typename CharType,typename Traits = std::char_traits<CharType> >
		class basic_filebuf : public __gnu_cxx::stdio_filebuf<CharType,Traits> {
		public:
			typedef std::basic_filebuf<CharType,Traits> my_base_type;

			basic_filebuf() : file_(0)
			{
			}
			~basic_filebuf()
			{
				if(file_)
					fclose(file_);
			}
			basic_filebuf *open(char const *s,std::ios_base::openmode mode)
			{
				if(file_) {
					fclose(file_);
					file_=0;
				}
				std::wstring name;
				try {
					name=convert(s).c_str();
				}
				catch(bad_utf const &e) {
					return 0;
				}
				std::wstring stdio_mode=get_mode(mode);

				if(stdio_mode.empty())
					return 0;
				file_ = _wfopen(name.c_str(),stdio_mode.c_str());
				if(!file_)
					return 0;
				if(my_base_type::open(file_,mode)) {
					return this;
				}
				fclose(file_);
				file_=0;
				return 0;
			}
			basic_filebuf *close()
			{
				bool res = my_base_type::close();
				if(file_) 
					fclose(file_);
				file_ = 0;
				return res ? this : 0;
			}
		private:
			static std::wstring get_mode(std::ios_base::openmode mode)
			{
				if(mode==(std::ios_base::in))
					return L"r";
				if(mode==(std::ios_base::out))
				if(mode==(std::ios_base::out | std::ios_base::trunc))
					return L"w";
				if(mode==(std::ios_base::out | std::ios_base::app))
					return L"a";
				if(mode==(std::ios_base::out | std::ios_base::in))
					return L"r+";
				if(mode==(std::ios_base::out | std::ios_base::trunc | std::ios_base::in))
					return L"w+";
				if(mode==(std::ios_base::out | std::ios_base::app | std::ios_base::in))
					return L"a+";
				if(mode==(std::ios_base::binary | std::ios_base::in))
					return L"br";
				if(mode==(std::ios_base::binary | std::ios_base::out))
				if(mode==(std::ios_base::binary | std::ios_base::out | std::ios_base::trunc))
					return L"bw";
				if(mode==(std::ios_base::binary | std::ios_base::out | std::ios_base::app))
					return L"ba";
				if(mode==(std::ios_base::binary | std::ios_base::out | std::ios_base::in))
					return L"br+";
				if(mode==(std::ios_base::binary | std::ios_base::out | std::ios_base::trunc | std::ios_base::in))
					return L"bw+";
				if(mode==(std::ios_base::binary | std::ios_base::out | std::ios_base::app | std::ios_base::in))
					return L"ba+";
					return L"";	
			}
			FILE *file_;
		};

		#endif

		#ifndef BOOSTER_NOWIDE_NATIVE 

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
