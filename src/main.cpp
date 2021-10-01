// C++17 doesn't have std::pi
#define _USE_MATH_DEFINES
#include <cmath>

#include <cstdlib>
#include <chrono>
#include <list>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <chicken3421/chicken3421.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <iostream>
#include "helperFunctions.hpp"
#include "vert.hpp"
#include "shapeObject.hpp"
#include "goatObject.hpp"
#include "snowFlakeObject.hpp"
#include "mainMenuScene.hpp"
#include "scene.hpp"
#include "shapeCreation.hpp"

//////////////////////
// PROGRAM SETTINGS //
//////////////////////

// Application window settings:
const char         *APP_TITLE         = "COMP3421 21T3 Assignment 1 [Minecraft: Goat Simulator]";
extern const int   SCREEN_WIDTH       = 900;   // Screen width of the program
extern const int   SCREEN_HEIGHT      = 900;   // Screen height of the program
extern const bool  SCREENSAVER_MODE   = false; // Whether to compile this program as a screensaver or not
extern const int   UNDEF_MOUSE_POS    = -1;    // The value to represent an undefined mouse position
extern const int   TOTAL_KEYS         = 350;   // The total amount of possible key presses

// Main menu settings
extern const int   MAIN_MENU_TIMER    = 600;   // How log the main menu lasts on the window
extern const int   MAX_FRAMES_MENU    = 119;   // How long the main menu animation lasts for (Must be a multiple of two - 1)
extern const int   AUTO_SKIP_TIME     = 2800;  // How long until the animation automatically starts (2.8k = 1 minute)
extern const int   TOTAL_SPL_TEX      = 20;    // How many variants of splash text
extern const float SPLASH_SCALE       = 0.30;  // Scale of splash text
extern const float SPLASH_ROT         = 12.5f; // How much the splash text is askewed
extern const float SPLASH_POS_X       = 0.69;  // X position of splash text
extern const float SPLASH_POS_Y       = 0.55;  // Y position of splash text
extern const float SPLASH_AMPLITUDE   = 0.07;  // How violent the splash text bobs

// Speed of animation:
extern const int   TICKS_TO_SECOND    = 20;    // Lower value = faster; Higher value = slower;

// Ground settings:
extern const float SCROLL_SPEED       = 0.01;  // How fast the objects scroll by the screen
extern const float GROUND_POS_Y       = -0.8;  // Y position of the ground
extern const float GROUND_SCALE       = 0.2;   // Dimension size of ground
extern const int   GROUND_TILES       = 20;    // How many tiles are in the shape

// Background object settings:
extern const int   FG_TIMER           = 500;   // How long foreground objects last on the screen
extern const int   PARALLAX_TIMER     = 4;     // How long background objects last = PARALLAX_TIMER * FG_TIMER
extern const float FG_POS_Y           = 0.9;   // Y position of the foreground objects
extern const float FG_SCALE           = 1.5;   // Size of foreground objects
extern const float PARALLAX_POS_Y     = 0.4;   // Y position of background objects
extern const float PARALLAX_POS_X     = 2.0;   // X position of background objects
extern const int   MAX_FRAMES_SKY     = 2;     // How many frames the night sky has
extern const float MOON_SCALE         = 0.2;   // Size of the Moon
extern const float MOON_POS_XY        = 0.6;   // X and Y position of the Moon
extern const int   TOTAL_MOON_TEX     = 8;     // Total possible Moon phases
extern const float FG_COOLDOWN        = 120;   // How long inbetween spawning foreground objects

// Goat settings:
extern const int   ANIM_FRAME_LEN     = 4;     // The length of the frames of the Goat's animation
extern const int   AIRBORNE_LEN_MAX   = 15;    // Total time the Goat is in the air
extern const int   MAX_FRAMES_GOAT    = 8;     // Total frames of the Goat's animation
extern const float GOAT_POS_Y         = -0.275;// Y position of Goat
extern const float GOAT_SCALE         = 0.4;   // Scale of the Goat
extern const float GOAT_JUMP_ROT      = 5.0f;  // How many degrees the Goat rotates in its jump
extern const float GOAT_WALK_SPEED    = 0.01;  // How far the Goat moves when "D" is pressed
extern const int   GOAT_WALK_RANGE    = 1;     // How far from the centre of the screen the Goat can move to

