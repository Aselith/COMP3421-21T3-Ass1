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

//////////////////////
// PROGRAM SETTINGS //
//////////////////////

// Application window settings:
const char *APP_TITLE  = "Minecraft: Goat Simulator - COMP3421 OpenGL Assignment 1";
#define SCREEN_WIDTH     900
#define SCREEN_HEIGHT    900

#define MAIN_MENU_TIMER  300  
#define MAX_FRAMES_MENU  119  // How long the main menu animation lasts for (Must be a multiple of two - 1)
#define TOTAL_SPL_TEX    20   // How many variants of splash text
#define SPLASH_SCALE     0.30 // Scale of splash text
#define SPLASH_ROT       12.5f// How much the splash text is askewed
#define SPLASH_POS_X     0.69 // X position of splash text
#define SPLASH_POS_Y     0.55 // Y position of splash text
#define SPLASH_AMPLITUDE 0.07 // How violent the splash text bobs

// Speed of animation:
#define TICKS_TO_SECOND  20 // Lower value = faster; Higher value = slower;

// Ground settings:
#define GROUND_POS_Y     -0.8 // Y position of the ground
#define GROUND_SCALE     0.2  // Dimension size of ground
#define GROUND_TILES     20   // How many tiles are in the shape

// Background object settings:
#define FG_TIMER         500  // How long foreground objects last on the screen
#define PARALLAX_TIMER   4    // How long background objects last = PARALLAX_TIMER * FG_TIMER
#define FG_POS_Y         0.9  // Y position of the foreground objects
#define FG_SCALE         1.5  // Size of foreground objects
#define PARALLAX_POS_Y   0.4  // Y position of background objects
#define PARALLAX_POS_X   2.0  // X position of background objects
#define MAX_FRAMES_SKY   2    // How many frames the night sky has
#define MOON_SCALE       0.2  // Size of the Moon
#define MOON_POS_XY      0.6  // X and Y position of the Moon
#define TOTAL_MOON_TEX   8    // Total possible Moon phases
#define FG_COOLDOWN      120  // How long inbetween spawning foreground objects

#define BG_SPAWN_CHANCE  400  // The chance of background spawning (1 / BG_SPAWN_CHANCE)

#define SCROLL_SPEED    0.01 // How fast the objects scroll by the screen

#define ANIM_FRAME_LEN   4    // The length of the frames of the Goat's animation
#define AIRBORNE_LEN_MAX 15   // Total time the Goat is in the air
#define MAX_FRAMES_GOAT  8    // Total frames of the Goat's animation
#define GOAT_POS_Y       -0.27// Y position of Goat
#define GOAT_SCALE       0.4  // Scale of the Goat
#define GOAT_JUMP_ROT    5.0f // How many degrees the Goat rotates in its jump
#define GOAT_WALK_SPEED  0.015// How far the Goat moves when "D" is pressed
#define GOAT_WALK_RANGE  1    // How far from the centre of the screen the Goat can move to

#define TOTAL_SF_TEX     4    // How many possible textures a snowflake can be
#define FLAKE_TOTAL      900  // How many flakes are present. MUST BE AN EVEN NUMBER
#define FLAKE_TIMER      1400 // How long the flakes last on the screen
#define FLAKE_ROT_SPEED  5.0f // How many degrees the flakes rotate
#define FLAKE_CHANCE     2    // The chance a snow flake spawns every tick (1 / FLAKE_CHANCE)
#define FLAKE_SCALE      0.03 // How big the flake is
#define FLAKE_POS_Y      1.02 // X position of where the flakes spawn
#define W_AMPLITUDE      0.01 // Wind's amplitude, controls how crazy the wind is
#define W_COEFFICIENT  M_PI/8 // Controls how short each wind bursts are
#define W_VERT_SHIFT    0.005 // Controls how effective each wind bursts are

#define TOTAL_FG_TEX     12   // The total amount of possible foreground textures
#define TOTAL_P_TEX      4    // The total amount of possible parallax textures
#define TREE_LOOP_POS_Y  0.4  // Y position of the looping trees in the background

