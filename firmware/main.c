///////////////////////////////////////////////////////////////////////////////
// Author: Christopher D. Granz
// 2013-02-25
//
// Copyright (c) 2013 - Christopher D. Granz
// All rights reserved.
//
// This code is written for the Microchip XC8 Compiler.
// MCU: 12F1822
///////////////////////////////////////////////////////////////////////////////

#include "PlatformPIC.h"

//#define USING_BOOTLOADER

// set microcontroller ID, used for firmware version number
#define FIRMWARE_VERSION                0x01

__IDLOC7(0x00,0x00,FIRMWARE_VERSION,0x00);

// processor configuration bits
__CONFIG(BOREN_OFF & PWRTE_OFF & WDTE_ON & MCLRE_OFF & FOSC_INTOSC & CLKOUTEN_OFF & IESO_OFF & FCMEN_OFF); // Configuration Word 1
__CONFIG(WRT_ALL & PLLEN_OFF & STVREN_ON & BORV_LO & LVP_OFF); // Configuration Word 2

///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////
typedef signed char				Int8;
typedef signed int				Int16;
typedef unsigned char			UInt8;
typedef unsigned int			UInt16;

#define _XTAL_FREQ				8000000 // for __delay_ms() and __delay_us macros

// I2C Slave address
#define I2C_DEFAULT_SLAVE_ADDRESS       0x32

// I/O config
#define I2C_SCL_TRIS_BIT		TRISA1
#define I2C_SDA_TRIS_BIT		TRISA2

#define FET_GPIO_BIT			LATA5
#define FET_TRIS_BIT			TRISA5

#define EEPROM_ADDRESS_I2C_ADDRESS      0x10

///////////////////////////////////////////////////////////////////////////////
// Globals
///////////////////////////////////////////////////////////////////////////////
volatile UInt8 SSP1STATSave; // I2C management variables
volatile UInt8 I2CInBuffer[5];
volatile UInt8 I2COutBuffer[5];
volatile UInt8 I2CInBufIndex;
volatile UInt8 I2COutBufIndex;

char Temp[6]; // Temperary storage for several functions

UInt8 Temp1;
UInt8 Temp2;

#ifdef USING_BOOTLOADER
// WARNING don't change this, it is simply here to make the compiler
// learn that memory location alone
int UserAppMode @ 0x79;
int Reserved_A @ 0x7C; // used by bootloader ISR
#endif

///////////////////////////////////////////////////////////////////////////////
// Function Prototypes
///////////////////////////////////////////////////////////////////////////////
void interrupt ISRDispatch(void);
void I2CSlaveSetup(UInt8 address);
void ISRI2C(void);
UInt8 EEPROMReadByte(UInt8 addr);
void EEPROMWriteByte(UInt8 addr, UInt8 data);
void DelayMs(UInt8 ms);

