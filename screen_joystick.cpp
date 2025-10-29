#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "grlib/grlib.h"
#include "Crystalfontz128x128_ST7735.h"
#include "joystick.h"

/* FOR REFERENCE ONLY - FOR INTEGRATION IN MAIN.CPP
// ============================================================================
// External references (from main.cpp)
// ============================================================================
extern tContext gContext;  // global LCD drawing context
extern bool btnLeft_isPressed();
extern bool btnRight_isPressed();
extern void DrawHeader(const char *title, bool leftActive, bool rightActive);
*/

// ============================================================================
// Local variables and objects
// ----------------------------------------------------------------------------
// Joystick object: handles analog reading and optional button input.
//  - JSX: analog input pin for X-axis
//  - JSY: analog input pin for Y-axis
//  - JS1: digital input pin for joystick button
// ============================================================================
static Joystick js(JSX, JSY, JS1);
static float joyX = 0.0f;   // normalized X-axis value (-1.0  +1.0)
static float joyY = 0.0f;   // normalized Y-axis value (-1.0  +1.0)

/*
This function initializes the Joystick
*/
void ScreenJoystick_Init(void)
{
    js.begin();             // Initialize joystick ADC inputs
    js.calibrateCenter(32); // Calibrate to remove offset
}


/*
This function draws a circle and a dot that follows the joystick coordinates on the screen
*/
void ScreenJoystick_Draw(void)
{
    js.tick();
    float joyX = js.x();  // −1.0 → +1.0
    float joyY = js.y();  // −1.0 → +1.0

    tRectangle bg = {0, 21, 127, 127};
    GrContextForegroundSet(&gContext, ClrBlack);
    GrRectFill(&gContext, &bg);

    DrawHeader("Joystick", btnLeft_isPressed(), btnRight_isPressed());

    int cx = 64, cy = 75, radius = 40;
    GrContextForegroundSet(&gContext, ClrWhite);
    GrCircleDraw(&gContext, cx, cy, radius);

    int px = cx + (int)(joyX * radius);
    int py = cy - (int)(joyY * radius);
    GrContextForegroundSet(&gContext, ClrCyan);
    GrCircleFill(&gContext, px, py, 3);

    char buf[32];
    snprintf(buf, sizeof(buf), "X=%.2f  Y=%.2f", joyX, joyY);
    GrContextForegroundSet(&gContext, ClrYellow);
    GrStringDrawCentered(&gContext, buf, -1, 64, 115, false);

    GrFlush(&gContext);
}