bool enableOverlay = true;    // Controls whether to render the overlay or not
bool gameState = false;

// Keeps track of every image and textures created in the code
std::list<chicken3421::image_t> listOfEveryImage;
std::list<GLuint> listOfEveryTexID;

// HELPER FUNCTIONS

/**
 * Randomly generates a number between -1 to 1 
 * @return float
 */
float rdmNumGen() {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

/**
 * Appends a random number at the end of a file name
 * @param int minRng for smallest possible value (inclusive of minRng)
 * @param int minRng for smallest possible value (inclusive of maxRng)
 * @return std::string
 */
std::string appendRdmNum(const std::string &fileName, int minRng, int maxRng) {
    std::string returnName = fileName.c_str();
    int rng = minRng + (rand() % (maxRng - minRng + 1));
    returnName.append(std::to_string(rng));
    returnName.append(".png");
    return returnName;
}

chicken3421::image_t makeImage(const std::string &fileName) {
    chicken3421::image_t loadedImage = chicken3421::load_image(fileName);
    listOfEveryImage.push_back(loadedImage);
    return loadedImage;
}

GLuint makeTexture(const std::string &fileName) {

    chicken3421::image_t texImg = makeImage(fileName);
    GLint format;
    if (texImg.n_channels == 3) {
        format = GL_RGB;
    } else {
        format = GL_RGBA;
    }

    // Enabling transparent pixels
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint tex;
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, texImg.width, texImg.height, 0, format, GL_UNSIGNED_BYTE, texImg.data);

    // Have filter to be GL_NEAREST to replicate the Minecraft pixel art
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Have textures repeat if it does not fit the shape
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    listOfEveryTexID.push_back(tex);

    return tex;
}

void printMessageTime() {
    auto currTime = time(0);
    char *chrTime = std::ctime(&currTime);
    std::string strTime(chrTime);
    // Removes the endline character from the string
    strTime.pop_back();
    std::cout << "\u001b[32m[" << strTime.c_str() << "]\033[0m ";
    return;
}

void deleteAllTexImg() {
    while (listOfEveryTexID.size() > 0) {
        /*
        printMessageTime();
        std::cout << "Deleted tex: " << &listOfEveryTexID.front() << "\n";
        */
        glDeleteTextures(1, &listOfEveryTexID.front());
        listOfEveryTexID.pop_front();
    }
    while (listOfEveryImage.size() > 0) {
        /*
        printMessageTime();
        std::cout << "Deleted img: " << listOfEveryImage.front().data << "\n";
        */
        chicken3421::delete_image(listOfEveryImage.front());
        listOfEveryImage.pop_front();
    }
}

// STRUCTS //
struct vertexGroup {
    glm::vec4 vertexCoords;
    glm::vec2 textureCoords;
};

struct shapeObject {

    GLuint vao;
    GLuint vbo;
    GLuint textureID;
    int totalVertices;
    std::vector<vertexGroup> vertices;

    glm::mat4 trans = glm::mat4(1.0f);
    glm::mat4 rot = glm::mat4(1.0f);
    glm::mat4 scale = glm::mat4(1.0f);

    void resetTransforms() {
        trans = glm::mat4(1.0f);
        rot = glm::mat4(1.0f);
        scale = glm::mat4(1.0f);
    }

    void deleteSelf() {
        vertices.clear();
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }
};

struct goatObject {
    shapeObject goatShape;
    GLuint goatAnimationFrames[MAX_FRAMES_GOAT];
    private:
        int currFrame = 0;
        int frameLifeTime = 0;
        bool isAirBorne = false;
        int airBorneLen = 0;
        float walkedDistance = 0;

