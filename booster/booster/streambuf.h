#ifndef BOOSTER_STREAMBUF_H
#define BOOSTER_STREAMBUF_H

#include <booster/config.h>
#include <streambuf>
#include <stdio.h>
#include <booster/hold_ptr.h>
#include <memory>
#include <vector>

namespace booster {

	///
	/// \brief This class is a base class of generic I/O device that can be
	/// used in very simple manner with booster::streambuf allowing to create
	/// iostreams easily
	///

	class io_device {
	public:
		///
		/// Seek reference 
		///
		typedef enum {
			set, //!< Set actual position (i.e. SEEK_CUR)
			cur, //!< Set relatively to current position (i.e. SEEK_CUR)
			end  //!< Set relatively to end of file (i.e. SEEK_END)
		} pos_type;

		///
		/// Read \a length bytes from the stream to buffer \a pos, return number of bytes
		/// actually read. If return value is less then length, it is considered end of file
		///
		/// If the stream is write only, do not implement (returns EOF by default)
		///
		virtual size_t read(char *pos,size_t length)
		{
			return 0;
		}
		///
		/// Write \a length bytes to the devise from buffer \a pos, return number of bytes
		/// actually written, if the result is less then \a length, considered as EOF.
		///
		/// If the stream is read only, do not implement (returns EOF by default)
		///
		virtual size_t write(char const *pos,size_t length)
		{
			return 0;
		}
		///
		/// Seek the device to \a position relatively to \a pos. Return current position
		/// in file.
		///
		/// If error occurs return -1.
		///
		/// If the stream is not seekable do not reimplement, returns -1 by default.
		///
		virtual long long seek(long long position,pos_type pos = set)
		{
			return -1;
		}
		virtual ~io_device()
		{
		}
	};

	///
	/// \brief this is an implementation of generic streambuffer
	///
	class BOOSTER_API streambuf : public std::streambuf {
	public:

		///
		/// Create a new stream buffer - without a stream
		///
		streambuf();
		~streambuf();
		
		///
		/// Assign an io_device to the streambuf transferring an ownership on it
		///
		void device(std::auto_ptr<io_device> d);
		///
		/// Assign an existing io_device to the streambuf.
		///
		void device(io_device &d);

		///
		/// Detach currently attached io_device from the streambuf. If it is owned, it is destroyed.
		///
		void reset_device();

		///
		/// Get the io_device that is in use 
		///
		io_device &device();
		///
		/// Set the size of the internal buffer that is used for read and write operations. Default
		/// is 1024 bytes for each direction
		///
		void set_buffer_size(size_t n);

	protected:

		// Seek

		virtual std::streampos seekoff(	std::streamoff off,
						std::ios_base::seekdir way,
						std::ios_base::openmode m = std::ios_base::in | std::ios_base::out);
		virtual std::streampos seekpos(	std::streampos pos,
						std::ios_base::openmode m = std::ios_base::in | std::ios_base::out);
		
		// Get
		
		virtual int underflow();
		virtual int pbackfail(int c = EOF);

		// Put
		
		virtual int overflow(int c = EOF);
		virtual int sync();
		
	private:

		std::vector<char> buffer_out_;
		std::vector<char> buffer_in_;

		size_t buffer_size_;

		struct _data;
		hold_ptr<_data> d; // for future use

		std::auto_ptr<io_device> device_auto_ptr_;
		io_device *device_;

	};
} // booster


#endif
