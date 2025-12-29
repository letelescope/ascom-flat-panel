/*
 * samd21-pwm.h
 * Copyright (C) 2025 - Present, Le Télescope - Ivry sur Seine - All Rights Reserved
 * Licensed under the MIT License. See the accompanying LICENSE file for terms.
 *
 * Description: Library for hardware based fast pwm for SAMM21 based boards.
 * This was tested on a Seeeduino XIAO.
 *
 * This leverages the 16 bits timers counters to generates fast pulses and allow for "high frequency" PWM.
 * The lib will fetch automatically wich timer to use according to the wanted output pin.
 *
 * Not every pin is addressable by a "TCC". Check the specs to know which pin to use.
 * If a pin uses a "TC" and not a "TCC" this lib will revert back automatically to the 8bit resolution.
 * Still you will have to set the wanted duty cycle using a 16bit value, the conversion to 8 bit will be made for you by the lib.
 *
 * Just a word of caution: Using a pin with a 16bit register does not guarant you get an effective 16bit resolution.
 * The rationale being that the CPU clock frequency (48 MHz) is the limiting factor here. For exemple at 20kHz pwm,
 * there is arout 2500 cpu cycle during a full period. Hence effectively the resolution is at most something aroun 11bit.
 * This was validated experimentally. The effective rsolution is just shy aboce 11bit in this case.
 *
 * Authors:
 * - Florian Thibaud
 * - Florian Gautier
 *
 * Heavily inspired, or more accurately shamelessly ripped, from the work of Khoih Hoang https://github.com/khoih-prog
 * More precisely SAMD_PWM  https://github.com/khoih-prog/SAMD_PWM/ . We do not use the library as we do not wnat to patch Arduino core.
 */
#include "wiring_private.h"

#define INVALID_SAMD_PIN 255

#define MAX_16BIT 65535UL

#define MAX_COUNT_16BIT 65536UL

static bool tcEnabled[TCC_INST_NUM + TC_INST_NUM] = {false};

class SAMD21_PWM
{
public:
  // dutycycle = 0.0f - 100.0f
  SAMD21_PWM(const uint8_t &pin, const float &frequency, const uint16_t &dutycycle)
  {
    _pinDesc = g_APinDescription[pin];
    _pinAttr = _pinDesc.ulPinAttribute;

    _tcNum = GetTCNumber(_pinDesc.ulPWMChannel);
    _tcChannel = GetTCChannelNumber(_pinDesc.ulPWMChannel);

    if (_pinAttr & PIN_ATTR_PWM)
    {
      _pin = pin;
      _frequency = frequency;

      _dutycycle = dutycycle;

      // Calc prescaler and _compareValue
      // Check which timer to use
      if (_tcNum >= TCC_INST_NUM)
      {
        calcTCPrescaler(frequency);
      }
      else
      {
        calcTCCPrescaler(frequency);
      }

      pinMode(pin, OUTPUT);

      return;
    }
  }

  ~SAMD21_PWM() {};

private:
  void calcTCCPrescaler(const float &frequency)
  {
    uint32_t period = 1000000 / frequency;

    if (period > 300000)
    {
      // Set prescaler to 1024
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV1024;
      _prescaler = 1024;
    }
    else if (80000 < period && period <= 300000)
    {
      // Set prescaler to 256
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV256;
      _prescaler = 256;
    }
    else if (20000 < period && period <= 80000)
    {
      // Set prescaler to 64
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV64;
      _prescaler = 64;
    }
    else if (10000 < period && period <= 20000)
    {
      // Set prescaler to 16
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV16;
      _prescaler = 16;
    }
    else if (5000 < period && period <= 10000)
    {
      // Set prescaler to 8
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV8;
      _prescaler = 8;
    }
    else if (2500 < period && period <= 5000)
    {
      // Set prescaler to 4
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV4;
      _prescaler = 4;
    }
    else if (1000 < period && period <= 2500)
    {
      // Set prescaler to 2
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV2;
      _prescaler = 2;
    }
    else if (period <= 1000)
    {
      // Set prescaler to 1
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV1;
      _prescaler = 1;
    }

    _compareValue = (uint32_t)(F_CPU / (_prescaler / (period / 1000000.0))) - 1;
  }