// Snow flake settings
extern const int   TOTAL_SF_TEX       = 4;     // How many possible textures a snowflake can be
extern const int   FLAKE_TOTAL        = 900;   // How many flakes are present. MUST BE AN EVEN NUMBER
extern const int   FLAKE_TIMER        = 1600;  // How long the flakes last on the screen
extern const float FLAKE_ROT_SPEED    = 5.0f;  // How many degrees the flakes rotate
extern const int   FLAKE_CHANCE       = 2;     // The chance a snow flake spawns every tick (1 / FLAKE_CHANCE)
extern const float FLAKE_SCALE        = 0.03;  // How big the flake is
extern const float FLAKE_POS_Y        = 1.02;  // X position of where the flakes spawn
extern const float W_AMPLITUDE        = 0.01;  // Wind's amplitude, controls how crazy the wind is
extern const int   W_COEFFICIENT      = 8;     // Controls how short each wind bursts are. Has pi as the numerator
extern const float W_VERT_SHIFT       = 0.005; // Controls how effective each wind bursts are

// Background and Parallax Settings:
extern const int   BG_SPAWN_CHANCE    = 400;   // The chance of background spawning (1 / BG_SPAWN_CHANCE)
extern const int   TOTAL_FG_TEX       = 12;    // The total amount of possible foreground textures
extern const int   TOTAL_P_TEX        = 4;     // The total amount of possible parallax textures
extern const float TREE_LOOP_POS_Y    = 0.4;   // Y position of the looping trees in the background

// Dynamic global variables
bool gameState = false;        // Determines if the main menu should scroll or not
double initialMousePosX = UNDEF_MOUSE_POS, initialMousePosY = UNDEF_MOUSE_POS;

/**
 * Main function which controls everything
 */
