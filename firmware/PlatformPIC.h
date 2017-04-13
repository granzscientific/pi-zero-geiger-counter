///////////////////////////////////////////////////////////////////////////////
// PlatformPIC.h
//
// This is generic library code.
//
// Copyright (c) 2011 - Christopher D. Granz
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#ifndef __PLATFORMPIC_H__
#define __PLATFORMPIC_H__

#if defined(HI_TECH_C)
	#include <htc.h>

	// PIC10/12/16 processor with HI-TECH PICC compiler
	#if (defined(_PIC12) || defined(_PIC14) || defined(_PIC14E))
		#define COMPILER_HITECH_PICC

	// PIC18 processor with HI-TECH PICC-18 compiler
	#elif defined(__PICC18__)
		#define COMPILER_HITECH_PICC18
	#else
		#error Unknown processor for HI-TECH compiler
	#endif

// PIC18 processor with Microchip C18 compiler
#elif defined(__18CXX)
	#define COMPILER_MPLAB_C18
	#include <p18cxxx.h>

// Microchip C30 compiler
#elif defined(__C30__)
	#define COMPILER_MPLAB_C30

	#if (defined(__PIC24F__) || defined(__PIC24FK__)) // PIC24F processor
		#include <p24Fxxxx.h>
	#elif defined(__PIC24H__) // PIC24H processor
		#include <p24Hxxxx.h>
	#elif defined(__PIC24E__) // PIC24E processor
		#include <p24Exxxx.h>
	#elif defined(__dsPIC33F__) // dsPIC33F processor
		#include <p33Fxxxx.h>
	#elif defined(__dsPIC33E__) // dsPIC33E processor
		#include <p33Exxxx.h>
	#elif defined(__dsPIC30F__) // dsPIC30F processor
		#include <p30fxxxx.h>
	#else // Microchip C30 compiler, but target "generic-16bit" processor.
		#include <p30sim.h>
		// Define some useful inline assembly functions which are normally in the 
		// processor header files, but absent from the generic p30sim.h file.
		#if !defined(Nop)
			#define Nop()    __builtin_nop()
			#define ClrWdt() {__asm__ volatile ("clrwdt");}
			#define Sleep()  {__asm__ volatile ("pwrsav #0");}
			#define Idle()   {__asm__ volatile ("pwrsav #1");}
		#endif
	#endif

// Microchip C32 compiler
#elif defined(__PIC32MX__)
	#define COMPILER_MPLAB_C32

	#include <p32xxxx.h>
	#include <plib.h>
#else
	#error Unknown compiler selected
#endif

// Definitions that apply to all except Microchip MPLAB C Compiler for PIC18 MCUs (C18)
/*
#if !defined(COMPILER_MPLAB_C18)
	#define memcmppgm2ram(a,b,c)	memcmp(a,b,c)
	#define strcmppgm2ram(a,b)		strcmp(a,b)
	#define memcpypgm2ram(a,b,c)	memcpy(a,b,c)
	#define strcpypgm2ram(a,b)		strcpy(a,b)
	#define strncpypgm2ram(a,b,c)	strncpy(a,b,c)
	#define strstrrampgm(a,b)		strstr(a,b)
	#define	strlenpgm(a)			strlen(a)
	#define strchrpgm(a,b)			strchr(a,b)
	#define strcatpgm2ram(a,b)		strcat(a,b)
#endif
*/

// Definitions that apply to all 8-bit devices
// (PIC10, PIC12, PIC16, PIC18)
#if defined(COMPILER_HITECH_PICC) || defined(COMPILER_HITECH_PICC18) || defined(COMPILER_MPLAB_C18)
	#define	__attribute__(a)

	#if defined(COMPILER_HITECH_PICC18) || defined(COMPILER_HITECH_PICC)
		#define rom						const
		#ifndef Nop()
			#define Nop()				asm("NOP");
		#endif
		#ifndef ClrWdt()
			#define ClrWdt()			asm("CLRWDT");
		#endif
		#ifndef Reset()
			#define Reset()				asm("RESET");
		#endif
		#ifndef Sleep()
			#define Sleep()				asm("SLEEP");
		#endif
	#endif
    
