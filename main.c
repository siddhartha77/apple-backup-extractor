#ifndef __MWERKS__
#include <Desk.h>
#include <Dialogs.h>
#include <Errors.h>
#include <Files.h>
#include <Fonts.h>
#include <Memory.h>
#include <Menus.h>
#include <StandardFile.h>
#include <TextEdit.h>
#include <Types.h>
#include <Windows.h>
#endif

#include "DSGlobals.h"
#include "DSUserProcs.h"
#include "DSAppleEvents.h"

#include "main.h"

Boolean		gDone, gOApped, gHasAppleEvents, gWasEvent;
EventRecord	gEvent;
MenuHandle	gAppleMenu, gFileMenu;
WindowPtr	gSplashScreen;

#ifdef MPW
extern void _DataInit();	
#endif

#pragma segment Initialize
void InitToolbox (void) 
{

#ifdef MPW
	UnloadSeg ((Ptr) _DataInit);
#endif

	InitGraf (&qd.thePort);
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (NULL);		// use of ResumeProcs no longer approved by Apple
	InitCursor ();
	FlushEvents (everyEvent, 0);
	
	// how about some memory fun!
	MoreMasters ();
	MoreMasters ();
}

/*
	Let's setup those global variables that the DropShell uses.
	
	If you add any globals for your own use,
	init them in the InitUserGlobals routine in DSUserProcs.c
*/
#pragma segment Initialize
Boolean InitGlobals (void) 
{
	long aLong;

	gDone			= false;
	gOApped			= false;	// probably not since users are supposed to DROP things!
	gHasAppleEvents	= Gestalt (gestaltAppleEventsAttr, &aLong) == noErr;
	gSplashScreen	= NULL;

	return InitUserGlobals();	// call the user proc
}

/*
	Again, nothing fancy.  Just setting up the menus.
	
	If you add any menus to your DropBox - insert them here!
*/
#pragma segment Initialize
void SetUpMenus (void)
{

	gAppleMenu = GetMenu (kMenuApple);
	AppendResMenu (gAppleMenu, 'DRVR');
	InsertMenu (gAppleMenu, 0);

	gFileMenu = GetMenu (kMenuFile);
	InsertMenu (gFileMenu, 0);
	DrawMenuBar ();
}

/*	--------------- Standard Event Handling routines ---------------------- */
#pragma segment Main
void ShowAbout () 
{
    CenterAlert (kAlertAbout);
	(void) Alert (kAlertAbout, NULL);
}

#pragma segment Main
void DoMenu (long retVal)
{
	short	menuID, itemID;
	Str255	itemStr;

	menuID = HiWord (retVal);
	itemID = LoWord (retVal);
	
	switch (menuID)
	{
		case kMenuApple:
			if (itemID == kMenuAppleAbout)
				ShowAbout ();	/*	Show the about box */
			else
			{
				GetMenuItemText(GetMenuHandle(kMenuApple), itemID, itemStr);
				OpenDeskAcc(itemStr);
			}
			break;
			
		case kMenuFile:
			if (itemID == kMenuFileExtract)
				SelectFile();		// call file selection userProc
			else
				SendQuitToSelf();	// send self a 'quit' event
			break;
		
		default:
			break;
			
	}
    HiliteMenu(0);		// turn it off!
}

#pragma segment Main
void DoMouseDown (EventRecord *curEvent)
{
	WindowPtr	whichWindow;
	short		whichPart;

	whichPart = FindWindow (curEvent->where, &whichWindow);
	switch (whichPart)
	{
		case inMenuBar:
			DoMenu (MenuSelect (curEvent->where));
			break;
		
		case inSysWindow:
			SystemClick (curEvent, whichWindow);
			break;
		
		case inDrag:
    		{
    			Rect	boundsRect = (*GetGrayRgn())->rgnBBox;
    			DragWindow (whichWindow, curEvent->where, &boundsRect);
			}
			break;
			
		default:
			break;
	}
}

#pragma segment Main
void DoKeyDown (EventRecord *curEvent)
{
	if (curEvent->modifiers & cmdKey)
		DoMenu (MenuKey ((char) curEvent->message & charCodeMask));
}

#pragma segment Main
void main () 
{
	InitToolbox ();
	if (InitGlobals ())
	{
		if (!gHasAppleEvents)
			ErrorAlert (kErrStringID, kCantRunErr, ERROR(kCantRunErr));
		else
		{
			InitAEVTStuff ();
			SetUpMenus ();
			
			while (!gDone)
			{
				gWasEvent = WaitNextEvent (everyEvent, &gEvent, 0, NULL);
				
				if (gWasEvent)
				{
					switch (gEvent.what)
					{
						case kHighLevelEvent:
							DoHighLevelEvent (&gEvent);
							break;
							
						case mouseDown:
							DoMouseDown (&gEvent);
							break;
							
						case keyDown:
						case autoKey:
							DoKeyDown (&gEvent);
							break;

						case diskEvt:
							if (HiWord(gEvent.message))
							{
								Point diskInitPt;
								
								diskInitPt.v = diskInitPt.h = 100;
								DILoad();
								DIBadMount(diskInitPt, gEvent.message);
								DIUnload();
							}
							break;
							
						default:
							break;
					}
				}
			}
		}
		DisposeUserGlobals();	// call the userproc to clean itself up
	}
	ExitToShell();
}
