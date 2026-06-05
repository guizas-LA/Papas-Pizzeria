# Papa's Pizzeria — GRUPO_2LEIC17_4
 
> A full remake of the classic **Papa's Pizzeria** game, written in **C** and running on **MINIX 3**, built on top of low-level hardware drivers developed across 5 lab modules.
 
---
 

## About the Project
 
This project is a **low-level systems programming remake** of the browser game *Papa's Pizzeria*, developed as the final project for the **Computer Laboratory (LCOM)** course. The entire game runs on **MINIX 3**, a microkernel-based operating system, and communicates directly with hardware through custom-built interrupt-driven drivers.
 
All game logic, rendering, input handling, and timing are implemented from scratch in **C**, relying solely on the drivers written throughout the lab assignments — no external game frameworks or high-level OS abstractions are used.
 
---


## Features
 
- Full **graphical interface** rendered via direct VRAM access in video mode
- Complete **mouse support** — navigate menus and interact with the game using cursor input
- **Keyboard input** for navigation and shortcuts
- Real-time **timer-based game loop** with consistent frame pacing
- **RTC integration** to display the current date/time and track in-game sessions
- Core **Papa's Pizzeria gameplay loop** — take orders, prepare pizzas, and serve customers
- Main menu, options screen, and in-game HUD

---


## Hardware Drivers & Libraries
 
This project integrates all five drivers developed throughout the lab assignments:
 
| Lab | Device | Description |
|-----|--------|-------------|
| **Lab 1** | **RTC** (Real-Time Clock) | Reads the current date and time; used for session timestamps and in-game clock |
| **Lab 2** | **Timer** (i8254) | Generates periodic interrupts; drives the main game loop and frame rate control |
| **Lab 3** | **KBC / Keyboard** (i8042) | Handles keyboard scancodes via interrupt; used for key-based navigation and shortcuts |
| **Lab 4** | **Mouse** (PS/2 via KBC) | Reads mouse packets; controls the in-game cursor for all menu and gameplay interactions |
| **Lab 5** | **Graphics** (Video Card) | Sets graphics video mode, maps VRAM, and renders all sprites, backgrounds, and UI elements |
 
---


## Screenshots
 
### Main Menu
<!-- img -->
> *[ Insert here]*
 
### Options Screen
<!-- img -->
> *[ Insert here]*
 
### Gameplay
<!-- img -->
> *[ Insert here]*
 
---
 

## Build & Run
 

 
### 1. Build the project
 
```bash
make
```
 
### 2. Run on MINIX
 
```bash
lcom_run proj
```
 
> The project must be compiled and executed inside the MINIX 3 environment. Running it outside MINIX will fail due to direct hardware access via I/O ports and interrupt vectors.
 
### 3. Clean build artifacts
 
```bash
make clean
```
 
---
 


## Authors
 
Signed: `Guilherme Silva`, `<student name>`, `<student name>`, `<student name>`  
Date: `<date>`

LCOM Project for group GRUPO_2LEIC17_4<p>.

Group members:

Guilherme Silva (up202404270@up.pt)
<first name> <family name> (<email address>)
<first name> <family name> (<email address>)
<first name> <family name> (<email address>)

---
 
## License
 
This project was developed for academic purposes. All game concept and original assets belong to their respective owners (Flipline Studios). This remake is a non-commercial educational project.



