/*
//	Floppy.h -- XkyOS Floppy Disks Library
//	Copyright (c) 2008 by Pablo Bravo
//	All Rights Reserved
*/

#ifndef __FLOPPY_INCLUDES__
#define __FLOPPY_INCLUDES__


// Error codes and messages
#define FLOPPY_ABNORMAL           0
#define FLOPPY_INVALIDCOMMAND     1
#define FLOPPY_EQUIPMENTCHECK     2
#define FLOPPY_ENDOFTRACK         3
#define FLOPPY_CRCERROR1          4
#define FLOPPY_DMAERROR           5
#define FLOPPY_INVALIDSECTOR      6
#define FLOPPY_WRITEPROTECT       7
#define FLOPPY_MISSINGADDRESSMARK 8
#define FLOPPY_CONTROLMARK        9
#define FLOPPY_CRCERROR2          10
#define FLOPPY_INVALIDTRACK       11
#define FLOPPY_BADTRACK           12
#define FLOPPY_BADADDRESSMARK     13
#define FLOPPY_TIMEOUT            14
#define FLOPPY_UNKNOWN            15

typedef volatile struct {
  unsigned headLoad;   // Head load timer
  unsigned headUnload; // Head unload timer
  unsigned stepRate;   // Step rate timer
  unsigned dataRate;   // Data rate
  unsigned gapLength;  // Gap length between sectors

} floppyDriveData;

static char *errorMessages[] = {
  "Abnormal termination - command did not complete",
  "Invalid command",
  "Equipment check - seek to invalid track",
  "The requested sector is past the end of the track",
  "ID byte or data - the CRC integrity check failed",
  "DMA transfer overrun or underrun",
  "No data - the requested sector was not found",
  "Write protect",
  "Missing address mark",
  "Sector control mark - data was not the expected type",
  "Data - the CRC integrity check failed",
  "Invalid or unexpected track",
  "Bad track",
  "Bad address mark",
  "Command timed out",
  "Unknown error"
};

static kernelPhysicalDisk *floppies[MAXFLOPPIES];
static int numberFloppies = 0;
static volatile int controllerLock = 0;
static volatile unsigned currentTrack = 0;
static volatile int readStatusOnInterrupt = 0;
static volatile int interruptReceived = 0;
static volatile unsigned char statusRegister0;
static volatile unsigned char statusRegister1;
static volatile unsigned char statusRegister2;
static volatile unsigned char statusRegister3;

// An area for doing floppy disk DMA transfers (physically aligned, etc)
static volatile void *xFerPhysical = NULL;
static volatile void *xFer = NULL;


#endif