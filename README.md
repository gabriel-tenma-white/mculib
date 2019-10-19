# mculib
Convenience functions &amp; classes for stm32 and compatible MCUs using libopencm3.

To use, simply add `include/` to your project include path, set the MCULIB_DEVICE_xxxxx define (using -D for g++), and add used modules to compilation.

For example (if using libopencm3 makefile) if your project uses dma_adc.cpp, add `$(MCULIB)/dma_adc.o` to `OBJS`.


# Devices
Not all modules support all devices. Check header file.

* MCULIB_DEVICE_STM32F030
* MCULIB_DEVICE_STM32F072
* MCULIB_DEVICE_STM32F103 - for STM32F103 and GD32F303
* MCULIB_DEVICE_STM32F303

# Modules

## fastwiring
An optimized wiring API.

`#include <mculib/fastwiring.hpp>`

`OBJS += $(MCULIB)/fastwiring.o`

```c++
namespace mculib {
  // mode is one of: INPUT, OUTPUT, INPUT_PULLUP, INPUT_PULLDOWN
  static inline void pinMode(Pad p, int mode);
  
  // bit must be either HIGH (1) or LOW(0)
  static inline void digitalWrite(Pad p, int bit);
  
  // returns 1 or 0
  static inline int digitalRead(Pad p);
  
  static void delayMicroseconds(uint32_t us);
  static void delay(int ms);
  
  static constexpr Pad PA0;
  static constexpr Pad PA1;
  // ...
}
```


## DMADriver
Primitive DMA driver for stm32.

`#include <mculib/fastwiring.hpp>`

`OBJS += $(MCULIB)/dma_driver.o`

```c++
namespace mculib {

enum class DMADirection {
  UNDEFINED=0,
  MEMORY_TO_PERIPHERAL,
  PERIPHERAL_TO_MEMORY
};

struct DMATransferParams {
  volatile void* address = nullptr;
  uint8_t bytesPerWord = 1;
  bool increment = false;
};

class DMADriver {
  public:
    DMADevice device;
    
    // d should be set to one of the predefined constants (e.g. DMA1) from libopencm3
    DMADriver(DMADevice d);
    
    // enable or disable the peripheral including clocks.
    // automatically called when the first DMAChannel is enabled on this controller.
    void enable();
    
    // automatically called when the last DMAChannel is disabled on this controller.
    void disable();
};

class DMAChannel {
  public:
    DMADriver& driver;
    int channel;
    bool enabled = false;

    DMAChannel(DMADriver& d, int ch);
 
    // enable() should be called before calling setTransferParams()
    void enable();
    void disable();
    
    // start/stop transfers
    void start();
    void stop();

    void setTransferParams(const DMATransferParams& srcParams,
              const DMATransferParams& dstParams,
              DMADirection dir, int nWords, bool repeat);

    // returns current memory offset into array (in words)
    uint32_t position();
};
}
```
