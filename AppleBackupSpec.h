#include "DSUtils.h"

#define kAlertID                    200

#define kABDiskFileMagicNum         'CMWL'
#define kABFileMagicNum             'RLDW'
#define kABEndOfBackupBytes         0xf6f6 // last file may be padded with 0xf6
#define kABDiskFileHeaderLen        0x200
#define kABFileHeaderLen            0x70
#define kABFirstFileHeaderOffset    0x600
#define kABFileHeaderBoundary       0x200
#define kABCopyBuffer               0x40000 // 256kb buffer
#define kABMaxDiskFileMatches       512
#define kABInvalidVRefNum           0x7fff
#define kABPartExtension            "\p.part"
#define kABBackupDirPrefix          "\pBackup "

/* Error codes */
enum
{
    kABErrEndOfBackup       = -1,
    kABNoErr,
    kABErrReadCountMismatch,
    kABErrDiskMagicNumberMismatch,
    kABErrFileMagicNumberMismatch,
    kABErrNoFirstDisk,
    kABErrVersionError,
    kABErrSizeMismatch,
    kABErrCorruptFile,
    kABErrBackupFolderExists
};

/* ABFileHeader.folderFlags */
enum
{
    kABFlagFolder           = 0x80,
    kABFlagFile             = 0x0,
    kABFlagSystemFolder     = 0x1
};

/* ABFileHeader.validityFlag */
enum
{
    kABFlagValidAttributes  = 0x1
};

typedef struct
{   
    UInt32  magicNumber;    // must be 'CMWL'
    UInt32  backupTime;     // time of backup in HFS epoch
    UInt32  sizeOnDisk;     // typically 0x161800 except last disk    
    UInt16  version;        // this spec is valid up to and including version 0x0104
    UInt16  diskNumber;     // value is between 1 and the number of disks
    UInt16  totalDisks;     // total number of disks used for the backup
    Str31   HDDName;
} ABDiskHeader;

typedef ABDiskHeader *      ABDiskHeaderPtr;

typedef struct
{
    union
    {
        FInfo   FInfoRec;   // FInfo struct containing info about this file (from HFileInfo)
        DInfo   DInfoRec;   // DInfo struct containing info about this folder (from DirInfo)
    };
    
    union
    {
        FXInfo  FXInfoRec;  // FXInfo struct containing info about this file (from HFileInfo)
        DXInfo  DXInfoRec;  // DXInfo struct containing info about this folder (from DirInfo)
    };
    
    UInt32  magicNumber;    // must be 'RLDW'
    UInt32  backupTime;     // same as in abDiskHeader->backupTime
    UInt32  headerOffset;   // where this header begins in the disk
    UInt32  created;        // HFS epoch
    UInt32  modified;       // HFS epoch
    UInt32  DFLen;          // total length of Data Fork of fully restored file  (0 for folders)
    UInt32  RFLen;          // total length of Res Fork of fully restored file  (0 for folders)
    UInt32  DFLenOnDisk;    // length of Data Fork this disk is providing for this file
    UInt32  RFLenOnDisk;    // length of Res Fork this disk is providing for this file
    UInt16  version;        // same as abDiskHeader->version
    UInt16  startDisk;      // disk number this file starts on
    UInt16  part;           // which part this file is
    UInt16  pathLen;        // max length of 33*50 (enough space for 50 colon-delimited path elements)
    UInt8   dirFlags;       // bit 7 = 1 for folder, 0 for file. 
                            // bit 0 = 1 for System Folder to be blessed
    UInt8   validityFlag;   // bit 0 = 1 if the following file info/attributes/dates are valid.
                            // bit 0 = 0 if this was a folder that is known to exist but its properties 
                            // could not be read during the backup.
                            // if a file's properties cannot be read, the file is skipped during the backup process.
                            // so bit 0 = 0 could only happen with folders.
    UInt8   ioFlAttrib;     // standard ioFlAttrib byte from Mac Toolbox HFileInfo/DirInfo struc
    Str31   filename;
} ABFileHeader;

typedef ABFileHeader *      ABFileHeaderPtr;

typedef struct
{
    UInt32                  abTotalLen;
    UInt32                  abCurrentLen;
    ABFileHeader            abFileHeader;
    Handle                  abFilePath2BPStrH; // first *two* bytes are the string length
} ABFileSpec;

typedef ABFileSpec *        ABFileSpecPtr;

Error ABParseDiskHeader(FSSpecPtr myFSSPtr, ABDiskHeaderPtr abDiskHeaderPtr);
Error ABParseFileSpec(FSSpecPtr myFSSPtr, ABFileSpecPtr abFileSpecPtr, long *headerOffsetPtr);
Error ABDiskFileRead(short refNum, long offset, long count, Ptr fieldPtr);
void ABFilePathHToPStr(Handle abFilePathH, StringPtr s);
Error ABUpdateDirDates(int vRefNum, ABFileSpecPtr abFileSpecPtr, long dirID);
Error ABCreateDirPath(int vRefNum, long parentDirID, Handle abFilePathH, long pathLen, long *createdDirIDPtr);
Error ABCreateFile(FSSpecPtr myFSSPtr, ABFileSpecPtr abFileSpecPtr, long parentDirID);
Error ABGetDiskFiles(FSSpecPtr myFSSPtr, ABDiskHeaderPtr abDeskHeaderPtr, FSSpecArrayHandle myFSSpecArrayH, long *diskFileCount);
