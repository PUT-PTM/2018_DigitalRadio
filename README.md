# **Digital Radio**

**Overview**

Radio receiver with manual frequency tuning and a command-driven interface.


**Description**

Digital Radio allows you to listen to radio stations, which are broadcasted in the VHF Band II (87.5 - 108.0 MHz). In order to set a desired frequency you have to either set it manually using a potentiometer and pressing the "User button" or use Hercules and enter the "s" command.


**List of commands**

s [frequency] - sets the desired frequency (eg. "s 103.9").
f - shows the current frequency.


**Tools**

Hercules 3.2.8
System Workbench for STM32


**How to compile**

No extra steps required here. Copy the program, open it in System Workbench for STM32, plug the Microntroller to USB port and run the project.


**Future improvements**

It is possible to add more received bands eg. Japanese FM band (76MHz to 91MHz).


**Attributions**

https://github.com/LonelyWolf/stm32/tree/master/tea5767


**Components**

Microntroller STM32F4 DISCOVERY
Stereo radio module TEA5767
Linear potentiometer
Speakers/Headphones


**License**

Distributed under MIT license


**Credits**

Jakub Szczepaniak
Paweł Świerczyński


The project was conducted during the Microprocessor Lab course held by the Institute of Control and Information Engineering, Poznan University of Technology.


Supervisor: Marek Kraft.
