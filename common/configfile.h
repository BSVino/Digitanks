// ConfigFile.h
// Class for reading named values from configuration files
// Richard J. Wagner  v2.1  24 May 2004  wagnerr@umich.edu

// Copyright (c) 2004 Richard J. Wagner
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

// Typical usage
// -------------
// 
// Given a configuration file "settings.inp":
//   atoms  = 25
//   length = 8.0  # nanometers
//   name = Reece Surcher
// 
// Named values are read in various ways, with or without default values:
//   ConfigFile config( "settings.inp" );
//   int atoms = config.read<int>( "atoms" );
//   double length = config.read( "length", 10.0 );
//   string author, title;
//   config.readInto( author, "name" );
//   config.readInto( title, "title", string("Untitled") );
// 
// See file example.cpp for more examples.

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <EASTL/string.h>
#include <EASTL/map.h>
#include <iostream>
#include <fstream>
#include <sstream>

using eastl::string;
using eastl::string16;

class ConfigFile {
// Data
protected:
	string16 myDelimiter;  // separator between key and value
	string16 myComment;    // separator between value and comments
	string16 mySentry;     // optional string to signal end of file
	eastl::map<string16,string16> myContents;  // extracted keys and values

	bool fileValid;

	typedef eastl::map<string16,string16>::iterator mapi;
	typedef eastl::map<string16,string16>::const_iterator mapci;

// Methods
public:
	ConfigFile( string16 filename,
	            string16 delimiter = L"=",
	            string16 comment = L"#",
				string16 sentry = L"EndConfigFile" );
	ConfigFile();
	
	// Search for key and read value or optional default value
	template<class T> T read( const string16& key ) const;  // call as read<T>
	template<class T> T read( const string16& key, const T& value ) const;
	template<class T> bool readInto( T& var, const string16& key ) const;
	template<class T>
	bool readInto( T& var, const string16& key, const T& value ) const;
	
	// Modify keys and values
	template<class T> void add( string16 key, const T& value );
	void remove( const string16& key );
	
	// Check whether key exists in configuration
	bool keyExists( const string16& key ) const;
	
	// Check or change configuration syntax
	string16 getDelimiter() const { return myDelimiter; }
	string16 getComment() const { return myComment; }
	string16 getSentry() const { return mySentry; }
	string16 setDelimiter( const string16& s )
		{ string16 old = myDelimiter;  myDelimiter = s;  return old; }  
	string16 setComment( const string16& s )
		{ string16 old = myComment;  myComment = s;  return old; }

	bool isFileValid() const { return fileValid; }
	
	// Write or read configuration
	friend std::wostream& operator<<( std::wostream& os, const ConfigFile& cf );
	friend std::wistream& operator>>( std::wistream& is, ConfigFile& cf );
	
protected:
	template<class T> static string16 T_as_string( const T& t );
	template<class T> static T string_as_T( const string16& s );
	static void trim( string16& s );
};

/* static */
template<class T>
string16 ConfigFile::T_as_string( const T& t )
{
	// Convert from a T to a string
	// Type T must support << operator
	std::wostringstream ost;
	ost << t;
	return ost.str().c_str();
}


/* static */
template<>
inline string16 ConfigFile::T_as_string<string16>( const string16& s )
{
	// Convert from a string to a string
	// In other words, do nothing
	return s;
}


/* static */
template<class T>
T ConfigFile::string_as_T( const string16& s )
{
	// Convert from a string to a T
	// Type T must support >> operator
	T t;
	std::wistringstream ist(s.c_str());
	ist >> t;
	return t;
}


/* static */
template<>
inline string16 ConfigFile::string_as_T<string16>( const string16& s )
{
	// Convert from a string to a string
	// In other words, do nothing
	return s;
}


/* static */
template<>
inline bool ConfigFile::string_as_T<bool>( const string16& s )
{
	// Convert from a string to a bool
	// Interpret "false", "F", "no", "n", "0" as false
	// Interpret "true", "T", "yes", "y", "1", "-1", or anything else as true
	bool b = true;
	string16 sup = s;
	for( string16::iterator p = sup.begin(); p != sup.end(); ++p )
		*p = toupper(*p);  // make string all caps
	if( sup==string16(L"FALSE") || sup==string16(L"F") ||
	    sup==string16(L"NO") || sup==string16(L"N") ||
	    sup==string16(L"0") || sup==string16(L"NONE") )
		b = false;
	return b;
}


template<class T>
T ConfigFile::read( const string16& key ) const
{
	// Read the value corresponding to key
	mapci p = myContents.find(key);
	if( p == myContents.end() ) return T();
	return string_as_T<T>( p->second );
}


template<class T>
T ConfigFile::read( const string16& key, const T& value ) const
{
	// Return the value corresponding to key or given default value
	// if key is not found
	mapci p = myContents.find(key);
	if( p == myContents.end() ) return value;
	return string_as_T<T>( p->second );
}


template<class T>
bool ConfigFile::readInto( T& var, const string16& key ) const
{
	// Get the value corresponding to key and store in var
	// Return true if key is found
	// Otherwise leave var untouched
	mapci p = myContents.find(key);
	bool found = ( p != myContents.end() );
	if( found ) var = string_as_T<T>( p->second );
	return found;
}


template<class T>
bool ConfigFile::readInto( T& var, const string16& key, const T& value ) const
{
	// Get the value corresponding to key and store in var
	// Return true if key is found
	// Otherwise set var to given default
	mapci p = myContents.find(key);
	bool found = ( p != myContents.end() );
	if( found )
		var = string_as_T<T>( p->second );
	else
		var = value;
	return found;
}


template<class T>
void ConfigFile::add( string16 key, const T& value )
{
	// Add a key with given value
	string16 v = T_as_string( value );
	trim(key);
	trim(v);
	myContents[key] = v;
	return;
}

#endif  // CONFIGFILE_H

// Release notes:
// v1.0  21 May 1999
//   + First release
//   + Template read() access only through non-member readConfigFile()
//   + ConfigurationFileBool is only built-in helper class
// 
// v2.0  3 May 2002
//   + Shortened name from ConfigurationFile to ConfigFile
//   + Implemented template member functions
//   + Changed default comment separator from % to #
//   + Enabled reading of multiple-line values
// 
// v2.1  24 May 2004
//   + Made template specializations inline to avoid compiler-dependent linkage
//   + Allowed comments within multiple-line values
//   + Enabled blank line termination for multiple-line values
//   + Added optional sentry to detect end of configuration file
//   + Rewrote messy trimWhitespace() function as elegant trim()
