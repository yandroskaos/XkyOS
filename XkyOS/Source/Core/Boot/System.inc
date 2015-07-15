;;
;; System.inc -- System constants configuration
;;
;; Copyright (c) 2007 by Pablo Bravo
;; All Rights Reserved
;;

%ifndef __SYSTEM_INCLUDES__
%define __SYSTEM_INCLUDES__

	;BOOT at 0x07C0:0000
%define BOOT_SEGMENT		0x07C0
%define BOOT_STACK_SEGMENT	0x0000
%define BOOT_STACK_OFFSET	0xFFFE
	
	;Loader at 9000:0000
%define LDR_DIRECTION_SEGMENT	0x9000
%define LDR_DIRECTION_OFFSET	0x0000
%define LDR_DIRECTION_FLAT	0x00090000

	;Kernel at 2000:0000
%define KERNEL_START_ADDRESS_SEGMENT	0x2000
%define KERNEL_START_ADDRESS_OFFSET	0x0000
%define KERNEL_START_ADDRESS_FLAT	0x00020000
	
	;Kernel Stack
%define KERNEL_STACK	0x0000FFFC

%endif ; __SYSTEM_INCLUDES__
