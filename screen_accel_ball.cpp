#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "grlib/grlib.h"
#include "Crystalfontz128x128_ST7735.h"
/*
// ============================================================================
// External references (defined in main.cpp)
// ============================================================================
extern tContext gContext;  // Global LCD drawing context
extern bool btnLeft_isPressed();
extern bool btnRight_isPressed();
extern void DrawHeader(const char *title, bool leftActive, bool rightActive);
*/

// ============================================================================
// Accelerometer ADC configuration
// ----------------------------------------------------------------------------
// The accelerometer outputs three analog signals (X, Y, Z).
// Only X and Y are used for this demo.
// Channels: PE0  AIN3, PE1  AIN2, PE2  AIN1
// ============================================================================
#define ACC_ADC_BASE     ADC0_BASE
#define ACC_ADC_SEQ      2
#define ACC_CH_X         ADC_CTL_CH3
#define ACC_CH_Y         ADC_CTL_CH2
#define ACC_CH_Z         ADC_CTL_CH1

// ---------------------------------------------------------------------------
// Accel_Init()
// ---------------------------------------------------------------------------
// Configures ADC0 to read the accelerometer inputs.
// ---------------------------------------------------------------------------
static void Accel_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);

    ADCSequenceConfigure(ACC_ADC_BASE, ACC_ADC_SEQ, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ACC_ADC_BASE, ACC_ADC_SEQ, 0, ACC_CH_X);
    ADCSequenceStepConfigure(ACC_ADC_BASE, ACC_ADC_SEQ, 1, ACC_CH_Y);
    ADCSequenceStepConfigure(ACC_ADC_BASE, ACC_ADC_SEQ, 2, ACC_CH_Z | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ACC_ADC_BASE, ACC_ADC_SEQ);
    ADCIntClear(ACC_ADC_BASE, ACC_ADC_SEQ);
}

// ---------------------------------------------------------------------------
// Accel_Read()
// ---------------------------------------------------------------------------
// Reads X and Y accelerometer channels using ADC0 and converts them
// to normalized values in the range [-1.0, +1.0].
// ---------------------------------------------------------------------------
static void Accel_Read(float *x, float *y)
{
    uint32_t raw[3];
    ADCProcessorTrigger(ACC_ADC_BASE, ACC_ADC_SEQ);
    while (!ADCIntStatus(ACC_ADC_BASE, ACC_ADC_SEQ, false));
    ADCIntClear(ACC_ADC_BASE, ACC_ADC_SEQ);
    ADCSequenceDataGet(ACC_ADC_BASE, ACC_ADC_SEQ, raw);

    // Convert raw 0-4095 ADC values  normalized -1.0 to +1.0 range
    *x = ((float)raw[0] - 2048.0f) / 2048.0f;
    *y = ((float)raw[1] - 2048.0f) / 2048.0f;
}

// ============================================================================
// Ball state variables
// ----------------------------------------------------------------------------
// posX, posY  position of the ball on the LCD
// velX, velY  velocity components (pixels per frame)
// ============================================================================
static float posX = 0.0f;
static float posY = 0.0f;
static float velX = 0.0f;
static float velY = 0.0f;

/************************************************ SCREEN-RELATED FUNCTIONS *************************************************** */

/*
This function initialize the accelerometer ball SCREEN
*/
void ScreenAccel_Init(void)
{
    Accel_Init();
    posX = rand() % 128;
    posY = 25 + rand() % 100;
    velX = velY = 0.0f;
}

/*
This function draw the SCREEN
*/
void ScreenAccel_Draw(void)
{
    float ax, ay;
    Accel_Read(&ax, &ay);

    velX += ax * 0.9f;
    velY += -ay * 0.9f;

    posX += velX;
    posY += velY;

    if (posX < 5)   { posX = 5;   velX *= -0.8f; }
    if (posX > 122) { posX = 122; velX *= -0.8f; }
    if (posY < 25)  { posY = 25;  velY *= -0.8f; }
    if (posY > 122) { posY = 122; velY *= -0.8f; }

    velX *= 0.99f;
    velY *= 0.99f;

    tRectangle bg = {0, 21, 127, 127};
    GrContextForegroundSet(&gContext, ClrBlack);
    GrRectFill(&gContext, &bg);

    DrawHeader("Accelerometer Ball", btnLeft_isPressed(), btnRight_isPressed());

    GrContextForegroundSet(&gContext, ClrYellow);
    GrCircleFill(&gContext, (int)posX, (int)posY, 5);
    GrFlush(&gContext);
}