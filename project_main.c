/* C Standard library */
#include <stdio.h>

/* XDCtools files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/i2c/I2CCC26XX.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/UART.h>
#include "summeridemo/buzzer.h"

/* Board Header files */
#include "Board.h"
#include "sensors/mpu9250.h"

// MPU power pin global variables
static PIN_Handle hMpuPin;
static PIN_Handle buttonHandle;
static PIN_Handle button2Handle;
static PIN_Handle ledHandle;
static PIN_Handle buzzerHandle;
static PIN_State buttonState;
static PIN_State button2State;
static PIN_State MpuPinState;
static PIN_State ledState;
static PIN_State buzzerState;

// Pin configuration
PIN_Config buttonConfig[] = {
   Board_BUTTON0  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_POSEDGE,
   PIN_TERMINATE 
};

PIN_Config button2Config[] = {
   Board_BUTTON1  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_POSEDGE,
   PIN_TERMINATE 
};

PIN_Config buzzerConfig[] = {
    Board_BUZZER | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW,
    PIN_TERMINATE
};

// MPU power pin
static PIN_Config MpuPinConfig[] = {
    Board_MPU_POWER  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

// The constant Board_LED0 corresponds to one of the LEDs
PIN_Config ledConfig[] = {
   Board_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX, 
   PIN_TERMINATE // The configuration table is always terminated with this constant
};


// MPU uses its own I2C interface
static const I2CCC26XX_I2CPinCfg i2cMPUCfg = {
    .pinSDA = Board_I2C0_SDA1,
    .pinSCL = Board_I2C0_SCL1
};

/* Task */
#define STACKSIZE 2048
Char sensorTaskStack[STACKSIZE];
Char uartTaskStack[STACKSIZE];

enum state { WAITING=1,
    INTERPRETING
};
enum state programState = WAITING;

float x1, y1, z1, x2, y2, z2;

int hz = 5000;

char message[128];

/*
void button2Fxn(PIN_Handle handle, PIN_Id pinId) {
    if (5000 == hz)
    {
        buzzerSetFrequency(3);
        hz = 3;    
    }
    else
    {
        buzzerSetFrequency(5000);
        hz = 5000;
    }    
}
*/

void buttonFxn(PIN_Handle handle, PIN_Id pinId) {
    uint_t pinValue = PIN_getOutputValue( Board_LED0 );
    pinValue =  !pinValue;
    PIN_setOutputValue( ledHandle, Board_LED0, pinValue );
    programState = INTERPRETING;
    System_printf("Button pressed.");
    System_flush();
}

/* Task Functions */
Void uartTaskFxn(UArg arg0, UArg arg1) {

    char msg[80];

    UART_Handle uart;
    UART_Params uartParams;

    UART_Params_init(&uartParams);

    uartParams.writeDataMode = UART_DATA_TEXT;
    uartParams.readDataMode = UART_DATA_TEXT;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.readMode = UART_MODE_BLOCKING;
    uartParams.baudRate = 9600;
    uartParams.dataLength = UART_LEN_8;
    uartParams.parityType = UART_PAR_NONE;
    uartParams.stopBits = UART_STOP_ONE;

    uart = UART_open(Board_UART0, &uartParams);
    if (uart == NULL)
    {
        System_abort("Error opening the UART");
    }

    while (1) {
        // Just for sanity check for exercise, you can comment this out
        System_printf("uartTask\n");
        System_flush();

        sprintf(msg, "\nax:%5.2f,ay:%5.2f,az:%5.2f,gx:%5.2f,gy:%5.2f,gz:%5.2f\n\r", x1, y1, z1, x2, y2, z2);
        UART_write(uart, msg, strlen(msg));
        UART_write(uart, message, strlen(message));

        // Once per second, you can modify this
        Task_sleep(200000 / Clock_tickPeriod);
    }
}

Void sensorTaskFxn(UArg arg0, UArg arg1) {

    I2C_Handle      i2cMPU;
    I2C_Params      i2cMPUParams;

    I2C_Params_init(&i2cMPUParams);
    i2cMPUParams.bitRate = I2C_400kHz;
	// Note the different configuration below
    i2cMPUParams.custom = (uintptr_t)&i2cMPUCfg;

    // MPU power on
    PIN_setOutputValue(hMpuPin,Board_MPU_POWER, Board_MPU_POWER_ON);

    // Wait 100ms for the MPU sensor to power up
	Task_sleep(100000 / Clock_tickPeriod);
    System_printf("MPU9250: Power ON\n");
    System_flush();

    // MPU open i2c
    i2cMPU = I2C_open(Board_I2C, &i2cMPUParams);
    if (i2cMPU == NULL) {
        System_abort("Error Initializing I2CMPU\n");
    }

    // MPU setup and calibration
	System_printf("MPU9250: Setup and calibration...\n");
	System_flush();

	mpu9250_setup(&i2cMPU);

	System_printf("MPU9250: Setup and calibration OK\n");
	System_flush();

    
    int i = 0;
    int spaces = 0;
    while (1) {
        if (programState == INTERPRETING)
        {
            System_printf("sensorTask\n");
            System_flush();
            mpu9250_get_data(i2cMPU, &x1,&y1,&z1,&x2,&y2,&z2);
            if (spaces >= 3){
                programState = WAITING;
            }
            else if (x2 > 200.0){
                message[i] = '.';
                spaces = 0;
                i++;
            }
            else if (y2 > 200.0){
                message[i] = '-';
                spaces = 0;
                i++;
            }
            else if(NULL){
                message[i] = ' ';
                spaces++;
                i++;
            }
            if (i % 10 == 0)
            {
                System_flush();
            }
            
            // 0.2s sleep
            Task_sleep(200000 / Clock_tickPeriod);
            }
        }
    }


    Int main(void) {

    // Task variables
    Task_Handle sensorTaskHandle;
    Task_Params sensorTaskParams;
    Task_Handle uartTaskHandle;
    Task_Params uartTaskParams;

    // Initialize board
    Board_initGeneral();
    Board_initI2C();
    Board_initUART();

    // virtapinnin käyttöönotto
    hMpuPin = PIN_open(&MpuPinState, MpuPinConfig);
    if (hMpuPin == NULL) {
    	System_abort("Pin open failed!");
    }
    /* Task */
    Task_Params_init(&sensorTaskParams);
    sensorTaskParams.stackSize = STACKSIZE;
    sensorTaskParams.stack = &sensorTaskStack;
    sensorTaskParams.priority=2;
    sensorTaskHandle = Task_create(sensorTaskFxn, &sensorTaskParams, NULL);
    if (sensorTaskHandle == NULL) {
        System_abort("Task create failed!");
    }

    ledHandle = PIN_open(&ledState, ledConfig);
    if(!ledHandle) {
        System_abort("Error initializing LED pins\n");
    }
/*
    buzzerHandle = PIN_open(&buzzerState, buzzerConfig);
    if (!buzzerHandle)
    {
        System_abort("Error initializing Buzzer pins\n");
    }
    // buzzerOpen(buzzerHandle);
    // buzzerSetFrequency(5000);
*/
    // Enable the pins for use in the program
    buttonHandle = PIN_open(&buttonState, buttonConfig);
    if(!buttonHandle) {
        System_abort("Error initializing button pins\n");
    }
    if (PIN_registerIntCb(buttonHandle, &buttonFxn) != 0) {
        System_abort("Error registering button callback function");
    }
/*
    button2Handle = PIN_open(&button2State, button2Config);
    if (PIN_registerIntCb(button2Handle, &button2Fxn) != 0) {
        System_abort("Error registering button callback function");
    }
*/

    Task_Params_init(&uartTaskParams);
    uartTaskParams.stackSize = STACKSIZE;
    uartTaskParams.stack = &uartTaskStack;
    uartTaskParams.priority=2;
    uartTaskHandle = Task_create(uartTaskFxn, &uartTaskParams, NULL);
    if (uartTaskHandle == NULL) {
        System_abort("Task create failed!");
    }

    /* Sanity check */
    System_printf("Hello world!\n");
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
