
#ifndef ZIPPER_H_
#define ZIPPER_H_

#include <unzip.h>
#include <zip.h>
#include <QString>

typedef enum
{
	ZIPPER_RESULT_ERROR = 0,
	ZIPPER_RESULT_SUCCESS,
	ZIPPER_RESULT_SUCCESS_EOF
} zipper_result_t;

zipper_result_t zipper_read( unzFile zfile, QByteArray &fileContents );

bool zipper_skip_file( unzFile zfile );
char *zipper_filename( unzFile zfile, bool *isutf8 );
bool zipper_isdir( unzFile zfile );
uint64_t zipper_filesize( unzFile zfile );

#endif /* ZIPPER_H__ */
