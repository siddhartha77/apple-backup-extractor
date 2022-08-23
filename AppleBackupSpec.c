/*
    Specification details obtained from:
    https://www.downtowndougbrown.com/2013/06/legacy-apple-backup-file
*/
#pragma segment AppleBackup

#include "AppleBackupSpec.h"

Error ABParseDiskHeader(FSSpecPtr myFSSPtr, ABDiskHeaderPtr abDiskHeaderPtr)
{
    OSErr               err = noErr;
    Error               error;
    short               refNum = 0;
    long                backupRootDirID = 0;
    
    /* Open the Data Fork */
    err = HOpenDF(myFSSPtr->vRefNum, myFSSPtr->parID, myFSSPtr->name, fsRdWrPerm, &refNum);    
    if (err) return ERROR(err);
    
    err = SetFPos(refNum, fsFromStart, 0);
    if (err) return ERROR(err);
    
    error = ABDiskFileRead(refNum, 0, sizeof(abDiskHeaderPtr->version), (Ptr)&abDiskHeaderPtr->version);
    error = ABDiskFileRead(refNum, 0, sizeof(abDiskHeaderPtr->magicNumber), (Ptr)&abDiskHeaderPtr->magicNumber);
    error = ABDiskFileRead(refNum, 0, sizeof(abDiskHeaderPtr->diskNumber), (Ptr)&abDiskHeaderPtr->diskNumber);
    error = ABDiskFileRead(refNum, 0, sizeof(abDiskHeaderPtr->totalDisks), (Ptr)&abDiskHeaderPtr->totalDisks);
    error = ABDiskFileRead(refNum, 0, sizeof(abDiskHeaderPtr->backupTime), (Ptr)&abDiskHeaderPtr->backupTime);
    /* Skip redundant time field */
    error = ABDiskFileRead(refNum, 4, sizeof(abDiskHeaderPtr->HDDName), (Ptr)&abDiskHeaderPtr->HDDName);    
    /* Skip redundant size field */
    error = ABDiskFileRead(refNum, 4, sizeof(abDiskHeaderPtr->sizeOnDisk), (Ptr)&abDiskHeaderPtr->sizeOnDisk);

    if (abDiskHeaderPtr->magicNumber != kABDiskFileMagicNum) return ERROR(kABErrDiskMagicNumberMismatch); 
	
	if (error.err) return error;
	
	err = FSClose(refNum);
    
    return ERROR(err);
}