// Definitions that apply to all 16-bit and 32-bit devices
// (PIC24F, PIC24H, dsPIC30F, dsPIC33F, and PIC32)
#else
	#define	rom						const

	// 16-bit specific defines (PIC24F, PIC24H, dsPIC30F, dsPIC33F)
	#if defined(__C30__)
		#define Reset()				asm("reset")
		#define far                 __attribute__((far))
	#endif

	// 32-bit specific defines (PIC32)
	#if defined(__PIC32MX__)
		#define far
		#define Reset()				SoftReset()
		#define ClrWdt()			(WDTCONSET = _WDTCON_WDTCLR_MASK)
	#endif
#endif

///////////////////////////////////////////////////////////////////////////////
// Generic types
///////////////////////////////////////////////////////////////////////////////

// Specify an extension for GCC based compilers
#if defined(__GNUC__)
	#define __EXTENSION__ __extension__
#else
	#define __EXTENSION__
#endif

#if !defined(__PACKED)
    #define __PACKED
#endif

//#include <stddef.h>
//typedef enum _Bool { False = 0, True } Bool;
typedef unsigned char		Bool;

#ifndef False
	#define False			0
#endif

#ifndef True
	#define True			1
#endif

typedef unsigned char		Byte;
typedef union
{
    Byte value;
    struct __PACKED
    {
        __EXTENSION__ Byte b0:1;
        __EXTENSION__ Byte b1:1;
        __EXTENSION__ Byte b2:1;
        __EXTENSION__ Byte b3:1;
        __EXTENSION__ Byte b4:1;
        __EXTENSION__ Byte b5:1;
        __EXTENSION__ Byte b6:1;
        __EXTENSION__ Byte b7:1;
    } bits;
} ByteBits;


typedef signed char			Int8;
typedef signed short int	Int16;
typedef signed long int		Int32;

typedef unsigned char		UInt8;
typedef union
{
	UInt8 value;

	struct __PACKED
	{
		__EXTENSION__ UInt8 b0:1;
		__EXTENSION__ UInt8 b1:1;
		__EXTENSION__ UInt8 b2:1;
		__EXTENSION__ UInt8 b3:1;
		__EXTENSION__ UInt8 b4:1;
		__EXTENSION__ UInt8 b5:1;
		__EXTENSION__ UInt8 b6:1;
		__EXTENSION__ UInt8 b7:1;
	} bits;
} UInt8Bits;

typedef unsigned short int	UInt16;
typedef union 
{
	UInt16 value;
	UInt8 v[2] __PACKED;

	struct __PACKED
	{
		UInt8 lb;
		UInt8 hb;
	} byte;

	struct __PACKED
	{
		__EXTENSION__ UInt8 b0:1;
		__EXTENSION__ UInt8 b1:1;
		__EXTENSION__ UInt8 b2:1;
		__EXTENSION__ UInt8 b3:1;
		__EXTENSION__ UInt8 b4:1;
		__EXTENSION__ UInt8 b5:1;
		__EXTENSION__ UInt8 b6:1;
		__EXTENSION__ UInt8 b7:1;
		__EXTENSION__ UInt8 b8:1;
		__EXTENSION__ UInt8 b9:1;
		__EXTENSION__ UInt8 b10:1;
		__EXTENSION__ UInt8 b11:1;
		__EXTENSION__ UInt8 b12:1;
		__EXTENSION__ UInt8 b13:1;
		__EXTENSION__ UInt8 b14:1;
		__EXTENSION__ UInt8 b15:1;
	} bits;
} UInt16Bits;

