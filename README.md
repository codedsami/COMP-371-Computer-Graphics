# 🎮 COMP-371 Computer Graphics - Flight Combat Simulator

A 3D flight combat simulator built with OpenGL, featuring realistic physics, dynamic lighting, and immersive aerial combat in a detailed city environment.

## 🚀 Project Overview

This project was developed as part of **COMP-371 Computer Graphics** course at Concordia University (Summer 2025). Experience thrilling aerial combat as you pilot a fully animated aircraft through a beautifully rendered 3D cityscape, complete with dynamic shadows and realistic lighting.

## ✨ Key Features

### 🎯 Core Gameplay
- **Flight Simulation**: Physics-based airplane controls with quaternion-based rotation system
- **Combat System**: Engage enemy aircraft with projectile-based weapons
- **Interactive Environment**: Explore a detailed 3D city with dynamic elements

### 🎨 Visual Excellence
- **Dynamic Lighting**: Real-time sun movement with realistic shadow casting and shadow mapping
- **Hierarchical Animation**: Multi-level aircraft animations (propeller, control surfaces, flaps)
- **Advanced Rendering**: Shadow mapping with depth buffer for realistic shadows
- **Textured Models**: High-quality textured 3D models throughout the environment

### 🎮 Controls
- **Mouse**: Camera orbit and zoom (third-person orbit camera)
- **W/S**: Pitch control with animated flap deployment
- **A/D**: Yaw control with animated rudder movement
- **Space**: Fire projectiles from wing-mounted positions
- **Mouse Wheel**: Camera zoom
- **Shift/Ctrl**: Speed control (accelerate/decelerate)

## 🏗️ Technical Architecture

### Built With (Course-Compliant Libraries)
- **OpenGL** - Graphics rendering
- **GLFW** - Window management and input handling
- **GLM** - Mathematics library for 3D transformations and quaternions
- **GLAD** - OpenGL extension loading
- **stb_image.h** - Texture loading
- **Assimp** - 3D model loading (via Model class)

### System Requirements
- **OS**: Windows 10/11, macOS, or Linux
- **Compiler**: C++17 compatible compiler
- **Graphics**: OpenGL 3.3+ compatible GPU
- **Dependencies**: CMake 3.10+

## 🛠️ Installation & Setup

### Prerequisites
Ensure you have the following installed:
- CMake (3.10 or higher)
- C++ compiler (GCC, Clang, or MSVC)
- OpenGL development libraries

### Quick Start
```bash
# Clone the repository
git clone https://github.com/codedsami/COMP-371-Computer-Graphics.git
cd COMP-371-Computer-Graphics

# Build the project
mkdir build && cd build
```
then run:
```
cmake ..

make

# Run the game
./ComputerGraphics
```

### Windows Users
```bash
# Using Visual Studio
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

## 📊 Project Milestones

### Milestone 1: Foundation ✅
- ✅ **Scene Setup**: Large, multi-textured 3D city environment with GLB models
- ✅ **Interactive Camera**: Third-person orbit camera with full 360° mouse control
- ✅ **Dynamic Lighting**: Orbiting sun model with changing light positions for shadows

### Milestone 2: Advanced Features ✅
- ✅ **Player Control**: Physics-based airplane with quaternion rotation system
- ✅ **Hierarchical Animation**: 3-level aircraft animation system:
  - **Level 1**: Propeller rotation linked to velocity (spins faster with speed)
  - **Level 2**: Rudder animation responding to yaw controls (A/D keys)
  - **Level 3**: Wing flaps animation responding to pitch controls (W/S keys)
- ✅ **Combat System**: Dual projectile firing system with enemy spawning
- ✅ **Shadow Mapping**: Dynamic shadows with depth buffer rendering
- ✅ **Collision Detection**: AABB-based collision system for buildings and enemies

## 🎓 Academic Requirements Met (COMP-371 Assignment 1)

| Requirement | Implementation Details |
|-------------|------------------------|
| **Core Libraries** | OpenGL, GLFW, GLM, GLAD, stb_image.h, Assimp (via Model class) |
| **Interactive Camera** | Third-person orbit camera with 360° mouse look and scroll zoom |
| **Camera Movement** | Relative movement system - camera moves relative to viewing direction |
| **Multiple Textures** | Distinct textures for city, aircraft, sun, and projectiles |
| **Hierarchical Animation** | 3-level deep animation system (propeller → rudder → flaps) |
| **Dynamic Lighting** | Moving sun light source that orbits the scene continuously |
| **Visual Differentiation** | All models and textures are unique (not from tutorials) |

## 🔧 Technical Implementation Details

### Quaternion-Based Flight System
- **Rotation System**: Uses GLM quaternions for smooth, gimbal-lock-free rotation
- **Physics Model**: Velocity-based movement with realistic flight dynamics
- **Control Surfaces**: Animated control surfaces respond to player input
- **Collision Detection**: AABB-based collision system for buildings and terrain

### Shadow Mapping System
- **Depth Buffer**: 2048x2048 shadow map with depth texture
- **Light Space Matrix**: Orthographic projection for directional sunlight
- **Dynamic Updates**: Shadows update as sun orbits the scene
- **Realistic Rendering**: Proper shadow casting for all objects

### Hierarchical Animation System
```cpp
Animation hierarchy structure:
 Plane (root)
   ├── Propeller (rotates based on velocity)
   ├── Rudder (yaws based on A/D input)
   └── Flaps (pitch based on W/S input)
