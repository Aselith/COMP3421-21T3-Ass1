/**
 * File contains mainMenuScene, which contains the shapes which consists of the main menu
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <chicken3421/chicken3421.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>

// Required external variables
extern const int SCREEN_HEIGHT;
extern const int SCREEN_WIDTH;
extern const float SPLASH_AMPLITUDE;
extern const float SPLASH_POS_X;
extern const float SPLASH_POS_Y;
extern const float SPLASH_ROT;
extern const float SPLASH_SCALE;
extern const int AUTO_SKIP_TIME;
extern const int MAIN_MENU_TIMER;
extern const int MAX_FRAMES_MENU;
extern const int TOTAL_SPL_TEX;

/**
 * Contains all things related to the main menu
 */
struct mainMenuScene {
    shapeObject mainMenu, splashText, zID;
    double menuScrollDist = 0, mainMenuTimer = MAIN_MENU_TIMER;
    float sceneWidth = SCREEN_WIDTH, sceneHeight = SCREEN_HEIGHT; 
    GLuint (*menuAnimationFrames) = new GLuint[MAX_FRAMES_MENU];
    int menuCurrFrame = 0;

    /**
     * Sets up all the textures needed for the main menu
     */
    void setupMenu() {
        // Uses textures from the mainMenu folder. If texture is not found,
        // default to the first frame's texture
        for (int i = 0; i < MAX_FRAMES_MENU; i++) {
            try {
                menuAnimationFrames[i] = makeTexture(appendRdmNum("res/img/mainMenu/mainmenu_", i + 1, i + 1));
            } catch (std::runtime_error) {
                menuAnimationFrames[i] = menuAnimationFrames[0];
            }
        }
        // Grabs a random splash text texture from the corresponding folder
        splashText.textureID = makeTexture(appendRdmNum("res/img/mainMenu/splashText/splash_", 1, TOTAL_SPL_TEX));
        zID.textureID = makeTexture("res/img/mainMenu/zid.png");
    }

    /**
     * Animates the main menu by one fram
     * @param gameState whether the game has started scrolling or not
     */
    void tickMainMenu(bool gameState) {

        float sceneScale = sceneHeight / sceneWidth;

        mainMenu.resetTransforms();
        zID.resetTransforms();
        if (gameState) {
            menuScrollDist -= SCROLL_SPEED;
            
            mainMenu.trans = glm::translate(mainMenu.trans, glm::vec3(menuScrollDist, 0.0, 0.0));
            zID.trans = glm::translate(zID.trans, glm::vec3(menuScrollDist, 0.0, 0.0));
            mainMenuTimer--;
        }
        mainMenu.textureID = menuAnimationFrames[menuCurrFrame];

        // Scales the main menu according to the entire scene sclae
        float multiplier = 1;
        if (sceneScale != 1) {
            multiplier = 1.2;
        }
        mainMenu.scale = glm::scale(mainMenu.scale, glm::vec3(multiplier * sceneScale, multiplier * sceneScale, 0.0));
        zID.trans = glm::translate(zID.trans, glm::vec3(0, abs((sceneHeight - sceneWidth) / sceneWidth), 0.0));

        // Splash text animation modelled with a sin curve
        float newScale = 1 + glm::sin((M_PI * menuCurrFrame) / 20) * SPLASH_AMPLITUDE;
        resetSplashText();
        splashText.scale = glm::scale(splashText.scale, glm::vec3(newScale, newScale, 0.0));
        splashText.trans = glm::translate(splashText.trans, glm::vec3(menuScrollDist, 0.0, 0.0));

        menuCurrFrame++;
        menuCurrFrame %= MAX_FRAMES_MENU;

    }

    /**
     * Resets the splash text's current transformations and then repositions it in the
     * right place on screen again
     */
    void resetSplashText() {
        float sceneScale = sceneHeight / sceneWidth;
        splashText.resetTransforms();
        splashText.scale = glm::scale(splashText.scale, glm::vec3(SPLASH_SCALE, SPLASH_SCALE, 0.0));
        splashText.scale = glm::scale(splashText.scale, glm::vec3(sceneScale, sceneScale, 0.0));
        splashText.rot = glm::rotate(splashText.rot, glm::radians(SPLASH_ROT), glm::vec3(0.0, 0.0, 1.0));
        splashText.trans = glm::translate(splashText.trans, glm::vec3(SPLASH_POS_X * sceneScale, SPLASH_POS_Y * sceneScale, 0.0));
    }

    /**
     * Scales the main menu down to fit the width and height
     */
    void adjustPosition(float width, float height) {

        mainMenu.resetTransforms();
        zID.resetTransforms();

        if (width >= height) {
            sceneHeight = height;
            sceneWidth = width;
        }
    }

    /**
     * Deletes the shapes that consists of the main menu
     */
    void deleteShapes() {
        mainMenu.deleteSelf();
        splashText.deleteSelf();
        delete[] menuAnimationFrames;
    }
};