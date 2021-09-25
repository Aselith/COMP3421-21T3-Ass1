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

#define SCREEN_WIDTH  720
#define SCREEN_HEIGHT 720
#define TICKS_TO_SECOND 20

#define GROUND_POS_Y -0.8
#define GROUND_SCALE 0.2
#define GROUND_TILES 20

#define FG_TIMER 500
#define PARALLAX_TIMER 3
#define FG_POS_Y 0.9
#define FG_SCALE 1.5

#define TREE_CHANCE 300
#define GOLEM_CHANCE 400

#define WALKING_SPEED 0.01

#define ANIM_FRAME_LEN 1
#define AIRBORNE_LEN_MAX 15
#define MAX_FRAMES_WALK 8
#define GOAT_POS_Y -0.18
#define GOAT_SCALE 0.5
#define GOAT_JUMP_ROT 5.0f
#define GOAT_WALK_SPEED 0.015
#define GOAT_WALK_RANGE 1

#define FLAKE_TOTAL 40
#define FLAKE_TIMER 400
#define FLAKE_ROT_SPEED 5.0f
#define FLAKE_CHANCE 15

#define TOTAL_FG_TEX 6


// HELPER FUNCTIONS

GLuint make_texture(const chicken3421::image_t &tex_img);
float rdmNumGen() {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
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
};

struct goatObject {
    shapeObject goatShape;
    GLuint goatAnimationFrames[MAX_FRAMES_WALK];
    int currFrame = 0;
    int frameLifeTime = 0;
    bool isAirBorne = false;
    int airBorneLen = 0;
    float walkedDistance = 0;

    goatObject() {
        chicken3421::image_t standStill = chicken3421::load_image("res/img/goat/goatTexture_1.png");
        chicken3421::image_t rightLegStart = chicken3421::load_image("res/img/goat/goatTexture_4.png");
        chicken3421::image_t rightLegEnd = chicken3421::load_image("res/img/goat/goatTexture_2.png");
        chicken3421::image_t leftLegStart = chicken3421::load_image("res/img/goat/goatTexture_5.png");
        chicken3421::image_t leftLegEnd = chicken3421::load_image("res/img/goat/goatTexture_3.png");

        // Sets up each frame of the animation
        goatAnimationFrames[0] = make_texture(standStill);
        goatAnimationFrames[1] = make_texture(rightLegStart);
        goatAnimationFrames[2] = make_texture(rightLegEnd);
        goatAnimationFrames[3] = goatAnimationFrames[1];
        goatAnimationFrames[4] = goatAnimationFrames[0];
        goatAnimationFrames[5] = make_texture(leftLegStart);
        goatAnimationFrames[6] = make_texture(leftLegEnd);
        goatAnimationFrames[7] = goatAnimationFrames[5];
    }

