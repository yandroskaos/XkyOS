/**
 * \file Floppy.cpp
 * \brief
 *		Floppy Control Library
 * \author Pablo Bravo Garcia
*/

#include "Types.h"
#include "IO.h"
#include "Floppy.h"

//==================================DATA======================================//
#pragma code_seg(".data")
//============================================================================//
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

PRIVATE char* errorMessages[] = {
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

PRIVATE kernelPhysicalDisk *floppies[MAXFLOPPIES];
PRIVATE int number_floppies = 0;
PRIVATE volatile int controller_lock = 0;
PRIVATE volatile unsigned current_track = 0;
PRIVATE volatile int read_status_on_interrupt = 0;
PRIVATE volatile int interrupt_received = 0;
PRIVATE volatile unsigned char status_register_0;
PRIVATE volatile unsigned char status_register_1;
PRIVATE volatile unsigned char status_register_2;
PRIVATE volatile unsigned char status_register_3;

// An area for doing floppy disk DMA transfers (physically aligned, etc)
PRIVATE volatile void *xFerPhysical = NULL;
PRIVATE volatile void *xFer = NULL;

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

PRIVATE void CommandWrite(byte command)
{
	// Waits until the floppy controller is ready for a new command (or part
	// thereof) in port 03F5h, and then writes it.
	dword start_time = TMR_Ticks();
	byte data;

	while (TMR_Ticks() < (start_time + 10))
	{
		// Get the drive transfer mode from the port
		//kernelProcessorDelay();
		InPortByte(0x03F4, data);
      
		// Check whether access is permitted
		if ((data & 0xC0) == 0x80)
			break;
	}

	OutPortByte(0x03F5, command);
	//kernelProcessorDelay();
}

PRIVATE byte StatusRead()
{
	// Waits until the floppy controller is ready for a read of port 03F5h,
	// and then reads it.
	dword start_time = TMR_Ticks();
	byte data;

	while(TMR_Ticks() < (start_time + 10))
	{
		// Get the drive transfer mode from the port
		//kernelProcessorDelay();
		InPortByte(0x03F4, data);
      
		// Check whether access is permitted
		if ((data >> 6) == 3)
			break;
	}

	InPortByte(0x03F5, data);
	return data;
}


PRIVATE int WaitOperationComplete()
{
	// This routine just loops, reading the "interrupt received" byte.  When
	// the byte becomes 1, it resets the byte and returns.  If the wait times
	// out, the function returns negative.  Otherwise, it returns 0.

	int status = 0;
	dword start_time = TMR_Ticks();

	while(!interrupt_received)
	{
		// Yield the rest of this timeslice if we are in multitasking mode
		// kernelMultitaskerYield();

		if (TMR_Ticks() > (start_time + 20))
			break;
	}

	if(interrupt_received)
	{
		interrupt_received = 0;
		status = 0;
	}
	else
	{
		// No interrupt -- timed out.
		//kernelError(kernel_error, errorMessages[FLOPPY_TIMEOUT]);
		//status = ERR_IO;
	}
	return status;
}


PRIVATE int EvaluateError()
{
	// This is an internal-only routine that takes no parameters and returns
	// no value.  It evaluates the returned bytes in the statusRegister[X] bytes
	// and matches conditions to error codes and error messages

	int error_code = 0;

	// Check for abnormal termination of command
	if((status_register_0 & 0xC0) == 0x40) 
		error_code = FLOPPY_ABNORMAL;

	// Check for invalid command
	else if ((status_register_0 & 0xC0) == 0x80)
		error_code = FLOPPY_INVALIDCOMMAND;
  
	// Check for Equipment check error
	if (status_register_0 & 0x10)  // bit 4 (status register 0)
		error_code = FLOPPY_EQUIPMENTCHECK;

	// Check for end-of-track
	if (status_register_1 & 0x80)  // bit 7 (status register 1)
		error_code = FLOPPY_ENDOFTRACK;

	// Bit 6 is unused in status register 1

	// Check for the first kind of data error
	if (status_register_1 & 0x20)  // bit 5 (status register 1)
		error_code = FLOPPY_CRCERROR1;

	// Check for overrun/underrun
	if (status_register_1 & 0x10)  // bit 4 (status register 1)
		errorCode = FLOPPY_DMAERROR;

	// Bit 3 is unused in status register 1

	// Check for no data error
	if (status_register_1 & 0x04)  // bit 2 (status register 1)
		error_code = FLOPPY_INVALIDSECTOR;

	// Check for write protect error
	if (status_register_1 & 0x02)  // bit 1 (status register 1)
		error_code = FLOPPY_WRITEPROTECT;

	// Check for missing address mark
	if (status_register_1 & 0x01)  // bit 0 (status register 1)
		error_code = FLOPPY_MISSINGADDRESSMARK;

	// Bit 7 is unused in status register 2

	// Check for control mark error
	if (status_register_2 & 0x40)  // bit 6 (status register 2)
		error_code = FLOPPY_CONTROLMARK;

	// Check for the second kind of data error
	if (status_register_2 & 0x20)  // bit 5 (status register 2)
		error_code = FLOPPY_CRCERROR2;

	// Check for invalid track / wrong cylinder
	if (status_register_2 & 0x10)  // bit 4 (status register 2)
		error_code = FLOPPY_INVALIDTRACK;

	// Bit 3 is unused in status register 2
	// Bit 2 is unused in status register 2

	// Check for bad track
	if (status_register_2 & 0x02)  // bit 1 (status register 2)
		error_code = FLOPPY_BADTRACK;

	// Check for bad address mark
	if (status_register_2 & 0x01)  // bit 0 (status register 2)
		error_code = FLOPPY_BADADDRESSMARK;

	if (!error_code)
		error_code = FLOPPY_UNKNOWN;

	return error_code;
}


PRIVATE void SelectDrive(byte drive_number)
{
	// Takes the floppy number to select as its only parameter.  Selects the
	// drive on the controller

	byte data = 0;

	// Select the drive on the controller

	// Get the current register value
	//kernelProcessorDelay();
	InPortByte(0x03F2, data);

	// Make sure the DMA/Interrupt and reset-off bits are set
	data |= 0x0C;
  
	// Clear out the selection bits
	data &= 0xFC;

	// Set the selection bits
	data |= drive_number;

	// Issue the command
	OutPortByte(0x03F2, data);
	//kernelProcessorDelay();
}


PRIVATE void Specify(byte drive_number)
{
	// Sends some essential timing information to the floppy drive controller
	// about the specified drive.

	byte command_byte;
	floppyDriveData *floppyData = (floppyDriveData *) floppies[drive_number]->driverData;

	// Construct the data rate byte
	command_byte = (byte) ((floppyDriveData *)(floppies[drive_number]->driverData))->dataRate;
	OutPortByte(0x03F7, command_byte);
	//kernelProcessorDelay();

	// Construct the command byte
	command_byte = (byte) 0x03;  // Specify command
	CommandWrite(command_byte);

	// Construct the step rate/head unload byte
	command_byte = (byte)((floppyData->stepRate << 4) | (floppyData->headUnload & 0x0F));
	CommandWrite(command_byte);

	// Construct the head load time byte.  Make sure that DMA mode is enabled.
	command_byte = (byte) ((floppyData->headLoad << 1) & 0xFE);
	CommandWrite(command_byte);
}


PRIVATE int SetMotorState(int drive_number, int on_off)
{
	// Turns the floppy motor on or off
	byte data = 0;
	byte tmp;

	// Select the drive
	select_drive(drive_number);

	// Read the port's current state
	//kernelProcessorDelay();
	InPortByte(0x03F2, data);

	// Move the motor select bit to the correct location [7:4]
	tmp = ((byte) 0x10 << drive_number); 

	// Test whether the motor is on already
	if (on_off)
	{
		if (!(data & tmp))
		{
			// Turn on the 'motor on' bit
			data |= tmp;
	  
			// Issue the command
			OutPortByte(0x03F2, data);
			//kernelProcessorDelay();
		}
	}
	else
	{
		// Move the motor select bit to the correct location [7:4]
		tmp = 0x0F | ((byte) 0xEF << drive_number); 

		// Turn off the 'motor on' bit
		data &= tmp;
	  
		// Issue the command
		OutPortByte(0x03F2, data);
		//kernelProcessorDelay();
	}

	floppies[driveNum]->motorState = onOff;

	return 0;
}

PRIVATE int ReadWriteSectors(dword drive_number, dword logical_sector, dword num_sectors, void* buffer, int read)
{
	// Reads or writes data to/from the disk.  Both types of operation are
	// combined here since the functionality is nearly identical.  Returns 0
	// on success, negative otherwise.

	int status = 0;
	kernelPhysicalDisk *theDisk = NULL;
	int errorCode = 0;
	unsigned head, track, sector;
	unsigned doSectors = 0;
	unsigned xFerBytes = 0;
	unsigned char commandByte, tmp;
	int retry = 0;
	int count;

	// Get a pointer to the requested disk
	theDisk = floppies[driveNum];

	// Wait for a lock on the controller
	status = kernelLockGet((void *) &controllerLock);
	if (status < 0)
		return (status);

	// Select the drive
	SelectDrive(drive_number);
  
	// We will have to make sure the motor is turned on
	if (theDisk->motorState == 0)
	{
		// Turn the drive motor on
		SetMotorState(drive_number, 1);
      
		// We don't have to wait for the disk to spin up on a read operation;
		// It will start reading when it's good and ready.  If it's a write
		// operation we have to wait for it.
		if (!read)
			// Wait half a second for the drive to spin up
			kernelMultitaskerWait(10);
	}

	// We don't want to cross a track boundary in one operation.  Some
	// floppy controllers can't do this.  Thus, if necessary we break up the
	// operation with this loop.

	while (num_sectors > 0)
	{
		// Calculate the physical head, track and sector to use
		head = ((logicalSector % (theDisk->sectorsPerCylinder * theDisk->heads)) / theDisk->sectorsPerCylinder);
		track = (logicalSector / (theDisk->sectorsPerCylinder * theDisk->heads));
		sector = (logicalSector % theDisk->sectorsPerCylinder) + 1;

		// Make sure the head, track, and sector are within the legal range
		// of values
		if ((sector > theDisk->sectorsPerCylinder) || (track >= theDisk->cylinders) || (head >= theDisk->heads))
		{
			kernelLockRelease((void *) &controllerLock);
			return (status = ERR_BADADDRESS);
		}

		// Here's where we check for crossing track boundaries
		doSectors = num_sectors;
		if (((head * theDisk->sectorsPerCylinder) + sector + (doSectors - 1)) > (theDisk->heads * theDisk->sectorsPerCylinder))
			doSectors = ((theDisk->heads * theDisk->sectorsPerCylinder) - ((head * theDisk->sectorsPerCylinder) + (sector - 1)));

		// We need to do a seek for every read/write operation

		// Tell the interrupt-received routine to issue the "sense interrupt
		// status" command after the operation
		readStatusOnInterrupt = 1;
		interruptReceived = 0;

		// Construct the command byte
		commandByte = 0x0F;  // Seek command
		CommandWrite(commandByte);

		// Construct the drive/head select byte
		// Format [00000 (head 1 bit)(drive 2 bits)]
		CommandByte = (unsigned char) (((head & 1) << 2) | (driveNum & 3));
		CommandWrite(commandByte);

		// Construct the track number byte
		commandByte = (unsigned char) track;
		CommandWrite(commandByte);

		// The drive should now be seeking.  While we wait for the seek to
		// complete, we can do some other things.

		// How many bytes will we transfer?
		xFerBytes = (doSectors * floppies[driveNum]->sectorSize);
      
		// If it's a write operation, copy xFerBytes worth of user data
		// into the transfer area
		if (!read)
			kernelMemCopy(buffer, (void *) xFer, xFerBytes);

		// Set up the DMA controller for the transfer.
		if (read)
			// Set the DMA channel for writing TO memory, demand mode
			status = kernelDmaOpenChannel(theDisk->dmaChannel, (void *) xFerPhysical, xFerBytes, DMA_WRITEMODE);
		else
			// Set the DMA channel for reading FROM memory, demand mode
			status = kernelDmaOpenChannel(theDisk->dmaChannel, (void *) xFerPhysical, xFerBytes, DMA_READMODE);
		if (status < 0)
		{
			kernelError(kernel_error, "Unable to open DMA channel");
			kernelLockRelease((void *) &controllerLock);
			return (status);
		}

		// Now wait for the seek to complete, check error conditions in the
		// status byte, and make sure that we are now at the correct track
		status = WaitOperationComplete();
		if ((status < 0) || ((statusRegister0 & 0xF8) != 0x20) || (currentTrack != track))
		{
			kernelDmaCloseChannel(theDisk->dmaChannel);
			kernelLockRelease((void *) &controllerLock);
			kernelError(kernel_error, "Seek error: %s", errorMessages[evaluateError()]);
			return (status = ERR_IO);
		}

		// Now proceed with the read/write operation

		// Tell the interrupt-received routine NOT to issue the "sense interrupt
		// status" command after the read/write operation
		readStatusOnInterrupt = 0;
		interruptReceived = 0;

		// Command byte
		// Drive/head select byte
		// Track number byte
		// Head number byte
		// Sector byte
		// Sector size code
		// End of track byte
		// Gap length byte
		// Custom sector size byte

		if (read)
			commandByte = 0xE6;  // "Read normal data" command
		else
			commandByte = 0xC5;  // "Write data" command
		CommandWrite(commandByte);
      
		// Construct the drive/head select byte
		// Format [00000 (head 1 bit)(drive 2 bits)]
		commandByte = (unsigned char) (((head & 1) << 2) | (driveNum & 3));
		CommandWrite(commandByte);

		// Construct the track number byte
		commandByte = (unsigned char) track;
		CommandWrite(commandByte);
      
		// Construct the head number byte
		commandByte = (unsigned char) head;
		CommandWrite(commandByte);

		// Construct the sector byte
		commandByte = (unsigned char) sector;
		CommandWrite(commandByte);

		// Construct the sector size code
		commandByte = (unsigned char) (floppies[driveNum]->sectorSize >> 8);
		CommandWrite(commandByte);

		// Construct the end of track byte
		commandByte = (unsigned char) floppies[driveNum]->sectorsPerCylinder;
		CommandWrite(commandByte);

		// Construct the gap length byte
		commandByte = (unsigned char)((floppyDriveData *)(floppies[driveNum]->driverData))->gapLength;
		CommandWrite(commandByte);

		// Construct the custom sector size byte
		commandByte = (unsigned char) 0xFF;  // Always FFh
		CommandWrite(commandByte);

		status = WaitOperationComplete();

		// Close the DMA channel
		kernelDmaCloseChannel(theDisk->dmaChannel);
		if (status < 0)
		{
			// The command timed out.  Save the error and return error.
			kernelLockRelease((void *) &controllerLock);
			return (status);
		}

		// We have to read the seven status bytes from the controller.  Save them
		// in the designated memory locations

		statusRegister0 = statusRead();
		statusRegister1 = statusRead();
		statusRegister2 = statusRead();
		statusRegister3 = statusRead();

		// We don't care about status registers 4-6.
		for (count = 0; count < 3; count ++)
			tmp = statusRead();

		// Save the current track
		currentTrack = (unsigned ) statusRegister3;

		// Now we can examine the status.  If the top two bits of register 0 are
		// clear, then the operation completed normally.
		if (statusRegister0 & 0xC0)
		{
			// We have an error.  Retry up to twice.
			if (retry < 2)
			{
				retry += 1;
				continue;
			}

			// We have an error.  We have to try to determine the cause and set
			// the error message.  We'll call a routine which does all of this
			// for us.
			errorCode = evaluateError();
			break;
		}
		else
		{
			// If this was a read operation, copy xFerBytes worth of data from
			// the transfer area to the user buffer
			if (read)
				kernelMemCopy((void *) xFer, buffer, xFerBytes);
		}
      
		logicalSector += doSectors;
		numSectors -= doSectors;
		buffer += (doSectors * theDisk->sectorSize);
		retry = 0;
      
	} // Per-operation loop
  
	// Unlock the controller
	kernelLockRelease((void *) &controllerLock);

	if (errorCode == FLOPPY_WRITEPROTECT)
		return (status = ERR_NOWRITE);

	else if (errorCode)
	{
		kernelError(kernel_error, "Read/write error: %s",  errorMessages[errorCode]);
		return (status = ERR_IO);
	}

	else return (status = 0);
}


PRIVATE void FloppyInterrupt(void)
{
	// This is the floppy interrupt handler.  It will simply change a data
	// value to indicate that one has been received, and acknowldege the
	// interrupt to the PIC.  It's up to the other routines to do something
	// useful with the information.

	void *address = NULL;

	kernelProcessorIsrEnter(address);
	kernelProcessingInterrupt = 1;

	// Check whether to do the "sense interrupt status" command.
	if (readStatusOnInterrupt)
	{
		// Tell the diskette drive that the interrupt was serviced.  This
		// helps the drive stop doing the operation, and returns some status
		// operation, which we will save.

		CommandWrite((unsigned char) 0x08);

		statusRegister0 = statusRead();
		currentTrack = (unsigned) statusRead();
      
		readStatusOnInterrupt = 0;
	}

	// Note that we got the interrupt.
	interruptReceived = 1;

	kernelPicEndOfInterrupt(INTERRUPT_NUM_FLOPPY);

	kernelProcessingInterrupt = 0;
	kernelProcessorIsrExit(address);
}


static int driverReset(int driveNum)
{
  // Does a software reset of the requested floppy controller.  Always
  // returns success.

  int status = 0;
  unsigned char data = 0;

  if (driveNum >= MAXFLOPPIES)
    return (status = ERR_BOUNDS);

  // Wait for a lock on the controller
  status = kernelLockGet((void *) &controllerLock);
  if (status < 0)
    return (status);

  // Select the drive
  selectDrive(driveNum);

  // Read the port's current state
  kernelProcessorDelay();
  kernelProcessorInPort8(0x03F2, data);

  // Mask off the 'reset' bit (go to reset mode)
  data &= 0xFB;

  // Issue the command
  kernelProcessorOutPort8(0x03F2, data);
  kernelProcessorDelay();
  kernelProcessorDelay();

  // Now mask on the 'reset' bit (exit reset mode)
  data |= 0x04;

  // Issue the command
  kernelProcessorOutPort8(0x03F2, data);
  kernelProcessorDelay();

  // Unlock the controller
  kernelLockRelease((void *) &controllerLock);

  return (status = 0);
}


static int driverRecalibrate(int driveNum)
{
  // Recalibrates the selected drive, causing it to seek to track 0

  int status = 0;
  unsigned char commandByte, driveByte;

  if (driveNum >= MAXFLOPPIES)
    return (status = ERR_BOUNDS);

  // Wait for a lock on the controller
  status = kernelLockGet((void *) &controllerLock);
  if (status < 0)
    return (status);

  // Select the drive
  selectDrive(driveNum);

  // Tell the interrupt-received routine to issue the "sense interrupt
  // status" command after the operation
  readStatusOnInterrupt = 1;
  interruptReceived = 0;

  // We have to send two byte commands to the controller
  commandByte = (unsigned char) 0x07;
  commandWrite(commandByte);
  driveByte = (unsigned char) driveNum;
  commandWrite(driveByte);

  // Now wait for the operation to end
  status = waitOperationComplete();

  // Unlock the controller
  kernelLockRelease((void *) &controllerLock);

  if (status < 0)
    return (status);

  // Check error status
  if ((statusRegister0 & 0xF8) != 0x20)
    return (status = ERR_IO);

  // Make sure that we are now at track 0
  if (currentTrack == 0)
    return (status = 0);
  else
    return (status = ERR_IO);
}


static int driverSetMotorState(int driveNum, int onOff)
{
  // Turns the floppy motor on or off

  int status = 0;

  // Wait for a lock on the controller
  status = kernelLockGet((void *) &controllerLock);
  if (status < 0)
    return (status);

  status = setMotorState(driveNum, onOff);

  // Unlock the controller
  kernelLockRelease((void *) &controllerLock);

  return (status);
}


static int driverDiskChanged(int driveNum)
{
  // This routine determines whether the media in the floppy has changed.
  // drive.  It takes no parameters, and returns 1 if the disk is missing
  // or has been changed, 0 if it has not been changed, and  negative if it
  // encounters some other type of error.

  int status = 0;
  unsigned char data = 0;

  if (driveNum >= MAXFLOPPIES)
    return (status = ERR_BOUNDS);

  // Wait for a lock on the controller
  status = kernelLockGet((void *) &controllerLock);
  if (status < 0)
    return (status);

  // Select the drive
  selectDrive(driveNum);

  // Now simply read port 03F7h.  Bit 7 is the only part that matters.
  kernelProcessorDelay();
  kernelProcessorInPort8(0x03F7, data);

  // Unlock the controller
  kernelLockRelease((void *) &controllerLock);

  if (data & 0x80)
    return (status = 1);
  else
    return (status = 0);
}


static int driverReadSectors(int driveNum, unsigned logicalSector,
			     unsigned numSectors, void *buffer)
{
  if (driveNum >= MAXFLOPPIES)
    return (ERR_BOUNDS);

  // This routine is a wrapper for the readWriteSectors routine.
  return (readWriteSectors(driveNum, logicalSector, numSectors, buffer,
			   1));  // Read operation
}


static int driverWriteSectors(int driveNum, unsigned logicalSector,
			      unsigned numSectors, const void *buffer)
{
  if (driveNum >= MAXFLOPPIES)
    return (ERR_BOUNDS);

  // This routine is a wrapper for the readWriteSectors routine.
  return (readWriteSectors(driveNum, logicalSector, numSectors,
			   (void *) buffer, 0));  // Write operation
}


PRIVATE int DriverDetect(void *driver)
{
	// This routine is used to detect and initialize each device, as well as
	// registering each one with any higher-level interfaces.  Also does
	// general driver initialization.

	int status = 0;
	kernelDevice *devices = NULL;
	kernelPhysicalDisk *floppyPointer = NULL;
	floppyDriveData *floppyData = NULL;
	int count;

	// Reset the number of floppy devices 
	numberFloppies = kernelOsLoaderInfo->floppyDisks;

	// Allocate memory for the floppy device(s)
	devices = kernelMalloc(numberFloppies * (sizeof(kernelDevice) + sizeof(kernelPhysicalDisk)));
	if (devices == NULL)
		return (status = 0);

	floppyPointer = ((void *) devices + (numberFloppies * sizeof(kernelDevice)));

	// Loop for each device
	for (count = 0; count < numberFloppies; count ++)
	{
		floppies[count] = &floppyPointer[count];

		// The device name and filesystem type
		sprintf((char *) floppies[count]->name, "fd%d", count);

		// The head, track and sector values we got from the loader
		floppies[count]->heads = kernelOsLoaderInfo->fddInfo[count].heads;
		floppies[count]->cylinders = kernelOsLoaderInfo->fddInfo[count].tracks;
		floppies[count]->sectorsPerCylinder = kernelOsLoaderInfo->fddInfo[count].sectors;
		floppies[count]->numSectors = (floppies[count]->heads * floppies[count]->cylinders * floppies[count]->sectorsPerCylinder);
		floppies[count]->biosType = kernelOsLoaderInfo->fddInfo[count].type;

		// Some additional universal default values
		floppies[count]->flags = (DISKFLAG_PHYSICAL | DISKFLAG_REMOVABLE | DISKFLAG_FLOPPY);
		floppies[count]->deviceNumber = count;
		floppies[count]->sectorSize = 512;
		floppies[count]->dmaChannel = 2;
		// Assume motor off for now

		// We do division operations with these values
		if ((floppies[count]->sectorsPerCylinder == 0) || (floppies[count]->heads == 0))
		{
			// We do division operations with these values
			kernelError(kernel_error, "NULL sectors or heads value");
			return (status = ERR_INVALID);
		}

		// Get memory for our private data
		floppyData = kernelMalloc(sizeof(floppyDriveData));
		if (floppyData == NULL)
		{
			kernelError(kernel_error, "Can't get memory for floppy drive data");
			return (status = ERR_MEMORY);
		}

		switch(floppies[count]->biosType)
		{
		case 1:
			// This is a 360 KB 5.25" Disk.  Yuck.
			floppies[count]->description = "360 Kb 5.25\" floppy"; 
			floppyData->stepRate = 0x0D;
			floppyData->gapLength = 0x2A;
			break;
	
		case 2:
			// This is a 1.2 MB 5.25" Disk.  Yuck.
			floppies[count]->description = "1.2 Mb 5.25\" floppy"; 
			floppyData->stepRate = 0x0D;
			floppyData->gapLength = 0x2A;
			break;
	
		case 3:
			// This is a 720 KB 3.5" Disk.  Yuck.
			floppies[count]->description = "720 Kb 3.5\" floppy"; 
			floppyData->stepRate = 0x0D;
			floppyData->gapLength = 0x1B;
			break;
	  
		case 5:
		case 6:
			// This is a 2.88 MB 3.5" Disk.
			floppies[count]->description = "2.88 Mb 3.5\" floppy"; 
			floppyData->stepRate = 0x0A;
			floppyData->gapLength = 0x1B;
		break;
      
		default:
			// Oh oh.  This is an unexpected value.  Make a warning and fall
			// through to 1.44 MB.
			kernelError(kernel_warn, "Floppy disk fd%d type %d is unknown.  Assuming 1.44 Mb.", floppies[count]->deviceNumber, floppies[count]->biosType);

		case 4:
			// This is a 1.44 MB 3.5" Disk.
			floppies[count]->description = "1.44 Mb 3.5\" floppy"; 
			floppyData->stepRate = 0x0A;
			floppyData->gapLength = 0x1B;
			break;
		}

		// Generic, regardless of type
		floppyData->headLoad = 0x02;
		floppyData->headUnload = 0x0F;
		floppyData->dataRate = 0;

		// Attach the drive data to the disk
		floppies[count]->driverData = (void *) floppyData;

		floppies[count]->driver = driver;

		devices[count].device.class = kernelDeviceGetClass(DEVICECLASS_DISK);
		devices[count].device.subClass = kernelDeviceGetClass(DEVICESUBCLASS_DISK_FLOPPY);
		devices[count].driver = driver;
		devices[count].data = (void *) floppies[count];
	}

	// Get memory for a disk transfer area.

	// We need to get a physical memory address to pass to the DMA controller.
	// Therefore, we ask the memory manager specifically for the physical
	// address.
	xFerPhysical = kernelMemoryGetPhysical(DISK_CACHE_ALIGN, DISK_CACHE_ALIGN, "floppy disk transfer");
	if (xFerPhysical == NULL) return (status = ERR_MEMORY);
  
	// Map it into the kernel's address space
	status = kernelPageMapToFree(KERNELPROCID, (void *) xFerPhysical, (void **) &xFer, DISK_CACHE_ALIGN);
	if (status < 0)
		return (status);

	// Clear it out, since the kernelMemoryGetPhysical() routine doesn't do
	// it for us
	kernelMemClear((void *) xFer, DISK_CACHE_ALIGN);

	// Clear the "interrupt received" byte
	interruptReceived = 0;
	readStatusOnInterrupt = 0;    
  
	// Register our interrupt handler
	status = kernelInterruptHook(INTERRUPT_NUM_FLOPPY, &floppyInterrupt);
	if (status < 0)
		return (status);

	// Turn on the interrupt
	kernelPicMask(INTERRUPT_NUM_FLOPPY, 1);

	// Loop again, for each device, to finalize the setup
	for (count = 0; count < numberFloppies; count ++)
	{
		// Select the drive on the controller
		selectDrive(floppies[count]->deviceNumber);

		// Send the controller info about the drive.
		specify(floppies[count]->deviceNumber);

		// Register the floppy disk device
		status = kernelDiskRegisterDevice(&devices[count]);
		if (status < 0)
			return (status);

		status = kernelDeviceAdd(NULL, &devices[count]);
		if (status < 0)
			return (status);
	}

	return (status = 0);
}


static kernelDiskOps floppyOps = {
  driverReset,
  driverRecalibrate,
  driverSetMotorState,
  NULL, // driverLockState
  NULL, // driverDoorState
  driverDiskChanged,
  driverReadSectors,
  driverWriteSectors,
};


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Below here, the functions are exported for external use
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


void kernelFloppyDriverRegister(void *driverData)
{
   // Device driver registration.

  kernelDriver *driver = (kernelDriver *) driverData;

  driver->driverDetect = driverDetect;
  driver->ops = &floppyOps;

  return;
}