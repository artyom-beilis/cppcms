/* -*-c++-*- */
/*
 *  $Id: FCgiIO.h,v 1.3 2007/07/02 18:48:19 sebdiaz Exp $
 *
 *  Copyright (C) 2002 Steve McAndrewSmith
 *  Copyright (C) 2002 Stephen F. Booth
 *                2007 Sebastien DIAZ <sebastien.diaz@gmail.com>
 *  Part of the GNU cgicc library, http://www.gnu.org/software/cgicc
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA*
 */

#ifndef _FCGIIO_H_
#define _FCGIIO_H_ 1

#ifdef __GNUG__
#  pragma interface
#endif

/*! \file FCgiIO.h
 * \brief Class that implements input and output through a FastCGI request.
 *
 * This class provides access to the input byte-stream and environment
 * variable interfaces of a FastCGI request.  It is fully compatible with the
 * Cgicc input API.
 *
 * It also provides access to the request's output and error streams, using a
 * similar interface.
 */

#include <ostream>
#include <string>
#include <map>

#include "fcgio.h"

#include "cgicc/CgiInput.h"

namespace cgicc {

  // ============================================================
  // Class FCgiIO
  // ============================================================
  
  /*! \class FCgiIO FCgiIO.h FCgiIO.h
   * \brief Class that implements input and output through a FastCGI request.
   *
   * This class provides access to the input byte-stream and environment
   * variable interfaces of a FastCGI request.  It is fully compatible with the
   * Cgicc input API.
   *
   * It also provides access to the request's output and error streams, using a
   * similar interface.
   */
  class CGICC_API FCgiIO : public cgicc::CgiInput, public std::ostream
  {
  public:
    
    // ============================================================
    
    /*! \name Constructor and Destructor */
    //@{
    
    /*!
     * \brief Constructor
     *
     * Create a new FCgiIO object
     */
    FCgiIO(FCGX_Request& request);
    
    /*!
     * \brief Copy constructor
     *
     */
    FCgiIO(const FCgiIO& io);
    
    /*!
     * \brief Destructor
     *
     * Delete this FCgiIO object
     */
    virtual inline
    ~FCgiIO()
    {}
    //@}
    
    // ============================================================
    
    /*! \name Data Sources */
    //@{
    
    /*!
     * \brief Read data from the request's input stream.
     *
     * \param data The target buffer
     * \param length The number of characters to read
     * \return The number of characters read
     */
    virtual inline size_t read(char *data, size_t length)
    {
      return FCGX_GetStr(data, length, fRequest.in);
    }
    
    /*!
     * \brief Query the value of an environment variable stored in the request.
     *
     * \param varName The name of an environment variable
     * \return The value of the requested environment variable, or an empty
     * string if not found.
     */
    virtual inline std::string getenv(const char *varName)
    {
      return fEnv[varName];
    }
    //@}
    
    // ============================================================
    
    /*! \name Data Target Streams */
    //@{
    
    /*!
     * \brief Provides access to the error stream.
   */
    inline std::ostream& err(void)
    {
      return fErr;
    }
    //@}
    
  protected:
    FCGX_Request& 			fRequest;
    fcgi_streambuf 			fOutBuf;
    fcgi_streambuf 			fErrBuf;
    std::ostream 			fErr;
    std::map<std::string, std::string> 	fEnv;
  };
  
} // namespace cgicc

#endif /* ! _FCGIIO_H_ */