    public:
    goatObject() {

        // Sets up each frame of the animation
        goatAnimationFrames[0] = makeTexture("res/img/goat/goatTexture_1.png");
        goatAnimationFrames[1] = makeTexture("res/img/goat/goatTexture_4.png");
        goatAnimationFrames[2] = makeTexture("res/img/goat/goatTexture_2.png");
        goatAnimationFrames[3] = goatAnimationFrames[1];
        goatAnimationFrames[4] = goatAnimationFrames[0];
        goatAnimationFrames[5] = makeTexture("res/img/goat/goatTexture_5.png");
        goatAnimationFrames[6] = makeTexture("res/img/goat/goatTexture_3.png");
        goatAnimationFrames[7] = goatAnimationFrames[5];
    }

    void nextFrame() {
        // Loops through the frames. Each frame lasts ANIM_FRAME_LEN long
        if (!isAirBorne) {
            if (frameLifeTime == 0) {
                goatShape.textureID = goatAnimationFrames[currFrame];
                currFrame = (currFrame + 1) % MAX_FRAMES_GOAT;
                frameLifeTime = ANIM_FRAME_LEN;
            } else {
                frameLifeTime -= 1;
            }
        } else {
            // Randomly select the texture for the goat jumping
            airBorneLen++;
            if (airBorneLen == AIRBORNE_LEN_MAX) {
                // Lands the goat back onto the ground
                airBorneLen = 0;
                goatShape.rot = glm::rotate(goatShape.rot, glm::radians(-GOAT_JUMP_ROT), glm::vec3(0.0, 0.0, 1.0));
                isAirBorne = false;
            } else {
                // Draws out a differentiated parabola on how far the shape goes up
                // Original equation = 0.5 * (0.3 * x - 0.02 * x * x)
                float velocity = 0.5 * (0.3 - 0.04 * airBorneLen);
                // printMessageTime();
                // std::cout << "Velocity at: " << velocity << "\n"; // FOR DEBUGGING
                goatShape.trans = glm::translate(goatShape.trans, glm::vec3(0.0, velocity, 0.0));
            }
        }
        
    }

    void walkRight() {
        if (walkedDistance < GOAT_WALK_RANGE) {
            walkedDistance += GOAT_WALK_SPEED;
            goatShape.trans = glm::translate(goatShape.trans, glm::vec3(GOAT_WALK_SPEED, 0.0, 0.0));
        }
    }

    void walkLeft() {
        if (walkedDistance > -GOAT_WALK_RANGE) {
            walkedDistance += -2 * GOAT_WALK_SPEED;
            goatShape.trans = glm::translate(goatShape.trans, glm::vec3(-2 * GOAT_WALK_SPEED, 0.0, 0.0));
        }
    }

    void jump() {
        // Toggles goat state to in the air
        if (!isAirBorne) {
            isAirBorne = true;
            // Rotate the goat a bit on jump
            goatShape.rot = glm::rotate(goatShape.rot, glm::radians(GOAT_JUMP_ROT), glm::vec3(0.0, 0.0, 1.0));
            // Randomly select a jumping texture for the goat
            if (rand() % 2 == 0) {
                goatShape.textureID = goatAnimationFrames[2];
            } else {
                goatShape.textureID = goatAnimationFrames[6];
            }
        }
    }
};

struct snowFlakeObject {
    shapeObject snowFlakeShape;
    bool isActive = true;
    int flakeLifeTime = 0;
    // True for anti-clockwise
    // False for clockwise
    bool rotDirection = (rand() % 2 == 0);
    float rotSpeed = FLAKE_ROT_SPEED + abs(rdmNumGen());
    float velMultiplier = 0.1 + abs(rdmNumGen());
    float velX = -0.01 * (abs(rdmNumGen()));
};

struct mainMenuScene {
    shapeObject mainMenu, splashText;
    double menuScrollDist = 0, mainMenuTimer = MAIN_MENU_TIMER;
    GLuint menuAnimationFrames[MAX_FRAMES_MENU];
    int menuCurrFrame = 0;