Error ABParseFileSpec(FSSpecPtr myFSSPtr, ABFileSpecPtr abFileSpecPtr, long *headerOffsetPtr)
{
    OSErr               err = noErr;
    Error               error;
    short               refNum = 0;
    ABFileHeaderPtr     abFileHeaderPtr = &abFileSpecPtr->abFileHeader;
    
    /* Open the Data Fork */
    err = HOpenDF(myFSSPtr->vRefNum, myFSSPtr->parID, myFSSPtr->name, fsRdPerm, &refNum);    
    if (err) return ERROR(err);
    
    err = SetFPos(refNum, fsFromStart, *headerOffsetPtr);
    if (err) return ERROR(err);
    
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->version), (Ptr)&abFileHeaderPtr->version);
    
    if (abFileHeaderPtr->version == kABEndOfBackupBytes) return ERROR(kABErrEndOfBackup);
    
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->magicNumber), (Ptr)&abFileHeaderPtr->magicNumber);
    
    if (abFileHeaderPtr->magicNumber != kABFileMagicNum) return ERROR(kABErrFileMagicNumberMismatch);
    
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->startDisk), (Ptr)&abFileHeaderPtr->startDisk);
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->backupTime), (Ptr)&abFileHeaderPtr->backupTime);
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->headerOffset), (Ptr)&abFileHeaderPtr->headerOffset);
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->filename), (Ptr)&abFileHeaderPtr->filename);
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->part), (Ptr)&abFileHeaderPtr->part);
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->dirFlags), (Ptr)&abFileHeaderPtr->dirFlags);
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->validityFlag), (Ptr)&abFileHeaderPtr->validityFlag);
    
    /* Folder */
    if (abFileHeaderPtr->dirFlags & kABFlagFolder)
    {
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->DInfoRec.frRect), (Ptr)&abFileHeaderPtr->DInfoRec.frRect);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->DInfoRec.frFlags), (Ptr)&abFileHeaderPtr->DInfoRec.frFlags);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->DInfoRec.frLocation), (Ptr)&abFileHeaderPtr->DInfoRec.frLocation);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->DInfoRec.frView), (Ptr)&abFileHeaderPtr->DInfoRec.frView);
        
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->DXInfoRec.frScroll), (Ptr)&abFileHeaderPtr->DXInfoRec.frScroll);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->DXInfoRec.frOpenChain), (Ptr)&abFileHeaderPtr->DXInfoRec.frOpenChain);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->DXInfoRec.frScript), (Ptr)&abFileHeaderPtr->DXInfoRec.frScript);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->DXInfoRec.frXFlags), (Ptr)&abFileHeaderPtr->DXInfoRec.frXFlags);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->DXInfoRec.frComment), (Ptr)&abFileHeaderPtr->DXInfoRec.frComment);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->DXInfoRec.frPutAway), (Ptr)&abFileHeaderPtr->DXInfoRec.frPutAway);
    }
    /* File */
    else
    {
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->FInfoRec.fdType), (Ptr)&abFileHeaderPtr->FInfoRec.fdType);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->FInfoRec.fdCreator), (Ptr)&abFileHeaderPtr->FInfoRec.fdCreator);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->FInfoRec.fdFlags), (Ptr)&abFileHeaderPtr->FInfoRec.fdFlags);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->FInfoRec.fdLocation), (Ptr)&abFileHeaderPtr->FInfoRec.fdLocation);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->FInfoRec.fdFldr), (Ptr)&abFileHeaderPtr->FInfoRec.fdFldr);
        
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->FXInfoRec.fdIconID), (Ptr)&abFileHeaderPtr->FXInfoRec.fdIconID);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->FXInfoRec.fdReserved), (Ptr)&abFileHeaderPtr->FXInfoRec.fdReserved);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->FXInfoRec.fdScript), (Ptr)&abFileHeaderPtr->FXInfoRec.fdScript);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->FXInfoRec.fdXFlags), (Ptr)&abFileHeaderPtr->FXInfoRec.fdXFlags);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->FXInfoRec.fdComment), (Ptr)&abFileHeaderPtr->FXInfoRec.fdComment);
        error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->FXInfoRec.fdPutAway), (Ptr)&abFileHeaderPtr->FXInfoRec.fdPutAway);
    }
    
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->ioFlAttrib), (Ptr)&abFileHeaderPtr->ioFlAttrib);
    /* Skip unused */
    error = ABDiskFileRead(refNum, 1, sizeof(abFileHeaderPtr->created), (Ptr)&abFileHeaderPtr->created);
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->modified), (Ptr)&abFileHeaderPtr->modified);
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->DFLen), (Ptr)&abFileHeaderPtr->DFLen);
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->RFLen), (Ptr)&abFileHeaderPtr->RFLen);
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->DFLenOnDisk), (Ptr)&abFileHeaderPtr->DFLenOnDisk);
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->RFLenOnDisk), (Ptr)&abFileHeaderPtr->RFLenOnDisk);    
    error = ABDiskFileRead(refNum, 0, sizeof(abFileHeaderPtr->pathLen), (Ptr)&abFileHeaderPtr->pathLen);

    if (error.err) return error;
    
    /* Get file path */
    abFileSpecPtr->abFilePath2BPStrH = NewHandle(abFileHeaderPtr->pathLen + sizeof(abFileHeaderPtr->pathLen));
    
    /*
        TODO: Populate res with
        https://dev.os9.ca/techpubs/mac/Memory/Memory-45.html#MARKER-2-404
    */
    if (MemError()) return ERROR(MemError());
    
    HLock(abFileSpecPtr->abFilePath2BPStrH);
    
    /* First two bytes of the file path are its length */
    error = ABDiskFileRead
    (
        refNum,
        -(sizeof(abFileHeaderPtr->pathLen)),    // offset mark back two bytes to capture file path length
        abFileHeaderPtr->pathLen + 
            sizeof(abFileHeaderPtr->pathLen),   // add in the length bytes
        *abFileSpecPtr->abFilePath2BPStrH
    );

    if (error.err) return error;
    
    HUnlock(abFileSpecPtr->abFilePath2BPStrH);
    
    /* DEBUG stuff */    
    //NumToString((*abFileSpecPtr->abFilePathH)[2], buff);
    //NumToString(abFileHeaderPtr->pathLen, buff);
    //ABFilePathHToPStr(abFileSpecPtr->abFilePathH, buff);
    //DateString(abFileHeaderPtr->modified, longDate, buff, NULL);
    //myAppendPStr(buff, "\p ");
    //TimeString(abDiskHeaderPtr->backupTime, false, buff2, NULL);
    //myAppendPStr(buff, buff2);
	//myCopyPStr(abFileSpecPtr->abFilePath, buff);
	//ParamText (buff, NULL, NULL, NULL);
	//Alert(kAlertID, NULL);
	
	/* Update the offset with the file mark and the fork lengths */
	err = GetFPos(refNum, headerOffsetPtr);
	if (err) return ERROR(err);
	
	*headerOffsetPtr += abFileHeaderPtr->DFLenOnDisk + abFileHeaderPtr->RFLenOnDisk;	
	abFileSpecPtr->abTotalLen = abFileHeaderPtr->DFLen + abFileHeaderPtr->RFLen;	
	abFileSpecPtr->abCurrentLen = 0;
	err = FSClose(refNum);
	    
    return ERROR(err);
}

