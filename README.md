# 🧟‍♂️ ZombieSurvivor

*A First Person Shooter with base-building and survival mechanics, built entirely from scratch in **C++**, using **SDL** and **OpenGL**.*

---

## 🎮 Project Description

This is a personal game development project aimed at building a complete **custom game engine** from the ground up.

As part of my transition from backend software engineering to professional game development, this project showcases both gameplay and a strong technical foundation.

🎥 [Check out more on my portfolio](https://albertonasarre.dev/game-2/)

---

## ✨ Key Features

- 🧰 **Custom Game Engine in C++**  
  Engine developed from scratch using **SDL** and **OpenGL**, without relying on commercial engines.

- 🚀 **Optimized Performance**  
  Techniques like **frustum culling**, **mesh batching**, and the **Flyweight pattern** were implemented to maximize performance.

- 🧠 **Clean Architecture & Scalable Code**  
  Code is structured using patterns such as **Composite**, **State**, and **Template** programming to ensure maintainability and scalability.

- 🎨 **Graphics & Animation from Scratch**  
  Includes support for **shaders**, **lighting**, **AI**, **skeletal animation**, **Collisions system**, and **manual mesh loading** (OBJ + MTL parsing), all coded manually from fundamentals.

- 🔁 **I/O Device Interaction**  
  Developed input systems for mouse, keyboard, and controllers. Includes custom image/audio loaders and math utilities.

- 🧪 **Level Editor & Map System**  
  In-game editor to create maps, saved and loaded from external files using a custom format.

---

## 📌 Project Status

🎯 **Alpha Demo Complete**

### 🚧 In Progress / Planned Improvements:
- Improved animation system  
- Additional build mechanics  
- Level design (currently a technical demo)  
- GUI editor tools (currently everything is loaded via code)  
- Separate editor and game runtime environments

---

## 🛠️ Technologies Used

- 💻 C++  
- 🧱 SDL  
- 🎨 OpenGL  
- 🧠 GLSL (for shaders)  
- 📦 OBJ + MTL (mesh format standard)

---

## 🎮 Controls

| Action           | Key/Input         |
|------------------|-------------------|
| Move             | WASD / Arrow Keys |
| Shoot            | Left Mouse Button |
| Place Fence      | Right Mouse Button |
| Pause/Menu       | Enter             |

---

## ▶️ How to Play

### 🛠 Option 1 — Compile the Project

1. Clone this repository  
2. Open the `.sln` file using **Visual Studio** (found in the `VisualStudio` folder)  
3. Switch to **Release Mode**, then click **Play**  
4. Run the generated `.exe` file

### 🎮 Option 2 — Download a Playable Build

👉 [Play the game on Itch.io](https://albertonasare.itch.io/zombieSurvivorProjects)

---

## 📁 Project Structure Overview

Inside Visual Studio Solution.

⚠️ Important, look the code into the solution to see the folders. Visual studio 2019 is the best choice.

```plaintext
Assets/
├── extra/                  # Some useful code for images and text
├── gfx /                   # graphics folder:animation, shaders, textires, images managing etc
├── Scene/                  # Entety class and all entinies on the scene(meshes, zombies, weapons etc)
├── Stages/                 # Game Stages, Just one stage at the same time (play, pausa, menu etc...)
├── utils/                  # Framework and another classes for maths and input interaction.
├── game.cpp/               # Class where manage all game
├── main.cpp/               # Main class where code start execute
├── ZombieManager.cpp/      # Class to manage the zombies spaw and status


```
## 🛠️ How to Read the code

The project follows a **Stages Pattern**. You can find all game states (such as menus and gameplay) inside the `Stages` folder — the most important one being `PlayStage`.

The engine is based on the classic game loop:  
**Input Reading → Update State → Render**,  
and each stage (or game state) implements its own `update()` and `render()` methods.  
The `GameStage` class acts as a base for all these stages.

In the `Scene` folder, you'll find all screen-rendered elements.  
We're using the **Composite Pattern** with `Entities`: the `EntityManager` maintains a root entity, and each entity can have **parents and children**, allowing for hierarchical scene structure.

⚙️ The code handles everything manually — from **mesh loading** to all **OpenGL configurations** — without external tools or game engines. This makes the system highly transparent, but sometimes difficult to read.  
However, it allows for easy future expansion, so if the project grows, this architecture will scale better over time.


🔗 [See this project on my portfolio](https://albertonasarre.dev/game-2/)

