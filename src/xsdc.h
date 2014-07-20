#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <zlib.h>
#include <mcrypt.h>

#define moreThan4gb	0xd1

typedef struct __attribute__ ((__packed__)) header_t
{
  uint32_t	headerSignature;
  uint32_t	xorSeed;
  uint32_t	headerSize;
  uint32_t	fileNameOffset;
  uint32_t	attributes;
  uint64_t	creationTime;
  uint64_t	accessTime;
  uint64_t	modificationTime;
  uint32_t	compressedSize;
  uint32_t	fileSize;
  uint8_t	isInflated;
  uint8_t	isSth;
  uint8_t	padding[2];
  uint32_t	padding2;
  uint32_t	unknown1;
  uint32_t	unknown2;
  uint32_t	fileNameLength;
  uint8_t	fileName;
} Header;

typedef struct __attribute__ ((__packed__)) header_4gb_t
{
  uint32_t	headerSignature;
  uint32_t	xorSeed;
  uint32_t	headerSize;
  uint32_t	fileNameOffset;
  uint32_t	attributes;
  uint64_t	creationTime;
  uint64_t	accessTime;
  uint64_t	modificationTime;
  uint64_t	compressedSize;
  uint32_t	fileSize;
  uint8_t	isInflated;
  uint8_t	isSth;
  uint8_t	padding[2];
  uint32_t	unknown1;
  uint32_t	unknown2;
  uint32_t	fileNameLength;
  uint8_t	fileName;
} Header4gb;

typedef union
{
  Header	header;
  Header4gb	header4gb;
} HeaderUnion;

typedef struct unpackdata_t
{
  uint32_t checksum;
  uint32_t xorVal;
  void *headerKey;
  void *fileNameKey;
  void *unformatted;
} UnpackData;

typedef enum
{
  FUS_OK = 0,	//success
  FUS_NFND,	//substring '^^' not found in edv
  FUS_LNG,	//length of a string doesn't match expected length
  FUS_NAN,	//one of numbers wasn't number at all
  FUS_ERROR	//unidentified error
} UnpackStatus;

typedef enum
{
  DD_OK = 0,
  DD_AO,	//module opening error
  DD_IE,	//encryption init error
  DD_DE,	//decryption error
  DD_DIE,	//encryption deinit error
  DD_ERR	//uidentified error
} DecrError;

typedef enum 
{
  PH_LONG = 0,
  PH_SHORT
} Shortness;

/*
 * prints help message to stdout, NAME is program name, SHORT determines if we want to print long or short usage variant
 */
void print_help(Shortness Short,char *name);

/*
 * just prints version info
 */
void print_version();

/*
 * xor every byte of a BUFFER by FACTOR
 */
void xorBuffer(uint8_t factor, unsigned char* buffer, uint32_t bufferSize);

/*
 * transforms edv in format "[int]^^[key2][key1][int]" into UnpackData struct,
 * returns FUS_OK on success and appropriate error on fail
 */
UnpackStatus fillUnpackStruct(UnpackData* unpackData, void* edv);

/*
 * decrypts data from BUFFER of BUFFERSIZE size in bytes using KEY of length of KEYLENGTH,
 * returns buffer with decrypted data and sets BUFFERSIZE according to its size
 */
DecrError decryptData(void* buffer, uint32_t* bufferSize, void* outputBuffer, void* key, uint32_t keyLength);

/*
 * get number of bytes that need to be allocated for decryptData's output buffer
 */
uint32_t getDataOutputSize(uint32_t inputSize);

/*
 * count and return crc of sdc file's data area
 */
ulong countCrc(FILE *f, uint32_t hdrSize);

/*
 * load sdc file header from current position in F into HDR buffer
 */
DecrError loadHeader(FILE* f, Header* hdr, uint32_t hdrSize, UnpackData* ud);

/*
 * converts MS-DOS path into UNIX path
 */
void dosPathToUnix(char *path);

/*
 * converts windows file date format to unix timestamp
 * windows date is number of 100-nanosecond ticks since 1st January 1601 (unsigned int)
 * unix is number of seconds since 1st January 1970 (signed int)
 * both dates are 64-bit values
 */
uint64_t winDateToUnix(uint64_t win);