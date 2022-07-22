#include <StandardFile.h>

#include "DSGlobals.h"
#include "DSUserProcs.h"

#include "AppleBackupSpec.h"

// Static Prototypes
static Error ProcessItem(FSSpecPtr myFSSPtr);
static OSErr ProcessFolder(FSSpecPtr myFSSPtr);

/*
	This routine is called during init time.
	
	It allows you to install more AEVT Handlers beyond the standard four
*/
#pragma segment Main
pascal void InstallOtherEvents (void)
{
}

/*	
	This routine is called when an OAPP event is received.
	
	Currently, all it does is set the gOApped flag, so you know that
	you were called initally with no docs, and therefore you shouldn't 
	quit when done processing any following odocs.
*/
#pragma segment Main
pascal void OpenApp (void)
{    
	gOApped = true;
	
	SelectFile();
}

/*	
	This routine is called when an QUIT event is received.
	
	We simply set the global done flag so that the main event loop can
	gracefully exit.  We DO NOT call ExitToShell for two reasons:
	1) It is a pretty ugly thing to do, but more importantly
	2) The Apple event manager will get REAL upset!
*/
#pragma segment Main
pascal void QuitApp (void)
{
	gDone = true;	/*	All Done! */
}

/*	
	This routine is the first one called when an ODOC or PDOC event is received.
	
	In this routine you would place code used to setup structures, etc. 
	which would be used in a 'for all docs' situation (like "Archive all
	dropped files")

	Obviously, the opening boolean tells you whether you should be opening
	or printing these files based on the type of event recieved.
	
	NEW IN 2.0!
	The itemCount parameter is simply the number of items that were dropped on
	the application and that you will be processing.  This gives you the ability
	to do a single preflight for memory allocation needs, rather than doing it
	once for each item as in previous versions.
	
	userDataHandle is a handle that you can create & use to store your own
	data structs.  This dataHandle will be passed around to the other 
	odoc/pdoc routines so that you can get at your data without using
	globals - just like the new StandardFile.  
	
	We also return a boolean to tell the caller if you support this type
	of event.  By default, our dropboxes don't support the pdoc, so when
	opening is FALSE, we return FALSE to let the caller send back the
	proper error code to the AEManager.

	You will probably want to remove the #pragma unused (currently there to fool the compiler!)
*/
#pragma segment Main
pascal Boolean PreFlightDocs (Boolean opening, short itemCount, Handle *userDataHandle)
{
    #pragma unused (itemCount)
    #pragma unused (userDataHandle)

	return opening;		// we support opening, but not printing - see above
}


/*	
	This routine is called for each file passed in the ODOC event.
	
	In this routine you would place code for processing each file/folder/disk that
	was dropped on top of you.
	
	You will probably want to remove the #pragma unused (currently there to fool the compiler!)
*/
#pragma segment Main
pascal void OpenDoc (FSSpecPtr myFSSPtr, Boolean opening, Handle userDataHandle)
{
#pragma unused (opening)
#pragma unused (userDataHandle)
	Error	error;

	error = ProcessItem(myFSSPtr);
	
	if (error.err) {
	    ErrorAlert(kErrStringListID, error.err, error);
	}
	
	/*switch (err) {
	    case noErr:
	        break;
	    case dupFNErr:
	        ErrorAlert(kErrStringListID, kOutputExistsErr, err);
	        break;
	    default:
	        ErrorAlert(kErrStringListID, kOutputErr, err);
	        break;
	}*/
}

/*	
	This routine is the last routine called as part of an ODOC event.
	
	In this routine you would place code to process any structures, etc. 
	that you setup in the PreflightDocs routine.

	NEW IN 2.0!
	The itemCount parameter was the number of items that you processed.
	It is passed here just in case you need it ;)  
	
	If you created a userDataHandle in the PreFlightDocs routines, this is
	the place to dispose of it since the Shell will NOT do it for you!
	
	You will probably want to remove the #pragma unusued (currently there to fool the compiler!)
*/
#pragma segment Main
pascal void PostFlightDocs (Boolean opening, short itemCount, Handle userDataHandle)
{
    #pragma unused (opening)
    #pragma unused (itemCount)
    #pragma unused (userDataHandle)

	if ((opening) && (!gOApped))
		gDone = true;	//	close everything up!

	/*
		The reason we do not auto quit is based on a recommendation in the
		Apple event Registry which specifically states that you should NOT
		quit on a 'pdoc' as the Finder will send you a 'quit' when it is 
		ready for you to do so.
	*/
}

