#include "filepath.h"

#include <cell/fs/cell_fs_file_api.h>
#include <sys/return_code.h>

#include "cell/DebugLog.h"

void FileClose(FileHandle* h) 
{
	if (*h != 0) 
		cellFsClose(*h);
	*h = -1;
}

bool FileOpen(CFilePath& fp, FileHandle* fd, EOpenMode mode) 
{
	*fd = -1;
	int ret;
	switch (mode) {
		case OPEN_READ: {
			ret = cellFsOpen(fp.c_str(), CELL_FS_O_RDONLY, fd, NULL, 0); 
			break;
		}
		case OPEN_WRITE: {
			ret = cellFsOpen(fp.c_str(), CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_TRUNC, fd, NULL, 0);
			break;
		}
		case OPEN_APPEND: {
			ret = cellFsOpen(fp.c_str(), CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_APPEND, fd, NULL, 0);
			break;
		}
		case OPEN_RDWR: {
			ret = cellFsOpen(fp.c_str(), CELL_FS_O_RDWR |  CELL_FS_O_CREAT, fd, NULL, 0);
			break;
		}
		// case OPEN_SECURE: {
		// 	static SceNpDrmKey KioskKey = {
		// 		{ 0xC0, 0xA3, 0xB3, 0x64, 0x1C, 0x2A, 0xD1, 0xEF, 0x23, 0x15, 0x3A, 0x48, 0xA3, 0xE1, 0x2F, 0xE7 }
		// 	};

		// 	//if (IsKioskDemo()) {
		// 	//	ret = cellFsSdataOpen(fp->Filepath, CELL_FS_O_RDWR |  CELL_FS_O_CREAT, fd, NULL, 0);
		// 	//	break;
		// 	//}

		// 	ret = sceNpDrmOpen(&KioskKey, fp.c_str(), CELL_FS_O_RDWR | CELL_FS_O_CREAT, fd, NULL, 0);
		// 	break;
		// }
		default: return false;
	}

	if (ret != CELL_FS_OK) 
		DebugLogChF(DC_RESOURCE, "Failed cellFsOpen(%s) %d\n", fp.c_str(), ret);

	return ret == CELL_FS_OK;

}

u64 FileRead(FileHandle h, void* out, u64 count) {
	u64 n;
	int ret = cellFsRead(h, out, count, &n);
	if (ret != CELL_FS_OK) {
		DebugLogChF(DC_RESOURCE, "Failed cellFsRead %d\n", ret);
		return 0;
	}
	return n;
}

u64 FileWrite(FileHandle h, void* bin, u64 count)
{
	u64 n;
	cellFsWrite(h, bin, count, &n);
	return n;
}

u64 FileSeek(FileHandle h, s64 newpos, u32 whence)
{
	u64 p;
	cellFsLseek(h, newpos, whence, &p);
	return 0;
}

bool FileStat(FileHandle h, u64* modtime, u64* size) 
{
	*modtime = 0;
	*size = 0;
	if (h != -1) 
	{
		CellFsStat status;
		if (cellFsFstat(h, &status) == CELL_FS_OK) 
		{
			*modtime = status.st_mtime;
			*size = status.st_size;
			return true;
		}
	}
	return false;
}

bool FileStat(CFilePath& fp, u64* modtime, u64* size)
{
	*modtime = 0;
	*size = 0;
    CellFsStat status;
    if (cellFsStat(fp.c_str(), &status) == CELL_FS_OK) 
	{
        *modtime = status.st_mtime;
        *size = status.st_size;
        return true;
    }
	return false;
}

bool FileExists(CFilePath& fp)
{
    u64 modtime, size;
    return FileStat(fp, &modtime, &size);
}

char* FileLoadText(CFilePath& fp)
{
	FileHandle fd;
	u64 modtime, size;
	if (FileOpen(fp, &fd, OPEN_READ) && FileStat(fd, &modtime, &size))
	{
		char* data = new char[size + 1];
		if (size != 0)
			FileRead(fd, data, size);
		FileClose(&fd);
		data[size] = '\0';
		
		return data;
	}

	return NULL;
}

MH_DefineFunc(_FileLoad, 0x0057b6ec, TOC1, bool, CFilePath const&, ByteArray&, CHash&)
bool FileLoad(CFilePath const& fp, ByteArray& bufout, CHash& hash_out)
{
	return _FileLoad(fp, bufout, hash_out);
}

MH_DefineFunc(_LinesLoad, 0x0057bcc8, TOC1, bool, const ByteArray&, CVector<MMString<char> >&, ParseFn)
bool LinesLoad(const ByteArray& bytes, CVector<MMString<char> >& out, ParseFn parsefunc)
{
	return _LinesLoad(bytes, out, parsefunc);
}

MH_DefineFunc(_StripAndIgnoreHash, 0x0057baf0, TOC1, bool, TextRange<char>& range)
bool StripAndIgnoreHash(TextRange<char>& range)
{
	return _StripAndIgnoreHash(range);
}