typedef unsigned long int   UInt32;
typedef union
{
    UInt32 value;
    UInt16 w[2] __PACKED;
    UInt8  v[4] __PACKED;

	struct __PACKED
	{
		UInt16 lw;
		UInt16 hw;
	} word;

	struct __PACKED
	{
		UInt8 LB;
		UInt8 HB;
		UInt8 UB;
		UInt8 MB;
	} byte;

	struct __PACKED
	{
		__EXTENSION__ UInt8 b0:1;
		__EXTENSION__ UInt8 b1:1;
		__EXTENSION__ UInt8 b2:1;
		__EXTENSION__ UInt8 b3:1;
		__EXTENSION__ UInt8 b4:1;
		__EXTENSION__ UInt8 b5:1;
		__EXTENSION__ UInt8 b6:1;
		__EXTENSION__ UInt8 b7:1;
		__EXTENSION__ UInt8 b8:1;
		__EXTENSION__ UInt8 b9:1;
		__EXTENSION__ UInt8 b10:1;
		__EXTENSION__ UInt8 b11:1;
		__EXTENSION__ UInt8 b12:1;
		__EXTENSION__ UInt8 b13:1;
		__EXTENSION__ UInt8 b14:1;
		__EXTENSION__ UInt8 b15:1;
		__EXTENSION__ UInt8 b16:1;
		__EXTENSION__ UInt8 b17:1;
		__EXTENSION__ UInt8 b18:1;
		__EXTENSION__ UInt8 b19:1;
		__EXTENSION__ UInt8 b20:1;
		__EXTENSION__ UInt8 b21:1;
		__EXTENSION__ UInt8 b22:1;
		__EXTENSION__ UInt8 b23:1;
		__EXTENSION__ UInt8 b24:1;
		__EXTENSION__ UInt8 b25:1;
		__EXTENSION__ UInt8 b26:1;
		__EXTENSION__ UInt8 b27:1;
		__EXTENSION__ UInt8 b28:1;
		__EXTENSION__ UInt8 b29:1;
		__EXTENSION__ UInt8 b30:1;
		__EXTENSION__ UInt8 b31:1;
	} bits;
} UInt32Bits;

#if defined(COMPILER_MPLAB_C18)
	// 24-bit type only available on C18 compiler
	typedef unsigned short long UInt24;
	typedef union
	{
		UInt24 value;
		UInt8 v[3] __PACKED;

		struct __PACKED
		{
			UInt8 lb;
			UInt8 hb;
			UInt8 ub;
		} byte;

		struct __PACKED
		{
			__EXTENSION__ UInt8 b0:1;
			__EXTENSION__ UInt8 b1:1;
			__EXTENSION__ UInt8 b2:1;
			__EXTENSION__ UInt8 b3:1;
			__EXTENSION__ UInt8 b4:1;
			__EXTENSION__ UInt8 b5:1;
			__EXTENSION__ UInt8 b6:1;
			__EXTENSION__ UInt8 b7:1;
			__EXTENSION__ UInt8 b8:1;
			__EXTENSION__ UInt8 b9:1;
			__EXTENSION__ UInt8 b10:1;
			__EXTENSION__ UInt8 b11:1;
			__EXTENSION__ UInt8 b12:1;
			__EXTENSION__ UInt8 b13:1;
			__EXTENSION__ UInt8 b14:1;
			__EXTENSION__ UInt8 b15:1;
			__EXTENSION__ UInt8 b16:1;
			__EXTENSION__ UInt8 b17:1;
			__EXTENSION__ UInt8 b18:1;
			__EXTENSION__ UInt8 b19:1;
			__EXTENSION__ UInt8 b20:1;
			__EXTENSION__ UInt8 b21:1;
			__EXTENSION__ UInt8 b22:1;
			__EXTENSION__ UInt8 b23:1;
		} bits;
	} UInt24Bits;

	#define INT24_MAX	0x7FFFFF
	#define INT24_MIN	0x800000

