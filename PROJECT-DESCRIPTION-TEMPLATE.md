# {NAME-OF-THE-PROJECT}

mi22009 - Ognjen Nešković  
An autumn forest scene at night.
Most of the scene is randomly generated trees, rocks and leaves.
There is a tent and, lantern and a lamp post.
To simulate a gust of wind leaves start falling in large groups at random intervals.
The leaves are animated using a linear / sinusoidal curve shaped like a spiral.
There are two point lights (code has support for n), a directional light and ambient light.

## Controls

W,A,S,D -> Move the camera forward/back/left/right
Arrow keys -> Move a lantern around in the xz plane
Use the mouse to rotate the camera

## Features

### Fundamental:

[x] Model with lighting

[x] Two types of lighting with customizable colors and movement through GUI or ACTIONS

[x] {ACTION_X} --- AFTER_M_SECONDS---Triggers---> {EVENT_A} ---> AFTER_N_SECONDS---Triggers---> {EVENT_B} (falling leaves?)


### Group A:

[x] Frame-buffers with post-processing   
[ ] Instancing  
[x] Off-screen Anti-Aliasing  
[ ] Parallax Mapping

### Group B:
[x] Bloom with the use of HDR  
[ ] Deferred Shading  
[ ] Point Shadows  
[ ] SSAO

### Engine improvement:

[x] Implement an animation controller to animate the position and rotation of a model using an animation curve defined by a set of parameterized linear - sinusoidal function blends.
[x] Implement an emissive map

## Models:
Tent and rocks from: https://poly.pizza/
Trees from: https://www.turbosquid.com/3d-models/low-poly-trees-1431822
Leaves from: https://poly.pizza/m/eWOUE6C8tZT
Lamp post from: https://www.cgtrader.com/free-3d-models/exterior/landmark/stylized-lamp-post-68002130-e7db-4ced-ab67-5ced636a09b9

## Textures
Same sources as the models or made myself.
