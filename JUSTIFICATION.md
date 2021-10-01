# Assignment 1 Justification

If you think any of the questions within the sections are not applicable, please write "N/A".

Application available controls:
- SPACE to jump
- A to walk left
- D to walk right
- TAB to toggle vignette
- F to maximise
- ESC to close application

## Section 1: It runs

The code compiles fine on the two computers that I've tested on (Both are Windows 10). A window of size 900x900 with a favicon should appear when the code is run.

### How have you fulfilled this requirement?

I've fufilled this requirement by creating an openGL window using the chicken3421 lib and also
creating a render program with the verts and frags. Then, inside a while loop that keeps looping until the window is destroyed, my code uses the render program and also fetches all shapes that need to be drawn and then renders it by drawing the shape's vertices and binding it's VAO, VBO and texture. Delta time is used to measure the timing of the program's animations.

Once the window's "should close" flag is set to true (by either pressing esc or closing the window), the while loop is exited and all the shapes, program and shaders are destroyed and the program ends successfully.

I've also tested that this code works and compiles on another laptop by setting up the course's basic environment (following the given "OpenGL CMake setup" guide and running run fetch_deps.sh -G Ninja) and then git cloning this repo and building it, which leads to a successful build and the program working as intended. I mainly use Windows 10.

From the starter code, I have made substantial changes to the main.cpp and added 8 more files which helps with organising the shapes being created. main.cpp is where the OpenGL window is created.

### When your application is run, what should the tutor look for or do to see that you have fulfilled this requirement? 

When the app is run, the tutor should be able to see a window pop up on their desktop with the title
"COMP3421 21T3 Assignment 1 [Minecraft: Goat Simulator]" and dimensions of 900x900. The title should also have a little Minecraft Goat favicon next to it. The tutor can resize the program to fit their computer, however the height can not exceed the width (intentional to keep the shapes in ratio).

### Where in the code should tutors look to see that you've fulfilled this requirement?

The OpenGL window creation that is esential to the code compiling
can be found in the main function located between line 110 to line 130 in main.cpp.
The while loop which renders everything and keeps the program alive is between line 310 to 364 in main.cpp. The destruction of the openGL window when the program ends can be found in main.cpp between 366 to 374.

A picture of what the program should look like when it first starts is included in /res/img/programScreenshot.png.


## Section 2: Visibility & Shaders

The vert.glsl and frag.glsl were edited so that textures can be visible on the shapes and that matrix multiplication can alter the positions of the shape vertices. Those files also allow for the shapes itself to appear.

### How have you fulfilled this requirement?

My program creates a render program with the verts and frags files found in the shaders file.
I've edited the verts file so that verts can read in the texture co-ordinates (used in mapping the textures onto the shapes) and also apply any transformations, rotations or scaling required during the animation.
I've edited the frag file as well so the shapes can sample the colors of their given texture and as such the texture can show up on the shape. I've also run the program on my main computer and my laptop to check for potential shader errors, which I have smoothened out. Textures with transparent pixels are enabled by glEnable(GL_BLEND) and glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA). This enables for more visually interesting textures.

### When your application is run, what should the tutor look for or do to see that you have fulfilled this requirement?

WHen the application is run, the tutor should immediately see the main menu screen, which consists of many shapes that has textures which depict:
* Randomised splashing yellow text
* Clouds
* Minecraft logo
* The goat
* Control schemes on controlling the goat
* Snowflakes falling
* Randomised background scenery (May not show anything as it is randomised. Tutor is encouraged to restart program to get different backgrounds)
* Blocky trees in the distance
* Moon with randomised phase
* The ground that the goat is standing on
* My zID

When the tutor presses any key (Besides F [reserved for changing screen size] and Esc [reserved for exiting the program]), the goat will start walking and more background shapes will be visible as the goat continues to walk. The main menu screen should now scroll off to the side. All these shapes displaying textures are made possible due to the aforementioned editing of the starter vert.glsl and frag.glsl files.

### Where in the code should tutors look to see that you've fulfilled this requirement?

The tutor should look at the vert.glsl and frag.glsl file found in res/shaders/ to see the edits made
The tutor should also see how these files are incorporated into the program by viewing lines 123 to 125 in main.cpp and the main while loop which uses the render program made with the vert and frag at line 318.

## Section 3: Creation and Use of 2D Shapes

This program completes all requirements 3a, 3b, 3c, 3d
2D shapes are used for each individual snowflake, the goat, each background shape, the main menu, the moon etc. etc. Matrix multiplications are run on each shape after a certain time elapses between each loop in the render loop. At the end of the loop, the shapes are redrawn based on the result of the matrix multiplications.

### How have you fulfilled this requirement?

3a and 3b.
When my program is first run, it creates all the required shapes and stores them in the appropriate place inside a struct called scene. This can be seen from line 130 to line 218 in main.cpp, where you'll see shapes being created using the functions found in shapeCreation.hpp and then stored inside "scene sceneObject".