    void setupMenu() {
        for (int i = 0; i < MAX_FRAMES_MENU; i++) {
            try {
                menuAnimationFrames[i] = makeTexture(appendRdmNum("res/img/mainMenu/mainmenu_", i + 1, i + 1));
            } catch (std::runtime_error) {
                menuAnimationFrames[i] = menuAnimationFrames[0];
            }
        }
        splashText.textureID = makeTexture(appendRdmNum("res/img/mainMenu/splashText/splash_", 1, TOTAL_SPL_TEX));
    }

    void tickMainMenu() {
        
        if (gameState) {
            mainMenu.trans = glm::translate(mainMenu.trans, glm::vec3(-SCROLL_SPEED, 0.0, 0.0));
            menuScrollDist -= SCROLL_SPEED;
            mainMenuTimer--;
        }
        mainMenu.textureID = menuAnimationFrames[menuCurrFrame];

        // Splash text animation
        float newScale = glm::sin((M_PI * menuCurrFrame) / 20) * SPLASH_AMPLITUDE;
        newScale++;
        resetSplashText();
        splashText.scale = glm::scale(splashText.scale, glm::vec3(newScale, newScale, 0.0));
        splashText.trans = glm::translate(splashText.trans, glm::vec3(menuScrollDist, 0.0, 0.0));

        menuCurrFrame++;
        menuCurrFrame %= MAX_FRAMES_MENU;

    }

    void resetSplashText() {
        splashText.resetTransforms();
        splashText.scale = glm::scale(splashText.scale, glm::vec3(SPLASH_SCALE, SPLASH_SCALE, 0.0));
        splashText.rot = glm::rotate(splashText.rot, glm::radians(SPLASH_ROT), glm::vec3(0.0, 0.0, 1.0));
        splashText.trans = glm::translate(splashText.trans, glm::vec3(SPLASH_POS_X, SPLASH_POS_Y, 0.0));
    }
};

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
    snowFlakeObject snowFlakes[FLAKE_TOTAL];
    goatObject goat;

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

        GLuint possibleTexID[TOTAL_FG_TEX];
        GLuint possibleParaTexID[TOTAL_P_TEX];
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
    }

    void deleteAllShapes() {
        overlay.deleteSelf();
        background.deleteSelf();
        moon.deleteSelf();
        foregroundObjA.deleteSelf();
        foregroundObjB.deleteSelf();
        ground.deleteSelf();
        parallaxObj.deleteSelf();
        goat.goatShape.deleteSelf();
        for (int i = 0; i < FLAKE_TOTAL; i++) {
            snowFlakes[i].snowFlakeShape.deleteSelf();
        }
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
    void tickAll() {
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
            mainMenuObj.tickMainMenu();
        }
        // Animates the background sky and the snowflakes
        background.textureID = skyAnimationFrames[rand() % MAX_FRAMES_SKY];
        tickSnowFlake();
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
            if (rand() % BG_SPAWN_CHANCE == 7) {
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
    double windInfluence() {
        double amplitude = W_AMPLITUDE;
        double xCoefficient = W_COEFFICIENT;
        double period = (2 * M_PI) / xCoefficient;
        // Only use vertical shift if the animation has started
        double vertShift = (gameState) ? W_VERT_SHIFT : 0;
        // The sin curve
        double sinCurveResult = amplitude * glm::sin(xCoefficient * sinCurveX) + vertShift;
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
    void tickSnowFlake() {
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
                    snowFlakes[i].snowFlakeShape.trans = glm::translate(snowFlakes[i].snowFlakeShape.trans, glm::vec3(xSpeed + windInfluence(), fallSpeed, 0.0));
                    if (snowFlakes[i].rotDirection) {
                        snowFlakes[i].snowFlakeShape.rot = glm::rotate(snowFlakes[i].snowFlakeShape.rot, glm::radians(snowFlakes[i].rotSpeed), glm::vec3(0.0, 0.0, 1.0));
                    } else {
                        snowFlakes[i].snowFlakeShape.rot = glm::rotate(snowFlakes[i].snowFlakeShape.rot, glm::radians(-snowFlakes[i].rotSpeed), glm::vec3(0.0, 0.0, 1.0));
                    }
                }   
            }
            
        }
    }
};

