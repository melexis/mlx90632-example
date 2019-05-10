[![Documentation](https://img.shields.io/badge/Documentation-published-brightgreen.svg)](https://melexis.github.io/mlx90632-library/)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](http://www.apache.org/licenses/LICENSE-2.0)
[![Contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](https://github.com/melexis/mlx90632-example/issues)

This example shows how the MLX90632 library is implemented for a specific MCU that is connected to the MLX90632 sensor. 
The mapping for this MCU is done using the software STMCubeMX. 
All functions that need to be implemented for this individual MCU are listed in `mlx90632_depends.h` file.

# Documentation
Compiled documentation is available on [melexis.github.io/mlx90632-library](https://melexis.github.io/mlx90632-library/).

Datasheet is available in [Melexis documentation](https://www.melexis.com/en/documents/documentation/datasheets/datasheet-mlx90632).

MCU documentation is available on [STM32F070](http://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-nucleo/nucleo-f070rb.html).

MCU documentation is available on [STM32F4Discovery] (https://www.st.com/content/ccc/resource/technical/document/user_manual/70/fe/4a/3f/e7/e1/4f/7d/DM00039084.pdf/files/DM00039084.pdf/jcr:content/translations/en.DM00039084.pdf).

# Software
Tool for STM32 configuration [STM32CubeMX](http://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-configurators-and-code-generators/stm32cubemx.html)
IDE [ARM Keil uVision 5](https://www.keil.com/demo/eval/arm.htm)

# Hardware connections
| MLX90632        | Nucleo - F070RB        |
| --------------- |:----------------------:| 
| pin 1: SDA      | CN7:  pin 21: PB7      | 
| pin 2: VDD      | CN7:  pin 16: +3V3     |
| pin 3: GND      | CN7:  pin 20: GND      |
| pin 4: SCL      | CN10: pin 17: PB6      |
| pin 5: ADDR     | CN7:  pin 20: GND      |


# Hardware connections
| MLX90632        |STM32F4 Discovery       |
| --------------- | ---------------------- | 
| pin 1: SDA      | p2:  pin 20: PB9       | 
| pin 2: VDD      | p2:  pin 6: +3V3       |
| pin 3: GND      | p2:  pin 2 : GND       |
| pin 4: SCL      | p2: pin 23: PB6        |
| pin 5: ADDR     | p2:  pin 1: GND        |

## External components STM32F070

- Capacitor of 10nF on VDD and GND (maximum 1cm from MLX90632 sensor)
- Pull-up resistors on SDA and SCL of 4k7
## External components STM32F4Discovery
  -  PB7 ------> I2C1_SDA internal not enough Resistor need replace to PB9

# Getting started
Open STM32CubeMX and create a "New Project" with the wizard.

Select the correct MCU (SMT32F070RB) and "Start Project".

Make sure "I2C1" is enabled.

In the Configuration tab, the I2C clock frequency can be changed.

For this example, Fast mode is selected and "Primary Address Length selection" is set to "7-bit".

Next, generate the code and make sure to select "MDK-ARM V5" in "Toolchain / IDE".

Code will now be generated and opened in ARM Keil uVision.


From this point you can include the [library](https://github.com/melexis/mlx90632-library) to your project in the IDE.
- `git clone <url> mlx90632-example`
- `cd mlx90632-example`
- `git submodule --init`
- Make sure to define the "BITS_PER_LONG = 32" as a compiler flag (-D)
- Include the path: "..\Library\inc"

The main.c file shows an example of the complete implementation and calls all functions in order to have a correct reading
of the ambient and object temperature with the MLX90632 sensor.

