/*
 -------------------------------------------------------------------------------
 | osgEphemeris - Copyright (C) 2007  Don Burns                                |
 |                                                                             |
 | This library is free software; you can redistribute it and/or modify        |
 | it under the terms of the GNU Lesser General Public License as published    |
 | by the Free Software Foundation; either version 3 of the License, or        |
 | (at your option) any later version.                                         |
 |                                                                             |
 | This library is distributed in the hope that it will be useful, but         |
 | WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY  |
 | or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public     |
 | License for more details.                                                   |
 |                                                                             |
 | You should have received a copy of the GNU Lesser General Public License    |
 | along with this software; if not, write to the Free Software Foundation,    |
 | Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.               |
 |                                                                             |
 -------------------------------------------------------------------------------
 */

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

#include <osgEphemeris/Shmem.h>




void *Shmem::operator new( size_t size, const std::string &file)
{
#ifdef _WIN32   // [
    HANDLE hFile = CreateFile( file.c_str(), GENERIC_READ | GENERIC_WRITE,
    		FILE_SHARE_READ | FILE_SHARE_WRITE,
		0,
		OPEN_ALWAYS, 0, 0 );

    SetFilePointer( hFile, size, 0, FILE_BEGIN );
    DWORD bytes;
    WriteFile( hFile, &size, sizeof( size ), &bytes, 0 );

    HANDLE hFileMap = CreateFileMapping( hFile, 
    			NULL, PAGE_READWRITE, 0, size, file.c_str()  );

    CloseHandle( hFile );

    Shmem *shm = (Shmem *)MapViewOfFile( hFileMap, 
    			FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0 );


#else // ][

    if((access( file.c_str(), F_OK )) < 0 )
    {
        int fd;
	    int pid = getpid();
        if( (fd = creat( file.c_str(), O_RDWR | 0666 )) < 0 )
	    {
	        char emsg[128];
	        printf( emsg, "Shmem: open(%s)", file.c_str() );
    	    perror( emsg );
            throw 3;
	    }
	    lseek( fd, size, 0 ); 
	    if( write( fd, &pid, sizeof( pid ) ) < 0 )
        {
            perror("write");
        }
	    close( fd );
    }

    int fd;
    if( (fd = open( file.c_str(), O_RDWR )) < 0 )
    {
	    char emsg[128];
	    sprintf( emsg, "Shmem: open(%s)", file.c_str() );
        perror( "OPEN");
        throw 4;
    }

    Shmem *shm = (Shmem *)mmap( 0L, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0 );
    if( shm == (Shmem *)-1 )
    {
        perror( "MMAP");
        throw 5;
    }
    close( fd );

#endif // ]


    shm->start = shm;
    shm->length = size;

    return shm;
}

void Shmem::operator delete( void *data )
{
    Shmem *shm = (Shmem *)data;
#ifdef _WIN32
    UnmapViewOfFile( shm->start );
#else
    munmap( shm->start, shm->length );
#endif
}

#ifdef WIN32
void Shmem::operator delete( void* data, const std::string &file)
{
    Shmem *shm = (Shmem *)data;
    UnmapViewOfFile( shm->start );
    // !WIN32
    //munmap( shm->start, shm->length );
}
#endif