/*
	This routine gets called for each item (which could be either a file or a folder)
	that the caller wants dropped.
*/
static Error ProcessItem(FSSpecPtr myFSSPtr)
{
	Error	            error;
	OSErr               err;
	int                 alertResult;
	int                 i;
    long                backupRootDirID = 0;	
	long                createdDirID;
	long                headerOffset = kABFirstFileHeaderOffset;
	long                diskFileCount = 0;
	ABDiskHeader        abDiskHeader;
	ABFileSpec          abFileSpec;
	Handle              dateFormatResH;
    FSSpecArrayHandle   myFSSpecArrayH;
    
	Str255              dateString;
	Str255              paramBuff2;
	Str255              paramBuff3;
	
	if (MemError()) return ERROR(MemError());
	
	SetCursor(*GetCursor(watchCursor));
	
	/* Get disk header */
	error = ABParseDiskHeader(myFSSPtr, &abDiskHeader);
	if (error.err) return error;
	
	/* Get disk file array */
	myFSSpecArrayH = NewFSSpecList(abDiskHeader.totalDisks * sizeof(FSSpec));	
	HLock((Handle)myFSSpecArrayH);
	
    /* Default the FSSpec to be invalid */
    for (i = 0 ; i < abDiskHeader.totalDisks ; ++i) (*myFSSpecArrayH)[i].vRefNum = kABInvalidVRefNum;
	    
	error = ABGetDiskFiles(myFSSPtr, &abDiskHeader, myFSSpecArrayH, &diskFileCount);
	HUnlock((Handle)myFSSpecArrayH);
	if (error.err) return error;
	
	/* Backup date to formatted string (resource it10 contains format) */
	UseResFile(CurResFile());
    dateFormatResH = Get1Resource('itl0', kResIDitl10);    
    DateString(abDiskHeader.backupTime, shortDate, dateString, dateFormatResH);
    ReleaseResource(dateFormatResH);
    NumToString(diskFileCount, paramBuff2);
	NumToString(abDiskHeader.totalDisks, paramBuff3);
	
	/* Confirmation dialog */
	SetCursor(&qd.arrow);
	CenterAlert(kAlertContinue);
	ParamText
	(
	    myFSSPtr->name,
	    dateString,
	    paramBuff2,
	    paramBuff3	    
	);
	alertResult = NoteAlert(kAlertContinue, NULL);
	
	/* Cancel extraction */
	if (alertResult == kAlertStdAlertCancelButton) return ERROR(noErr);
	
	SetCursor(*GetCursor(watchCursor));
	
	/* Create backup folder */
	myPrefixPStr(dateString, kABBackupDirPrefix);
	err = DirCreate(myFSSPtr->vRefNum, myFSSPtr->parID, dateString, &backupRootDirID); 
	if (err == dupFNErr) err = kABErrBackupFolderExists;
	if (err) return ERROR(err);
	
	HLock((Handle)myFSSpecArrayH);
	
    for (i = 0 ; i < abDiskHeader.totalDisks ; ++i, headerOffset = kABFirstFileHeaderOffset)
    {
        myFSSPtr = &(*myFSSpecArrayH)[i];
            
        if (myFSSPtr->vRefNum == kABInvalidVRefNum)
        {
            /* Missing disk file */
        }
        else
        {   
            while (headerOffset < kABMaxDiskSize - kABFileHeaderLen)
            {
                error = ABParseFileSpec(myFSSPtr, &abFileSpec, &headerOffset);
                
                if (error.err == kABErrEndOfBackup)
                {
                    error.err = kABNoErr;
                    break; // done!
                }
                
                if (error.err) return error;
                
                if (abFileSpec.abFileHeader.dirFlags & kABFlagFolder)
                {
                    /* Create the folder */
                    //error = ABCreateDir(myFSSPtr, &abFileSpec, backupRootDirID, &createdDirID);
                    error = ABCreateDirPath
                    (
                        myFSSPtr->vRefNum,
                        backupRootDirID,
                        abFileSpec.abFilePath2BPStrH,
                        abFileSpec.abFileHeader.pathLen,
                        &createdDirID
                    );
                    
                    if (error.err) return error;
                    
                    error = ABUpdateDirDates(myFSSPtr->vRefNum, &abFileSpec, createdDirID);                    
                    if (error.err) return error;
                }
                else
                {
                    /*
                        Create the path. Done here in case this is an incomplete backup
                        and the folder was never completed.
                    */
                    error = ABCreateDirPath
                    (
                        myFSSPtr->vRefNum,
                        backupRootDirID,
                        abFileSpec.abFilePath2BPStrH,
                        /* Strip filename from path */
                        abFileSpec.abFileHeader.pathLen - abFileSpec.abFileHeader.filename[0],
                        &createdDirID
                    );
                    
                    if (error.err) return error;
                    
                    /* Create the file */
                    error = ABCreateFile(myFSSPtr, &abFileSpec, createdDirID);                    

                    if (error.err) return error;
                }

                DisposeHandle(abFileSpec.abFilePath2BPStrH);

                /* Move the offset to the next byte boundary */
                headerOffset += kABFileHeaderBoundary - (headerOffset % kABFileHeaderBoundary);
            }
        }
    }
    
    HUnlock((Handle)myFSSpecArrayH);
    
	DisposeFSSpecList(myFSSpecArrayH);
	SetCursor(&qd.arrow);
		
	CenterAlert (kAlertComplete);
	NoteAlert(kAlertComplete, NULL);
	
	return error;
}

/*
	This routine is called when the user chooses "Select File…" from the
	File Menu.
	
	Currently it simply calls the new StandardGetFile routine to have the
	user select a single file (any type, numTypes = -1) and then calls the
	SendODOCToSelf routine in order to process it.  
			
	The reason we send an odoc to ourselves is two fold: 1) it keeps the code
	cleaner as all file openings go through the same process, and 2) if events
	are ever recordable, the right things happen (this is called Factoring!)

	Modification of this routine to only select certain types of files, selection
	of multiple files, and/or handling of folder & disk selection is left 
	as an exercise to the reader.
*/
pascal void SelectFile (void)
{
	StandardFileReply	stdReply;
	SFTypeList			theTypeList = {};

	StandardGetFile(NULL, -1, theTypeList, &stdReply);
	if (stdReply.sfGood)                    // user did not cancel
	{
		SendODOCToSelf(&stdReply.sfFile);	// so send me an event!
		QuitApp();
	}
}

/*
	This routine is called during the program's initialization and gives you
	a chance to allocate or initialize any of your own globals that your
	dropbox needs.
	
	You return a boolean value which determines if you were successful.
	Returning false will cause DropShell to exit immediately.
*/
pascal Boolean InitUserGlobals(void)
{
	return true;	// nothing to do, it we must be successful!
}

/*
	This routine is called during the program's cleanup and gives you
	a chance to deallocate any of your own globals that you allocated 
	in the above routine.
*/
pascal void DisposeUserGlobals(void)
{
	// nothing to do for our sample dropbox
}
