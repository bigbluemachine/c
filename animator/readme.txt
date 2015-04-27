Build:

  gcc driver.c -mwindows glut32.lib -lfreeglut -lopengl32 -lglu32 -o animator

In brief, this utility has two major features: customizable input files, and
interactive keyframe editing. The following keyboard commands are available:

Movie controls

  p         toggle pause/play
  [, ]      decrement/increment current frame (hold shift for greater change)
  -, +      decrease/increase FPS (hold shift for greater change)

Information controls

  space     toggle text display
  i         toggle detailed info display

Selection controls

  s         toggle selection
  g         select previous part
  h         select next part

Editing controls

  e         switch editing mode
  r         switch editing resolution
  x, y, z   increase respective position or angle (hold shift to decrease)

I/O controls

  F1  - exit
  F5  - reload file
  F8  - save file
  F12 - capture state

Proposed additional features:

  Primitives other than the ellipsoid
  Interactive editing of definitions
  Further camera manipulations such as panning and zooming
  Edit keyframes of individual parts (as demonstrated in the back flip file)
