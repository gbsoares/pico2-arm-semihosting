#pragma once

/* File operations */
#define SYS_OPEN   0x01 /*!< Open a file or stream on the host system. */
#define SYS_ISTTY  0x09 /*!< Check whether a file handle is associated with a file or a stream/terminal such as stdout. */
#define SYS_WRITE  0x05 /*!< Write to a file or stream. */
#define SYS_READ   0x06 /*!< Read from a file at the current cursor position. */
#define SYS_CLOSE  0x02 /*!< Closes a file on the host which has been opened by SYS_OPEN. */
#define SYS_FLEN   0x0C /*!< Get the length of a file. */
#define SYS_SEEK   0x0A /*!< Set the file cursor to a given position in a file. */
#define SYS_TMPNAM 0x0D /*!< Get a temporary absolute file path to create a temporary file. */
#define SYS_REMOVE 0x0E /*!< Remove a file on the host system. Possibly insecure! */
#define SYS_RENAME 0x0F /*!< Rename a file on the host system. Possibly insecure! */

/* Terminal I/O operations */
#define SYS_WRITEC 0x03 /*!< Write one character to the debug terminal. */
#define SYS_WRITE0 0x04 /*!< Write a 0-terminated string to the debug terminal. */
#define SYS_READC  0x07 /*!< Read one character from the debug terminal. */

/* Time operations */
#define SYS_CLOCK    0x10
#define SYS_ELAPSED  0x30
#define SYS_TICKFREQ 0x31
#define SYS_TIME     0x11

/* System/Misc. operations */
#define SYS_ERRNO       0x13 /*!< Returns the value of the C library errno variable that is associated with the semihosting implementation. */
#define SYS_GET_CMDLINE 0x15 /*!< Get commandline parameters for the application to run with (argc and argv for main()) */
#define SYS_HEAPINFO    0x16
#define SYS_ISERROR     0x08
#define SYS_SYSTEM      0x12