Error ABCreateFile(FSSpecPtr myFSSPtr, ABFileSpecPtr abFileSpecPtr, long parentDirID)
{
    Error               error;
    OSErr               err = noErr;
    CInfoPBRec          cipbOutputFile;
    short               backupFileRefNum;
    short               targetFileRefNum;
    long                dataForkOffset;
    long                resForkOffset;
    long                bytesCopied;
    long                bytesToCopy;
    long                outputFileSize = 0;
    Str255              partName;
    Handle              bufferH;
    
    /* file.part name */
    myCopyPStr(abFileSpecPtr->abFileHeader.filename, partName);
    myAppendPStr(partName, kABPartExtension);
    mySafeFilename(partName);
    
    err = HCreate
    (
        myFSSPtr->vRefNum,
        parentDirID,
        partName,
        abFileSpecPtr->abFileHeader.FInfoRec.fdCreator,
        abFileSpecPtr->abFileHeader.FInfoRec.fdType
    );
    
    /* Ignore duplicate filenames as we have multipart files */
    if (err && err != dupFNErr) return ERROR(err);
    
    /* TODO: If multipart file then update abFileSpecPtr->abCurrentLen */
    
    bufferH = NewHandle(kABCopyBuffer);    
    HLock(bufferH);
    
    /* Source data (the backup file's data fork) */
    err = HOpenDF(myFSSPtr->vRefNum, myFSSPtr->parID, myFSSPtr->name, fsRdPerm, &backupFileRefNum);
    if (err) return ERROR(err);

    /* ===================== DATA FORK ===================== */
    
    if (abFileSpecPtr->abFileHeader.DFLen > 0) // && abFileSpecPtr->abFileHeader.DFLen == abFileSpecPtr->abFileHeader.DFLenOnDisk)
    {
        /* The data fork is immediately after the header and the full file path */
        dataForkOffset = 
            abFileSpecPtr->abFileHeader.headerOffset + 
            kABFileHeaderLen + 
            abFileSpecPtr->abFileHeader.pathLen;

        err = SetFPos(backupFileRefNum, fsFromStart, dataForkOffset);
        if (err) return ERROR(err);

        /* Destination data */
        err = HOpenDF(myFSSPtr->vRefNum, parentDirID, partName, fsRdWrPerm, &targetFileRefNum);
        if (err) return ERROR(err);
        
        /* Target file needs to append data as files might be split across disk files */
        err = SetFPos(targetFileRefNum, fsFromLEOF, 0);
        if (err) return ERROR(err);

        /* The copy loop */
        bytesCopied = 0;
        
        while (bytesCopied < abFileSpecPtr->abFileHeader.DFLenOnDisk)
        {
            bytesToCopy = kABCopyBuffer;

            if ((abFileSpecPtr->abFileHeader.DFLenOnDisk - bytesCopied) < bytesToCopy)
            {
                bytesToCopy = abFileSpecPtr->abFileHeader.DFLenOnDisk - bytesCopied;
            } 

            error = ABDiskFileRead(backupFileRefNum, 0, bytesToCopy, *bufferH);
            if (error.err) return error;

            err = FSWrite(targetFileRefNum, &bytesToCopy, *bufferH);
            if (err) return ERROR(err);

            bytesCopied += bytesToCopy;
        }
        
        err = FSClose(targetFileRefNum);
        if (err) return ERROR(err);
        
        abFileSpecPtr->abCurrentLen += bytesCopied;
    }
    
    /* ===================== RESOURCE FORK ===================== */
    
    if (abFileSpecPtr->abFileHeader.RFLen > 0) // && abFileSpecPtr->abFileHeader.RFLen == abFileSpecPtr->abFileHeader.RFLenOnDisk)
    {
        /* The resource fork is immediately after the data fork */
        resForkOffset = 
            abFileSpecPtr->abFileHeader.headerOffset + 
            kABFileHeaderLen + 
            abFileSpecPtr->abFileHeader.pathLen + 
            abFileSpecPtr->abFileHeader.DFLenOnDisk;

        err = SetFPos(backupFileRefNum, fsFromStart, resForkOffset);
        if (err) return ERROR(err);

        /* Destination data */
        err = HOpenRF(myFSSPtr->vRefNum, parentDirID, partName, fsRdWrPerm, &targetFileRefNum);
        if (err) return ERROR(err);

        err = SetFPos(targetFileRefNum, fsFromLEOF, 0);
        if (err) return ERROR(err);

        /* The copy loop */
        bytesCopied = 0;
        
        while (bytesCopied < abFileSpecPtr->abFileHeader.RFLenOnDisk)
        {
            bytesToCopy = kABCopyBuffer;

            if ((abFileSpecPtr->abFileHeader.RFLenOnDisk - bytesCopied) < bytesToCopy)
            {
                bytesToCopy = abFileSpecPtr->abFileHeader.RFLenOnDisk - bytesCopied;
            } 

            error = ABDiskFileRead(backupFileRefNum, 0, bytesToCopy, *bufferH);
            if (error.err) return error;

            err = FSWrite(targetFileRefNum, &bytesToCopy, *bufferH);
            if (err) return ERROR(err);

            bytesCopied += bytesToCopy;
        }
        
        err = FSClose(targetFileRefNum);
        if (err) return ERROR(err);
        
        abFileSpecPtr->abCurrentLen += bytesCopied;
        
        /*
        myCopyPStr(abFileSpecPtr->abFileHeader.filename, tmpName);
        myAppendPStr(tmpName, "\p.part");
        mySafeFilename(tmpName);
        err = HRename(myFSSPtr->vRefNum, parentDirID, abFileSpecPtr->abFileHeader.filename, tmpName);
        if (err) return ERROR(err);
        */
    }
    
    cipbOutputFile.hFileInfo.ioCompletion   = NULL;
    cipbOutputFile.hFileInfo.ioNamePtr      = partName;
    cipbOutputFile.hFileInfo.ioVRefNum      = myFSSPtr->vRefNum;
    cipbOutputFile.hFileInfo.ioFDirIndex    = 0;            // search by ioNamePtr
    cipbOutputFile.hFileInfo.ioDirID        = parentDirID;
    
    err = PBGetCatInfoSync((CInfoPBPtr)&cipbOutputFile);
    if (err) return ERROR(cipbOutputFile.hFileInfo.ioResult);

    cipbOutputFile.hFileInfo.ioDirID        = parentDirID;  // reset since set to file index by PBGetCatInfo    
    cipbOutputFile.hFileInfo.ioNamePtr      = partName;     // reset
    cipbOutputFile.hFileInfo.ioFlFndrInfo   = abFileSpecPtr->abFileHeader.FInfoRec;
    cipbOutputFile.hFileInfo.ioFlCrDat      = abFileSpecPtr->abFileHeader.created;
    cipbOutputFile.hFileInfo.ioFlMdDat      = abFileSpecPtr->abFileHeader.modified;
    cipbOutputFile.hFileInfo.ioFlBkDat      = abFileSpecPtr->abFileHeader.backupTime;
    cipbOutputFile.hFileInfo.ioFlXFndrInfo  = abFileSpecPtr->abFileHeader.FXInfoRec;
    
    outputFileSize = cipbOutputFile.hFileInfo.ioFlLgLen + cipbOutputFile.hFileInfo.ioFlRLgLen;

    /* Set the folder's Finder information */
    err = PBSetCatInfoSync(&cipbOutputFile);    	
    if (err) return ERROR(cipbOutputFile.hFileInfo.ioResult);

    err = FSClose(backupFileRefNum);
    if (err) return ERROR(err);

    HUnlock(bufferH);
    DisposeHandle(bufferH);
    
    /*
        Compare to outputFileSize since that would reflect a multipart file
        whereas abFileSpecPtr->abCurrentLen would only reflect the current file header size
    */
    if (abFileSpecPtr->abCurrentLen == abFileSpecPtr->abTotalLen || outputFileSize == abFileSpecPtr->abTotalLen)
    {
        err = HRename(myFSSPtr->vRefNum, parentDirID, partName, abFileSpecPtr->abFileHeader.filename);
        if (err) return error;
    }
    
    return ERROR(err);
}

