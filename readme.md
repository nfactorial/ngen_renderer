nGen Renderer
=============
Rendering library for nGen game engine.

Each rendering library implements the nGen management for device resources and render
operations. This includes the low-level rendering operations for the selected API.

The display library sits on top of the rendering library, the display library is aimed at simplifying
the rendering interface into higher order objects for use by the game itself.

This library is intended to be a low-level wrapper around the rendering API's, abstracted for use by
the higher level libraries supplied as part of nGen.

DEPENDECIES
===========
nGen uses SDL to manage system initialization, you must have SDL available on your system in-order for
compilation to succeed. Please see https://www.libsdl.org/

OpenGL
======
The OpenGL library contains a suitable implementation that uses OpenGL to render the game scene.

DirectX
=======
This folder contains a suitable implementation that uses DirectX to render the game scene. Suitable
for use on Windows platforms as well as some consoles.

GLES
====
Contains the render library implementation that uses OpenGL ES as a rendering platform, suitable for
use on mobile phones and tablets.