    void nextFrame() {
        // Loops through the frames. Each frame lasts ANIM_FRAME_LEN long
        if (!isAirBorne) {
            if (frameLifeTime == 0) {
                goatShape.textureID = goatAnimationFrames[currFrame];
                currFrame = (currFrame + 1) % MAX_FRAMES_WALK;
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
};

struct scene {
    shapeObject background;
    shapeObject ground;
    shapeObject foregroundObjA;
    shapeObject foregroundObjB;
    shapeObject parallaxObj;
    snowFlakeObject snowFlakes[FLAKE_TOTAL];
    goatObject goat;

    float translatedGroundPos = 0;

    int fgObjATimer = FG_TIMER;
    int fgObjBTimer = FG_TIMER;
    int parallaxTimer = PARALLAX_TIMER * FG_TIMER;
    bool fgObjASpawned = false;
    bool fgObjBSpawned = false;
    bool pallxSpawned = false;

    GLuint possibleTexID[TOTAL_FG_TEX];

    scene() {
        // Loading in all possible textures
        chicken3421::image_t tree = chicken3421::load_image("res/img/treeTexture.png");
        chicken3421::image_t golemA = chicken3421::load_image("res/img/snowGolemATexture.png");
        chicken3421::image_t golemB = chicken3421::load_image("res/img/snowGolemBTexture.png");
        chicken3421::image_t mossyPile = chicken3421::load_image("res/img/mossyPileTexture.png");
        chicken3421::image_t berryA = chicken3421::load_image("res/img/berryBushesATexture.png");
        chicken3421::image_t berryB = chicken3421::load_image("res/img/berryBushesBTexture.png");

        possibleTexID[0] = make_texture(tree);
        possibleTexID[1] = make_texture(golemA);
        possibleTexID[2] = make_texture(golemB);
        possibleTexID[3] = make_texture(mossyPile);
        possibleTexID[4] = make_texture(berryA);
        possibleTexID[5] = make_texture(berryB);

    }

    std::list<shapeObject> getAllObjects() {
        std::list<shapeObject> returnList;
        returnList.emplace_back(background);
        returnList.emplace_back(parallaxObj);
        for (int i = 0; i < FLAKE_TOTAL / 2; i++) {
            returnList.emplace_back(snowFlakes[i].snowFlakeShape);
        }
        returnList.emplace_back(foregroundObjA);
        returnList.emplace_back(foregroundObjB);
        returnList.emplace_back(goat.goatShape);
        for (int i = FLAKE_TOTAL / 2; i < FLAKE_TOTAL; i++) {
            returnList.emplace_back(snowFlakes[i].snowFlakeShape);
        }
        returnList.emplace_back(ground);
        return returnList;
    }

    void tickGoat() {
        goat.nextFrame();   
    }

    void tickGround() {
        if (translatedGroundPos < 0) {
            translatedGroundPos = 1;
            ground.trans = glm::mat4(1.0f);
            ground.rot = glm::mat4(1.0f);
            ground.scale = glm::mat4(1.0f);
            ground.trans = glm::translate(ground.trans, glm::vec3(0.0, GROUND_POS_Y, 0.0));
            ground.scale = glm::scale(ground.scale, glm::vec3(GROUND_SCALE, GROUND_SCALE, 0.0));
        }
        ground.trans = glm::translate(ground.trans, glm::vec3(-WALKING_SPEED, 0.0, 0.0));
        translatedGroundPos -= 0.005;
    }

    void tickFgObjA() {
        if (fgObjASpawned) {
            foregroundObjA.trans = glm::translate(foregroundObjA.trans, glm::vec3(-WALKING_SPEED, 0.0, 0.0));
            fgObjATimer -= 1;
            if (fgObjATimer < 0) {
                std::cout << "ObjA has reached the end\n";
                fgObjATimer = FG_TIMER;
                foregroundObjA.resetTransforms();
                foregroundObjA.scale = glm::scale(foregroundObjA.scale, glm::vec3(FG_SCALE, FG_SCALE, 0.0));
                foregroundObjA.trans = glm::translate(foregroundObjA.trans, glm::vec3(2.5, FG_POS_Y, 0.0));
                fgObjASpawned = false;
            }
        } else {
            if (rand() % TREE_CHANCE == 0) {
                foregroundObjA.textureID = possibleTexID[rand() % TOTAL_FG_TEX];
                std::cout << "ObjA spawned\n";
                fgObjASpawned = true;
            }
        }
    }

    void tickFgObjB() {
        if (fgObjBSpawned) {
            foregroundObjB.trans = glm::translate(foregroundObjB.trans, glm::vec3(-WALKING_SPEED, 0.0, 0.0));
            fgObjBTimer -= 1;
            if (fgObjBTimer < 0) {
                std::cout << "ObjB has reached the end\n";
                fgObjBTimer = FG_TIMER;
                foregroundObjB.resetTransforms();
                foregroundObjB.scale = glm::scale(foregroundObjB.scale, glm::vec3(FG_SCALE, FG_SCALE, 0.0));
                foregroundObjB.trans = glm::translate(foregroundObjB.trans, glm::vec3(2.5, FG_POS_Y, 0.0));
                fgObjBSpawned = false;
            }
        } else {
            if (rand() % GOLEM_CHANCE == 3) {
                foregroundObjB.textureID = possibleTexID[rand() % TOTAL_FG_TEX];
                std::cout << "ObjB spawned\n";
                fgObjBSpawned = true;
            }
        }
    }

    void tickParallax() {
        if (pallxSpawned) {
            parallaxObj.trans = glm::translate(parallaxObj.trans, glm::vec3(-(WALKING_SPEED / PARALLAX_TIMER), 0.0, 0.0));
            parallaxTimer -= 1;
            if (parallaxTimer < 0) {
                std::cout << "Parallax has reached the end\n";
                parallaxTimer = PARALLAX_TIMER * FG_TIMER;
                parallaxObj.resetTransforms();
                parallaxObj.trans = glm::translate(parallaxObj.trans, glm::vec3(1.8, 0.4, 0.0));
                pallxSpawned = false;
            }
        } else {
            if (rand() % TREE_CHANCE == 0) {
                std::cout << "Parallax spawned\n";
                pallxSpawned = true;
            }
        }
    }

    void tickSnowFlake() {
        if (rand() % FLAKE_CHANCE == 0) {
            // A chance to make a random snow flake active
            snowFlakes[rand() % FLAKE_TOTAL].isActive = true;
        } 
        for (int i = 0; i < FLAKE_TOTAL; i++) {
            // Loops through all snowflakes to animate them
            if (snowFlakes[i].isActive) {
                if (snowFlakes[i].flakeLifeTime == 0) {
                    // Reset its lifetimer and its transformations
                    snowFlakes[i].isActive = false;
                    snowFlakes[i].flakeLifeTime = FLAKE_TIMER;
                    snowFlakes[i].snowFlakeShape.resetTransforms();

                    // Scale it back down and translate it to the top of the screen
                    snowFlakes[i].snowFlakeShape.scale = glm::scale(snowFlakes[i].snowFlakeShape.scale, glm::vec3(0.05, 0.05, 0.0));
                    snowFlakes[i].snowFlakeShape.trans = glm::translate(snowFlakes[i].snowFlakeShape.trans, glm::vec3(0.0, 1.1, 0.0));
                    
                    // Randomly decide the x co-ordinate of the shape
                    float random = rdmNumGen();
                    if (rand() % 2 == 0) {
                        // Randomly flip the direction of the x co-ordinate
                        random *= -1;
                    }

                    // Widens the range of where the snowflake can spawn and offsets it to the right
                    random *= 1.5;
                    random += 1;
                    snowFlakes[i].snowFlakeShape.trans = glm::translate(snowFlakes[i].snowFlakeShape.trans, glm::vec3(random, 0.0, 0.0));

                } else {
                    // Decreases snowflakes life time and moves it down left direction
                    float fallSpeed = -0.01 * snowFlakes[i].velMultiplier;
                    snowFlakes[i].flakeLifeTime--;
                    snowFlakes[i].snowFlakeShape.trans = glm::translate(snowFlakes[i].snowFlakeShape.trans, glm::vec3(-WALKING_SPEED, fallSpeed, 0.0));
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

// HELPER FUNCTIONS //

/**
 * Randomly generates a number between -1 to 1 
 * @return float
 */

GLuint make_texture(const chicken3421::image_t &tex_img) {
    GLint format = tex_img.n_channels == 3 ? GL_RGB : GL_RGBA;

    // Enabling transparent pixels
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    GLuint tex;
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, tex_img.width, tex_img.height, 0, format, GL_UNSIGNED_BYTE, tex_img.data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

shapeObject createShape(std::vector<vertexGroup> vert) {
    GLuint vao;
    glGenVertexArrays(1, &vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, (GLintptr)(sizeof(vertexGroup) * vert.size()), vert.data(), GL_STATIC_DRAW);

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

goatObject createGoat() {
    std::vector<vertexGroup> vert = {
        {{ 1, 1, 0, 1 }, {1, 1.0}},      // top-right
        {{ 1, -1, 0, 1 }, {1, 0.0}},        // bottom-right
        {{ -1, -1, 0, 1 }, {0.0, 0.0}},             // bottom-left

        {{ 1, 1, 0, 1 }, {1, 1.0}},           // top-right
        {{ -1, -1, 0, 1 }, {0.0, 0.0}},           // bottom-left
        {{ -1, 1, 0, 1 }, {0.0, 1.0}},           // top-left
    };

    shapeObject returnShape = createShape(vert);
    returnShape.scale = glm::scale(returnShape.scale, glm::vec3(GOAT_SCALE, GOAT_SCALE, 0.0));
    returnShape.trans = glm::translate(returnShape.trans, glm::vec3(0, GOAT_POS_Y, 0.0));
    goatObject returnGoat;
    returnGoat.goatShape = returnShape;
    return returnGoat;
}

shapeObject createBackground() {
    std::vector<vertexGroup> vert = {
        {{ 1, 1, 0, 1 }, {1, 1.0}},      // top-right
        {{ 1, -1, 0, 1 }, {1, 0.0}},        // bottom-right
        {{ -1, -1, 0, 1 }, {0.0, 0.0}},             // bottom-left

        {{ 1, 1, 0, 1 }, {1, 1.0}},           // top-right
        {{ -1, -1, 0, 1 }, {0.0, 0.0}},           // bottom-left
        {{ -1, 1, 0, 1 }, {0.0, 1.0}},           // top-left
    };

    return createShape(vert);
}

snowFlakeObject createSnowFlake() {
    std::vector<vertexGroup> vert = {
        {{ 1, 1, 0, 1 }, {1, 1.0}},      // top-right
        {{ 1, -1, 0, 1 }, {1, 0.0}},        // bottom-right
        {{ -1, -1, 0, 1 }, {0.0, 0.0}},             // bottom-left

        {{ 1, 1, 0, 1 }, {1, 1.0}},           // top-right
        {{ -1, -1, 0, 1 }, {0.0, 0.0}},           // bottom-left
        {{ -1, 1, 0, 1 }, {0.0, 1.0}},           // top-left
    };

    shapeObject returnShape = createShape(vert);
    snowFlakeObject returnFlakeObject;
    returnFlakeObject.snowFlakeShape = returnShape;

    // returnShape.trans = glm::translate(returnShape.trans, glm::vec3(2.5, FG_POS_Y, 0.0));
    return returnFlakeObject;
}

shapeObject createBackgroundElement() {
    std::vector<vertexGroup> vert = {
        {{ 0.7, 1, 0, 1 }, {1, 1.0}},      // top-right
        {{ 0.7, -1, 0, 1 }, {1, 0.0}},        // bottom-right
        {{ -0.7, -1, 0, 1 }, {0.0, 0.0}},             // bottom-left

        {{ 0.7, 1, 0, 1 }, {1, 1.0}},           // top-right
        {{ -0.7, -1, 0, 1 }, {0.0, 0.0}},           // bottom-left
        {{ -0.7, 1, 0, 1 }, {0.0, 1.0}},           // top-left
    };

    shapeObject returnShape = createShape(vert);
    returnShape.scale = glm::scale(returnShape.scale, glm::vec3(FG_SCALE, FG_SCALE, 0.0));
    returnShape.trans = glm::translate(returnShape.trans, glm::vec3(2.5, FG_POS_Y, 0.0));
    return returnShape;
}

shapeObject createGround(int totalRepeats) {

    std::vector<vertexGroup> vert = {
        {{ totalRepeats, 1, 0, 1 }, {totalRepeats, 1.0}},      // top-right
        {{ totalRepeats, -1, 0, 1 }, {totalRepeats, 0.0}},        // bottom-right
        {{ -totalRepeats, -1, 0, 1 }, {0.0, 0.0}},             // bottom-left

        {{ totalRepeats, 1, 0, 1 }, {totalRepeats, 1.0}},           // top-right
        {{ -totalRepeats, -1, 0, 1 }, {0.0, 0.0}},           // bottom-left
        {{ -totalRepeats, 1, 0, 1 }, {0.0, 1.0}},           // top-left
    };

    shapeObject returnShape = createShape(vert);
    // Moves shape to the correct spot on the screen
    returnShape.trans = glm::translate(returnShape.trans, glm::vec3(0.0, GROUND_POS_Y, 0.0));
    returnShape.scale = glm::scale(returnShape.scale, glm::vec3(GROUND_SCALE, GROUND_SCALE, 0.0));
    return returnShape;

}



int main() {
    stbi_set_flip_vertically_on_load(true);

    GLFWwindow *win = chicken3421::make_opengl_window(SCREEN_WIDTH, SCREEN_HEIGHT, "Assignment 1 - Minecraft Nightier Night");

    GLuint vs = chicken3421::make_shader("res/shaders/vert.glsl", GL_VERTEX_SHADER);
    GLuint fs = chicken3421::make_shader("res/shaders/frag.glsl", GL_FRAGMENT_SHADER);

    scene sceneObjects;

    // Loading in images //
    // Snowy ground
    chicken3421::image_t ground = chicken3421::load_image("res/img/snowyGroundTexture.png");
    // Backdrop
    chicken3421::image_t sky = chicken3421::load_image("res/img/nightSkyTexture.png");
    chicken3421::image_t parallax = chicken3421::load_image("res/img/mountainParallax.png");
    // Snowflake variant A + B
    chicken3421::image_t snowFlakeA = chicken3421::load_image("res/img/snowFlakeATexture.png");
    chicken3421::image_t snowFlakeB = chicken3421::load_image("res/img/snowFlakeBTexture.png");


    // Shape making //
    // Creating the focal point Goat and set a pointer to that goat
    goatObject goatObj = createGoat();
    goatObj.nextFrame();
    sceneObjects.goat = goatObj;
    glfwSetWindowUserPointer(win, &sceneObjects.goat);

    // Creating the shape for the background
    shapeObject backgroundObj = createBackground();
    backgroundObj.textureID = make_texture(sky);
    sceneObjects.background = backgroundObj;

    // Creating the shape for the ground
    shapeObject groundSceneObj = createGround(GROUND_TILES);
    groundSceneObj.textureID = make_texture(ground);
    sceneObjects.ground = groundSceneObj;

    // Creating the shape for the back mountains
    shapeObject parallaxObj = createBackground();
    parallaxObj.trans = glm::translate(parallaxObj.trans, glm::vec3(1.8, 0.4, 0.0));
    parallaxObj.textureID = make_texture(parallax);
    sceneObjects.parallaxObj = parallaxObj;

    // Creating the shape for the tree
    sceneObjects.foregroundObjA = createBackgroundElement();
    // Creating the shape for the snow golem
    sceneObjects.foregroundObjB = createBackgroundElement();

    // Creating the snowflakes
    GLuint variantA = make_texture(snowFlakeA);
    GLuint variantB = make_texture(snowFlakeB);
    for (int i = 0; i < FLAKE_TOTAL; i++) {
        sceneObjects.snowFlakes[i] = createSnowFlake();
        if (rand() % 2 == 0) {
            sceneObjects.snowFlakes[i].snowFlakeShape.textureID = variantA;
        } else {
            sceneObjects.snowFlakes[i].snowFlakeShape.textureID = variantB;
        }
    }
    sceneObjects.tickSnowFlake();

    GLuint render_prog = chicken3421::make_program(vs, fs);

    // Gets the transform uniform location
    GLint transformLoc = glGetUniformLocation(render_prog, "transform");
    chicken3421::expect(transformLoc != -1, "No uniform variable named: transform in program: " + std::to_string(transformLoc));

    // Setting up callbacks //
    // Keeps the window at a 1:1 width:height ratio
    glfwSetWindowSizeCallback(win, [](GLFWwindow* window, int width, int height) {
        std::cout << "Window size change detected, adjusting viewport\n";
        glViewport(0, 0, width, width);
    });
    // Spacebar to jump, A to slow down, D to speed up
    glfwSetKeyCallback(win, [](GLFWwindow *win, int key, int scancode, int action, int mods) {
        if (action != GLFW_RELEASE) {
            goatObject *goat = (goatObject *) glfwGetWindowUserPointer(win);
            switch (key) {
                case GLFW_KEY_SPACE:
                    goat->jump();
                    break;
                case GLFW_KEY_A:
                    goat->walkLeft();
                    break;
                case GLFW_KEY_D:
                    goat->walkRight();
                    break;
            }
        }
    });

    std::list<shapeObject> listOfObjects = sceneObjects.getAllObjects();

    while (!glfwWindowShouldClose(win)) {
        glUseProgram(render_prog);
        using namespace std::chrono;
        long long now = time_point_cast<milliseconds>(system_clock::now()).time_since_epoch().count();

        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 0, 0, 1);
        //glClearColor(std::cos(now/1000), std::sin(now/1000), std::cos(now/1000), 1);

        if (now % TICKS_TO_SECOND == 0) {
            sceneObjects.tickGoat();
            sceneObjects.tickGround();
            sceneObjects.tickFgObjA();
            sceneObjects.tickFgObjB();
            sceneObjects.tickParallax();
            sceneObjects.tickSnowFlake();
        }

        // Draw all objects in the sceneObjects list
        std::list<shapeObject> listOfObjects = sceneObjects.getAllObjects();

        for (std::list<shapeObject>::iterator obj = listOfObjects.begin(); obj != listOfObjects.end(); obj++) {
            glBindVertexArray(obj->vao);
            glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);
            glBindTexture(GL_TEXTURE_2D, obj->textureID);
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(obj->trans * obj->rot * obj->scale));

            glDrawArrays(GL_TRIANGLES, 0, obj->totalVertices);
        }


        
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);

        glfwSwapBuffers(win);
    }


    return EXIT_SUCCESS;
}