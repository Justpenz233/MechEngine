# Notes at rendering module

[Notion link](https://smiling-calcium-8e1.notion.site/Structure-7cae99d9d65b423b810893ebc6b62c0f?pvs=4)


## About luisa compute:

### How to identify one is GPU code or CPU code:

GPU code/structure/class should use snake_case, like python.  
CPU code should use UpperCamelCase, like UE.
The bridge, which means will carry data in CPU, and used in GPU as Buffer<T>, shoule 
use lowerCamelCase. 

The pure GPU code, suck like ray_intersection, should use T, and const T& as parameter in shader.  
The bridge, like materialData and cameraData, should use Var<T> and Expr<T> as parameter in shader. Corresponding to T and const T& in CPU code.

### Shader variant: When to use CPU/GPU condition and data

Pure GPU data and code will create only one shader, which means compile only one time. For example:
```cpp
ImageView<Float3> diffuse_texture; 
Float2 uv_bias;
Float3 get_color(const Float2& uv) { return diffuse_texture.sample(uv + uv_bias); }
```
But if we use cpu data here, every time the data change, a new shader will be compiled. For example:
```cpp
ImageView<Float3> diffuse_texture; 
float2 uv_bias; // everytime this value change, a new shader will be compiled
Float3 get_color(const Float2& uv) { return diffuse_texture.sample(uv + uv_bias); }
```
This is useful, since we could use flags to create different material, each material is a shader variant.

We can also access cpu class and use complicated logic when create shader variant.

But usually, the frequently changed data should be put in buffer, otherwise every time the data change, a new shader will be compiled.   
For example, even thought we only support one camera now, but the transform and FOV of camera will change every frame,   
so we should put these data in buffer, rather than access directly in shader.