#include <cstring>
#ifdef _WIN32
#include <iowin32.h>
#endif

#include "string_builder.h"
#include "zip_handler.h"

#define BUF_SIZE	8192
#define MAX_NAMELEN 256

zipper_result_t zipper_read( unzFile zfile, zipper_read_cb_t cb, void *thunk )
{
	unsigned char tbuf[BUF_SIZE];
	int red;
	int ret;

	if ( zfile == nullptr || cb == nullptr )
		return ZIPPER_RESULT_ERROR;

	ret = unzOpenCurrentFile( zfile );
	if ( ret != UNZ_OK )
		return ZIPPER_RESULT_ERROR;

	while ( ( red = unzReadCurrentFile( zfile, tbuf, sizeof( tbuf ) ) ) > 0 )
	{
		cb( tbuf, red, thunk );
	}

	if ( red < 0 )
	{
		unzCloseCurrentFile( zfile );
		return ZIPPER_RESULT_ERROR;
	}

	unzCloseCurrentFile( zfile );
	if ( unzGoToNextFile( zfile ) != UNZ_OK )
		return ZIPPER_RESULT_SUCCESS_EOF;
	return ZIPPER_RESULT_SUCCESS;
}

static void zipper_read_buf_cb( const unsigned char *buf, size_t buflen, void *thunk )
{
	auto *sb = static_cast<str_builder_t *>( thunk );
	str_builder_add_str( sb, (const char *)buf, buflen );
}

zipper_result_t zipper_read_buf( unzFile zfile, unsigned char **buf, size_t *buflen )
{
	str_builder_t *sb;
	zipper_result_t ret;

	sb = str_builder_create();
	ret = zipper_read( zfile, zipper_read_buf_cb, sb );
	if ( ret != ZIPPER_RESULT_ERROR )
		*buf = (unsigned char *)str_builder_dump( sb, buflen );
	str_builder_destroy( sb );
	return ret;
}

char *zipper_filename( unzFile zfile, bool *isutf8 )
{
	char name[MAX_NAMELEN];
	unz_file_info64 finfo;
	int ret;

	if ( zfile == nullptr )
		return nullptr;

	ret = unzGetCurrentFileInfo64( zfile, &finfo, name, sizeof( name ), nullptr, 0, nullptr, 0 );
	if ( ret != UNZ_OK )
		return nullptr;
	if ( isutf8 != nullptr )
		*isutf8 = ( finfo.flag & ( 1 << 11 ) ) != 0;
	return strdup( name );
}

bool zipper_isdir( unzFile zfile )
{
	char name[MAX_NAMELEN];
	unz_file_info64 finfo;
	size_t len;
	int ret;

	if ( zfile == nullptr )
		return false;

	ret = unzGetCurrentFileInfo64( zfile, &finfo, name, sizeof( name ), nullptr, 0, nullptr, 0 );
	if ( ret != UNZ_OK )
		return false;

	len = strlen( name );
	if ( finfo.uncompressed_size == 0 && len > 0 && name[len - 1] == '/' )
		return true;
	return false;
}

bool zipper_skip_file( unzFile zfile )
{
	if ( unzGoToNextFile( zfile ) != UNZ_OK )
		return false;
	return true;
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