int main() {
    printMessageTime();
    std::cout << "Program start\n";

    // Creates opengl window and sets the window icon
    GLFWwindow *win = chicken3421::make_opengl_window(SCREEN_WIDTH, SCREEN_HEIGHT, APP_TITLE);
    chicken3421::image_t goatIcon = makeImage("res/img/goatFavicon.png");
    GLFWimage faviconImg = {goatIcon.width, goatIcon.height, (unsigned char *)goatIcon.data};
    glfwSetWindowIcon(win, 1, &faviconImg);

    // Flips the textures
    stbi_set_flip_vertically_on_load(true);

    GLuint vertShader = chicken3421::make_shader("res/shaders/vert.glsl", GL_VERTEX_SHADER);
    GLuint fragShader = chicken3421::make_shader("res/shaders/frag.glsl", GL_FRAGMENT_SHADER);
    GLuint renderProgram = chicken3421::make_program(vertShader, fragShader);

    scene sceneObjects;
    glfwSetWindowUserPointer(win, &sceneObjects);

    //////////////////
    // Shape making //
    //////////////////

    // Makes the animation frames for the main menu
    // If file is not found, use the first frame as the animation

    // Creating the shape for the splash text
    sceneObjects.mainMenuObj.splashText = createFlatSquare();
    sceneObjects.mainMenuObj.mainMenu = createFlatSquare();
    sceneObjects.mainMenuObj.setupMenu();

    // Creating the focal point Goat and set a pointer to that goat
    goatObject goatObj = createGoat();
    goatObj.nextFrame();
    sceneObjects.goat = goatObj;

    // Creating the shape for the background
    shapeObject backgroundObj = createFlatSquare();
    sceneObjects.background = backgroundObj;

    // Creating the shape for the clouds
    shapeObject cloudsObj = createFlatSquare();
    cloudsObj.textureID = makeTexture("res/img/cloudsTexture.png");
    sceneObjects.clouds = cloudsObj;

    // Creating the shape for the overlay
    shapeObject overlayObj = createFlatSquare();
    overlayObj.textureID = makeTexture("res/img/overlay.png");
    sceneObjects.overlay = overlayObj;

    // Creating the shape for the moon. The moon has a random phase
    // for each time the code runs
    shapeObject moonObj = createFlatSquare();
    moonObj.textureID = makeTexture(appendRdmNum("res/img/moon/moon_", 1, TOTAL_MOON_TEX));
    sceneObjects.moon = moonObj;
    sceneObjects.adjustPositions(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Creating the shape for the ground
    shapeObject groundSceneObj = createGround(GROUND_TILES);
    groundSceneObj.textureID = makeTexture("res/img/snowyGroundTexture.png");
    sceneObjects.ground = groundSceneObj;

    // Creating the shape for the back mountains
    shapeObject parallaxObj = createFlatSquare();
    parallaxObj.trans = glm::translate(parallaxObj.trans, glm::vec3(PARALLAX_POS_X, PARALLAX_POS_Y, 0.0));
    sceneObjects.parallaxObj = parallaxObj;

    // Creating the shape for the background elements
    sceneObjects.foregroundObjA = createBackgroundElement();
    sceneObjects.foregroundObjB = createBackgroundElement();
    sceneObjects.parallaxLoopObj = createParallaxLoop();
    sceneObjects.parallaxLoopObj.textureID = makeTexture("res/img/treeParallax.png");

    // Creating the snowflakes, with a random texture applied to each snow flake
    GLuint variantA = makeTexture("res/img/snowFlakeATexture.png");
    GLuint variantB = makeTexture("res/img/snowFlakeBTexture.png");
    GLuint variantC = makeTexture("res/img/snowFlakeCTexture.png");
    GLuint variantD = makeTexture("res/img/snowFlakeDTexture.png");
    for (int i = 0; i < FLAKE_TOTAL; i++) {
        sceneObjects.snowFlakes[i] = createSnowFlake();
        switch (rand() % TOTAL_SF_TEX) {
            case 0:
                sceneObjects.snowFlakes[i].snowFlakeShape.textureID = variantA;
                break;
            case 1:
                sceneObjects.snowFlakes[i].snowFlakeShape.textureID = variantB;
                break;
            case 2:
                sceneObjects.snowFlakes[i].snowFlakeShape.textureID = variantC;
                break;
            case 3:
                sceneObjects.snowFlakes[i].snowFlakeShape.textureID = variantD;
                break;
        }
    }
    // Tick a few frames ahead so that the first rendered frame has a chance
    // to not look so empty
    for (int i = 0; i < 450; i++) {
        if (rand() % 3 == 0) {
            sceneObjects.tickSnowFlake(gameState);
        }
        sceneObjects.tickFgObjA();
        sceneObjects.tickFgObjB();
        sceneObjects.tickParallax();
    }

    //////////////////////////
    // Setting up callbacks //
    //////////////////////////

    // Window size
    glfwSetWindowSizeCallback(win, [](GLFWwindow* window, int width, int height) {
        scene *sceneObjects = (scene *) glfwGetWindowUserPointer(window);
        // Keeps the window at a 1:1 width:height ratio
        printMessageTime();
        std::cout << "Window size change detected, adjusting viewport\n";
        if (height < width) {
            // Adjusts so that the scene still fits the rectangular screen naturally
            glViewport(0, (height - width) / 2, width, width);
        } else if (height > width) {
            // Makes sure that the window is not a vertical rectangle
            glfwSetWindowSize(window, height, height);
        } else {
            glViewport(0, 0, height, height);
        }
        sceneObjects->adjustPositions(width, height);
        
    });

    // Sets callbacks for key presses. Current control scheme is:
    // A for left, D for right, Space to jump, Tab to toggle vignette, Esc to close program
    // Left Ctrl to toggle between maximised and minimised window
    glfwSetKeyCallback(win, [](GLFWwindow *win, int key, int scancode, int action, int mods) {

        // Grabs the window scene
        scene *sceneObjects = (scene *) glfwGetWindowUserPointer(win);

        // Enables and disables if key is pressed
        if (action == GLFW_RELEASE) {
            sceneObjects->isKeyPressed[key] = false;
        } else if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            if (key != GLFW_KEY_LEFT_CONTROL && !gameState) {
                // Enables gameState if any key is pressed besides Left Ctrl
                gameState = true;
            }
            sceneObjects->isKeyPressed[key] = true;
        }

    });

    // If screen saver mode is enabled, make the program full screen and detect for any mouse movements
    if (SCREENSAVER_MODE) {
        // Sets to full screen mode
        GLFWmonitor *winMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* vidMode = glfwGetVideoMode(winMonitor);
        glfwSetWindowMonitor(win, winMonitor, 0, 0, vidMode->width, vidMode->height, vidMode->refreshRate);

        // Detects if cursor moves
        glfwSetCursorPosCallback(win, [](GLFWwindow *win,  double xPos, double yPos) {
            if (initialMousePosX == UNDEF_MOUSE_POS || initialMousePosY == UNDEF_MOUSE_POS) {
                initialMousePosY = yPos; // initialMousePosX is a global variable
                initialMousePosX = xPos; // initialMousePosY is a global variable
            } else if (initialMousePosX != xPos || initialMousePosY != yPos){
                // Closes window when it detects mouse movement
                glfwSetWindowShouldClose(win, GLFW_TRUE);
            }
        });

        // Hides cursor
        glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        printMessageTime();
        std::cout << "Experimental screensaver mode enabled\n";
    }

    /////////////////////////////////////////////////
    // While loop to control renders and animation //
    /////////////////////////////////////////////////

    // Gets the transform uniform location
    GLint transformLoc = glGetUniformLocation(renderProgram, "transform");
    chicken3421::expect(transformLoc != -1, "Unknown uniform variable name");

    // Variables to manage when to animate a frame for each scene object
    using namespace std::chrono;
    std::list<shapeObject> shapeList;     // List which will contain the shapes to be drawn
    std::list<shapeObject>::iterator obj; // An iterator of the above list

    // Time related variables
    long startLoop = time_point_cast<milliseconds>(system_clock::now()).time_since_epoch().count();
    long endLoop = 0, deltaTime = TICKS_TO_SECOND;
    int autoSkipTimer = AUTO_SKIP_TIME;

    while (!glfwWindowShouldClose(win)) {
        
        if (!gameState && autoSkipTimer == 0) {
            gameState = true;
            printMessageTime();
            std::cout << "Auto-skipped main menu\n";
        }

        glUseProgram(renderProgram);
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 0, 0, 1);

        // When deltaTime exceeds TICKS_TO_SECOND, animate each scene object
        if (deltaTime >= TICKS_TO_SECOND) {
            if (!gameState && autoSkipTimer > 0) {
                autoSkipTimer -= 1;
            }
            deltaTime -= TICKS_TO_SECOND;
            sceneObjects.tickAll(gameState);
            sceneObjects.checkKeyInputs(win);
        }

        // Draw all objects in the sceneObjects list
        shapeList = sceneObjects.getAllObjects();

        for (obj = shapeList.begin(); obj != shapeList.end(); obj++) {
            // Renders the shape that the iterator is pointing at
            glBindVertexArray(obj->vao);
            glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);
            glBindTexture(GL_TEXTURE_2D, obj->textureID);
            // Applies the transformations onto the matrices
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(obj->trans * obj->rot * obj->scale));

            glDrawArrays(GL_TRIANGLES, 0, obj->vertices.size());
        }
        shapeList.clear();

        // Resets vertex arrays and buffers
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);

        glfwSwapBuffers(win);

        // Calculates the time elapsed between start and end of loop and adds it
        // to the deltaTime.
        endLoop = time_point_cast<milliseconds>(system_clock::now()).time_since_epoch().count();
        deltaTime += endLoop - startLoop;
        // Caps deltaTime at 3 times TICKS_TO_SECOND to skip a few frames if the system is struggling
        deltaTime %= 3 * TICKS_TO_SECOND;
        startLoop = endLoop;
    }

    // Tearing down program once closed
    printMessageTime();
    std::cout << "Closing program\n";
    glfwDestroyWindow(win);
    chicken3421::delete_program(renderProgram);
    chicken3421::delete_shader(fragShader);
    chicken3421::delete_shader(vertShader);
    sceneObjects.deleteAllShapes();
    deleteAllTexImg();

    return EXIT_SUCCESS;
}