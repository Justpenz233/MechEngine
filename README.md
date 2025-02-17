# Mech Engine ⚙
![Front](https://github.com/user-attachments/assets/c77cb739-a7e4-448f-91b3-e9abdfc58636)

Mech Engine is a lightweight C++ game engine, especially for scientific research of geometry processing.  

This engine is designed to:
* One stop from experiment to result rendering.
* Provide unified interfaces for efficient **geometry processing**, such as mesh manipulation via the StaticMesh module.
* Offer powerful debugging tools for mesh visualization and inspection.
* Enable fast iteration and parameter tuning using a **reflection system**, eliminating the need to manually write and bind ImGUI widgets.
* Provide **built-in materials** like transparent rendering and outlines, powered by Luisa-Compute as the rendering backend.


> Mech Engine adopts several concepts from Unreal Engine (UE). You get started quickly with Mech Engine if familiar with UE, and vice versa.

# Getting started 🚀
Mech Engine conveniently manages all dependencies via Git submodules or CMake FetchContent. You can skip the headaches and dive right in!

This repo should only be a submod or subdirectory of your project. For a Cmake template project, please see:
> https://github.com/Justpenz233/MechEngineExamples

## Steps to Get Started:
* Clone the template project.
* Rename folders and update the project name to suit your needs.
* Configure the required CMake variables. It’s straightforward.

## Basic concept
* Integrated CGAL, libigl, and libpmp, simplify your geometry processing tasks.
* `Object`: The base class for all entities, providing reflection and garbage collection functionality. Derive your custom classes from Object to unlock these features.
* `World`: The main stage where Actor instances and their components interact. Write your logic here and build your simulation or application.
* `PostEdit()`: Override this function to define behavior triggered by property modifications in your `Object`.
* We are going to use lua as a World script, so you don't need to recompile anymore.

# Supported features 🔥

## Learning
*  Reference paper are provided for each feature when possible. You can find the paper in the corresponding function!
*  Rendering techniques(shaders) are implemented in C++ thanks to the Luisa-Compute. Enjoy fully IDE-supported shader exploration!

## Framework
* Logger system: Automatically saves logs to time-stamped files.
* Config system: Load and save configuration files. Fast fine tune your parameters!
* TODO: Lua script system. Hot reload and rerun your experiment!


## Material
* Phong and PBR material.
* Support alpha blending and transparent.
* Outline and wireframes rendering.
* TODO: Catoness shadow.

## Rendering
* A path tracer with SVGF denoising implemented.
* A hybrid renderer with compute shader based rasterizer.

## Editor
* View control widget.
* Object transform control widget.
* TODO: Ground grid.

## Debug tools
* Auto saved log.
* Draw lines or circles in your world.(see World.DebugDrawXXXX)
* TODO: a sequencer which can replay your object motions.
