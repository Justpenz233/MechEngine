# Mech Engine ⚙
Mech Engine is a C++ simple game engine, especially designed for geometry processing scientific researcher, originate from [CGL@SUTD](https://sutd-cgl.github.io/).  

This engine aims to provide unified interfaces for fast and unified acessing infrastructure of geometry processing, for example, mesh processing within StaticMesh.

Beyond that, we also notice the demands for fast iteration and parameters tuning.  Instead of manually write a ImGUI widget and update inside UI, you can simply use reflection system and register a callback function triggering when the parameter be modified.

Also,we can statisfy various demands for visualization of your geometry. So we are using Luisa-Compute as our renderer backend, providing many **built-in materials** such as transparent and outline rendering.

> Our engine is a subset of Unreal Engine. We refer serval concepts from UE. So if you are familiar with UE you will get start quickly, or vice versa.

# Getting started 🚀
Mech Engine had properly setup all the dependence either by git submodule or cmake fetch. So you donnot need to panic.

This repo should only be a submod or subdirectory of your project. For a Cmake template project, please see:
> https://github.com/Justpenz233/MechEngineExamples

We suggest you cloning the template project and rename folders or project name as you need, cause we have several necessary cmake variable needed for our engine to run.

## Basic concept

* Object is the base class for all classes. Provides reflection, and garbage collection functionality. Do remember to derive from Object.

* World is your stage to show your Actor. You can also build many many components and attach to your Actor. This shoule be the place where you can write your code.

* Override the PostEdit() function, where you can make some change when you modifying a property of your Object.

* We are going to use lua as a World script, so you don't need to recompile anymore.

# Supported features 🔥

## Material

* Phong and PBR material.
* Support alpha blending and transparent.
* Outline and wireframes rendering.
* TODO: Catoness shadow.
* TODO: Real time path tracing.

## Editor

* View control widget.
* Object transform control widget.
* TODO: Ground

## Debug tools

* Auto saved log.
* Draw lines or circles in your world.(see World.DebugDrawXXXX)
* TODO: a sequencer which can replay your object motions.