///////////////////////////////////////////////////////////////////////////////
// Main program entry point.
///////////////////////////////////////////////////////////////////////////////
void main()
{
    UInt8 newI2CAddress;
    UInt8 lockoutI2CAddressChange;
    UInt8 i, j, k;

    // Set the oscillator frequency to 16MHz internal clock
    OSCCON = 0b01111000;

    // Set watchdog timeout for 4 seconds
    WDTCON = 0b00011001;

    ANSELA = 0x00; // all pins as digital

    INTCON = 0b11000000;

    // Now enable the correct outputs (others stay as inputs)
    FET_GPIO_BIT = 0;
    FET_TRIS_BIT = 1;

    __delay_ms(200); // power up delay before reading EEPROM

    // Setup the I2C slave module
    if ((newI2CAddress = EEPROMReadByte(EEPROM_ADDRESS_I2C_ADDRESS)) == 0x00
            || newI2CAddress == 0xFF)
    {
        newI2CAddress = I2C_DEFAULT_SLAVE_ADDRESS;
    }

    //I2CSlaveSetup(newI2CAddress);
    I2CSlaveSetup(I2C_DEFAULT_SLAVE_ADDRESS);

    newI2CAddress = 0x00;
    lockoutI2CAddressChange = 0;

    // setup the PWM module
    CCP1SEL = 1; // PWM on RA5
//    PR2 = 0x65;
    PR2 = 0xFF;
    CCPR1L  = 0x00;
    CCP1CON = 0b00001100; // single output mode = 00, LSBs of duty-cycle = 11, active-high for all signals = 1100

    PIR1bits.TMR2IF = 0;
    T2CON = 0b01111100;

    // wait for timer overflow
    while (!PIR1bits.TMR2IF) { }

    FET_TRIS_BIT = 0; // now enable output driver to begin PWM output

    // The firmware version is always the third byte if it is read...
    I2COutBuffer[2] = FIRMWARE_VERSION;

    /*
    CCPR1L = 238;
    CCP1CON &= 0b11001111;
    FET_TRIS_BIT = 0;
    while(1){}
*/

    while(1)
    {
        asm("CLRWDT"); // Clear watchdog timer

        __delay_ms(1);

        // Disable I2C interrupts--critical section for I2C buffer variables
        SSP1IE = 0;

        /*
        // Do we need to update the I2C address?
        if (!lockoutI2CAddressChange && newI2CAddress != 0x00
                && newI2CAddress != 0xFF)
        {
            SSPEN = 0; // disable module
            __delay_ms(20);
            I2CSlaveSetup(newI2CAddress);
            EEPROMWriteByte(0x10, newI2CAddress);
            newI2CAddress = 0x00;
            SSP1IE = 0;
        }

        if (!lockoutI2CAddressChange && I2CInBuffer[0] == 0x91)
        {
            if (I2CInBuffer[1] == 0x24 && I2CInBufIndex > 2)
            {
                I2CInBuffer[0] = 0;
                I2CInBuffer[1] = 0;

                newI2CAddress = I2CInBuffer[2];

                I2CInBuffer[2] = 0;
            }
        }
        else */
        {
            k = I2CInBuffer[0];

            if (k < 1)
                FET_TRIS_BIT = 1; // disable output
            else
            {
                lockoutI2CAddressChange = 1;
                GCEN = 0;
                FET_TRIS_BIT = 0; // enable output
            }
        }

        SSP1IE = 1;

        CCPR1L = k;
        CCP1CON &= 0b11001111;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Main interrupt service routine.
///////////////////////////////////////////////////////////////////////////////
void interrupt ISRDispatch()
{
    // NOTE: this processor has automatic context saving

    if (SSP1IF)
    {
        SSP1IF = 0;
        ISRI2C();
    }

#ifdef USING_BOOTLOADER
    // do a normal RETURN here, not a RETURN-FROM-INTERRUPT because of bootloader
    // (which has the "real" ISR)
#asm
    RETURN ;
#endasm
#endif
}

///////////////////////////////////////////////////////////////////////////////
void ISRI2C()
{
    static UInt8 generalCall = 0;
    // We are interested in these bits of the status register:
    //  bit 7 - SMP - Don't care
    //  bit 6 - CKE - Don't care
    //  bit 5 - D/!A: Data/!Address bit
    //  1 = Last byte received/transmitted was a data byte
    //  0 = Last byte received/transmitted was an address byte
    //  bit 4 - P - Don't care
    //  bit 3 - S - Start bit
    //				1 = Indicates that a Start bit has been detected last
    //				0 = Start bit was not detected last
    //	bit 2 - R/W: Read/Write bit information
    //				This bit holds the R/W bit information following the last address match
    //				This bit is only valid from the address match to the next Start bit, Stop bit, or not ACK bit
    //				1 = Read
    //				0 = Write
    //	bit 1 - UA - Don't care
    //	bit 0 - BF: Buffer Full Status bit
    //				Receive (SPI and I2C modes):
    //					1 = Receive complete, SSP1BUF is full
    //					0 = Receive not complete, SSP1BUF is empty
    //				Transmit (I2C mode only):
    //					1 = Data transmit in progress (does not include the ACK and Stop bits), SSP1BUF is full
    //					0 = Data transmit complete (does not include the ACK and Stop bits), SSP1BUF is empty

    // save a snapshot so it doesn't change while we are reading it
    SSP1STATSave = (SSP1STAT & 0b00101101);

    if (SSP1STATSave == 0b00001001) // State 1: Master Write, Last Byte was an Address
    {
        Temp1 = SSP1BUF; // clear buffer

        if (Temp1 == 0x00)
            generalCall = 1;
        else
            generalCall = 0;

        I2CInBufIndex = 0;
        CKP = 1; // release SCL
        return;
    }

    if (SSP1STATSave == 0b00101001) // State 2: Master Write, Last Byte was Data
    {
GeneralCallWrite:
        I2CInBuffer[I2CInBufIndex] = SSP1BUF;

        if (++I2CInBufIndex == sizeof(I2CInBuffer))
            I2CInBufIndex = 0;

        CKP = 1; // release SCL
        return;
    }

    SSP1STATSave &= 0b00101100;

    if (SSP1STATSave == 0b00001100) // State 3: Master Read, Last Byte was an Address
    {
        Temp1 = SSP1BUF; // clear buffer

        if (Temp1 == 0x00)
        {
            generalCall = 1;
            I2CInBufIndex = 0;
        }
        else
        {
            generalCall = 0;
            SSP1BUF = I2COutBuffer[0];
            I2COutBufIndex = 1;
        }

        CKP = 1; // release SCL
        return;
    }

    if (SSP1STATSave == 0b00101100 && !CKP) // State 4: Master Read, Last Byte was Data
    {
        if (generalCall == 1)
            goto GeneralCallWrite;

        if (!ACKSTAT)
        {
            SSP1BUF = I2COutBuffer[I2COutBufIndex];

            if (++I2COutBufIndex == sizeof(I2COutBuffer))
                I2COutBufIndex = 0;
        }

        CKP = 1; // release SCL
        return;
    }

    SSP1STATSave &= 0b00101000;

    if (SSP1STATSave == 0b00101000) // Master reset with NACK
    {
        if (generalCall == 1)
            goto GeneralCallWrite;

        CKP = 1; // release SCL
        return;
    }
    else
    {
        if (generalCall == 1)
            goto GeneralCallWrite;

        // Error condition
        CKP = 1; // release SCL
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Configure the I2C module for slave operation.
///////////////////////////////////////////////////////////////////////////////
void I2CSlaveSetup(UInt8 address)
{
    UInt8 i;

    // set I/Os as inputs for I2C operation
    I2C_SCL_TRIS_BIT = 1;
    I2C_SDA_TRIS_BIT = 1;

    for (i = 0; i < sizeof(I2CInBuffer); i++)
        I2CInBuffer[i] = 0;

    for (i = 0; i < sizeof(I2COutBuffer); i++)
        I2COutBuffer[i] = 0;

    SSP1ADD  = address;

    SSP1CON1 = 0b00110110;
    SSP1CON2 = 0b10000001;
    SSP1CON3 = 0b00010000;

    SSP1STAT = 0b10000000;

    SSPEN = 1; // enable the module

    SSP1IF = 0;
    SSP1IE = 1;
}

///////////////////////////////////////////////////////////////////////////////
UInt8 EEPROMReadByte(UInt8 addr)
{
    Temp1 = addr;

#asm
    BANKSEL _Temp1 ;
    MOVF _Temp1, W ;
    BANKSEL EEADRL ;
    MOVWF EEADRL ; Data Memory
    ; Address to read
    BCF _EECON1, 6  ; Deselect Config space
    BCF _EECON1, 7 ; Point to DATA memory
    BSF _EECON1, 0 ; EE Read
    MOVF EEDATL, W ; W = EEDATL
    BANKSEL _Temp2 ;
    MOVWF _Temp2 ; Temp2 = W
#endasm

    return Temp2;
}

///////////////////////////////////////////////////////////////////////////////
void EEPROMWriteByte(UInt8 addr, UInt8 data)
{
    Temp1 = addr;
    Temp2 = data;

#asm
    BANKSEL _Temp1 ;
    MOVF _Temp1, W ;
    BANKSEL EEADRL ;
    MOVWF EEADRL ; Data Memory Address to write
    BANKSEL _Temp2 ;
    MOVF _Temp2, W ;
    BANKSEL EEADRL ;
    MOVWF EEDATL ; Data Memory Value to write
    BCF _EECON1, 6 ; Deselect Configuration space
    BCF _EECON1, 7 ; Point to DATA memory
    BSF _EECON1, 2 ; Enable writes
    BCF _INTCON, 7 ; Disable INTs.
    MOVLW 55h ;
    MOVWF _EECON2 ; Write 55h
    MOVLW 0AAh ;
    MOVWF _EECON2 ; Write AAh
    BSF _EECON1, 1 ; Set WR bit to begin write
    BSF _INTCON, 7 ; Enable Interrupts
    BCF _EECON1, 2 ; Disable writes
    BTFSC _EECON1, 1 ; Wait for write to complete
    GOTO $-2 ; Done
#endasm
}

///////////////////////////////////////////////////////////////////////////////
// Delay for the specified number of milliseconds.
///////////////////////////////////////////////////////////////////////////////
void DelayMs(UInt8 ms)
{
    // Just use the built-in delay function for now
    // We need this wrapper when using a variable to specify the time
    // because the __delay_ms() function is inline, and only works
    // with a constant value.
    while (ms-- > 0)
        __delay_ms(1);
}

