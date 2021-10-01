/**
 * File contains general helper functions and also keeps track of all
 * textures and images made
 */
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <chicken3421/chicken3421.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>

// Keeps track of every image and textures created in the code
std::list<chicken3421::image_t> listOfEveryImage;
std::list<GLuint> listOfEveryTexID;

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

/**
 * Creates an image with the given filename
 * @param string
 * @return chicken3421::image_t
 */
chicken3421::image_t makeImage(const std::string &fileName) {
    chicken3421::image_t loadedImage = chicken3421::load_image(fileName);
    listOfEveryImage.push_back(loadedImage);
    return loadedImage;
}

/**
 * Creates the texture with the image at the given filename
 * @param string
 * @return GLuint texture handler
 */
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


/**
 * Prints the system time without \n at the end
 */
void printMessageTime() {
    auto currTime = time(0);
    char *chrTime = std::ctime(&currTime);
    std::string strTime(chrTime);
    // Removes the endline character from the string
    strTime.pop_back();
    std::cout << "\u001b[32m[" << strTime.c_str() << "]\033[0m ";
    return;
}

/**
 * Uses the global variables that contains pointers to all texture IDs
 * and images and deletes them all
 */ 
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