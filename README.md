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


## Gameplay

The game recreates the main gameplay loop of *Papa's Pizzeria*:

1. **Take the customer's order**
2. **Prepare the pizza**
3. **Add the requested toppings**
4. **Bake the pizza**
5. **Serve the customer**
6. **Receive a score based on the quality of the order**

The project also includes menus, navigation screens, an in-game interface, and interactive elements controlled using the keyboard and mouse.

---


## Screenshots

### Main Menu

<img width="1917" height="1078" alt="Main Menu" src="https://github.com/user-attachments/assets/3ddf777f-33a3-42ff-90e6-524fe44e23d5" />

### Game Interface

<img width="1917" height="1078" alt="Game Interface" src="https://github.com/user-attachments/assets/2f8a2b93-8004-454f-8742-fe96eca88351" />

### Order Selection

<img width="1917" height="1078" alt="Order Selection" src="https://github.com/user-attachments/assets/bb59ced7-b726-43ec-ab9b-e9d29e951254" />

### Pizza Preparation

<img width="1917" height="1078" alt="Pizza Preparation" src="https://github.com/user-attachments/assets/dc96ae2c-841d-4c70-b6f3-19149a557780" />

### Toppings

<img width="1917" height="1078" alt="Toppings" src="https://github.com/user-attachments/assets/19cc7637-a02e-4fa3-84e3-fe2b30ac53f7" />

### Cooking

<img width="1917" height="1078" alt="Cooking" src="https://github.com/user-attachments/assets/0b217073-b762-4eec-8cf6-8d50b0f36290" />

### Serving

<img width="1917" height="1077" alt="Serving" src="https://github.com/user-attachments/assets/ce85d4d5-5184-49ee-bafd-b1d4aba0ac68" />

### Score

<img width="1917" height="1077" alt="Score" src="https://github.com/user-attachments/assets/9f4d4742-aa8d-45d8-9c08-5038abc89d4c" />

### Gameplay

<img width="1917" height="1078" alt="Gameplay" src="https://github.com/user-attachments/assets/4cb06faa-b998-4cf4-bad4-5f683e848603" />

### Final Screen

<img width="1917" height="1078" alt="Final Screen" src="https://github.com/user-attachments/assets/64c68775-4236-443b-b616-e90e3ed360e1" />

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

This project was developed by **GRUPO_2LEIC17_4** as part of the **Computer Laboratory (LCOM)** course at **Faculdade de Engenharia da Universidade do Porto (FEUP)** during the **2025/2026 academic year**.

| Name | Student Number | Email |
|---|---|---|
| **Afonso Bouça** | `up202304970` | `up202304970@up.pt` |
| **Bruno Dias** | `up202405613` | `up202405613@up.pt` |
| **Diogo Coelho** | `up202406324` | `up202406324@up.pt` |
| **Guilherme Silva** | `up202404270` | `up202404270@up.pt` |

**Course:** Computer Laboratory (LCOM)  
**Institution:** Faculdade de Engenharia da Universidade do Porto (FEUP)  
**Academic Year:** 2025/2026  
**Group:** GRUPO_2LEIC17_4

---
 
## License
 
This project was developed for academic purposes. All game concept and original assets belong to their respective owners (Flipline Studios). This remake is a non-commercial educational project.