///////////////////////////
// MORE HELPER FUNCTIONS //
///////////////////////////

shapeObject createShape(std::vector<vertexGroup> vert) {
    GLuint vao;
    glGenVertexArrays(1, &vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    // Binding and enabling Vertex Array Objects and Buffer Objects
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, (GLintptr)(sizeof(vertexGroup) * vert.size()), vert.data(), GL_STATIC_DRAW);

    // Pointing to first 4 = vertex co-ordinates; next 2 = texture co=ordinates
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertexGroup), (void *)(0 + offsetof(vertexGroup, vertexCoords)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertexGroup), (void *)(0 + offsetof(vertexGroup, textureCoords)));
 
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    shapeObject returnShape;
    returnShape.vao = vao;
    returnShape.vbo = vbo;
    returnShape.vertices = vert;
    returnShape.totalVertices = vert.size();

    return returnShape;
}

void renderShapeObj(std::list<shapeObject>::iterator obj, GLint transformLoc) {
    glBindVertexArray(obj->vao);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);
    glBindTexture(GL_TEXTURE_2D, obj->textureID);
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(obj->trans * obj->rot * obj->scale));

    glDrawArrays(GL_TRIANGLES, 0, obj->totalVertices);
}

goatObject createGoat() {
    std::vector<vertexGroup> vert = {
        // 1st Triangle
        {{  1,  1,  0,  1}, {  1,  1}},
        {{  1, -1,  0,  1}, {  1,  0}},
        {{ -1, -1,  0,  1}, {  0,  0}},
        // 2nd Triangle
        {{  1,  1,  0,  1}, {  1,  1}},
        {{ -1, -1,  0,  1}, {  0,  0}},
        {{ -1,  1,  0,  1}, {  0,  1}},
    };

    shapeObject returnShape = createShape(vert);
    returnShape.scale = glm::scale(returnShape.scale, glm::vec3(GOAT_SCALE, GOAT_SCALE, 0.0));
    returnShape.trans = glm::translate(returnShape.trans, glm::vec3(0, GOAT_POS_Y, 0.0));
    goatObject returnGoat;
    returnGoat.goatShape = returnShape;
    return returnGoat;
}

shapeObject createFlatSquare() {
    std::vector<vertexGroup> vert = {
        // 1st Triangle
        {{  1,  1,  0,  1}, {  1,  1}},
        {{  1, -1,  0,  1}, {  1,  0}},
        {{ -1, -1,  0,  1}, {  0,  0}},
        // 2nd Triangle
        {{  1,  1,  0,  1}, {  1,  1}},
        {{ -1, -1,  0,  1}, {  0,  0}},
        {{ -1,  1,  0,  1}, {  0,  1}},
    };

    return createShape(vert);
}

shapeObject createParallaxLoop() {
    std::vector<vertexGroup> vert = {
        // 1st Triangle
        {{  2,  1,  0,  1}, {  2,  1}},
        {{  2, -1,  0,  1}, {  2,  0}},
        {{ -2, -1,  0,  1}, {  0,  0}},
        // 2nd Triangle
        {{  2,  1,  0,  1}, {  2,  1}},
        {{ -2, -1,  0,  1}, {  0,  0}},
        {{ -2,  1,  0,  1}, {  0,  1}},
    };
    shapeObject returnObj = createShape(vert);
    returnObj.trans = glm::translate(returnObj.trans, glm::vec3(0.0, TREE_LOOP_POS_Y, 0.0));
    return returnObj;
}

