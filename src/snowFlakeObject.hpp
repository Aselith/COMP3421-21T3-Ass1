/**
 * File contains snowFlakeObject struct, which contains all the related date
 * for one individual snowflake
 */
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <chicken3421/chicken3421.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>

// Required external variables
extern const float FLAKE_ROT_SPEED;

/**
 * Contains all things related to an individual snowflake
 */
struct snowFlakeObject {
    shapeObject snowFlakeShape;
    bool isActive = true;
    int flakeLifeTime = 0;
    // True for anti-clockwise
    // False for clockwise
    bool rotDirection = (rand() % 2 == 0);
    // Random rotational speed added onto the base speed
    float rotSpeed = FLAKE_ROT_SPEED + abs(rdmNumGen());
    // Random gravity multiplier (controls how fast the flake falls)
    float velMultiplier = 0.1 + abs(rdmNumGen());
    // Controls how fast the snowflake scrolls to the left
    float velX = -0.01 * (abs(rdmNumGen()));
};