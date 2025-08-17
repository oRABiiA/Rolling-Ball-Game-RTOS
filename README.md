# 🎮 Rolling Ball Game (C, DOS)

A retro-style **rolling ball / car game** written in **C for DOS**, featuring direct video memory rendering, obstacles, collectibles, and PC speaker sound effects.  
This project demonstrates **low-level graphics programming** and **inline assembly** for keyboard input and hardware control.  

---

## 🚀 Features
- 🟢 **Direct Video Memory Rendering** (`0xB8000`)  
- 🎵 **PC Speaker Sound Effects & Music** (collision, slide, game start, game over)  
- 🎯 **Game Mechanics**:  
  - Obstacles (`#`)  
  - Collectible points (`$`)  
  - Slides (`^`) with ramp-like effects  
- 🕹️ **Keyboard Input with Inline Assembly**  
  - Arrow keys to move left/right  
- 💥 **Collision Detection & Lives System**  
- 🌈 **Dynamic Background Colors** as score increases  

---

## 🛠️ Technologies Used
- **C (Turbo C / Borland C / DOS)**  
- **Inline Assembly (x86)** for keyboard input & hardware control  
- **PC Speaker Sound Programming** via I/O ports  
- **Game Loop & Collision Detection**  
- **Low-level Graphics** (direct manipulation of text-mode video memory)  

---

## 📷 Demo (Screenshot)
*(Optional — add here if you can capture the DOS screen, or use an emulator like DOSBox to take a screenshot and upload it.)*

---

## 🎮 How to Play
1. Run the game in **DOSBox** or a similar DOS emulator.  
2. Use **Left Arrow** `←` to move left.  
3. Use **Right Arrow** `→` to move right.  
4. Avoid obstacles `#`, collect points `$`, and ride slides `^`.  
5. Survive as long as possible to increase your score!  

---

## 📦 Installation & Run
1. Install [DOSBox](https://www.dosbox.com/).  
2. Clone this repository:
   ```bash
   git clone https://github.com/<your-username>/rolling-ball-game.git
   cd rolling-ball-game