Error ABUpdateDirDates(int vRefNum, ABFileSpecPtr abFileSpecPtr, long dirID)
{   
    OSErr               err = noErr;
    CInfoPBRec          cipbOutputDir;
    
    cipbOutputDir.dirInfo.ioCompletion   = NULL;
    cipbOutputDir.dirInfo.ioNamePtr      = NULL;                // unneeded since ioFDirIndex is negative
    cipbOutputDir.dirInfo.ioVRefNum      = vRefNum;
    cipbOutputDir.dirInfo.ioFDirIndex    = -1;                  // search by ioDrDirID
    cipbOutputDir.dirInfo.ioDrDirID      = dirID;
    
    err = PBGetCatInfoSync((CInfoPBPtr)&cipbOutputDir);
    if (err) return ERROR(cipbOutputDir.dirInfo.ioResult);

    cipbOutputDir.dirInfo.ioDrUsrWds     = abFileSpecPtr->abFileHeader.DInfoRec;
    cipbOutputDir.dirInfo.ioDrCrDat      = abFileSpecPtr->abFileHeader.created;
    cipbOutputDir.dirInfo.ioDrMdDat      = abFileSpecPtr->abFileHeader.modified;
    cipbOutputDir.dirInfo.ioDrBkDat      = abFileSpecPtr->abFileHeader.backupTime;
    cipbOutputDir.dirInfo.ioDrFndrInfo   = abFileSpecPtr->abFileHeader.DXInfoRec;

    /* Set the folder's Finder information */
    err = PBSetCatInfoSync(&cipbOutputDir);    	
    if (err) return ERROR(cipbOutputDir.dirInfo.ioResult);
    
    return ERROR(err);
}

