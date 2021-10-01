/**
 * File contains all helper functions related to creating shapes
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <chicken3421/chicken3421.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>


extern const float GOAT_SCALE;
extern const float GOAT_POS_Y;

/**
 * Creates a shape struct with the given vertices and returns it
 * @param std::vector<vert> a group of vertices
 * @return shapeObject
 */
shapeObject createShape(std::vector<vert> vert_t) {

    shapeObject returnShape;
    returnShape.vertices = vert_t;

    glGenVertexArrays(1, &returnShape.vao);

    glGenBuffers(1, &returnShape.vbo);

    // Binding and enabling Vertex Array Objects and Buffer Objects
    glBindVertexArray(returnShape.vao);
    glBindBuffer(GL_ARRAY_BUFFER, returnShape.vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        (GLintptr)(sizeof(vert) * returnShape.vertices.size()),
        returnShape.vertices.data(),
        GL_STATIC_DRAW
    );

    // Pointing to first 4 = vertex co-ordinates; next 2 = texture co=ordinates
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vert), (void *)(0 + offsetof(vert, vertexCoords)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vert), (void *)(0 + offsetof(vert, textureCoords)));
 
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    return returnShape;
}

/**
 * Creates a basic flat square which covers up the entire screen
 * @return shapeObject
 */
shapeObject createFlatSquare() {
    std::vector<vert> vert = {
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

/**
 * Creates the goat object and translates and scales it appropriately
 * @return goatObject
 */
goatObject createGoat() {
    std::vector<vert> vert = {
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

/**
 * Creates a large shape with a looping texture
 * @return shapeObject
 */
shapeObject createParallaxLoop() {
    std::vector<vert> vert = {
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

/**
 * Creates a shape and stores it in a snowFlakeObject
 * @return snowFlakeObject
 */
snowFlakeObject createSnowFlake() {
    std::vector<vert> vert = {
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

    return returnFlakeObject;
}

/**
 * Creates a background element and scales and translates it offscreen
 * @return shapeObject
 */
shapeObject createBackgroundElement() {
    std::vector<vert> vert = {
        // 1st Triangle
        {{  0.7,   1,  0,  1}, {  1,  1}},
        {{  0.7,  -1,  0,  1}, {  1,  0}},
        {{ -0.7,  -1,  0,  1}, {  0,  0}},
        // 2nd Triangle
        {{  0.7,   1,  0,  1}, {  1,  1}},
        {{ -0.7,  -1,  0,  1}, {  0,  0}},
        {{ -0.7,   1,  0,  1}, {  0,  1}},
    };

    // Scales and translates the background element to offscreen
    shapeObject returnShape = createShape(vert);
    returnShape.scale = glm::scale(returnShape.scale, glm::vec3(FG_SCALE, FG_SCALE, 0.0));
    returnShape.trans = glm::translate(returnShape.trans, glm::vec3(2.5, FG_POS_Y, 0.0));
    return returnShape;
}

/**
 * Creates the shape for the ground with repeating textures.
 * Takes in totalRepeats, refering to how many times the texture should loop
 * @param int totalRepeats
 * @return shapeObject
 */
shapeObject createGround(int totalRepeats) {

    std::vector<vert> vert = {
        // 1st Triangle
        {{  totalRepeats,  1,  0,  1}, {totalRepeats,  1}},
        {{  totalRepeats, -1,  0,  1}, {totalRepeats,  0}},
        {{ -totalRepeats, -1,  0,  1}, {           0,  0}},
        // 2nd Triangle
        {{  totalRepeats,  1,  0,  1}, {totalRepeats,  1}},
        {{ -totalRepeats, -1,  0,  1}, {           0,  0}},
        {{ -totalRepeats,  1,  0,  1}, {           0,  1}},
    };

    shapeObject returnShape = createShape(vert);
    // Moves shape to the correct spot on the screen
    returnShape.trans = glm::translate(returnShape.trans, glm::vec3(0.0, GROUND_POS_Y, 0.0));
    returnShape.scale = glm::scale(returnShape.scale, glm::vec3(GROUND_SCALE, GROUND_SCALE, 0.0));
    return returnShape;
}