The functions found in shapeCreation.hpp to creates triangles which forms rectangles and squares and stores it inside another struct called shapeObject, which represents an individual shape. During the main while loop, the program calls on a sceneObject to return the shapes that will be visible in the viewport for vao/vbo binding and texture applying.

Multiple shape creation can be best seen for the snowflakes. The snowflake creation can be found from line 186 to line 207 in main.cpp which creates a basic 2D square, stores the square in an array, and then binds a random snow flake texture to it. Other shapes follow similar procedure and can be found in the aforementioned line 130 to line 218 in main.cpp.

3c.
My program uses the glm library to perform matrix transformations.
Matrix transformations is supported by the edited vert.glsl and is applied to the respective shape at line 343 in main.cpp.

Matrix transformations can be found in almost all the shapes visible. A list of matrix transformations (may be missing a few) that cam occur when the program is running:
* The splashing effect of the yellow text at the main menu follows a sin curve in scaling it up and down.
* Each snowflake struct has a random rotation speed, gravity and velocity, which are all used in rotating the shape and translating the shape into different positions to mimic wind and gravity
* While the goat is walking, all shapes (besides the goat) are translated to the left by a bit each frame to simulate the goat walking. The goat can also move on the screen when A, D and SPACE are pressed. The goat jumping follows a derivative curve which translates the goat up and then down after the turning point of the curve.
* Depending on the width and height of the window, the title screen will be scaled down to fit the new width. The moon and clouds will be translated downwards and the zID will be translated upwards.

3d.
I have edited the starter code render loop to make use of delta time. Before the loop starts, the program takes the current epoch time and then at the end of the loop, it takes the current time again and subtracts it from the previous epoch time. The difference is then added to a variable called deltaTime. With each loop the program checks if the deltaTime has exceeded TICKS_TO_SECOND. If it has, then it will call the tick functions inside scene.hpp which will calculate the position of all the shapes in the next frame. Then, all the shapes are then redrawn onto the screen. TICKS_TO_SECOND is then subtracted from deltaTime and the loop repeats.


### When your application is run, what should the tutor look for or do to see that you have fulfilled this requirement?

3a and 3b.
The tutor should look for the many snowflakes that show on the screen as an example for multiple shapes being created. All the scenery elements are all individual shapes with a different texture applied to it. The tutor should then press any key (besides F/Esc) so that the animation begins and more shapes will be generated and rendered onto the window.

3c.
Matrix transformations can be found in all shapes on the screen. An example would be how the snowflakes are animated, as it was all created with matrix transforms. The tutor should press any key to see more translations applied to the scenery so that they scroll to the left to simulate the camera moving to the right. Pressing A will translate the goat to the left, D translates the goat the right, and pressing space applies a subtle rotation matrix and translates the goat up and then down. A more subtle example of matrix transformations that could be missed is resizing the window, which applies the correct transformations so that all important shapes can still be visible on screen.

3d.
The tutor should move the goat around using A and D to best see the render loop being used to animate the goat's walk cycle and jump cycle. The snowflakes can also be seen being rotated and translated with the render loop. If the tutor wants to see a clear example that the render loop is being used for the timing, the tutor can change the external variable TICKS_TO_SECOND on line 54 in main.cpp to change the speed between each frame of the animation. Higher value means that the render loop will need to loop more times before animating the next frame.

### Where in the code should tutors look to see that you've fulfilled this requirement?

3a and 3b
All shape creation functions can be found in shapeCreation.hpp.
The place where all shapes are stored can be found in scene.hpp. This helps when many shapes are being created.
An example of many shapes being created can be found from line 186 to line 207 in main.cpp (Snowflake creation).
All shapes are prepared in the main loop between line 110 to 218 in main.cpp.
All shapes are made visible on the window between line 337 to 346 in main.cpp.

3c.
Matrix transformations are mostly found in function calls called "tick". Tutor can see these functions being called in the render loop between line 325 to line 332 in main.cpp. This section of the code calls sceneObjects.tickAll(), which can be traced to the file scene.hpp which starts at line 177. All functions that has the word "tick" in it in this file will involve matrix transformations applied to shapeObjects.

The actual multiplication of matrix the shape can be found at line 343 in main.cpp, where it takes the stored glm::mat4 and applies it to the gl_Position in the vert.glsl file.

3d.
The render loop can be found in main.cpp from line 304 to line 364 (Including the variables used to calculate the delta time). The loop decides whether to perform matrix transformations on all the shapes (by calling the respective tick functions) between line 325 to 332 in main.cpp.

## Section 4: Use of Textures

This program completes all requirements 4a, 4b, 4c
Many textures (evidenced by the abundance .png files in /res/img/) are used and applied to multiple or one shape. Application alters textures during runtime for the goat shape (for its run animation) and the main menu shpae (for the two little blue squid animation).

