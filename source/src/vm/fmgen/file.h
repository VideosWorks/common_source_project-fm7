//	$Id: file.h,v 1.6 1999/11/26 10:14:09 cisc Exp $

#if !defined(win32_file_h)
#define win32_file_h

#if defined(_USE_AGAR) || defined(_USE_SDL)
# include <agar/core.h>
# include "common.h"
#define MAX_PATH AG_PATHNAME_MAX

typedef enum  
{
   FILE_BEGIN = AG_SEEK_SET,
   FILE_CURRENT = AG_SEEK_CUR,
   FILE_END = AG_SEEK_END
} SeekMethod;
#define _tcsncpy(d,s,n) strncpy(d,s,n)
#define _tcsncat(d,s,n) strncat(d,s,n)

#else
# include <tchar.h>
#endif
//# include "types.h"

// ---------------------------------------------------------------------------

class FileIO
{
public:
	enum Flags
	{
		open		= 0x000001,
		readonly	= 0x000002,
		create		= 0x000004,
	};

	enum SeekMethod
	{
		begin = 0, current = 1, end = 2,
	};

	enum Error
	{
		success = 0,
		file_not_found,
		sharing_violation,
		unknown = -1
	};

public:
	FileIO();
	FileIO(const _TCHAR* filename, uint flg = 0);
	virtual ~FileIO();

	bool Open(const _TCHAR* filename, uint flg = 0);
	bool CreateNew(const _TCHAR* filename);
	bool Reopen(uint flg = 0);
	void Close();
	Error GetError() { return error; }

	int32 Read(void* dest, int32 len);
	int32 Write(const void* src, int32 len);
	bool Seek(int32 fpos, SeekMethod method);
	int32 Tellp();
	bool SetEndOfFile();

	uint GetFlags() { return flags; }
	void SetLogicalOrigin(int32 origin) { lorigin = origin; }

private:
#if defined(_USE_AGAR) || defined(_USE_SDL)
        AG_DataSource *hfile;
#else
	HANDLE hfile;
#endif
        uint flags;
	uint32 lorigin;
	Error error;
	_TCHAR path[_MAX_PATH];
	
	FileIO(const FileIO&);
	const FileIO& operator=(const FileIO&);
};

#endif // 
