/**
 * File contains shapeObject struct, which contains the VAO, VBO, TexID etc. for any
 * individual shape.
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <chicken3421/chicken3421.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>


/**
 * Contains all the vertex attribute/buffer objects and the textureID
 * of an individual shape
 */
struct shapeObject {

    GLuint vao;
    GLuint vbo;
    GLuint textureID;
    std::vector<vert> vertices;

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
