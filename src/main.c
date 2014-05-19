/** INCLUDES *******************************************************/
#include "usb.h"
#include "usb_function_cdc.h"
#include "HardwareProfile.h"

/** CONFIGURATION **************************************************/
     #pragma config WDTEN = OFF          //WDT disabled (enabled by SWDTEN bit)
     #pragma config PLLDIV = 3           //Divide by 3 (12 MHz oscillator input)
     #pragma config STVREN = ON          //stack overflow/underflow reset enabled
     #pragma config XINST = OFF          //Extended instruction set disabled
     #pragma config CPUDIV = OSC3_PLL3// CPU System Clock Postscaler (CPU system clock divide by 3)
     #pragma config CP0 = OFF            //Program memory is not code-protected
     #pragma config OSC = HSPLL          //HS oscillator, PLL enabled, HSPLL used by USB
     #pragma config T1DIG = ON           //Sec Osc clock source may be selected
     #pragma config LPT1OSC = OFF        //high power Timer1 mode
     #pragma config FCMEN = OFF          //Fail-Safe Clock Monitor disabled
     #pragma config IESO = OFF           //Two-Speed Start-up disabled
     #pragma config WDTPS = 32768        //1:32768
     #pragma config DSWDTOSC = INTOSCREF //DSWDT uses INTOSC/INTRC as clock
     #pragma config RTCOSC = T1OSCREF    //RTCC uses T1OSC/T1CKI as clock
     #pragma config DSBOREN = OFF        //Zero-Power BOR disabled in Deep Sleep
     #pragma config DSWDTEN = OFF        //Disabled
     #pragma config DSWDTPS = 8192       //1:8,192 (8.5 seconds)
     #pragma config IOL1WAY = OFF        //IOLOCK bit can be set and cleared
     #pragma config MSSP7B_EN = MSK7     //7 Bit address masking
     #pragma config WPFP = PAGE_1        //Write Protect Program Flash Page 0
     #pragma config WPEND = PAGE_0       //Start protection at page 0
     #pragma config WPCFG = OFF          //Write/Erase last page protect Disabled
     #pragma config WPDIS = OFF          //WPFP[5:0], WPEND, and WPCFG bits ignored  


/** I N C L U D E S **********************************************************/

#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "usb_config.h"
#include "usb_device.h"
#include "usb.h"
#include "main_tasks.h"

#include "HardwareProfile.h"
#include "hw.h"

/** V A R I A B L E S ********************************************************/
#if defined(__18CXX)
    #pragma udata
#endif

char USB_In_Buffer[64];
char USB_Out_Buffer[64];
unsigned char usb_state;
extern unsigned char memory_bank;
unsigned int memory_ptr;
extern volatile key_var keys;
unsigned char ds;
/** P R I V A T E  P R O T O T Y P E S ***************************************/
static void InitializeSystem(void);
void ProcessIO(void);
void USBDeviceTasks(void);
void YourHighPriorityISRCode();
void YourLowPriorityISRCode();
void USBCBSendResume(void);
void UserInit(void);


/** VECTOR REMAPPING ***********************************************/
#if defined(__18CXX)
	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x1000
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x1008
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x1018
	#elif defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)	
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x800
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x808
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x818
	#else	
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x00
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x08
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x18
	#endif
	
	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
	extern void _startup (void);        // See c018i.c in your C18 compiler dir
	#pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS
	void _reset (void)
	{
	    _asm goto _startup _endasm
	}
	#endif
	#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
	void Remapped_High_ISR (void)
	{
	     _asm goto YourHighPriorityISRCode _endasm
	}
	#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
	void Remapped_Low_ISR (void)
	{
	     _asm goto YourLowPriorityISRCode _endasm
	}
	
	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)

	#pragma code HIGH_INTERRUPT_VECTOR = 0x08
	void High_ISR (void)
	{
	     _asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
	}
	#pragma code LOW_INTERRUPT_VECTOR = 0x18
	void Low_ISR (void)
	{
	     _asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
	}
	#endif	//end of "#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_LEGACY_CUSTOM_CLASS_BOOTLOADER)"

	#pragma code
	
	
	//These are your actual interrupt handling routines.
	#pragma interrupt YourHighPriorityISRCode
	void YourHighPriorityISRCode()
	{

        #if defined(USB_INTERRUPT)
	        USBDeviceTasks();
        #endif
	timer_task();
	}	//This return will be a "retfie fast", since this is in a #pragma interrupt section 
	#pragma interruptlow YourLowPriorityISRCode
	void YourLowPriorityISRCode()
	{
        }

#endif


#if defined(__18CXX)
    #pragma code
#endif

unsigned char main_usb_state,main_nonusb_state;
unsigned int pll_startup_counter;

