# QRhi Tutorials

## Tutorials

### Getting Started

- [x] [tutorial-01](https://learnopengl.com/Getting-started/Hello-Triangle): Hello Triangle
- [x] [tutorial-02](https://learnopengl.com/Getting-started/Textures): Textures
- [x] [tutorial-03](https://learnopengl.com/Getting-started/Coordinate-Systems): Coordinate Systems
- [ ] [tutorial-04](https://learnopengl.com/Getting-started/Camera): Camera

### Lighting

- [x] [tutorial-05](https://learnopengl.com/Lighting/Colors): Colors
- [x] [tutorial-06](https://learnopengl.com/Lighting/Basic-Lighting): Basic Lighting
- [x] [tutorial-07](https://learnopengl.com/Lighting/Materials): Materials
- [ ] [tutorial-08](https://learnopengl.com/Lighting/Lighting-maps): Lighting Maps
- [ ] [tutorial-09](https://learnopengl.com/Lighting/Light-casters): Light Casters
- [ ] [tutorial-10](https://learnopengl.com/Lighting/Multiple-lights): Multiple Lights

### Model Loading

- [x] [tutorial-11](https://learnopengl.com/Model-Loading/Assimp): Loading

## Usage

```bash
# compilation
cd qrhi-tutorials && mkdir build && cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Environment

- `C++` :  `C++20`
- `Qt` : `>= 6.8.0`
- `CMake` : `>= 3.22`
- `Assimp` : `>= 5.2.0`

## References

- [LearnOpenGL](https://learnopengl.com/)
- [QRhi](https://doc.qt.io/qt-6/qrhi.html)
- [Modern Graphics Engine Guide](https://italink.github.io/ModernGraphicsEngineGuide/)