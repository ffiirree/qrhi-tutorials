# QRhi Tutorials

## Tutorials

### Getting Started

- [x] [tutorial-01](https://learnopengl.com/Getting-started/Hello-Triangle): Hello Triangle
- [x] [tutorial-02](https://learnopengl.com/Getting-started/Textures): Textures
- [x] [tutorial-03](https://learnopengl.com/Getting-started/Coordinate-Systems): Coordinate Systems
- [x] [tutorial-04](https://learnopengl.com/Getting-started/Camera): Camera

### Lighting

- [x] [tutorial-05](https://learnopengl.com/Lighting/Colors): Colors
- [x] [tutorial-06](https://learnopengl.com/Lighting/Basic-Lighting): Basic Lighting
- [x] [tutorial-07](https://learnopengl.com/Lighting/Materials): Materials
- [x] [tutorial-08](https://learnopengl.com/Lighting/Lighting-maps): Lighting Maps
- [ ] [tutorial-09](https://learnopengl.com/Lighting/Light-casters): Light Casters
- [ ] [tutorial-10](https://learnopengl.com/Lighting/Multiple-lights): Multiple Lights

### Model Loading

- [x] [tutorial-11](https://learnopengl.com/Model-Loading/Assimp): Loading

### Advanced OpenGL

- [ ] [tutorial-12](https://learnopengl.com/Advanced-OpenGL/Depth-testing) Depth Testing
- [ ] [tutorial-13](https://learnopengl.com/Advanced-OpenGL/Stencil-testing) Stencil Testing
- [ ] [tutorial-14](https://learnopengl.com/Advanced-OpenGL/Blending) Blending
- [ ] [tutorial-15](https://learnopengl.com/Advanced-OpenGL/Face-culling) Face culling
- [ ] [tutorial-16](https://learnopengl.com/Advanced-OpenGL/Framebuffers) Framebuffers
- [ ] [tutorial-17](https://learnopengl.com/Advanced-OpenGL/Cubemaps) Cubemaps
- [ ] [tutorial-18](https://learnopengl.com/Advanced-OpenGL/Advanced-Data) Advanced Data
- [ ] [tutorial-19](https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL) Advanced GLSL
- [ ] [tutorial-20](https://learnopengl.com/Advanced-OpenGL/Geometry-Shader) Geometry Shader
- [ ] [tutorial-21](https://learnopengl.com/Advanced-OpenGL/Instancing) Instancing
- [ ] [tutorial-22](https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing) Anti-Aliasing
- [ ] [tutorial-23](https://learnopengl.com/Guest-Articles/2020/OIT/Weighted-Blended) Weighted Blended

### Advanced Lighting

- [ ] [tutorial-24](https://learnopengl.com/Advanced-Lighting/Advanced-Lighting) Advanced Lighting
- [ ] [tutorial-25](https://learnopengl.com/Advanced-Lighting/Gamma-Correction) Gamma Correction
- [ ] [tutorial-26](https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping) Shadow Mapping
- [ ] [tutorial-27](https://learnopengl.com/Advanced-Lighting/Shadows/Point-Shadows) Point Shadows
- [ ] [tutorial-28](https://learnopengl.com/Advanced-Lighting/Normal-Mapping) Normal Mapping
- [ ] [tutorial-29](https://learnopengl.com/Advanced-Lighting/Parallax-Mapping) Parallax Mapping
- [ ] [tutorial-30](https://learnopengl.com/Advanced-Lighting/HDR) HDR
- [ ] [tutorial-31](https://learnopengl.com/Advanced-Lighting/Bloom) Bloom
- [ ] [tutorial-32](https://learnopengl.com/Advanced-Lighting/Deferred-Shading) Deferred Shading
- [ ] [tutorial-33](https://learnopengl.com/Advanced-Lighting/SSAO) SSAO
- [ ] [tutorial-34](https://learnopengl.com/Guest-Articles/2021/CSM) Cascaded Shadow Mapping
- [ ] [tutorial-35](https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom) Phys. Based Bloom
- [ ] [tutorial-36](https://learnopengl.com/Guest-Articles/2022/Area-Lights) Area Lights

### PBR

- [ ] [tutorial-37](https://learnopengl.com/PBR/Lighting) Lighting
- [ ] [tutorial-38](https://learnopengl.com/PBR/IBL/Diffuse-irradiance) Diffuse Irradiance
- [ ] [tutorial-39](https://learnopengl.com/PBR/IBL/Specular-IBL) Specular IBL

### Animation

- [ ] [tutorial-40](https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation) Skeletal Animation

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