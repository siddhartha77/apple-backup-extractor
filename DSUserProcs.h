#ifndef __DSUSERPROCS_H__
#define __DSUSERPROCS_H__

#include "DSGlobals.h"
#include "DSUtils.h"
	
pascal void     InstallOtherEvents (void);

pascal void 	OpenApp (void);
pascal void 	QuitApp (void);
pascal Boolean	PreFlightDocs (Boolean opening, short itemCount, Handle *userDataHandle);
pascal void 	OpenDoc (FSSpecPtr myFSSPtr,  Boolean opening, Handle userDataHandle);
pascal void 	PostFlightDocs (Boolean opening, short itemCount, Handle userDataHandle);
pascal void 	SelectFile (void);

pascal Boolean	InitUserGlobals(void);
pascal void		DisposeUserGlobals(void);

#endif
