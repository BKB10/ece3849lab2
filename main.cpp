/**
 * Lab 1 — Multi-Screen Skeleton
 * ------------------------------
 * Base project for students:
 * - Two screens (LUX and MIC)
 * - Circular navigation with S1 / S2
 * - Placeholder bars and random mic level
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

extern "C" {
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/fpu.h"
#include "Crystalfontz128x128_ST7735.h"
#include "grlib/grlib.h"
#include "timerLib.h"
}

#include "button.h"
#include "elapsedTime.h"

#include "buzzer.cpp"
#include "screen_accel_ball.cpp"
#include "screen_joystick.cpp"
#include "screen_mic.cpp"

#define SCREEN_COUNT 4

// ============================================================================
// Globals
// ============================================================================
tContext gContext;
uint32_t gSysClk;
Timer timer;

Button btnLeft(S1);
Button btnRight(S2);

elapsedMillis drawTimer(timer);
elapsedMillis inputTimer(timer);

// ============================================================================
// Screen Management
// ============================================================================
enum ScreenID { SCREEN_ACCEL = 0, SCREEN_MIC, SCREEN_JOYSTICK, SCREEN_LUX };
uint8_t currentScreen = SCREEN_LUX;

// ============================================================================
// LCD + Header
// ============================================================================
void LCD_Init(void)
{
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);
    GrContextInit(&gContext, &g_sCrystalfontz128x128);
    GrContextFontSet(&gContext, &g_sFontFixed6x8);
}

void DrawHeader(const char *title, bool leftActive, bool rightActive)
{
    tRectangle header = {0, 0, 127, 20};
    GrContextForegroundSet(&gContext, ClrDarkBlue);
    GrRectFill(&gContext, &header);

    // Navigation arrows
    GrContextForegroundSet(&gContext, leftActive ? ClrYellow : ClrGray);
    GrStringDraw(&gContext, "<", -1, 4, 6, false);
    GrContextForegroundSet(&gContext, rightActive ? ClrYellow : ClrGray);
    GrStringDraw(&gContext, ">", -1, 118, 6, false);

    // Title centered
    GrContextForegroundSet(&gContext, ClrWhite);
    GrStringDrawCentered(&gContext, title, -1, 64, 7, false);
}

void Draw_Mic()
{
    tRectangle bg = {0, 21, 127, 127};
    GrContextForegroundSet(&gContext, ClrBlack);
    GrRectFill(&gContext, &bg);

    DrawHeader("Microphone", btnLeft.isPressed(), btnRight.isPressed());

    ScreenMic_Draw();

    GrFlush(&gContext);
}

void Draw_Accel()
{
    tRectangle bg = {0, 21, 127, 127};
    GrContextForegroundSet(&gContext, ClrBlack);
    GrRectFill(&gContext, &bg);

    DrawHeader("Acceleration", btnLeft.isPressed(), btnRight.isPressed());

    ScreenJoystick_Draw();

    GrFlush(&gContext);
}

void Draw_Joystick()
{
    tRectangle bg = {0, 21, 127, 127};
    GrContextForegroundSet(&gContext, ClrBlack);
    GrRectFill(&gContext, &bg);

    DrawHeader("Joystick", btnLeft.isPressed(), btnRight.isPressed());

    ScreenAccel_Draw();

    GrFlush(&gContext);
}

// ============================================================================
// Button callbacks (carousel navigation)
// ============================================================================
// When pressing S1 (left):
//   - Go one screen back
//   - If we're already at the first screen, wrap to the last
void OnLeftClick() {
    Buzzer_Beep(400, 250);

    if (currentScreen == 0)
        currentScreen = SCREEN_COUNT - 1;
    else
        currentScreen--;
}

// When pressing S2 (right):
//   - Go one screen forward
//   - If we're already at the last, wrap to the first
void OnRightClick() {
    Buzzer_Beep(700, 250);

    currentScreen++;
    if (currentScreen >= SCREEN_COUNT)
        currentScreen = 0;
}

// ============================================================================
// Main
// ============================================================================
int main(void)
{
    FPUEnable();
    FPULazyStackingEnable();
    gSysClk = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                  SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), 120000000);

    //buzzerSysClk = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
    //                              SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    LCD_Init();
    timer.begin(gSysClk, TIMER0_BASE);

    btnLeft.begin(); btnRight.begin();
    btnLeft.attachClick(&OnLeftClick);
    btnRight.attachClick(&OnRightClick);

    Buzzer_Init();

    while (1)
    {
        if (inputTimer >= 10) {
            btnLeft.tick();
            btnRight.tick();
            inputTimer = 0;
        }

        if (drawTimer >= 50) {
            switch (currentScreen) {
                case SCREEN_LUX:
                    ScreenLux_Draw(luxValue);
                    break;
                case SCREEN_MIC:
                    Draw_Mic();
                    break;
                case SCREEN_ACCEL:
                    Draw_Accel();
                    break;
                case SCREEN_JOYSTICK:
                    Draw_Joystick();
                    break;
            }
            drawTimer = 0;
        }
    }
}