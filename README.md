# Signal Tower Control & Automation

## Project Overview

This Arduino sketch allows direct and remote control of a PATLITE signal tower, specifically a LE-FBP model I picked up at a flea market for $10.  Key features include:
- 24V DC operation (required by the PATLITE LE-FBP)
- Direct control of each light in the tower, separately
- Use of MQTT both to remotely set the state (on or off) of any light in the tower and to report state changes in any light
- MQTT operations consistent with Home Assistant integration, allowing the signal tower to be monitored and controlled via Home Assistant, including turning any light in the tower on and off individually. 


## Signal Tower Operations
I could not find a product datasheet for the PATLITE LE-FBP but was able to puzzle out its operation through comparison with information provided in the [datasheet](https://www.patlite.com/support/enddata/catalog/lme-cat-en.pdf) for the LME series product.  Note that my LE-FBP is not equivalent to the LME series, so the wiring is not identical but instead what is described below:

The PATLITE LE-FBP wiring harness provides a separate control wire for each light in the tower.  The tower's built-in controller is capable of driving five different lights -- red, amber, green, blue, and clear, and can determine dynamically (through a clever analog wiring scheme) which lights are present in any installation as well as where they are in the stack.  Turning any light on requires applying 24V to the appropriate control wire. All lights share a common ground connection.

My signal tower works as follows (NOTE: Yours may be different!):

| Wire color (PATLITE) | Wire function |
| ---------- | ---------- |
| Black | Ground (common for all lights) |
| Grey | 24VDC Common (for all lights) |
| Red | 24VDC to illuminate the Red light |
| Orange | 24VDC to illuminate the Amber light |
| Green | 24VDC to illuminate the Green light |
| Blue | 24VDC to illuminate the Blue light |
| White | 24VDC to illuminate the Clear light |
| Violet | 24VDC to activate Buzzer #1 |
| Skyblue | 24VDC to activate Buzzer #2 |
| Brown | 24VDC Flashing Common |

Turning on any light means applying 24VDC to the appropriate control wire (with 24VDC also supplied to the grey common wire) and with the black wire connected to ground.

## Hardware Control

This control setup here is not what I was familar with in using low voltage microcontrollers to drive higher voltage loads, e.g. using a 5V Arduino to control a 12V LED strip.  In those situations the lower voltage microcontroller output drives the base of an NPN transistor that has its emitter connected to ground and the LED strip connected between the 12V suppply and the transistor's collector as shown in Figure 1 below.  This is called a "low side switch" because the transistor is serving as a digital switch and is on the low (ground) side of the load.  Clearly this wouldn't work for my PATLITE signal tower as I need to digitally turn on and off the 24V supply to each light's control wire separately.

Instead the PATLITE signal tower is an example of needing a "high side switch", with the  transistor (switch) between the supply voltage and the signal tower light (load), as shown in Figure 2 below.
(Looking at this another way, for a low side switch each load has its own separate ground circuit and shares a common supply voltage connection, and for a high side switch each load has its own separate supply voltage circuit and shares a common ground connection.)

![Low Side and High Side Switches](./low_high_side_switches.png)

Fortunately the high side switch use case is quite common, to the point that it is supported by readily available integrated circuits like the Toshiba TBD62783A, which combines eight such high side switch drivers on a single 18-pin IC.  You'll find Toshiba's [application note](https://toshiba.semicon-storage.com/info/TBD62783APG_application_note_en_20160516_AKX00417.pdf?did=35900&prodName=TBD62783APG) for the TBD62783A provides all the necessary details including a great application circuit example in driving LEDs in high side switch configuration.

## Usage Notes

TBD -- stay tuned.



