///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_SERIALIZATION_CLASSES_H
#define CPPCMS_SERIALIZATION_CLASSES_H

#include <cppcms/defs.h>
#include <booster/copy_ptr.h>
#include <string>
#include <booster/backtrace.h>

#include <booster/traits/enable_if.h>
#include <booster/traits/is_base_of.h>

namespace cppcms {

	#ifdef CPPCMS_DOXYGEN_DOCS
	///
	/// \brief Special traits class that describes how to serialize various 
	/// objects that are not defived from serializable_base.
	///
	template<typename Object>
	struct archive_traits
	{
		///
		/// Save object to archive function
		/// 
		static void save(Object const &d,archive &a);
		///
		/// Load object from archive function
		///
		static void load(Object &d,archive &a);
	};
	
	#else
	 
	template<typename Object,typename Enable = void>
	struct archive_traits;
	#endif


	///
	/// \brief Error thrown in case of serialization error
	///
	class archive_error : public booster::runtime_error {
	public:
		archive_error(std::string const &e) : booster::runtime_error("cppcms::archive_error: " + e)
		{
		}
	};


	///
	/// \brief Class that represents a binary archive that can be stored in persistent storage or
	/// transfered.
	/// 
	class CPPCMS_API archive {
	public:

		///
		/// Reserve some memory before we write actual data
		///
		void reserve(size_t size);

		///
		/// Write a chunk of size len to archive
		///
		void write_chunk(void const *begin,size_t len);

		///
		/// Read a chunk of size len from archive
		///
		void read_chunk(void *begin,size_t len);
		
		///
		/// Get the size of the next chunk that can be read
		///
		size_t next_chunk_size();

		///
		/// Get if we got to the end of archive while reading
		///
		bool eof();


		///
		/// Read next chunk as std::string 
		///
		std::string read_chunk_as_string();

		///
		/// Operations mode on archive
		///
		typedef enum {
			save_to_archive,
			load_from_archive
		} mode_type; 

		///
		/// Set IO mode, resets pointer 
		///
		void mode(mode_type m);

		///
		/// Get IO mode
		///
		mode_type mode();

		///
		/// Reset IO pointer
		///
		void reset();

		///
		/// Get serialized object memory
		///
		std::string str();

		///
		/// Set serialized object memory, sets mode to load_from_archive
		///
		void str(std::string const &str);

		///
		/// Create new archive, by default in save_to_archive mode
		///
		archive();

		///
		/// Destructor
		///
		~archive();

		///
		/// Copy archive (avoid it)
		///
		archive(archive const &);

		///
		/// Assign archive (avoid it)
		///
		archive const &operator=(archive const &);

	private:
		std::string buffer_;
		size_t ptr_;
		mode_type mode_;	

		struct _data;
		booster::copy_ptr<_data> d;

	};

	///
	/// Operator that performs load or store \a object operations
	/// on archive \a 
	///
	template<typename Archivable>
	archive & operator &(archive &a,Archivable &object)
	{
		if(a.mode() == archive::save_to_archive)
			archive_traits<Archivable>::save(object,a);
		else
			archive_traits<Archivable>::load(object,a);
		return a;
	}

	///
	/// Operator that saves an object to the archive
	///
	template<typename Archivable>
	archive & operator <<(archive &a,Archivable const &object)
	{
		archive_traits<Archivable>::save(object,a);
		return a;
	}

	///
	/// Operator that loads an object from the archive
	///
	template<typename Archivable>
	archive & operator >>(archive &a,Archivable &object)
	{
		archive_traits<Archivable>::load(object,a);
		return a;
	}

	
	
	
	///
	/// \brief Base abstract class for object that can be serialized into std::string
	///
	class serializable_base {
	public:
		///
		/// Load object from archive
		///
		virtual void load(archive &serialized_object) = 0;
		///
		/// Save object to archive
		///
		virtual void save(archive &serialized_object) const = 0;

		virtual ~serializable_base()
		{
		}
	};

	///
	/// \brief Abstract class for serialization object
	/// 
	class serializable : public serializable_base {
	public:

		///
		/// Abstract function that should be implemented for correct serialization
		/// of an object, it allows implementing only one function for load and save instead of two.
		///
		/// For example:
		///
		/// \code
		///  
		///  struct persone : public serializable {
		///    double age;
		///    std::string name;
		///    std::vector<std::string> kids_names;
		///
		///    void serialize(archive &a) 
		///    {
		///         a & age & name & kids_names;
		///    }
		///  };
		/// \endcode
		///
		 
		virtual void serialize(archive &a) = 0;
		///
		/// Calls serialize member functions
		///
		virtual void load(archive &a)
		{
			serialize(a);
		}
		///
		/// Const-casts and calls serialize member function
		///
		virtual void save(archive &a) const
		{
			const_cast<serializable *>(this)->serialize(a);
		}
	};


	#ifdef CPPCMS_DOXYGEN_DOCS
	
	///
	/// \brief This is the traits class for serialization traits.
	///
	/// It allows user to use
	/// arbitrary serialization libraries or use its own serialization rules,
	/// for example you can use boost::serialization.
	///
	/// For this purpose user should specialize the serialization_traits struct for his type.
	///
	/// For example: We want to allow a serialization of a point class:
	///
	/// \code
	///
	//// namespace cppcms {
	///    template<>
	///    struct serialization_traits<point> {
	///  
	///      void load(std::string const &serialized_object,point &real_object)
	///      {
	///	   std::stringstream ss(serialized_object);
	///	   ss >> real_object.x >> real_object.y;
	///	 }
	///	 void save(point const &real_object,std::string &serialized_object)
	///	 {
	///	   std::stringstream ss;
	///	   ss << real_object.x << ' ' << real_object.y;
	///	   serialized_object = ss.str();
	///	 }
	///    };
	///  } // cppcms
	///
	/// \endcode
	///

	template<typename Object>
	struct serialization_traits {
		///
		/// Load \a real_object from the \a serialized_object representation (std::string)
		///
		static void load(std::string const &serialized_object,Object &real_object);

		///
		/// Save \a real_object to the \a serialized_object representation (std::string)
		///
		static void save(Object const &real_object,std::string &serialized_object);
	};
	
	#else

	template<typename Object,typename Enable = void>
	struct serialization_traits;

	///
	/// \brief Traits for serializable objects - converts object to and from string
	/// using archive class
	///
	template<typename D>
	struct serialization_traits<D,typename booster::enable_if<booster::is_base_of<serializable_base,D> >::type> {
		///
		/// Convert string to object
		///
		static void load(std::string const &serialized_object,serializable_base &real_object)
		{
			archive a;
			a.str(serialized_object);
			real_object.load(a);
		}
		///
		/// Convert object to string
		///
		static void save(serializable_base const &real_object,std::string &serialized_object)
		{
			archive a;
			real_object.save(a);
			serialized_object = a.str();
		}
	};


	#endif


} /// cppcms
#endif
