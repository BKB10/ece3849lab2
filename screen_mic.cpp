#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "grlib/grlib.h"
#include "Crystalfontz128x128_ST7735.h"

// ============================================================================
// External references (shared from main.cpp)
// ============================================================================
extern tContext gContext;
extern bool btnLeft_isPressed();
extern bool btnRight_isPressed();
extern void DrawHeader(const char *title, bool leftActive, bool rightActive);

// ============================================================================
// Microphone input configuration
// ----------------------------------------------------------------------------
// The microphone output on the BoosterPack MKII is connected to PE5  AIN8.
// We use ADC0, Sample Sequencer 3 (single channel, manual trigger).
// ============================================================================
#define MIC_ADC_BASE      ADC0_BASE
#define MIC_ADC_SEQ       3
#define MIC_ADC_CHANNEL   ADC_CTL_CH8



static void Mic_Init()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5);

    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH8 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);
    ADCIntClear(ADC0_BASE, 3);
}

static uint16_t Mic_Read()
{
    uint32_t value;
    ADCProcessorTrigger(ADC0_BASE, 3);
    while (!ADCIntStatus(ADC0_BASE, 3, false)); // wait until ready
    ADCIntClear(ADC0_BASE, 3);
    ADCSequenceDataGet(ADC0_BASE, 3, &value);
    return (uint16_t)value;
}


static float Mic_Level()
{
    const int N = 128;   // number of samples
    float sum = 0.0f;

    for (int i = 0; i < N; i++) {
        uint16_t s = Mic_Read();          // raw ADC value
        float v = (float)s / 4095.0f;     // normalize to 0–1
        float ac = v - 0.5f;              // remove DC offset
        sum += ac * ac;                   // accumulate squared amplitude
    }

    float rms = sqrtf(sum / N);           // root mean square
    return rms;                           // 0.0 (silent) → ~0.5 (loud)
}

// ============================================================================
// ScreenMic_Init()
// ============================================================================
// Called once at program start. Initializes the ADC channel for the microphone.
// ============================================================================
void ScreenMic_Init()
{
    Mic_Init();
}

void DrawBar(int x, int yTop, int yBottom, float level, bool vertical)
{
    if (level < 0) level = 0;
    if (level > 1) level = 1;

    if (vertical) {
        // Vertical bar (for mic)
        const int width = 12;
        int height = yBottom - yTop;
        int filled = (int)(height * level);
        int yStart = yBottom - filled;

        GrContextForegroundSet(&gContext, ClrGray);
        tRectangle border = {x - 2, yTop - 2, x + width + 2, yBottom + 2};
        GrRectDraw(&gContext, &border);

        GrContextForegroundSet(&gContext, ClrGreen);
        tRectangle fill = {x, yStart, x + width, yBottom};
        GrRectFill(&gContext, &fill);
    }
    else {
        // Horizontal bar (for lux)
        const int height = 10;
        int width = yBottom - yTop;
        int filled = (int)(width * level);

        GrContextForegroundSet(&gContext, ClrDarkGray);
        tRectangle bg = {x, yTop, x + width, yTop + height};
        GrRectFill(&gContext, &bg);

        GrContextForegroundSet(&gContext, ClrGreen);
        tRectangle fg = {x, yTop, x + filled, yTop + height};
        GrRectFill(&gContext, &fg);
    }
}

// ============================================================================
// ScreenMic_Draw()
// ============================================================================
// Called periodically from main.cpp (every ~50 ms).
// Reads microphone level and updates the bar meter on screen.
//
// Visualization:
//   - Green = low volume
//   - Yellow = medium volume
//   - Red = high volume
// ============================================================================
void ScreenMic_Draw()
{
    float level = Mic_Level();
    DrawBar(58, 25, 120, level, true);

    char buf[16];
    snprintf(buf, sizeof(buf), "%.0f %%", level * 100.0f);
    GrContextForegroundSet(&gContext, ClrCyan);
    GrStringDrawCentered(&gContext, buf, -1, 64, 110, false);
}
//must add GrFlush(&gContext); after calling this function