///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_HTTP_FILE_H
#define CPPCMS_HTTP_FILE_H

#include <cppcms/defs.h>
#include <cppcms/cstdint.h>
#include <booster/hold_ptr.h>
#include <booster/nowide/fstream.h>
#include <booster/noncopyable.h>
#include <sstream>
#include <fstream>

namespace cppcms {

/// \cond INTERNAL
namespace impl { class multipart_parser; }
/// \endcond

namespace http {

	class request;

	///
	/// \brief This class holds a uploaded file, it is generally fetched via widgets::file or via http::request::files
	///
	/// It provides full information about uploaded data as it was send by browser and allows to read the file via
	/// std::istream seek-able interface or save to the file system
	///
	/// Note: this class does not perform any validations, for checking the data use widgets::file that allows to perform 
	/// numerous checks on the file data.
	///
	class CPPCMS_API file : public booster::noncopyable {
	public:
		///
		/// Get the name of the POST field (i.e. <input name="value" ...>)
		///
		std::string name() const;
		///
		/// Get the content-type of the file as it was sent by the browser.
		///
		std::string mime() const;
		///
		/// Returns true if content type defined
		///
		/// \ver{v1_2}
		bool has_mime() const;
		///
		/// Get the filename as it was sent by the browser.
		///
		std::string filename() const;

		///
		/// Get std::istream on the data, please note, you need to call data().seekg(0) when using this 
		/// stream first time.
		///
		std::istream &data();
		
		///
		/// Get the size of the file.
		///
		long long size();

		///
		/// Specify the path to the output file, note if is_temporary is true
		/// than the file would be deleted on cppcms::http::file destruction,
		/// unless save_to is called, otherwise it would remain persistent
		///
		/// \ver{v1_2}
		void output_file(std::string const &name,bool is_temporary = false);

		///
		/// Make sure that file created by output_file member function is not removed in destructor
		///
		/// \ver{v1_2}
		void make_permanent();
		///
		/// Close the file if it is still open, if the file temporary it is deleted, the the
		/// file in memory its content is removed, data() would return non-usable stream
		///
		/// Returns 0 in case of sucess and -1 in case of failure
		///
		/// \ver{v1_2}
		int close();
		
		///
		/// Save file to file named \a filename. Throws cppcms_error in case of failure.
		///
		/// Notes:
		///
		/// - this function maybe more efficient then just reading the stream and writing it to newly created file, as
		///   in case of big files, it would try to move it over the file system
		/// - Under Win32 \a filename should be UTF-8 string
		///
		void save_to(std::string const &filename);

		/// \cond INTERNAL 

		void name(std::string const &);
		void mime(std::string const &);
		void filename(std::string const &);
		std::ostream &write_data();

		file();
		~file();
		
		/// \endcond

		///
		/// Set the maximal size of file that would be stored in memory instead of file system
		/// 
		/// \ver{v1_2}
		void set_memory_limit(size_t size);
		///
		/// Set the temporary directory where uploaded files are created
		///
		/// \ver{v1_2}
		void set_temporary_directory(std::string const &dir);


	private:

		std::string name_;
		std::string mime_;
		std::string filename_;
		size_t size_limit_;

		booster::nowide::fstream res1_;
		std::stringstream res2_;
		std::string res3_;
		std::string res4_;

		void save_by_copy(std::string const &file_name,std::istream &in);
		void copy_stream(std::istream &in,std::ostream &out);

		
		uint32_t removed_ : 1 ;
		uint32_t file_specified_ : 1;
		uint32_t file_temporary_: 1;
		uint32_t reserverd_ : 29;

		struct impl_data; // for future use
		booster::hold_ptr<impl_data> d;
		friend class request;
	};


} } //::cppcms::http


#endif
