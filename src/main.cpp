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
#define TICKS_TO_SECOND 40

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

struct scene {
    shapeObject background;
    shapeObject ground;

    float translatedGroundPos = 0;

    std::list<shapeObject> getAllObjects() {
        std::list<shapeObject> returnList;
        returnList.emplace_back(background);
        returnList.emplace_back(ground);
        return returnList;
    }

    void tickGround() {
        if (translatedGroundPos < 0) {
            translatedGroundPos = 1;
            ground.trans = glm::mat4(1.0f);
            ground.rot = glm::mat4(1.0f);
            ground.scale = glm::mat4(1.0f);
            ground.trans = glm::translate(ground.trans, glm::vec3(0.0, -0.75, 0.0));
            ground.scale = glm::scale(ground.scale, glm::vec3(0.25, 0.25, 0.0));
        }
        ground.trans = glm::translate(ground.trans, glm::vec3(-0.01, 0.0, 0.0));
        translatedGroundPos -= 0.01;
    }
};



// HELPER FUNCTIONS //

GLuint make_texture(const chicken3421::image_t &tex_img) {
    GLint format = tex_img.n_channels == 3 ? GL_RGB : GL_RGBA;

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
    returnShape.trans = glm::translate(returnShape.trans, glm::vec3(0.0, -0.75, 0.0));
    returnShape.scale = glm::scale(returnShape.scale, glm::vec3(0.25, 0.25, 0.0));
    return returnShape;

}



int main() {
    GLFWwindow *win = chicken3421::make_opengl_window(SCREEN_WIDTH, SCREEN_HEIGHT, "Assignment 1 - Minecraft Nightier Night");

    GLuint vs = chicken3421::make_shader("res/shaders/vert.glsl", GL_VERTEX_SHADER);
    GLuint fs = chicken3421::make_shader("res/shaders/frag.glsl", GL_FRAGMENT_SHADER);

    scene sceneObjects;

    // Loading in images //
    stbi_set_flip_vertically_on_load(true);
    // Snowy ground
    chicken3421::image_t ground = chicken3421::load_image("res/img/snowyGroundTexture.png");
    // Backdrop
    chicken3421::image_t sky = chicken3421::load_image("res/img/nightSkyTexture.png");
    //

    // Shape making //
    // Creating the shape for the background
    shapeObject backgroundObj = createBackground();
    backgroundObj.textureID = make_texture(sky);
    sceneObjects.background = backgroundObj;
     // Creating the shape for the ground
    shapeObject groundSceneObj = createGround(10);
    groundSceneObj.textureID = make_texture(ground);
    sceneObjects.ground = groundSceneObj;

    GLuint render_prog = chicken3421::make_program(vs, fs);

    

    GLint transformLoc = glGetUniformLocation(render_prog, "transform");
    chicken3421::expect(transformLoc != -1, "No uniform variable named: transform in program: " + std::to_string(transformLoc));

    // Setting up callbacks
    glfwSetWindowSizeCallback(win, [](GLFWwindow* window, int width, int height)
    {
        std::cout << "Window size change detected, adjusting viewport\n";
        glViewport(0, 0, width, width);
    });


    while (!glfwWindowShouldClose(win)) {
        glUseProgram(render_prog);
        using namespace std::chrono;
        long long now = time_point_cast<milliseconds>(system_clock::now()).time_since_epoch().count();

        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 0, 0, 1);
        //glClearColor(std::cos(now/1000), std::sin(now/1000), std::cos(now/1000), 1);

        if (now % TICKS_TO_SECOND == 0) {
            sceneObjects.tickGround();
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