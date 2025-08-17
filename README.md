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
<img width="435" height="286" alt="Picture5" src="https://github.com/user-attachments/assets/6af36bb8-d80b-414b-b1b0-479322bcc9f0" />
<img width="336" height="219" alt="Picture4" src="https://github.com/user-attachments/assets/37d3667b-8c7d-481e-af35-5e89dfedbcca" />
<img width="326" height="212" alt="Picture3" src="https://github.com/user-attachments/assets/4c3fd371-055b-4287-a945-488653d94eb9" />
<img width="303" height="197" alt="Picture2" src="https://github.com/user-attachments/assets/9c130e65-ef50-4959-af35-5acf95e83982" />
<img width="314" height="205" alt="Picture1" src="https://github.com/user-attachments/assets/784c1016-3f19-4401-b4b7-861c9c8f20e6" />

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
