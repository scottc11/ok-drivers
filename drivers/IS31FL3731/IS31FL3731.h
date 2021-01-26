#ifndef __IS31FL3731_H
#define __IS31FL3731_H

#include <mbed.h>
#include <OK_I2C.h>
#include <BitwiseMethods.h>

#define IS31FL3731_ADDR 0xE8


class IS31FL3731 : public OK_I2C
{
public:
    IS31FL3731(I2C *i2c_ptr, uint8_t addr = IS31FL3731_ADDR) {
        address = addr;
        i2c = i2c_ptr;
    };


    /*
    The Command Register should be configured first after writing in the slave address to choose the available register (Frame Registers
    and Function Registers). Then write data in the choosing register.
    For example, when write “0000 0011” in the Command Register (FDh), the data which writing after will be stored in the Frame 4 Register. Write
    new data can configure other registers.

    NOTE:  The data of Frame Registers is not assured when power on. Please initialize the Frame Registers first to ensure operate normally. 
    */
    void init() {

        setFrame(FRAME_9);
        i2cWrite(SHUTDOWN_REG, 0x00); // shutdown

        setFrame(FRAME_1);
        for (int i = 0; i < 0x12; i++) // open all 12 leds lanes for page 0
        {
            if (i < 3) {
                i2cWrite(i, 0b01010101); // led ON
            } else {
                i2cWrite(i, 0x00); // led OFF
            }
        }
        
        setFrameBlink(false);

        setFramePWM_(40);

        setFrame(FRAME_9);
        i2cWrite(SHUTDOWN_REG, 0x01);       // normal operation
        i2cWrite(CONFIG_REG, PICTURE_MODE); // Configure the operation mode
        i2cWrite(PICTUREFRAME_REG, 0x00);   // Set the display frame in Picture Mode
        i2cWrite(AUTO_PLAY_CTRL_REG_1, 0x00); // Set the way of display in Auto Frame Play Mode

        setFrame(FRAME_1);
    }

    /**
     * @param x The x position, starting with 0 for left-most side
     * @param y The y position, starting with 0 for top-most side
     * @param state the on or off state of LED, either 1 or 0
    */
    void setPixel(uint8_t x, uint8_t y, uint8_t state)
    {
        // zero indexed coordinate system
        // grid map formula == x + (y * 16)
        uint8_t data;
        uint8_t bitPos;
        uint8_t matrixPos;
        
        if (x < 8) {
            bitPos = x;
            matrixPos = 0;
        } else {
            bitPos = x - 8;
            matrixPos = 1;
        }

        y = (y * 2) + matrixPos;

        data = bitWrite(0x00, bitPos, state);
        
        i2cWrite(y, data);
    }

    void setPixelPWM(uint8_t x, uint8_t y, uint8_t pwm)
    {
        uint8_t lednum = x + (y * 16); // convert x y
        if (lednum >= 144)
            return;
        i2cWrite(FRAME_PWM_REG + lednum, pwm);
    }

    /**
     * PWM Registers modulate the 144 LEDs in 256 steps. 
     * The value of the PWM Registers decides the output current of each LED.
    */
    void setFramePWM_(uint8_t value) {
        for (int i = 0x24; i < 0xB4; i++) {
            i2cWrite(i, value > 255 ? 0 : value); // PWM set
        }
    }

    /**
     * The Blink Control Registers configure the blink function of each LED in the Matrix A and B. Please refer to the detail information in Table 7.
    */
    void setFrameBlink(bool blink) {
        for (int i = 0x12; i < 0x24; i++) {
            i2cWrite(i, blink ? 0xff : 0x00);
        }
    }

    /**
     * By setting the BE bit of the Display Option Register (05h) to “1”, blink function enable.
     * If the BE bit is set to “1”, each LED can be controlled by the Blink Control Registers (12h~23h in Page One to Page Eight).
     * The Display Option Register (05h) is used to set the blink period time, BPT, and the duty cycle is 50% (Figure 11).
    */
   void setBlinkRate(int value) {

   }

    // turn all leds off for the current frame
    void clearFrame() {
        for (int i = 0; i < 0x12; i++) {
            i2cWrite(i, 0x00); // led OFF
        }
    }

    void setFrame(int frame) {
        i2cWrite(COMMAND_REG, frame);
    }

    void displayFrame(int frame) {
    }

private:
    enum Registers
    {
        CONFIG_REG = 0x00,
        COMMAND_REG = 0xFD, // used to 'point' to a page / frame.
        FUNCTION_REG = 0xB,
        PICTUREFRAME_REG = 0x01,
        AUTO_PLAY_CTRL_REG_1 = 0x02,
        AUTO_PLAY_CTRL_REG_2 = 0x03,
        DISPLAY_OPT_REG = 0x05,
        SHUTDOWN_REG = 0x0A,
        FRAME_LED_REG = 0x24,
        FRAME_BLINK_REG = 0x12,
        FRAME_PWM_REG = 0x24,
    };

    enum DisplayMode
    {
        PICTURE_MODE = 0x00,
        AUTO_FRAME_PLAY_MODE = 0x01,
        AUDIO_FRAME_PLAY_MODE = 0x02,
    };

    enum Frames
    {
        FRAME_1 = 0,
        FRAME_2 = 1,
        FRAME_3 = 2,
        FRAME_4 = 3,
        FRAME_5 = 4,
        FRAME_6 = 5,
        FRAME_7 = 6,
        FRAME_8 = 7,
        FRAME_9 = 0x0B,
    };
};

#endif