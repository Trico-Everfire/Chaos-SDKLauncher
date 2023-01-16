#include "ziphandler.h"
#include <QDebug>

CZipHandler::Result CZipHandler::Read( QByteArray &fileContents )
{
	if ( !isValid )
		return Result::ZIPPER_RESULT_ERROR;

	std::array<unsigned char, BUF_SIZE> tbuf {};
	int red;
	int ret;

	if ( zfile == nullptr )
	{
		return Result::ZIPPER_RESULT_ERROR;
	}

	ret = unzOpenCurrentFile( zfile );
	if ( ret != UNZ_OK )
	{
		return Result::ZIPPER_RESULT_ERROR;
	}

	while ( ( red = unzReadCurrentFile( zfile, tbuf.data(), BUF_SIZE ) ) > 0 )
	{
		fileContents.append( QByteArray( reinterpret_cast<char *>( tbuf.data() ), red ) );
	}

	if ( red < 0 )
	{
		unzCloseCurrentFile( zfile );
		return Result::ZIPPER_RESULT_ERROR;
	}

	unzCloseCurrentFile( zfile );

	if ( unzGoToNextFile( zfile ) != UNZ_OK )
	{
		return Result::ZIPPER_RESULT_SUCCESS_EOF;
	}

	return Result::ZIPPER_RESULT_SUCCESS;
}

QString CZipHandler::GetFilename( bool *isutf8 )
{
	if ( !isValid )
		return {};
	std::array<char, BUF_SIZE> name {};
	unz_file_info64 finfo;
	int ret;

	if ( zfile == nullptr )
		return {};

	ret = unzGetCurrentFileInfo64( zfile, &finfo, name.data(), MAX_NAMELEN, nullptr, 0, nullptr, 0 );

	if ( ret != UNZ_OK )
		return {};

	if ( isutf8 != nullptr )
		*isutf8 = ( finfo.flag & ( 1 << 11 ) ) != 0;

	return { name.data() };
}

bool CZipHandler::IsDir()
{
	if ( !isValid )
		return false;

	std::array<char, BUF_SIZE> name {};
	unz_file_info64 finfo;
	size_t len;
	int ret;

	if ( zfile == nullptr )
		return false;

	ret = unzGetCurrentFileInfo64( zfile, &finfo, name.data(), MAX_NAMELEN, nullptr, 0, nullptr, 0 );
	if ( ret != UNZ_OK )
		return false;

	len = strlen( name.data() );
	if ( finfo.uncompressed_size == 0 && len > 0 && name[len - 1] == '/' )
		return true;

	return false;
}

bool CZipHandler::SkipFile()
{
	if ( !isValid )
		return false;
	return unzGoToNextFile( zfile ) != UNZ_OK;
}

uint64_t CZipHandler::GetFileSize()
{
	if ( !isValid )
		return -1;

	unz_file_info64 finfo;
	int ret;

	if ( zfile == nullptr )
		return 0;

	ret = unzGetCurrentFileInfo64( zfile, &finfo, nullptr, 0, nullptr, 0, nullptr, 0 );
	if ( ret != UNZ_OK )
		return 0;
	return finfo.uncompressed_size;
}

CZipHandler::CZipHandler( QString path )
{
	zfile = unzOpen64( path.toStdString().c_str() );

	if ( zfile == nullptr )
	{
		qCritical() << QString::asprintf( "Could not open %s for unzipping\n", path.toStdString().c_str() );
		isValid = false;
	}
}
