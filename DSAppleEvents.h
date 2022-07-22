#ifndef __DSAPPLEEVENTS_H__
#define __DSAPPLEEVENTS_H__

#include <AppleEvents.h>

#include "DSGlobals.h"
#include "DSUtils.h"
#include "DSUserProcs.h"

pascal void		InitAEVTStuff(void);
OSErr			GotRequiredParams(AppleEvent *theAppleEvent);
void			FailErr(OSErr err);

pascal OSErr	_HandleDocs (AppleEvent *theAppleEvent, AppleEvent *reply, Boolean opening);

pascal OSErr	HandleOAPP(AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);
pascal OSErr	HandleQuit(AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);
pascal OSErr	HandleODOC(AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);
pascal OSErr	HandlePDOC(AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);
pascal void		DoHighLevelEvent(EventRecord *event);

#endif
