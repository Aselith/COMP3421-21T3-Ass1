/**
 * File contains goatObject struct
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <chicken3421/chicken3421.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>

// Required external variables
extern const int MAX_FRAMES_GOAT;
extern const int ANIM_FRAME_LEN;
extern const int AIRBORNE_LEN_MAX;
extern const float GOAT_JUMP_ROT;
extern const float GOAT_WALK_SPEED;
extern const int GOAT_WALK_RANGE;
extern const float SCROLL_SPEED;

/**
 * Contains everything related to the goat, including functions to control it
 * and animate it
 */
struct goatObject {
    shapeObject goatShape;
    GLuint (*goatAnimationFrames) = new GLuint[MAX_FRAMES_GOAT];
private:
    int frameLength = ANIM_FRAME_LEN;
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

    /**
     * Animates the subsequent frame
     */
    void nextFrame() {
        // Loops through the frames. Each frame lasts "frameLength" long
        if (!isAirBorne) {
            if (frameLifeTime == 0) {
                goatShape.textureID = goatAnimationFrames[currFrame];
                currFrame = (currFrame + 1) % MAX_FRAMES_GOAT;
                frameLifeTime = frameLength;
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

    /**
     * Changes the length of each frame for the goat's walk cycle
     */
    void changeAnimationLength(int newLength) {
        frameLength = newLength;
    }

    /**
     * Changes the frame that the goat's walk cycle is on
     */
    void changeAnimationFrame(int newFrame) {
        currFrame = newFrame;
    }

    /**
     * Getter for walked distance
     */
    float getWalkedDistance() {
        return walkedDistance;
    }

    /**
     * Getter for whether the goat is in the air or not
     */
    bool getIsAirBorne() {
        return isAirBorne;
    }

    /**
     * Translate the goat to the right
     */
    void walkRight() {
        if (walkedDistance < GOAT_WALK_RANGE) {
            walkedDistance += GOAT_WALK_SPEED;
            goatShape.trans = glm::translate(goatShape.trans, glm::vec3(GOAT_WALK_SPEED, 0.0, 0.0));
        }
    }

    /**
     * Translate the goat to the left
     */
    void walkLeft() {
        if (walkedDistance > -GOAT_WALK_RANGE) {
            walkedDistance += -SCROLL_SPEED;
            goatShape.trans = glm::translate(goatShape.trans, glm::vec3(-SCROLL_SPEED, 0.0, 0.0));
        }
    }

    /**
     * Puts the goat in an airborne state
     */
    void jump() {
        // Toggles goat state to in the air
        if (!isAirBorne) {
            isAirBorne = true;
            // Rotate the goat a bit on jump
            goatShape.rot = glm::rotate(goatShape.rot, glm::radians(GOAT_JUMP_ROT), glm::vec3(0.0, 0.0, 1.0));
            // Selects the jumping frame that is closest to the current frame
            if (abs(currFrame - 2) < abs(currFrame - 6)) {
                goatShape.textureID = goatAnimationFrames[2];
                changeAnimationFrame(3);
            } else {
                goatShape.textureID = goatAnimationFrames[6];
                changeAnimationFrame(7);
            }
        }
    }

    void deleteSelf() {
        delete[] goatAnimationFrames;
        goatShape.deleteSelf();
    }
};