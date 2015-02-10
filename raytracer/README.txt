Outputs a ray-traced scene.

Build:

  gcc driver.c -o raytracer

With warnings and optimization:

  gcc driver.c -Wall -O2 -s -o raytracer

Run:

  ./raytracer in_file [-d depth] [-o out_file]

where in_file is the name of the input file. Optionally you may specify depth,
the number of times to reflect a ray. This is 0 by default. A reasonable amount
for most images would be 10. You may also specify out_file, the name of the
output file. By default this is "out.bmp". NOTE: The output is a BMP file.
Please add the ".bmp" extension.

The input file should contain specifications of the scene to be rendered, in a
syntax similar to those of the included sample files (which I'm too lazy to
explain in detail now). The objects are placed using model-view matrix
transformations similar to those used in OpenGL: rotate, translate, and scale.
