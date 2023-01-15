#include <cstring>
#ifdef _WIN32
#include <iowin32.h>
#endif

// #include "string_builder.h"
#include "zip_handler.h"

#define BUF_SIZE	8192
#define MAX_NAMELEN 256

QString getStringFromUnsignedChar( unsigned char *str, int length ){
	QString result = "";
	int lengthOfString = length;

	// print string in reverse order
	QString s;
	for( int i = 0; i < lengthOfString; i++ ){
		s = QString( "%1" ).arg( str[i], 0, 16 );

		// account for single-digit hex values (always must serialize as two digits)
		if( s.length() == 1 )
			result.append( "0" );

		result.append( s );
	}

	return result;
}

zipper_result_t zipper_read( unzFile zfile, QByteArray &fileContents )
{
	auto *tbuf = new unsigned char[BUF_SIZE];
	int red;
	int ret;

	if ( zfile == nullptr )
	{
		delete[] tbuf;
		return ZIPPER_RESULT_ERROR;
	}

	ret = unzOpenCurrentFile( zfile );
	if ( ret != UNZ_OK )
	{
		delete[] tbuf;
		return ZIPPER_RESULT_ERROR;
	}

	while ( ( red = unzReadCurrentFile( zfile, tbuf, BUF_SIZE ) ) > 0 )
	{
		fileContents.append( QByteArray(reinterpret_cast<char*>(tbuf), red) );
	}

	if ( red < 0 )
	{
		unzCloseCurrentFile( zfile );
		delete[] tbuf;
		return ZIPPER_RESULT_ERROR;
	}

	unzCloseCurrentFile( zfile );
	if ( unzGoToNextFile( zfile ) != UNZ_OK )
	{
		delete[] tbuf;
		return ZIPPER_RESULT_SUCCESS_EOF;
	}

	delete[] tbuf;
	return ZIPPER_RESULT_SUCCESS;
}

char *zipper_filename( unzFile zfile, bool *isutf8 )
{
	char *name = new char[MAX_NAMELEN];
	unz_file_info64 finfo;
	int ret;

	if ( zfile == nullptr )
	{
		delete[] name;
		return nullptr;
	}
	ret = unzGetCurrentFileInfo64( zfile, &finfo, name, MAX_NAMELEN, nullptr, 0, nullptr, 0 );
	if ( ret != UNZ_OK )
	{
		delete[] name;
		return nullptr;
	}
	if ( isutf8 != nullptr )
		*isutf8 = ( finfo.flag & ( 1 << 11 ) ) != 0;

	auto strupName = strdup( name );
	delete[] name;
	return strupName;
}

bool zipper_isdir( unzFile zfile )
{
	char *name = new char[MAX_NAMELEN];
	unz_file_info64 finfo;
	size_t len;
	int ret;

	if ( zfile == nullptr )
	{
		delete[] name;
		return false;
	}

	ret = unzGetCurrentFileInfo64( zfile, &finfo, name, MAX_NAMELEN, nullptr, 0, nullptr, 0 );
	if ( ret != UNZ_OK )
	{
		delete[] name;
		return false;
	}

	len = strlen( name );
	if ( finfo.uncompressed_size == 0 && len > 0 && name[len - 1] == '/' )
	{
		delete[] name;
		return true;
	}
	delete[] name;
	return false;
}

bool zipper_skip_file( unzFile zfile )
{
	return unzGoToNextFile( zfile ) != UNZ_OK;
}

uint64_t zipper_filesize( unzFile zfile )
{
	unz_file_info64 finfo;
	int ret;

	if ( zfile == nullptr )
		return 0;

	ret = unzGetCurrentFileInfo64( zfile, &finfo, nullptr, 0, nullptr, 0, nullptr, 0 );
	if ( ret != UNZ_OK )
		return 0;
	return finfo.uncompressed_size;
}