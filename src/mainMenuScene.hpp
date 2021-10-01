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
    shapeObject mainMenu, splashText;
    double menuScrollDist = 0, mainMenuTimer = MAIN_MENU_TIMER, sceneScale = 1;
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
    }

    /**
     * Animates the main menu by one fram
     */
    void tickMainMenu(bool gameState) {
        mainMenu.resetTransforms();
        if (gameState) {
            menuScrollDist -= SCROLL_SPEED;
            
            mainMenu.trans = glm::translate(mainMenu.trans, glm::vec3(menuScrollDist, 0.0, 0.0));
            mainMenuTimer--;
        }
        mainMenu.textureID = menuAnimationFrames[menuCurrFrame];

        // Scales the main menu according to the entire scene sclae
        mainMenu.scale = glm::scale(mainMenu.scale, glm::vec3(sceneScale, sceneScale, 0.0));

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
        sceneScale = 1;

        if (width > height) {
            sceneScale = ( height / width );
        }
    }

    /**
     * Deletes the shapes that consists of the main menu
     */
    void deleteShapes() {
        delete[] menuAnimationFrames;
        mainMenu.deleteSelf();
        splashText.deleteSelf();
    }
};