snowFlakeObject createSnowFlake() {
    std::vector<vertexGroup> vert = {
        // 1st Triangle
        {{  1,  1,  0,  1}, {  1,  1}},
        {{  1, -1,  0,  1}, {  1,  0}},
        {{ -1, -1,  0,  1}, {  0,  0}},
        // 2nd Triangle
        {{  1,  1,  0,  1}, {  1,  1}},
        {{ -1, -1,  0,  1}, {  0,  0}},
        {{ -1,  1,  0,  1}, {  0,  1}},
    };

    shapeObject returnShape = createShape(vert);
    snowFlakeObject returnFlakeObject;
    returnFlakeObject.snowFlakeShape = returnShape;

    // returnShape.trans = glm::translate(returnShape.trans, glm::vec3(2.5, FG_POS_Y, 0.0));
    return returnFlakeObject;
}

shapeObject createFlatSquareElement() {
    std::vector<vertexGroup> vert = {
        // 1st Triangle
        {{  0.7,   1,  0,  1}, {  1,  1}},
        {{  0.7,  -1,  0,  1}, {  1,  0}},
        {{ -0.7,  -1,  0,  1}, {  0,  0}},
        // 2nd Triangle
        {{  0.7,   1,  0,  1}, {  1,  1}},
        {{ -0.7,  -1,  0,  1}, {  0,  0}},
        {{ -0.7,   1,  0,  1}, {  0,  1}},
    };

    shapeObject returnShape = createShape(vert);
    returnShape.scale = glm::scale(returnShape.scale, glm::vec3(FG_SCALE, FG_SCALE, 0.0));
    returnShape.trans = glm::translate(returnShape.trans, glm::vec3(2.5, FG_POS_Y, 0.0));
    return returnShape;
}

shapeObject createGround(int totalRepeats) {

    std::vector<vertexGroup> vert = {
        // 1st Triangle
        {{  totalRepeats,  1,  0,  1}, {totalRepeats,  1}},      // top-right
        {{  totalRepeats, -1,  0,  1}, {totalRepeats,  0}},        // bottom-right
        {{ -totalRepeats, -1,  0,  1}, {           0,  0}},             // bottom-left
        // 2nd Triangle
        {{  totalRepeats,  1,  0,  1}, {totalRepeats,  1}},           // top-right
        {{ -totalRepeats, -1,  0,  1}, {           0,  0}},           // bottom-left
        {{ -totalRepeats,  1,  0,  1}, {           0,  1}},           // top-left
    };

    shapeObject returnShape = createShape(vert);
    // Moves shape to the correct spot on the screen
    returnShape.trans = glm::translate(returnShape.trans, glm::vec3(0.0, GROUND_POS_Y, 0.0));
    returnShape.scale = glm::scale(returnShape.scale, glm::vec3(GROUND_SCALE, GROUND_SCALE, 0.0));
    return returnShape;
}

