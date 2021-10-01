/**
 * File contains the vert struct, which contains the texture and vertex co-ordinates
 * of a shape
 */

#include <glm/glm.hpp>

/**
 * Contains the vertex and texture co-ordinates of a point in a shape
 */
struct vert {
    glm::vec4 vertexCoords;
    glm::vec2 textureCoords;
};