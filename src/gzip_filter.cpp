///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////

#include <zlib.h>
#include <booster/streambuf.h>
#include <booster/backtrace.h>
#include <ostream>

namespace cppcms {
	namespace impl {

		class gzip_device : public booster::io_device {
		public:
			gzip_device() :
				opened_(false),
				z_stream_(z_stream()),
				out_(0),
				level_(-1),
				buffer_(4096)
			{
			}

			void level(int v) 
			{
				level_ = v;
			}

			void buffer(size_t size)
			{
				if(size < 256)
					size = 256;
				buffer_ = size;
			}

			void open()
			{
				if(deflateInit2(&z_stream_,
						level_,
						Z_DEFLATED,
						15 + 16, // 15 window bits+gzip = 16,
						8, // memuse
						Z_DEFAULT_STRATEGY) != Z_OK)
				{
					std::string error = "ZLib init failed";
					if(z_stream_.msg) {
						error+=":";
						error+=z_stream_.msg;
					}
					throw booster::runtime_error(error);
				}
				opened_ = true;
				chunk_.resize(buffer_,0);
			}

			void output(std::ostream &out)
			{
				out_ = &out;
			}

			size_t write(char const *src,size_t n)
			{
				if(!out_ || !opened_) {
					return 0;
				}
				
				if(n==0) {
					return 0;
				}

				z_stream_.avail_in = n;
				z_stream_.next_in = (Bytef*)(src);
				z_stream_.avail_out = chunk_.size();
				z_stream_.next_out = (Bytef*)(&chunk_[0]);

				do {
					deflate(&z_stream_,Z_NO_FLUSH);
					size_t have = chunk_.size() - z_stream_.avail_out;
					if(!out_->write(&chunk_[0],have)) {
						close();
						return 0;
					}
				} while(z_stream_.avail_out == 0);

				return n;
			}

			void close()
			{
				if(!opened_)
					return;
				if(out_) {
					z_stream_.avail_in = 0;
					z_stream_.next_in = 0;
					z_stream_.avail_out = chunk_.size();
					z_stream_.next_out = (Bytef*)(&chunk_[0]);
					do {
						deflate(&z_stream_,Z_FINISH);
						size_t have = chunk_.size() - z_stream_.avail_out;
						if(!out_->write(&chunk_[0],have)) {
							break;
						}
					} while(z_stream_.avail_out == 0);
					out_->flush();
				}

				deflateEnd(&z_stream_);
				opened_ = false;
				z_stream_ = z_stream();
				chunk_.clear();

			}
			~gzip_device()
			{
				if(opened_)
					deflateEnd(&z_stream_);
			}
		private:
			bool opened_;
			std::vector<char> chunk_;
			z_stream z_stream_;
			std::ostream *out_;
			int level_;
			size_t buffer_;
		};

		class gzip_filter : public std::ostream, public gzip_device {
		public:
			gzip_filter() : 
				std::ios(&buf_),
				std::ostream(&buf_)
			{
				init(&buf_);
				buf_.device(*this);
			}
			void close()
			{
				flush();
				gzip_device::close();
			}
		private:
			gzip_device device_;
			booster::streambuf buf_;

		};

	} // impl
} // cppcms

#include <iostream>
#include <fstream>

int main()
{
	cppcms::impl::gzip_filter filter;
	std::ifstream in("gzip_filter.cpp");
	std::ofstream out("test.cpp.gz");
	filter.output(out);
	filter.open();
	std::cerr << "A" << std::endl;
	filter << in.rdbuf();
	std::cerr << "B" << std::endl;
	filter.close();
	std::cerr << "C" << std::endl;
}