  // _compareValue max 8-bit = 255
  void calcTCPrescaler(const float &frequency)
  {
    uint32_t period = 1000000 / frequency;

    uint32_t newPeriod = period / 256;

    if (newPeriod > 300000)
    {
      // Set prescaler to 1024
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV1024;
      _prescaler = 1024;
    }
    else if (80000 < newPeriod && newPeriod <= 300000)
    {
      // Set prescaler to 256
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV256;
      _prescaler = 256;
    }
    else if (20000 < newPeriod && newPeriod <= 80000)
    {
      // Set prescaler to 64
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV64;
      _prescaler = 64;
    }
    else if (10000 < newPeriod && newPeriod <= 20000)
    {
      // Set prescaler to 16
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV16;
      _prescaler = 16;
    }
    else if (5000 < newPeriod && newPeriod <= 10000)
    {
      // Set prescaler to 8
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV8;
      _prescaler = 8;
    }
    else if (2500 < newPeriod && newPeriod <= 5000)
    {
      // Set prescaler to 4
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV4;
      _prescaler = 4;
    }
    else if (1000 < newPeriod && newPeriod <= 2500)
    {
      // Set prescaler to 2
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV2;
      _prescaler = 2;
    }
    else if (newPeriod <= 1000)
    {
      // Set prescaler to 1
      _prescalerConfigBits = TC_CTRLA_PRESCALER_DIV1;
      _prescaler = 1;
    }

    _compareValue = (uint32_t)(F_CPU / (_prescaler / (period / 1000000.0))) - 1;
  }

public:
  // dutycycle from 0-65535 for 0%-100%
  bool setPWM(const uint8_t &pin, const float &frequency, const uint16_t &dutycycle)
  {
    bool freqChange = false;

    // Reprogram freq if necessary
    if (frequency != _frequency)
    {

      _frequency = frequency;
      freqChange = true;

      // Recalc _compareValue
      // Check which timer to use
      if (_tcNum >= TCC_INST_NUM)
      {
        calcTCPrescaler(frequency);
      }
      else
      {
        calcTCCPrescaler(frequency);
      }
    }

    _dutycycle = dutycycle;

    // Arduino or Adafruit SAMD21
    if (_pinAttr & PIN_ATTR_PWM)
    {
      // To avoid uint32_t overflow
      uint32_t newDC = ((float)dutycycle / MAX_COUNT_16BIT) * _compareValue;

      if (_pinAttr & PIN_ATTR_TIMER)
      {
        pinPeripheral(pin, PIO_TIMER);
      }
      else
      {
        pinPeripheral(pin, PIO_TIMER_ALT);
      }

      if ((!tcEnabled[_tcNum]) || freqChange)
      {
        // New pin or freqChange
        tcEnabled[_tcNum] = true;

        uint16_t GCLK_CLKCTRL_IDs[] = {
            GCLK_CLKCTRL_ID(GCM_TCC0_TCC1), // TCC0
            GCLK_CLKCTRL_ID(GCM_TCC0_TCC1), // TCC1
            GCLK_CLKCTRL_ID(GCM_TCC2_TC3),  // TCC2
            GCLK_CLKCTRL_ID(GCM_TCC2_TC3),  // TC3
            GCLK_CLKCTRL_ID(GCM_TC4_TC5),   // TC4
            GCLK_CLKCTRL_ID(GCM_TC4_TC5),   // TC5
            GCLK_CLKCTRL_ID(GCM_TC6_TC7),   // TC6
            GCLK_CLKCTRL_ID(GCM_TC6_TC7),   // TC7
        };

        GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_IDs[_tcNum]);

        while (GCLK->STATUS.bit.SYNCBUSY == 1)
          ;

        // Check which timer to use
        if (_tcNum >= TCC_INST_NUM)
        {
          // Convert to 8-bit
          newDC = newDC >> 8;

          // -- Configure TC
          Tc *TCx = (Tc *)GetTC(_pinDesc.ulPWMChannel);

          // reset
          TCx->COUNT8.CTRLA.bit.SWRST = 1;

          while (TCx->COUNT16.STATUS.bit.SYNCBUSY)
            ;

          // Disable TCx
          TCx->COUNT8.CTRLA.bit.ENABLE = 0;

          while (TCx->COUNT16.STATUS.bit.SYNCBUSY)
            ;

          // Set Timer counter Mode to 8 bits, normal PWM, PRESCALER_DIV256
          TCx->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT8 | TC_CTRLA_WAVEGEN_NPWM | TC_CTRLA_PRESCALER_DIV256;

          while (TCx->COUNT16.STATUS.bit.SYNCBUSY)
            ;

          // Set the Dutycycle
          TCx->COUNT8.CC[_tcChannel].reg = (uint8_t)newDC;

          while (TCx->COUNT16.STATUS.bit.SYNCBUSY)
            ;

          // Set PER to _compareValue to match frequency
          // convert to 8-bit
          TCx->COUNT8.PER.reg = _compareValue >> 8;

          while (TCx->COUNT16.STATUS.bit.SYNCBUSY)
            ;

          // Enable TCx
          TCx->COUNT8.CTRLA.bit.ENABLE = 1;

          while (TCx->COUNT16.STATUS.bit.SYNCBUSY)
            ;
        }
        else
        {

          // -- Configure TCC
          Tcc *TCCx = (Tcc *)GetTC(_pinDesc.ulPWMChannel);

          // Disable TCCx
          TCCx->CTRLA.bit.ENABLE = 0;

          while (TCCx->SYNCBUSY.reg & TCC_SYNCBUSY_MASK)
            ;

          // Set prescaler
          TCCx->CTRLA.reg |= _prescalerConfigBits;

          while (TCCx->SYNCBUSY.reg & TCC_SYNCBUSY_MASK)
            ;

          // Set TCCx as normal PWM
          TCCx->WAVE.reg |= TCC_WAVE_WAVEGEN_NPWM;

          while (TCCx->SYNCBUSY.reg & TCC_SYNCBUSY_MASK)
            ;

          // Set the Dutycycle
          TCCx->CC[_tcChannel].reg = newDC;

          while (TCCx->SYNCBUSY.reg & TCC_SYNCBUSY_MASK)
            ;

          // Set PER to _compareValue to match frequency
          TCCx->PER.reg = _compareValue;

          while (TCCx->SYNCBUSY.reg & TCC_SYNCBUSY_MASK)
            ;

          // Enable TCCx
          TCCx->CTRLA.bit.ENABLE = 1;

          while (TCCx->SYNCBUSY.reg & TCC_SYNCBUSY_MASK)
            ;
        }
      }
      else
      {
        // Old pin and same freq
        // Check which timer to use
        if (_tcNum >= TCC_INST_NUM)
        {
          Tc *TCx = (Tc *)GetTC(_pinDesc.ulPWMChannel);

          // Set the Dutycycle
          TCx->COUNT16.CC[_tcChannel].reg = newDC;

          while (TCx->COUNT16.STATUS.bit.SYNCBUSY)
            ;
        }
        else
        {
          Tcc *TCCx = (Tcc *)GetTC(_pinDesc.ulPWMChannel);
          TCCx->CTRLBSET.bit.LUPD = 1;

          while (TCCx->SYNCBUSY.reg & TCC_SYNCBUSY_MASK)
            ;

          // Set the Dutycycle
          TCCx->CCB[_tcChannel].reg = newDC;

          while (TCCx->SYNCBUSY.reg & TCC_SYNCBUSY_MASK)
            ;

          TCCx->CTRLBCLR.bit.LUPD = 1;

          while (TCCx->SYNCBUSY.reg & TCC_SYNCBUSY_MASK)
            ;
        }
      }

      return true;
    }

    return false;
  }

  bool startPWM()
  {
    return setPWM(_pin, _frequency, _dutycycle);
  }

  inline float getActualDutyCycle()
  {
    return (((float)_dutycycle) * 100 / (1 << _resolution));
  }

  inline float getActualFreq()
  {
    return _frequency;
  }

  inline float getPWMPeriod()
  {
    return (1000000.0f / _frequency);
  }

  inline uint32_t get_freq_CPU()
  {
    return F_CPU;
  }

  inline uint32_t getPin()
  {
    return _pin;
  }

private:
  PinDescription _pinDesc;
  uint32_t _pinAttr;

  float _frequency;

  // For PWM frequency TOP register
  uint32_t _compareValue;

  // dutycycle from 0-65535 for 0%-100% to make use of full 16-bit TOP register
  uint32_t _dutycycle;

  uint32_t _prescalerConfigBits;
  uint16_t _prescaler = 1;

  // Timer info
  uint32_t _tcNum;
  uint8_t _tcChannel;

  uint8_t _pin = INVALID_SAMD_PIN;

  // In number of bits
  uint8_t _resolution = 16;
};