### How have you fulfilled this requirement?

4a. and 4b.
To replicate the Minecraft aesthetic, all shapes have a pixel art texture applied to it taken from the original Minecraft game. All textures are created by Jasper "JAPPA" Boerstra. My program loads in a image file using chicken3421::load_image and then turns it into a texture handler similar to tute02. The texture handler is then stored in the shapeObject if that texture is to be applied to that shape. This is how I was able to load in multiple textures and apply it to different shapes

4c.
My application is able to alter a shapes texture by replacing the texture handler in the shapeObject's GLuint textureID with a new texture handler representing another texture. I store all related textureIDs in a GLuint array and then change the texture of a shape by indexing the array accordingly. At the end, all textures and images are destroyed.

### When your application is run, what should the tutor look for or do to see that you have fulfilled this requirement?

4a. and 4b.
The application begins with a plethora of shapes with different textures. If the tutor wants to see more textures, they can either restart the application to get a newly generated scenery, or leave the application running to see different shapes of different textures scroll past the screen. When the program first starts, the tutor should see a flat square with the main menu texture, followed by smaller different shapes that has the goat texture, tree texture, splash text texture, snow flake texture etc.

4c.
The main menu animation with the blue squid (Glow Squid) is achieved by quickly switching out the texture IDs of the main menu shape. The program iterates through an array of GLuint with each cell representing a texture handler for a frame in the animation. During the iteration, it will assign the cell values to the shapes textureID, which will swap the textures in the next frame to a different texture, creating an illusion of movment. The Goat's walk cycle also uses the same technique.

A more subtle texture change would be the background objects. Once the background object reaches offscreen, its texture changes and is then translated back to the beginning. This way, we won't need to create too many shapes.

### Where in the code should tutors look to see that you've fulfilled this requirement?

4a. and 4b.
All texture files can be seen in /res/img. Example of multiple texture creation can be found in scene.hpp between lines 80 to 108 and at the beginning of the main function in main.cpp between lines 138 to line 207. Multiple related textures are stored in arrays, seen for the possible background textures in scene.hpp line 83 to 94.

The frames for the goat walk animation can be seen loaded in at line 38 to 48 in goatObject.hpp.

The main menu frames can be seen between line 40 to 53 in mainMenuScene.hpp.

4c.
Related textures that are apart of the same frame can be found in /res/img/ starting with the same word but ending in a different number which indicates its frame. Texture swapping can be found in the goat's animation in goatObject.hpp between lines 56 to 62 where we can see the textureID changing to one of the textureID stored in goatAnimationFrames "goatShape.textureID = goatAnimationFrames[currFrame];" The texture swapping of the main menu animation can be found in mainMenuScene.hpp between lines 59 to 91.

The subtle texture changes to the background objects can be found in scene.hpp at the end of functions tickFgObjA(), tickFgObjB() and tickParallax(), where it finds a random index in the array which stores the texture handlers and uses the handler stored at that index as the new texture.

### Section 5: Subjective Mark

This project is a homage to Notch Persson's Minecraft, which is currently owned by Mojang, a subsidiary of Microsoft. My project  and borrows a lot of textures from that game. All textures used in this project are taken from Minecraft's lead pixel artist Jasper "JAPPA" Boerstra and are assembled together into individual files by me. The aim of this project for me was to try and recreate as much as possible from Minecraft. The randomly placed background scenery was my attempt at recreating the randomly generated worlds of Minecraft, the splash yellow text was also my attempt at recreating the yellow text found beneath the title screen whenever the player starts up Minecraft. The message used for the splash text were all suggested by my friends.

Each background scenery also represents something that can appear in the world of Minecraft, such as the ruined Nether Portal and the Igloo. I have also chosen the Goat animal as it is one of the latest animals added to Minecraft as of 10/1/2021 and has been a meme around the community.

The 2D parallax scenery was also inspired by Hollow Knight's use of parallax images, and I've used it here to try and capture the 3D space of Minecraft within a 2D space limitation.

### How have you fulfilled this requirement?

I had the texture filters to be GL_NEAREST to try and emulate the pixel art aesthetic of Minecraft, which in turn evokes nostalgia on old school games which use also uses pixel art.

The transitions of background scenery at different rates to simulate a 3D space.

Many varying random elements in the animation (snowflakes, background scenery, moon phases, splash text) to try and make every run of the animation unique.

Responsive control scheme to mimic the controls in Minecraft (Being able to press space and arrow keys at the same time to combine movement/matrix transformations)

The abundant breezy snow flakes which follow a sin curve to try and evoke a sense of tranquility in the audience.


## Use of External Libraries

### What does the library do?

I used the OpenGL Mathematics Library (glm)

### Why did you decide to use it?

To simplify the matrix transformations, rotations and scaling by calling the functions glm::rotate, glm::translate and glm::scale.