/* If the path is for a file, pathLen should subtract the filename length plus one for the ':' */
Error ABCreateDirPath(int vRefNum, long parentDirID, Handle abFilePathH, long pathLen, long *createdDirIDPtr)
{
    OSErr   err = noErr;
    Str255  buffer;
    long    i;
    int     j;
    CInfoPBRec  folderInfo;
    
    /* Start at 2 because length is two bytes */
    for (i = 2, j = 1 ; i <= pathLen + 1 ; ++i, j = 1)
    {
        while (i <= pathLen + 1 && (*abFilePathH)[i] != ':')
        {
            buffer[j++] = (*abFilePathH)[i++];
        }
        
        buffer[0] = j - 1;
        
        err = DirCreate(vRefNum, parentDirID, buffer, createdDirIDPtr);
        
        if (err == dupFNErr) // ignore duplicate folders
        {
            folderInfo.dirInfo.ioCompletion = NULL;
            folderInfo.dirInfo.ioNamePtr = buffer;
            folderInfo.dirInfo.ioVRefNum = vRefNum;
            folderInfo.dirInfo.ioFDirIndex = 0;
            folderInfo.dirInfo.ioDrDirID = parentDirID;
            
            err = PBGetCatInfoSync(&folderInfo);            
            if (err) return ERROR(err);            
            
            parentDirID = folderInfo.dirInfo.ioDrDirID;
            *createdDirIDPtr = folderInfo.dirInfo.ioDrDirID;
            
            continue;
        }
        
        parentDirID = *createdDirIDPtr;
        
        if (err) return ERROR(err);
    }
    
    return ERROR(err);
}

Error ABDiskFileRead(short refNum, long offset, long count, Ptr fieldPtr)
{
    OSErr   err = noErr;
    long    bytesRead = count;
    
    if (offset != 0) err = SetFPos(refNum, fsFromMark, offset);
    if (err) return ERROR(err);
    err = FSRead(refNum, &bytesRead, fieldPtr);
    if (err) return ERROR(err);
    if (bytesRead != count) err = kABErrReadCountMismatch;
    
    return ERROR(err);
}

