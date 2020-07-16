#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

namespace Archive
{
	void Write(const char* path, const char* buffer, unsigned int size);
	void Append(const char* path, const char* buffer);
};

#endif //__ARCHIVE_H__