

# Mars Rover Docking System

## Overview

Embedded system for autonomous docking using load cell feedback, stepper motors, limit switches, and relay control.

---
<img width="1021" height="885" alt="image" src="https://github.com/user-attachments/assets/bc205df3-3aec-47c9-b0ee-2405d449dd6a" />


## Hardware

* Arduino-compatible board
* HX711 + load cell
* 2× stepper motors + drivers
* 2× limit switches
* Relay module
* Stop signal input

---

## Pins

| Function  | Pin |
| --------- | --- |
| HX711 DT  | A0  |
| HX711 SCK | A1  |
| X STEP    | 2   |
| X DIR     | 5   |
| X LIMIT   | 9   |
| Y STEP    | 3   |
| Y DIR     | 6   |
| Y LIMIT   | 10  |
| EN        | 8   |
| STOP      | 12  |
| RELAY     | 13  |

---

## Logic

* **Weight > 200**

  * Move forward

* **Weight < 200**

  * Reverse to home (limit switches)

* **Stop signal HIGH**

  * Stop motion
  * Trigger relay sequence (runs once)

---

## Behavior

* Forward ignores limits
* Reverse stops per-axis on limit
* Full reset when both limits pressed

---

## Notes

* Uses `millis()` for relay timing
* Constant speed (no acceleration)
* Threshold-based control

---
