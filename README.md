**This device is intended as an implementation example of [YAHNC](https://github.com/elbowz/yahnc) libs**

## SWECS
###### Switch External Control & State

SWECS is an advanced Switch made up 3 [Homie](https://github.com/homieiot/homie-esp8266) Nodes basing on [YAHNC](https://github.com/elbowz/yahnc) libs:
* `SwitchNode` aka *Command*
* `ButtonNode` aka *External Control*
* `BinarySensorNode` aka *External State* (optional)

This switch can be controlled remotely thanks to MQTT (`SwitchNode`), locally by a physical switch (`ButtonNode`) and it can use an optional *External State*, directly from the device (`BinarySensorNode`).

SWECS was designed to control the "wall light" (like Shelly or Sonoff) with a pre-existing system (bulb, switch/button, relay, wires), but can be also used for other devices (eg fans, gate...). The *External State* should be used when we can't by-pass the device main switch, and we want still use it normally.

> **tip:** In presence of a step relay and/or we are interested to device consumption (ie Ampere/Watt) see [SWECS-P][#SWECS-P]

### Notes

When `EXT_STATE` (build flag) is not set, the device state is manage internally and taken it from the `SwitchNode` output, otherwise is taken from the `BinarySensorNode` and published under `/state/state` topic.

The *External Control* can be:
0. *Toggle:* switch change => device state change
1. *Impulsive:* button push => device state change 
2. *Follow:* switch On (Off) => device state On (Off)

and can be set as `HomieSetting`: *externalControlMode*

*External Control* have some predefined "macro/gesture".
eg hold button > 1s and release, set device timeout proportional to press duration (for more info see func `extControlHandler()`) 

## SWECS-P
###### Switch External Control & State - Power

Very similar to SWECS, dd only two features:
* Power meter by the use of an ACS7128 (`ACS7128Node` extend `AdcNode`)
* Impulsive support for `SwitchNode` (aka *Command*), useful for step relay

### Notes

The purpose of SWECS-P is to show a different implementation method, where instead of callback is used inheritance way.

An example is the class `AdvSwitchNode` that extends `SwitchNode` and `BinarySensorNode` through a multiple inheritance (and causing a diamond problem). Thanks to this approach we can create a single a new `HomieNode` with some advantage, for example have a single topic `/switch` with a `/command` and `/state` subtopic.

> **note:** the added features can be also added to SWECS by "callback way". 

## State Machine diagram

![state machine diagram](imgs/state-machine.png)