Error ABGetDiskFiles(FSSpecPtr myFSSPtr, ABDiskHeaderPtr abDiskHeaderPtr, FSSpecArrayHandle myFSSpecArrayH, long *diskFileCount)
{
    OSErr               err = noErr;
    int                 i;
    long                diskIndex;
    HParamBlockRec      hpbrSearch;
    CInfoPBRec          cipbrSearchInfo1;
    CInfoPBRec          cipbrSearchInfo2;
    Error               error;
    ABDiskHeader        abResultDiskHeader;
    FSSpecArrayHandle   myFSSpecMatchesH;
    
    myFSSpecMatchesH = (FSSpecArrayHandle)NewHandle(sizeof(FSSpec) * kABMaxDiskFileMatches);
    
    if (MemError()) return ERROR(MemError());
    
    HLock((Handle)myFSSpecMatchesH);
    
    hpbrSearch.csParam.ioCompletion = NULL;
	hpbrSearch.csParam.ioNamePtr = NULL;
	hpbrSearch.csParam.ioMatchPtr = *myFSSpecMatchesH;
	hpbrSearch.csParam.ioVRefNum = myFSSPtr->vRefNum;
	hpbrSearch.csParam.ioReqMatchCount = kABMaxDiskFileMatches;
	hpbrSearch.csParam.ioSearchBits = fsSBFlAttrib + fsSBFlParID;
    hpbrSearch.csParam.ioSearchTime = 0;
    hpbrSearch.csParam.ioCatPosition.initialize = 0;
    hpbrSearch.csParam.ioOptBuffer = NULL;
    hpbrSearch.csParam.ioOptBufSize = NULL;
    
    cipbrSearchInfo1.hFileInfo.ioFlParID = myFSSPtr->parID;
	cipbrSearchInfo1.hFileInfo.ioFlAttrib = 0x0;
	cipbrSearchInfo2.hFileInfo.ioFlParID = myFSSPtr->parID;
	cipbrSearchInfo2.hFileInfo.ioFlAttrib = 0x10; // ignore folders
	
	hpbrSearch.csParam.ioSearchInfo1 = &cipbrSearchInfo1;
	hpbrSearch.csParam.ioSearchInfo2 = &cipbrSearchInfo2;
	
	err = PBCatSearchSync(&hpbrSearch.csParam);
	
	if (err && err != eofErr) return ERROR(err);
	
	/* Reset potential eofErr */
	err = noErr;
	
	/* Start out assuming everything matches our disk file */
	*diskFileCount = hpbrSearch.csParam.ioActMatchCount;
	
	for (i = 0 ; i < hpbrSearch.csParam.ioActMatchCount ; ++i)
	{
	    error = ABParseDiskHeader(&(*myFSSpecMatchesH)[i], &abResultDiskHeader);	    
	    diskIndex = abResultDiskHeader.diskNumber - 1;
	    
	    /* If there are any errors then move on to the next file*/
	    if (error.err)
	    {
	        error.err = noErr;
	        (*diskFileCount)--;
	        continue;
	    }
	    
	    /* Do validity checks against our parent disk file */
	    if (abDiskHeaderPtr->backupTime != abResultDiskHeader.backupTime)
	    {
	        (*diskFileCount)--;
	        continue;
	    }
	    
	    if (abDiskHeaderPtr->version != abResultDiskHeader.version)
	    {
	        (*diskFileCount)--;
	        continue;
	    }
	    
	    if (abDiskHeaderPtr->totalDisks != abResultDiskHeader.totalDisks)
	    {
	        (*diskFileCount)--;
	        continue;
	    }
	    
	    if (diskIndex < 0 || diskIndex >= abResultDiskHeader.totalDisks)
	    {
	        (*diskFileCount)--;
	        continue;
	    }
	    
	    if (EqualString(abDiskHeaderPtr->HDDName, abResultDiskHeader.HDDName, true, true) == false)
	    {
	        (*diskFileCount)--;
	        continue;
	    }
	    
	    /* If we made it this far then we have a valid backup file */
	    InsertIntoFSSpecList(&(*myFSSpecMatchesH)[i], diskIndex, myFSSpecArrayH);
	}
	
	HUnlock((Handle)myFSSpecMatchesH);	
	DisposeHandle((Handle)myFSSpecMatchesH);
    
    return ERROR(err);
}

void ABFilePathHToPStr(Handle abFilePathH, StringPtr s)
{
    int i;
    
    /* Second byte is the length (<= 0xff) */
    s[0] = (*abFilePathH)[1];
    
    for (i = 1; i <= s[0] ; ++i)
    {
        s[i] = (*abFilePathH)[i + 1];
    }
}
