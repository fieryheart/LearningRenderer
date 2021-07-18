
# 渲染结果
### 仅加载纹理
<div align=center>
<img width="300" height="300" src="./examples/TexShader/Marry_2400*2400.png"/>
</div>
<!-- ![avatar](./examples/TexShader/Marry_2400*2400.png) -->  


### Blinn Phong 模型 (平行光)
<div align=center>
<img width="300" height="300" src="./examples/PhongShader/Marry_2400*2400.png"/>
</div>


### 硬阴影
<div align=center>
<img width="300" height="300" src="./examples/ShadowShader/Marry_2400*2400.png"/>
</div>

深度图
<div align=center>
<img width="300" height="300" src="./examples/ShadowShader/Marry_depth_2400*2400.png"/>
</div>

### SSAO
<div align=center>
<img width="300" height="300" src="./examples/SSAO/SSAO_800*800.png"/>
<p>800*800</p>
<p>采样数：像素点\*方向(8)*步数(1000)</p>
<p>pow系数：50</p>
</div>

<div align=center>
<img width="300" height="300" src="./examples/SSAO/SSAO_2400*2400.png"/>
<p>2400*2400</p>
<p>采样数：像素点\*方向(8)*步数(1000)</p>
<p>pow系数：50</p>
</div>


# 项目结构

- Labs  
  - lab_tex: 纹理加载
  - lab_phong: blinn phong 渲染
  - lab_shadow: two passes hard shadow 渲染
  - lab_SSAO: SSAO 渲染

- QGL
  - pieces.h
    - Frame: 帧数据
    - Zbuffer: Zbuffer结构
    - Sample2D: 纹理采样结构
    - Timer: 计时器结构
    - Log: debug输出
  - types.h
    - ComType | MapType | ModelType | MatrialType

- Models
  - model.h
    - StrangeModel: .obj模型文件
    - BuilinModel

- Rasterizer  
光栅化渲染器

- Shaders (因为只有main引用这些文件，所以都用.h文件来保存)
  - TexShader.h
  - PhongShader.h

- PathTracing
  - PathTracing.h

- Geometry
  - geometry.h
    - Vec2<> | Vec3<> | Vec4<> | Matrix<>

- Utils
  - stb_image : 读取存储图片(https://github.com/nothings/stb)