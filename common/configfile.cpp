// ConfigFile.cpp

#include "configfile.h"

#include <string>

#include <strutils.h>
#include <common.h>

ConfigFile::ConfigFile( string16 filename, string16 delimiter,
                        string16 comment, string16 sentry )
	: myDelimiter(delimiter), myComment(comment), mySentry(sentry)
{
	// Construct a ConfigFile, getting keys and values from given file
	
	std::wifstream in( convertstring<char16_t, char>(filename).c_str() );

	fileValid = !!in;

	if (in)
		in >> (*this);
}


ConfigFile::ConfigFile()
	: myDelimiter( string16(1,L'=') ), myComment( string16(1,L'#') )
{
	// Construct a ConfigFile without a file; empty

	fileValid = false;
}


void ConfigFile::remove( const string16& key )
{
	// Remove key and its value
	myContents.erase( myContents.find( key ) );
	return;
}


bool ConfigFile::keyExists( const string16& key ) const
{
	// Indicate whether key is found
	mapci p = myContents.find( key );
	return ( p != myContents.end() );
}


/* static */
void ConfigFile::trim( string16& s )
{
	// Remove leading and trailing whitespace
	static const char16_t whitespace[] = L" \n\t\v\r\f";
	s.erase( 0, s.find_first_not_of(whitespace) );
	s.erase( s.find_last_not_of(whitespace) + 1U );
}


std::wostream& operator<<( std::wostream& os, const ConfigFile& cf )
{
	// Save a ConfigFile to os
	for( ConfigFile::mapci p = cf.myContents.begin();
	     p != cf.myContents.end();
		 ++p )
	{
		os << p->first.c_str() << " " << cf.myDelimiter.c_str() << " ";
		os << p->second.c_str() << std::endl;
	}
	return os;
}


std::wistream& operator>>( std::wistream& is, ConfigFile& cf )
{
	// Load a ConfigFile from is
	// Read in keys and values, keeping internal whitespace
	typedef string16::size_type pos;
	const string16& delim  = cf.myDelimiter;  // separator
	const string16& comm   = cf.myComment;    // comment
	const string16& sentry = cf.mySentry;     // end of file sentry
	const pos skip = delim.length();        // length of separator
	
	string16 nextline = L"";  // might need to read ahead to see where value ends

	TAssert(sizeof(wchar_t) == sizeof(char16_t));

	while( is || nextline.length() > 0 )
	{
		// Read an entire line at a time
		std::wstring sline;
		string16 line;
		if( nextline.length() > 0 )
		{
			line = nextline;  // we read ahead; use it now
			nextline = L"";
		}
		else
		{
			std::getline( is, sline );
			line = sline.c_str();
		}
		
		// Ignore comments
		line = line.substr( 0, line.find(comm) );
		
		// Check for end of file sentry
		if( sentry != L"" && line.find(sentry) != string16::npos ) return is;
		
		// Parse the line if it contains a delimiter
		pos delimPos = line.find( delim );
		if( delimPos < string16::npos )
		{
			// Extract the key
			string16 key = line.substr( 0, delimPos );
			line.replace( 0, delimPos+skip, L"" );
			
			// See if value continues on the next line
			// Stop at blank line, next line with a key, end of stream,
			// or end of file sentry
			bool terminate = false;
			while( !terminate && is )
			{
				std::wstring snextline;
				std::getline( is, snextline );
				nextline = snextline.c_str();
				terminate = true;
				
				string16 nlcopy = nextline;
				ConfigFile::trim(nlcopy);
				if( nlcopy == L"" ) continue;
				
				nextline = nextline.substr( 0, nextline.find(comm) );
				if( nextline.find(delim) != string16::npos )
					continue;
				if( sentry != L"" && nextline.find(sentry) != string16::npos )
					continue;
				
				nlcopy = nextline;
				ConfigFile::trim(nlcopy);
				if( nlcopy != L"" ) line += L"\n";
				line += nextline;
				terminate = false;
			}
			
			// Store key and value
			ConfigFile::trim(key);
			ConfigFile::trim(line);
			cf.myContents[key] = line;  // overwrites if key is repeated
		}
	}
	
	return is;
}
