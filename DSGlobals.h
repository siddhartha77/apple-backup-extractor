#ifndef	__DSGLOBALS_H__
#define	__DSGLOBALS_H__


#ifndef __MWERKS__
#include <Types.h>
#include <Memory.h>
#include <QuickDraw.h>
#include <OSUtils.h>
#include <ToolUtils.h>
#include <Menus.h>
#include <Packages.h>
#include <Traps.h>
#include <Files.h>
#endif

#include <Aliases.h>
#include <AppleEvents.h>
#include <Gestalt.h>
#include <Processes.h>

#define kDebugErrorInfo     true

/* Menus */
#define	kMenuApple	        128
#define kMenuAppleAbout     1
#define	kMenuFile   	    129
#define kMenuFileExtract    1

/* Dialogs */
#define kAlertAbout         128
#define kAlertContinue      129
#define kAlertComplete      130
#define kAlertMultipleFiles 131
#define	kAlertError         200

/* Custom errors */
#define kErrStringListID    300
#define kUndefinedErrorID   0

/* AE errors */
#define	kErrStringID	    100
#define	kCantRunErr		    1
#define	kAEVTErr		    2

/* Other Resource IDs */
#define kResIDitl10         128
		
extern Boolean		gDone, gOApped, gHasAppleEvents, gWasEvent;
extern EventRecord	gEvent;
extern MenuHandle	gAppleMenu, gFileMenu;
extern WindowPtr	gSplashScreen;

#endif