#else
	// MPLAB C Compiler for PIC18 does not support 64-bit integers
	__EXTENSION__ typedef signed long long    Int64;
	__EXTENSION__ typedef unsigned long long  UInt64;

	typedef union
	{
		UInt64 value;
		UInt32 d[2] __PACKED;
		UInt16 w[4] __PACKED;
		UInt8 v[8]  __PACKED;

		struct __PACKED
		{
			UInt32 ld;
			UInt32 hd;
		} dword;

		struct __PACKED
		{
			UInt16 lw;
			UInt16 hw;
			UInt16 uw;
			UInt16 mw;
		} word;

		struct __PACKED
		{
			__EXTENSION__ UInt8 b0:1;
			__EXTENSION__ UInt8 b1:1;
			__EXTENSION__ UInt8 b2:1;
			__EXTENSION__ UInt8 b3:1;
			__EXTENSION__ UInt8 b4:1;
			__EXTENSION__ UInt8 b5:1;
			__EXTENSION__ UInt8 b6:1;
			__EXTENSION__ UInt8 b7:1;
			__EXTENSION__ UInt8 b8:1;
			__EXTENSION__ UInt8 b9:1;
			__EXTENSION__ UInt8 b10:1;
			__EXTENSION__ UInt8 b11:1;
			__EXTENSION__ UInt8 b12:1;
			__EXTENSION__ UInt8 b13:1;
			__EXTENSION__ UInt8 b14:1;
			__EXTENSION__ UInt8 b15:1;
			__EXTENSION__ UInt8 b16:1;
			__EXTENSION__ UInt8 b17:1;
			__EXTENSION__ UInt8 b18:1;
			__EXTENSION__ UInt8 b19:1;
			__EXTENSION__ UInt8 b20:1;
			__EXTENSION__ UInt8 b21:1;
			__EXTENSION__ UInt8 b22:1;
			__EXTENSION__ UInt8 b23:1;
			__EXTENSION__ UInt8 b24:1;
			__EXTENSION__ UInt8 b25:1;
			__EXTENSION__ UInt8 b26:1;
			__EXTENSION__ UInt8 b27:1;
			__EXTENSION__ UInt8 b28:1;
			__EXTENSION__ UInt8 b29:1;
			__EXTENSION__ UInt8 b30:1;
			__EXTENSION__ UInt8 b31:1;
			__EXTENSION__ UInt8 b32:1;
			__EXTENSION__ UInt8 b33:1;
			__EXTENSION__ UInt8 b34:1;
			__EXTENSION__ UInt8 b35:1;
			__EXTENSION__ UInt8 b36:1;
			__EXTENSION__ UInt8 b37:1;
			__EXTENSION__ UInt8 b38:1;
			__EXTENSION__ UInt8 b39:1;
			__EXTENSION__ UInt8 b40:1;
			__EXTENSION__ UInt8 b41:1;
			__EXTENSION__ UInt8 b42:1;
			__EXTENSION__ UInt8 b43:1;
			__EXTENSION__ UInt8 b44:1;
			__EXTENSION__ UInt8 b45:1;
			__EXTENSION__ UInt8 b46:1;
			__EXTENSION__ UInt8 b47:1;
			__EXTENSION__ UInt8 b48:1;
			__EXTENSION__ UInt8 b49:1;
			__EXTENSION__ UInt8 b50:1;
			__EXTENSION__ UInt8 b51:1;
			__EXTENSION__ UInt8 b52:1;
			__EXTENSION__ UInt8 b53:1;
			__EXTENSION__ UInt8 b54:1;
			__EXTENSION__ UInt8 b55:1;
			__EXTENSION__ UInt8 b56:1;
			__EXTENSION__ UInt8 b57:1;
			__EXTENSION__ UInt8 b58:1;
			__EXTENSION__ UInt8 b59:1;
			__EXTENSION__ UInt8 b60:1;
			__EXTENSION__ UInt8 b61:1;
			__EXTENSION__ UInt8 b62:1;
			__EXTENSION__ UInt8 b63:1;
		} bits;
	} UInt64Bits;
#endif

#define INT8_MAX	0x7F
#define INT16_MAX	0x7FFF
#define INT32_MAX	0x7FFFFFFF
#define INT64_MAX	0x7FFFFFFFFFFFFFFF

#define INT8_MIN	0x80
#define INT16_MIN	0x8000
#define INT32_MIN	0x80000000
#define INT64_MIN	0x8000000000000000

// Base RAM and ROM pointer types for different architectures
#if defined(COMPILER_HITECH_PICC18)
	#define PointerGeneric		UInt16
	#define PointerGenericROM	UInt32
#elif defined(COMPILER_MPLAB_C18)
	#define PointerGeneric		UInt16
	#define PointerGenericROM	UInt24
#elif defined(COMPILER_MPLAB_C30)
	#define PointerGeneric		UInt16
	#define PointerGenericROM	UInt16
#elif defined(COMPILER_MPLAB_C32)
	#define PointerGeneric		UInt32
	#define PointerGenericROM	UInt32
#endif

#undef __EXTENSION__

#endif // __PLATFORMPIC_H__

