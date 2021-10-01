/**
 * File contains scene struct, which encompasses all shapes to be rendered in the program.
 * Struct also contains all the currently pressed keyboard keys
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <chicken3421/chicken3421.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <iostream>

extern const bool SCREENSAVER_MODE;
extern const float FG_COOLDOWN;
extern const float FG_POS_Y;
extern const float FG_SCALE;
extern const float FLAKE_POS_Y;
extern const float FLAKE_SCALE;
extern const float GROUND_POS_Y;
extern const float GROUND_SCALE;
extern const float MOON_POS_XY;
extern const float MOON_SCALE;
extern const float PARALLAX_POS_X;
extern const float PARALLAX_POS_Y;
extern const float TREE_LOOP_POS_Y;
extern const float W_AMPLITUDE;
extern const float W_VERT_SHIFT;
extern const int BG_SPAWN_CHANCE;
extern const int FG_TIMER;
extern const int FLAKE_CHANCE;
extern const int FLAKE_TIMER;
extern const int FLAKE_TOTAL;
extern const int MAX_FRAMES_SKY;
extern const int PARALLAX_TIMER;
extern const int TOTAL_FG_TEX;
extern const int TOTAL_KEYS;
extern const int TOTAL_P_TEX;
extern const int W_COEFFICIENT;

bool enableOverlay = true;

struct scene {
    mainMenuScene mainMenuObj;
    shapeObject overlay, background;
    shapeObject moon;
    shapeObject clouds;
    shapeObject ground;
    shapeObject foregroundObjA;
    shapeObject foregroundObjB;
    shapeObject parallaxObj;
    shapeObject parallaxLoopObj;
    snowFlakeObject (*snowFlakes) = new snowFlakeObject[FLAKE_TOTAL];
    goatObject goat;
    bool (*isKeyPressed) = new bool[TOTAL_KEYS];

private:
    float translatedGroundPos = 0, translatedParallaxLoopPos = 0;

    int fgObjATimer = FG_TIMER;
    int fgObjBTimer = FG_TIMER;
    int parallaxTimer = PARALLAX_TIMER * FG_TIMER;
    bool fgObjASpawned = false;
    bool fgObjBSpawned = false;
    bool pallxSpawned = false;
    int coolDownTimer = 0;
    float sinCurveX = 0;

    GLuint (*possibleTexID) = new GLuint[TOTAL_FG_TEX];
    GLuint (*possibleParaTexID) = new GLuint[TOTAL_P_TEX];
    GLuint skyAnimationFrames[2];

public:
    scene() {
        // Loading in all possible textures

        possibleTexID[0] = makeTexture("res/img/treeATexture.png");
        possibleTexID[1] = makeTexture("res/img/treeBTexture.png");
        possibleTexID[2] = makeTexture("res/img/snowGolemATexture.png");
        possibleTexID[3] = makeTexture("res/img/snowGolemBTexture.png");
        possibleTexID[4] = makeTexture("res/img/mossyPileTexture.png");
        possibleTexID[5] = makeTexture("res/img/berryBushesATexture.png");
        possibleTexID[6] = makeTexture("res/img/berryBushesBTexture.png");
        possibleTexID[7] = makeTexture("res/img/plainGrassATexture.png");
        possibleTexID[8] = makeTexture("res/img/plainGrassBTexture.png");
        possibleTexID[9] = makeTexture("res/img/ruinedPortalTexture.png");
        possibleTexID[10] = makeTexture("res/img/iceSpikeATexture.png");
        possibleTexID[11] = makeTexture("res/img/iceSpikeBTexture.png");

        possibleParaTexID[0] = makeTexture("res/img/mountainAParallax.png");
        possibleParaTexID[1] = makeTexture("res/img/mountainBParallax.png");
        possibleParaTexID[2] = makeTexture("res/img/mountainCParallax.png");
        possibleParaTexID[3] = makeTexture("res/img/mountainDParallax.png");

        skyAnimationFrames[0] = makeTexture("res/img/sky/nightSky_1.png");
        skyAnimationFrames[1] = makeTexture("res/img/sky/nightSky_2.png");

        for (int i = 0; i < TOTAL_KEYS; i++) {
            // Initialises all key presses to be false (aka not pressed down)
            isKeyPressed[i] = false;
        }
    }

    /**
     * Tears down all the shapes in the scene
     */
    void deleteAllShapes() {
        mainMenuObj.deleteShapes();
        overlay.deleteSelf();
        background.deleteSelf();
        moon.deleteSelf();
        foregroundObjA.deleteSelf();
        foregroundObjB.deleteSelf();
        ground.deleteSelf();
        parallaxObj.deleteSelf();
        goat.deleteSelf();
        for (int i = 0; i < FLAKE_TOTAL; i++) {
            snowFlakes[i].snowFlakeShape.deleteSelf();
        }
        delete[] snowFlakes;
        delete[] isKeyPressed;
    }

    /**
     * Returns every shape that needs to be rendered, picking from the shapes that are active
     */
    std::list<shapeObject> getAllObjects() {
        std::list<shapeObject> returnList;
        returnList.emplace_back(background);
        returnList.emplace_back(moon);
        returnList.emplace_back(clouds);
        returnList.emplace_back(parallaxLoopObj);
        if (pallxSpawned) {
            returnList.emplace_back(parallaxObj);
        }
        for (int i = 0; i < FLAKE_TOTAL / 2; i++) {
            // Places first half at a lower layer on the list so it appears beneath shapes
            if (snowFlakes[i].isActive) {
                returnList.emplace_back(snowFlakes[i].snowFlakeShape);
            }
        }
        if (fgObjASpawned) {
            returnList.emplace_back(foregroundObjA);
        }
        if (fgObjBSpawned) {
            returnList.emplace_back(foregroundObjB);
        }
        returnList.emplace_back(goat.goatShape);
        for (int i = FLAKE_TOTAL / 2; i < FLAKE_TOTAL; i++) {
            // Places second half at a lower layer on the list so it appears above shapes
            if (snowFlakes[i].isActive) {
                returnList.emplace_back(snowFlakes[i].snowFlakeShape);
            }
        }
        returnList.emplace_back(ground);
        if (enableOverlay) {
            returnList.emplace_back(overlay);
        }
        if (mainMenuObj.mainMenuTimer > 0) {
            returnList.emplace_back(mainMenuObj.mainMenu);
            returnList.emplace_back(mainMenuObj.splashText);
        }
        return returnList;
    }

    /**
     * Animates everything by one frame
     */
    void tickAll(bool gameState) {
        // Tick following objects only when gameState is true
        if (gameState) {
            if (coolDownTimer > 0) {
                // Decreases object spawning cool down timer
                coolDownTimer--;
            }
            goat.nextFrame();
            tickGround();
            tickFgObjA();
            tickFgObjB();
            tickParallax();
        }
        // Tick only main menu if its timer has not expired yet
        if (mainMenuObj.mainMenuTimer > 0) {
            mainMenuObj.tickMainMenu(gameState);
        }
        // Animates the background sky and the snowflakes
        background.textureID = skyAnimationFrames[rand() % MAX_FRAMES_SKY];
        tickSnowFlake(gameState);
    }

    /**
     * Moves the ground to the left and loops it around if needed
     */
    void tickGround() {
        // Ticks the immediate ground
        if (translatedGroundPos < 0) {
            // printMessageTime();
            // std::cout << "Reset ground\n";
            translatedGroundPos = 1;
            ground.resetTransforms();
            ground.trans = glm::translate(ground.trans, glm::vec3(0.0, GROUND_POS_Y, 0.0));
            ground.scale = glm::scale(ground.scale, glm::vec3(GROUND_SCALE, GROUND_SCALE, 0.0));
        }
        ground.trans = glm::translate(ground.trans, glm::vec3(-SCROLL_SPEED, 0.0, 0.0));
        translatedGroundPos -= SCROLL_SPEED / 2;

        // Ticks the tree loop in the background
        if (translatedParallaxLoopPos < 0) {
            // printMessageTime();
            // std::cout << "Reset tree loop\n";
            translatedParallaxLoopPos = 1;
            parallaxLoopObj.resetTransforms();
            parallaxLoopObj.trans = glm::translate(parallaxLoopObj.trans, glm::vec3(0.0, TREE_LOOP_POS_Y, 0.0));
        }
        parallaxLoopObj.trans = glm::translate(parallaxLoopObj.trans, glm::vec3(-SCROLL_SPEED / 10, 0.0, 0.0));
        translatedParallaxLoopPos -= SCROLL_SPEED / 10;

    }

    /**
     * Animates the next frame of foreground object A
     */
    void tickFgObjA() {
        if (fgObjASpawned) {
            foregroundObjA.trans = glm::translate(foregroundObjA.trans, glm::vec3(-SCROLL_SPEED, 0.0, 0.0));
            fgObjATimer -= 1;
            if (fgObjATimer < 0) {
                printMessageTime();
                std::cout << "ObjA has reached the end\n";
                fgObjATimer = FG_TIMER;
                foregroundObjA.resetTransforms();
                foregroundObjA.scale = glm::scale(foregroundObjA.scale, glm::vec3(FG_SCALE, FG_SCALE, 0.0));
                foregroundObjA.trans = glm::translate(foregroundObjA.trans, glm::vec3(2.5, FG_POS_Y, 0.0));
                fgObjASpawned = false;
            }
        } else {
            if (rand() % BG_SPAWN_CHANCE == 0 && coolDownTimer == 0) {
                coolDownTimer = FG_COOLDOWN;
                foregroundObjA.textureID = possibleTexID[rand() % TOTAL_FG_TEX];
                printMessageTime();
                std::cout << "ObjA spawned with texture ID: " << foregroundObjA.textureID << "\n";
                fgObjASpawned = true;
            }
        }
    }

    /**
     * Animates the next frame of foreground object B
     */
    void tickFgObjB() {
        if (fgObjBSpawned) {
            foregroundObjB.trans = glm::translate(foregroundObjB.trans, glm::vec3(-SCROLL_SPEED, 0.0, 0.0));
            fgObjBTimer -= 1;
            if (fgObjBTimer < 0) {
                printMessageTime();
                std::cout << "ObjB has reached the end\n";
                fgObjBTimer = FG_TIMER;
                foregroundObjB.resetTransforms();
                foregroundObjB.scale = glm::scale(foregroundObjB.scale, glm::vec3(FG_SCALE, FG_SCALE, 0.0));
                foregroundObjB.trans = glm::translate(foregroundObjB.trans, glm::vec3(2.5, FG_POS_Y, 0.0));
                fgObjBSpawned = false;
            }
        } else {
            if (rand() % BG_SPAWN_CHANCE == 3 && coolDownTimer == 0) {
                coolDownTimer = FG_COOLDOWN;
                foregroundObjB.textureID = possibleTexID[rand() % TOTAL_FG_TEX];
                printMessageTime();
                std::cout << "ObjB spawned with texture ID: " << foregroundObjB.textureID << "\n";
                fgObjBSpawned = true;
            }
        }
    }

    /**
     * Animates the next frame of background object
     */
    void tickParallax() {
        if (pallxSpawned) {
            parallaxObj.trans = glm::translate(parallaxObj.trans, glm::vec3(-(SCROLL_SPEED / PARALLAX_TIMER), 0.0, 0.0));
            parallaxTimer -= 1;
            if (parallaxTimer < 0) {
                printMessageTime();
                std::cout << "Parallax has reached the end\n";
                parallaxTimer = PARALLAX_TIMER * FG_TIMER;
                parallaxObj.resetTransforms();
                parallaxObj.trans = glm::translate(parallaxObj.trans, glm::vec3(PARALLAX_POS_X, PARALLAX_POS_Y, 0.0));
                pallxSpawned = false;
            }
        } else {
            if (rand() % BG_SPAWN_CHANCE == 0) {
                parallaxObj.textureID = possibleParaTexID[rand() % TOTAL_P_TEX];
                printMessageTime();
                std::cout << "Parallax spawned with texture ID: " << parallaxObj.textureID << "\n";
                pallxSpawned = true;
            }
        }
    }

    /**
     * Calculates the wind influence (sin curve). Used to give the wavy effect on
     * snow flakes
     */
    double windInfluence(bool gameState) {
        double xCoefficient = M_PI / W_COEFFICIENT;
        double period = (2 * M_PI) / xCoefficient;
        // Only use vertical shift if the animation has started
        double vertShift = (gameState) ? W_VERT_SHIFT : 0;
        // The sin curve
        double sinCurveResult = W_AMPLITUDE * glm::sin(xCoefficient * sinCurveX) + vertShift;
        if (sinCurveX > period) {
            // Loops the X axis once it passes the sin curve period
            sinCurveX -= period;
        }

        return sinCurveResult;
    }

    /**
     * Animates the next frame of each individual snowflake
     * and controls whether to spawn one or not
     */
    void tickSnowFlake(bool gameState) {
        if (rand() % FLAKE_CHANCE == 0) {
            // A chance to make a random snow flake active
            snowFlakes[rand() % FLAKE_TOTAL].isActive = true;
        } 
        sinCurveX += 0.1;
        for (int i = 0; i < FLAKE_TOTAL; i++) {
            // Loops through all snowflakes to animate them
            if (snowFlakes[i].isActive) {
                if (snowFlakes[i].flakeLifeTime == 0) {
                    // Reset its lifetimer and its transformations
                    snowFlakes[i].isActive = false;
                    snowFlakes[i].flakeLifeTime = FLAKE_TIMER;
                    snowFlakes[i].snowFlakeShape.resetTransforms();

                    // Scale it back down and translate it to the top of the screen
                    snowFlakes[i].snowFlakeShape.scale = glm::scale(snowFlakes[i].snowFlakeShape.scale, glm::vec3(FLAKE_SCALE, FLAKE_SCALE, 0.0));
                    snowFlakes[i].snowFlakeShape.trans = glm::translate(snowFlakes[i].snowFlakeShape.trans, glm::vec3(0.0, FLAKE_POS_Y, 0.0));
                    
                    // Randomly decide the x co-ordinate of the shape
                    float random = rdmNumGen();
                    if (rand() % 2 == 0) {
                        // Randomly flip the direction of the x co-ordinate
                        random *= -1;
                    }

                    // Widens the range of where the snowflake can spawn and offsets it to the right
                    random *= 2;
                    random += (gameState) ? 1 : 0;
                    snowFlakes[i].snowFlakeShape.trans = glm::translate(snowFlakes[i].snowFlakeShape.trans, glm::vec3(random, 0.0, 0.0));

                } else {
                    // Decreases snowflakes life time and moves it down left direction
                    float fallSpeed = -0.01 * snowFlakes[i].velMultiplier;
                    float xSpeed = (gameState) ?  -SCROLL_SPEED + snowFlakes[i].velX : 0;
                    snowFlakes[i].flakeLifeTime--;
                    snowFlakes[i].snowFlakeShape.trans = glm::translate(snowFlakes[i].snowFlakeShape.trans, glm::vec3(xSpeed + windInfluence(gameState), fallSpeed, 0.0));
                    if (snowFlakes[i].rotDirection) {
                        snowFlakes[i].snowFlakeShape.rot = glm::rotate(snowFlakes[i].snowFlakeShape.rot, glm::radians(snowFlakes[i].rotSpeed), glm::vec3(0.0, 0.0, 1.0));
                    } else {
                        snowFlakes[i].snowFlakeShape.rot = glm::rotate(snowFlakes[i].snowFlakeShape.rot, glm::radians(-snowFlakes[i].rotSpeed), glm::vec3(0.0, 0.0, 1.0));
                    }
                }   
            }
            
        }
    }

    /**
     * Adjusts the positions of scene objects according to the new width/height of the screen
     */
    void adjustPositions(float width, float height) {
        // Resets the transformations of cloud and moon
        moon.resetTransforms();
        clouds.resetTransforms();
        mainMenuObj.mainMenu.resetTransforms();
        moon.scale = glm::scale(moon.scale, glm::vec3(MOON_SCALE, MOON_SCALE, 0.0));
        moon.trans = glm::translate(moon.trans, glm::vec3(MOON_POS_XY, MOON_POS_XY, 0.0));

        // Readjusting cloud and moon based on the new width/height
        if (width > height) {
            moon.trans = glm::translate(moon.trans, glm::vec3(0.0, (height - width) / width, 0.0));
            clouds.trans = glm::translate(clouds.trans, glm::vec3(0.0, (height - width) / width, 0.0));
        }
        mainMenuObj.adjustPosition(width, height);
    }

    /**
     * Updates all the shapes based on the key inputs
     */
    void checkKeyInputs(GLFWwindow *win) {
        // Loops through the entire array to check if the key is pressed or not
        for (int keyNo = 0; keyNo < TOTAL_KEYS; keyNo++) {
            // If key i is not pressed, skip to the next loop. If D is not pressed, reset the animation length
            if (!isKeyPressed[keyNo] && keyNo == GLFW_KEY_D) goat.changeAnimationLength(ANIM_FRAME_LEN);
            if (!isKeyPressed[keyNo]) continue;

            switch (keyNo) {
                case GLFW_KEY_ESCAPE:
                    glfwSetWindowShouldClose(win, GLFW_TRUE);
                    break;
                case GLFW_KEY_SPACE:
                    goat.jump();
                    // Allows for multiple key presses
                    break;
                case GLFW_KEY_A:
                    goat.walkLeft();
                    if (!isKeyPressed[GLFW_KEY_D] && !isKeyPressed[GLFW_KEY_SPACE] && goat.getWalkedDistance() > -GOAT_WALK_RANGE) {
                        // Sets texture to idle position if the space key or D is not pressed
                        goat.goatShape.textureID = goat.goatAnimationFrames[0];
                        goat.changeAnimationFrame(1);
                    }
                    if (goat.getIsAirBorne()) {
                        // Walk to the left two more times as to imitate hopping backwards
                        goat.walkLeft();
                    }
                    break;
                case GLFW_KEY_D:
                    goat.walkRight();
                    // Quickens the animation frame length if the goat has not walked out of range
                    if (goat.getWalkedDistance() < GOAT_WALK_RANGE && !isKeyPressed[GLFW_KEY_A]) {
                        goat.changeAnimationLength(ANIM_FRAME_LEN / 4);
                    } else {
                        goat.changeAnimationLength(ANIM_FRAME_LEN);
                    }
                    break;
                case GLFW_KEY_TAB:
                    // Toggle overlay on or off
                    printMessageTime();
                    std::cout << "Overlay set to " << !enableOverlay << "\n";
                    enableOverlay = !enableOverlay;
                    isKeyPressed[keyNo] = false;
                    break;
                case GLFW_KEY_LEFT_CONTROL:
                    // Makes the screen fullscreen unless screen saver mode is enabled
                    if (SCREENSAVER_MODE) break;
                    if (glfwGetWindowAttrib(win, GLFW_MAXIMIZED)) {
                        glfwRestoreWindow(win);
                    } else {
                        glfwMaximizeWindow(win);
                    }
                    isKeyPressed[keyNo] = false;
                    break;
            }

        }
    }
};