int main() {
    // Flips the textures
    stbi_set_flip_vertically_on_load(true);

    GLFWwindow *win = chicken3421::make_opengl_window(SCREEN_WIDTH, SCREEN_HEIGHT, APP_TITLE);

    GLuint vertShader = chicken3421::make_shader("res/shaders/vert.glsl", GL_VERTEX_SHADER);
    GLuint fragShader = chicken3421::make_shader("res/shaders/frag.glsl", GL_FRAGMENT_SHADER);

    scene sceneObjects;

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
    glfwSetWindowUserPointer(win, &sceneObjects.goat);

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

    moonObj.scale = glm::scale(moonObj.scale, glm::vec3(MOON_SCALE, MOON_SCALE, 0.0));
    moonObj.trans = glm::translate(moonObj.trans, glm::vec3(MOON_POS_XY, MOON_POS_XY, 0.0));
    sceneObjects.moon = moonObj;

    // Creating the shape for the ground
    shapeObject groundSceneObj = createGround(GROUND_TILES);
    groundSceneObj.textureID = makeTexture("res/img/snowyGroundTexture.png");
    sceneObjects.ground = groundSceneObj;

    // Creating the shape for the back mountains
    shapeObject parallaxObj = createFlatSquare();
    parallaxObj.trans = glm::translate(parallaxObj.trans, glm::vec3(PARALLAX_POS_X, PARALLAX_POS_Y, 0.0));
    sceneObjects.parallaxObj = parallaxObj;

    // Creating the shape for the background elements
    sceneObjects.foregroundObjA = createFlatSquareElement();
    sceneObjects.foregroundObjB = createFlatSquareElement();
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
    // Tick a few ticks ahead so that the first rendered frame has a chance
    // to not look so empty
    for (int i = 0; i < 450; i++) {
        if (rand() % 2 == 0) {
            sceneObjects.tickSnowFlake();
        }
        sceneObjects.tickFgObjA();
        sceneObjects.tickFgObjB();
        sceneObjects.tickParallax();
    }

    GLuint renderProgram = chicken3421::make_program(vertShader, fragShader);

    // Gets the transform uniform location
    GLint transformLoc = glGetUniformLocation(renderProgram, "transform");
    chicken3421::expect(transformLoc != -1, "No uniform variable named: transform in program: " + std::to_string(transformLoc));

    //////////////////////////
    // Setting up callbacks //
    //////////////////////////

    glfwSetWindowSizeCallback(win, [](GLFWwindow* window, int width, int height) {
        // Keeps the window at a 1:1 width:height ratio
        printMessageTime();
        std::cout << "Window size change detected, adjusting viewport\n";
        glViewport(0, 0, height, height);
    });

    glfwSetKeyCallback(win, [](GLFWwindow *win, int key, int scancode, int action, int mods) {
        // Spacebar to jump, A to slow down, D to speed up
        if (action != GLFW_RELEASE) {
            gameState = true;
            goatObject *goat = (goatObject *) glfwGetWindowUserPointer(win);
            switch (key) {
                case GLFW_KEY_ESCAPE:
                    glfwSetWindowShouldClose(win, GLFW_TRUE);
                    break;
                case GLFW_KEY_SPACE:
                    goat->jump();
                    break;
                case GLFW_KEY_A:
                    goat->walkLeft();
                    break;
                case GLFW_KEY_D:
                    goat->walkRight();
                    break;
                case GLFW_KEY_TAB:
                    // Toggle overlay on or off
                    printMessageTime();
                    std::cout << "Overlay set to " << !enableOverlay << "\n";
                    enableOverlay = !enableOverlay;
                    break;
            }
        }
    });

    // Variables to manage when to animate a frame for each scene object
    using namespace std::chrono;
    long startLoop = 0, endLoop = 0, deltaTime = TICKS_TO_SECOND;

    while (!glfwWindowShouldClose(win)) {
        
        startLoop = time_point_cast<milliseconds>(system_clock::now()).time_since_epoch().count();

        glUseProgram(renderProgram);
        

        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1, 1, 1, 1);
        // glClearColor(std::cos(now/1000), std::sin(now/1000), std::cos(now/1000), 1);

        // When deltaTime exceeds TICKS_TO_SECOND, animate each scene object
        if (deltaTime >= TICKS_TO_SECOND) {
            sceneObjects.tickAll();
            deltaTime -= TICKS_TO_SECOND;
        }

        // Draw all objects in the sceneObjects list
        std::list<shapeObject> listOfObjects = sceneObjects.getAllObjects();

        for (std::list<shapeObject>::iterator obj = listOfObjects.begin(); obj != listOfObjects.end(); obj++) {
            renderShapeObj(obj, transformLoc);
        }

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);

        glfwSwapBuffers(win);

        // Calculates the time elapsed between start and end of loop and adds it
        // to the deltaTime.
        endLoop = time_point_cast<milliseconds>(system_clock::now()).time_since_epoch().count();
        deltaTime += endLoop - startLoop;
    }

    glfwDestroyWindow(win);
    chicken3421::delete_program(renderProgram);
    chicken3421::delete_shader(fragShader);
    chicken3421::delete_shader(vertShader);
    sceneObjects.deleteAllShapes();
    deleteAllTexImg();
    printMessageTime();
    std::cout << "Closing program\n";

    return EXIT_SUCCESS;
}