```

## 👥 Development Team

| Name | GitHub | Student ID | Role |
|------|--------|------------|------|
| **Miskat Mahmud** | [@codedsami](https://github.com/codedsami) | 40250110 | SuperVisor, CodeReviewer,  Critical Tester|
| **Sadee Shadman** | [@sadeeshadman](https://github.com/sadeeshadman) | 40236919 | Graphics & Animation |
| **Maharaj Teertha Deb** | [@TeerthaDeb](https://github.com/TeerthaDeb) | 40227747 | Physics & Controls |

## 🎨 Asset Credits

### 3D Models
- **City Environment**: [Casa City Logo](https://sketchfab.com/3d-models/casa-city-logo-b300821c7bed47329f504f1129a26161) by [Digital Urban](https://sketchfab.com/digitalurban)
- **Aircraft**: [Colombian EMB-314 Tucano](https://sketchfab.com/3d-models/colombian-emb-314-tucano-985512d205e6417b981fc009c8ded388) by [42manako](https://sketchfab.com/42manako)
- **Missile**: [AIM-120C AMRAAM](https://sketchfab.com/3d-models/aim-120c-amraam-62b79b0f76e44684ad43adcc2ae3cdb9) by [Planetrix23](https://sketchfab.com/Planetrix23)
- **Sun Model**: Sphere from [MIT WebLogo](https://web.mit.edu/djwendel/www/weblogo/shapes/basic-shapes/sphere/sphere.obj)
- **Exploision** : [Explosion](https://sketchfab.com/3d-models/explosion-46fb54741fbc4cc0854c03b5ef5d0624#download) by [andersdt](https://sketchfab.com/andersdt)

## 📈 Performance Optimization

- **Frustum Culling**: Optimized rendering for large scenes
- **Level of Detail**: Dynamic model complexity based on distance
- **Efficient Shaders**: Optimized GLSL shaders for performance
- **Collision Optimization**: AABB-based collision detection for efficiency

## 🐛 Troubleshooting

### Common Issues
1. **OpenGL Version Error**: Ensure your GPU supports OpenGL 3.3+
2. **Missing Textures**: Verify all model files are in the correct directory
3. **Build Errors**: Check CMake and compiler compatibility
4. **GLM Include Errors**: Ensure GLM library is properly installed

### Debug Mode
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
./ComputerGraphics
```

## 📄 License

This project is developed for educational purposes as part of COMP-371 Computer Graphics course at Concordia University (Summer 2025 - Assignment 1).

## 🎯 Future Enhancements

- [ ] Sound effects and background music
- [ ] Additional aircraft types with different flight characteristics
- [ ] Weather system with dynamic clouds and lighting
- [ ] Particle effects for explosions and contrails
- [ ] Enhanced AI for enemy aircraft behavior

---

<p align="center">
  <i>Built with ❤️ for COMP-371 Computer Graphics - Concordia University</i>
</p>
