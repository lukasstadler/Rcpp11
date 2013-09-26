//
// S4.h:  S4 objects
//
// Copyright (C) 2010 - 2011 Dirk Eddelbuettel and Romain Francois
//
// This file is part of Rcpp11.
//
// Rcpp11 is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Rcpp11 is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Rcpp11.  If not, see <http://www.gnu.org/licenses/>.

#ifndef Rcpp_S4_h
#define Rcpp_S4_h                     

#include <RcppCommon.h>
#include <Rcpp/RObject.h>

namespace Rcpp{ 

    /**
     * S4 object
     */
    class S4 : public RObject{
    public:
        S4() ;
        
        /**
         * checks that x is an S4 object and wrap it.
         *
         * @param x must be an S4 object
         */
        S4(SEXP x) ; 
        
        /**
         * copy constructor
         *
         * @param other other S4 object
         */
        S4(const S4& other) ;
        
        S4( S4&& other) { 
            m_sexp = other.m_sexp ;
            other.m_sexp = R_NilValue ;
        }
        S4& operator=( S4&& other ){
            RCPP_DEBUG_CLASS( S4, "::operator=( %s&& )", DEMANGLE(S4) )
            if( this != &other ){
                Rcpp_ReleaseObject(m_sexp) ;
                m_sexp = other.m_sexp ;
                other.m_sexp = R_NilValue ;
            }
            return *this ;
        }

        S4(const RObject::SlotProxy& proxy ) ;
        S4(const RObject::AttributeProxy& proxy ) ;
        
        /**
         * assignment operator. 
         */
        S4& operator=( const S4& other);
        
        S4& operator=( SEXP other ) ; 
        
        /**
         * Creates an S4 object of the requested class. 
         *
         * @param klass name of the target S4 class
         * @throw not_s4 if klass does not map to a known S4 class
         */
        S4( const std::string& klass ) ;
        
        /**
         * Indicates if this object is an instance of the given S4 class
         */
        bool is( const std::string& clazz) ;
        
    private:
        void set_sexp( SEXP x) ;
    } ;

} // namespace Rcpp

#endif