void main(void)
	{   
    InitializeSystem();
    main_init();
    usb_state=0;
//      OSCCON = 0x03;
//	DSCONH = 0x80;
//	Sleep();
    main_usb_state =0;
    OSCCON = 0x63;


//    				OSCCON = 0x40;  //40
//				pll_startup_counter = 6000;
//				OSCTUNEbits.PLLEN = 1;
//				while(pll_startup_counter--);
//				USBDeviceInit();
//				USBDeviceAttach();
//    while(1)
//    {
//    ProcessIO();
//    }
main_nonusb_state = STATE_PRESS;

    while(1)
	    {
		if (main_usb_state==0)
			{
//			OSCTUNE = 0x80;
			main_nonusb_state = main_loop_nonusb(main_nonusb_state);
                        if (main_nonusb_state== STATE_SHDN)
                            {
                            while (1)
                                {
                                LATB = 0;
                                LATC = 0;
                                LATA = 0;
                                LATBbits.LATB2 = 1;
                                INTCON = 0x10;
                                OSCCON = 0x03;
//                                DSCONH = 0x80;
                                Sleep();
                                LATB = 0;
                                LATC = 0;
                                LATA = 0;
                                if (K1==0)
                                    Reset();
                                }
                            }
			if(USB_BUS_SENSE)
				{
				OSCCON = 0x40;  //40
				pll_startup_counter = 6000;
				OSCTUNEbits.PLLEN = 1;
				while(pll_startup_counter--);
				USBDeviceInit();
				USBDeviceAttach();
                                main_loop_usb_init();
				main_usb_state =1;
				}
			
			}
		
		if (main_usb_state==1)
			{
			ProcessIO();
                        main_loop_usb(0);
			if(USB_BUS_SENSE==0)
				{
				USBDeviceDetach();
				OSCTUNEbits.PLLEN = 0;
				main_usb_state =0;
				OSCCON = 0x63;
				}
			}			
			
	    }
	}





static void InitializeSystem(void)
{
    tris_usb_bus_sense = INPUT_PIN; // See HardwareProfile.h
}//end InitializeSystem

void ProcessIO(void)
{   
    unsigned int datar;
    unsigned char len;
    // User Application USB tasks
    ds = USBDeviceState;
    if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;
    LED = ~ LED;
    if (usb_state==0)
        {
        memory_ptr=0;
        if (keys.k2)
            {
            keys.k2=0;
            usb_state=1;
            }
        }
    if (usb_state==1)
        {
        if(mUSBUSARTIsTxTrfReady())
            {
            datar = read_mem_location(memory_ptr,memory_bank);
            memory_ptr++;
            if (datar==0xFFFF)
                {
                sprintf(USB_In_Buffer,"---end of bank %d---\r\n",memory_bank);
                putUSBUSART(USB_In_Buffer,strlen(USB_In_Buffer));
                usb_state=0;
                }
            else
                {
                sprintf(USB_In_Buffer,"%d,%d\r\n",memory_ptr,datar);
                putUSBUSART(USB_In_Buffer,strlen(USB_In_Buffer));
                }
            }
        }
    if(USBUSARTIsTxTrfReady())
        {
        getsUSBUSART(USB_Out_Buffer,64);
        }
    CDCTxService();
}		//end ProcessIO


void USBCBSuspend(void)
{
}


void USBCBWakeFromSuspend(void)
{

}

void USBCB_SOF_Handler(void)
{
}

void USBCBErrorHandler(void)
{

}



void USBCBCheckOtherReq(void)
{
    USBCheckCDCRequest();
}//end


void USBCBStdSetDscHandler(void)
{
    // Must claim session ownership if supporting this request
}//end

void USBCBInitEP(void)
{
    //Enable the CDC data endpoints
    CDCInitEP();
}


void USBCBSendResume(void)
{
    static WORD delay_count;

    if(USBGetRemoteWakeupStatus() == TRUE) 
    {
        //Verify that the USB bus is in fact suspended, before we send
        //remote wakeup signalling.
        if(USBIsBusSuspended() == TRUE)
        {
            USBMaskInterrupts();
            
            //Clock switch to settings consistent with normal USB operation.
            USBCBWakeFromSuspend();
            USBSuspendControl = 0; 
            USBBusIsSuspended = FALSE;  //So we don't execute this code again, 
                                        //until a new suspend condition is detected.

            //Section 7.1.7.7 of the USB 2.0 specifications indicates a USB
            //device must continuously see 5ms+ of idle on the bus, before it sends
            //remote wakeup signalling.  One way to be certain that this parameter
            //gets met, is to add a 2ms+ blocking delay here (2ms plus at 
            //least 3ms from bus idle to USBIsBusSuspended() == TRUE, yeilds
            //5ms+ total delay since start of idle).
            delay_count = 3600U;        
            do
            {
                delay_count--;
            }while(delay_count);
            
            //Now drive the resume K-state signalling onto the USB bus.
            USBResumeControl = 1;       // Start RESUME signaling
            delay_count = 1800U;        // Set RESUME line for 1-13 ms
            do
            {
                delay_count--;
            }while(delay_count);
            USBResumeControl = 0;       //Finished driving resume signalling

            USBUnmaskInterrupts();
        }
    }
}


#if defined(ENABLE_EP0_DATA_RECEIVED_CALLBACK)
void USBCBEP0DataReceived(void)
{
}
#endif


BOOL USER_USB_CALLBACK_EVENT_HANDLER(int event, void *pdata, WORD size)
{
    switch( event )
    {
        case EVENT_TRANSFER:
            //Add application specific callback task or callback function here if desired.
            break;
        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            USBCBSuspend();
            break;
        case EVENT_RESUME:
            USBCBWakeFromSuspend();
            break;
        case EVENT_CONFIGURED: 
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;
        case EVENT_BUS_ERROR:
            USBCBErrorHandler();
            break;
        case EVENT_TRANSFER_TERMINATED:
            break;
        default:
            break;
    }      
    return TRUE; 
}


/** EOF main.c *************************************************/

