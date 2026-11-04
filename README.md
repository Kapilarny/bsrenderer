# bsrenderer

A very bad software renderer, with minimal dependencies to maximize the learning factor.

## about it

- renders it all on the CPU :)
- parallelizes the rendering work using thread pool and a "cell-based" rendering system
- it can load in basic 3d models using the Waveform format (.obj files)
- has an "opengl like" shader system which allows for writing vertex and fragment shaders
- renders 3d text too
- has windows and linux build

## performance

On my machine (Intel Core Ultra 5 225F (6+4) @ 4.40 GHz) the benchmark scene
with 100 model instances visible on the screen runs in ~22ms per frame (~45fps), dropping to
~10ms (100fps) when less geometry is visible on screen.

Mileage may vary depending on core count performance since the renderer heavily utilizes
as many cores at it can get.

f.e. artificial limit on thread count on my machine to 1 increased frame time to ~77ms (13 fps)

## todos

- [ ] optimize using SIMD (could be fun)
- [ ] optimize not using SIMD (there's some questionable code there)
- [ ] add wayland cleanups xdd

## dependencies

- [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)
- [wayland](https://wayland.freedesktop.org/) (on Linux)