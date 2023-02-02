#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "shader.h"

#include <unistd.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <bits/stdc++.h>

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "layout (location = 1) in vec2 aTex;\n"
                                 "out vec2 TexCoord;\n"
                                 "uniform vec3 moveChar;\n"
                                 "uniform vec3 moveCoin;\n"
                                 "uniform vec3 moveZap;\n"
                                 "uniform vec3 randCoin;\n"
                                 "uniform vec3 randObs;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos+moveCoin+moveChar+moveZap+randCoin+randObs, 1.0);\n"
                                 "   TexCoord = aTex;\n"
                                 "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
                                   "layout (location = 0) out vec4 FragColor;\n"
                                   "layout (location = 1) out vec4 BrightColor;\n"
                                   "in vec2 TexCoord;\n"
                                   "uniform vec2 transform;\n"
                                   "uniform sampler2D ourTexture;\n"
                                   "uniform float opaq;\n"
                                   "uniform vec3 blurr;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = texture(ourTexture, TexCoord + transform);\n"
                                   "   if(blurr.r >= 0.9 && length(FragColor.rgb) <= 0.05) FragColor.a = 0.f;\n"
                                   "   BrightColor = vec4(blurr, 1.0f);\n"
                                   "   BrightColor.a = FragColor.a;\n"
                                   "}\n\0";


void framebuffer_size_callback(GLFWwindow *, int width, int height)
{
    glViewport(0, 0, width, height);
}

void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color);

float n = 0.001f;   // Translating textures 
float m = 0.001f;   // For moving the Character
float o = 1.0f;     // For moving coin
float t = 0.001f;   // For vertical translation of obstacle/zapper
float th = 1.0f;    // For horizontal translation of obstacle/zapper
float lvlSpeedZap = 0.6f;
unsigned int lvlNum = 1;
float lvlStartTime;
unsigned int numCoins = 0;
float y;    // rand Coin 
float x;    // rand Zapper
float timeNow;
float accel_up = 1.0f;
float accel_down = 1.0f;
bool flag = false;
bool gameOver = false;
bool wonGame = false;

// hdr 
float exposure = 1.0f;
/////

struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;
unsigned int VAO_C, VBO_C;

void processInput(GLFWwindow *window, float vertices[], unsigned int blurLoc, unsigned int opaqLoc)
{
    float s = glfwGetTime();
    if(s - lvlStartTime >= 10)
    {
        lvlNum++;
        lvlSpeedZap += 0.4;
        lvlStartTime = glfwGetTime();
        n = 0.001f;
        m = 0.001f;
        o = 1.0f;
        t = 0.001f;
        th = 1.0f;
        if(lvlNum == 4) 
        {
            lvlNum = 3;
            gameOver = true;
            wonGame = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_INSERT) == GLFW_RELEASE)
    {
        accel_down = 1.0f;
        glUniform3fv(blurLoc, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
        glUniform1f(opaqLoc, 0.4);
        if(m <= 1.3f)
        {
            m += ((float)(s - timeNow)) * accel_up;
            if(accel_up <= 1.3f) accel_up += 0.1f;
        }
        else
        {
            accel_up = 1.0f;
            accel_down = 1.2f;
        }
        // (m <= 1.2f) ? m += ((float)(s - timeNow)) * accel_up : m += 0.0f;
    }
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_INSERT) == GLFW_RELEASE)
    {
        glUniform3fv(blurLoc, 1, glm::value_ptr(glm::vec3(0.0f)));
        glUniform1f(opaqLoc, 1.0);

        accel_up = 1.0f;
        if(m <= 0.0f)
        {
            m = 0.0f;
            accel_down = 1.0f;
        }
        else
        {
            m -= ((float)(s - timeNow)) * accel_down;
            if(accel_down <= 1.4f) accel_down += 0.2f;
        }
        // (m <= 0.0f) ? m = 0.0f : m -= ((float)(s - timeNow) * accel);
    }
    if(glfwGetKey(window, GLFW_KEY_INSERT) == GLFW_RELEASE)
    {
        n -= (float)(s - timeNow) * 0.8f;            
        o -= (float)(s - timeNow) * 0.8f; 
        th -= (float)(s - timeNow) * 0.4; 
        if(!flag)
        {
            if(t + vertices[61] + x >= -0.5f)
            {
                t -= (float)(s - timeNow) * lvlSpeedZap;
            }
            else flag = true;
            // (t+vertices[61] >= -0.5f && !flag) ? t -= (float)(s - timeNow) * 0.5f : flag = true;
        }
        else
        {
            if(t + vertices[66] + x <= 1.0f)
            {
                t += (float)(s - timeNow) * lvlSpeedZap;
            }
            else flag = false;
            // (t+vertices[66] <= 1.0f && flag) ? t += (float)(s - timeNow) * 0.5f : flag = false;
        }
        if((vertices[46] + y > vertices[1] + m && vertices[41] + y < vertices[11] + m) &&
           (vertices[40] + o < vertices[5] && vertices[45] + o > vertices[0]) 
        )
        {
            numCoins++;
            o = 1.0f;
            y = -0.5f + ((float)rand()/(float)RAND_MAX);
        }
        else if(o <= -1.8f) 
        {
            o = 1.0f;
            y = -0.5f + ((float)rand()/(float)RAND_MAX);
        }

        if((vertices[66] + t + x > vertices[1] + m && vertices[61] + t + x < vertices[11] + m) &&
           (vertices[60] + th < vertices[5] && vertices[65] + th > vertices[0]) 
        )
        {
            // std::cout << "Game Over" << std::endl;
            gameOver = true;
        }
        else if(th <= -1.8f)
        {
            th = 1.0f;
            t = -0.5f + ((float)rand()/(float)RAND_MAX);
            x = -0.5f + ((float)rand()/(float)RAND_MAX);
        }
    }
}

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{
    srand(time(0));

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "LearningOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to Create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infolog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infolog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infolog << std::endl;
    }

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infolog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infolog << std::endl;
    }

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infolog);
        std::cout << "ERROR::PROGRAM::VERTEX::COMPILATION_FAILED\n"
                  << infolog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    //////////////////////////////////////

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // compile and setup the shader
    // ----------------------------
    Shader shader("../src/text.vs", "../src/text.fs");
    Shader shaderBlur("../src/blur.vs", "../src/blur.fs");  
    // Shader shaderBloom("../src/bloom.vs", "../src/bloom.fs");  
    Shader screenShader("../src/hdrBuf.vs", "../src/hdrBuf.fs");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

	// find path to font
    std::string font_name = "../fonts/Antonio-Bold.ttf";
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }
	
	// load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &VAO_C);
    glGenBuffers(1, &VBO_C);
    glBindVertexArray(VAO_C);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_C);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //////////////////////////////////////

    float vertices[] = {
        // positions       // textures

        // player:-
        -0.7f, -0.5f, 0.0f,   1.0f, 1.0f,  // left bot
        -0.62f, -0.3f, 0.0f,  0.0f, 0.0f,  // right top
        -0.7f, -0.3f, 0.0f,   1.0f, 0.0f,  // left top
        -0.62f, -0.5f, 0.0f,  0.0f, 1.0f,  // right bot

        // tiles:-
        -1.0f, -1.0f, 0.5f,   1.0f, 1.0f,  // left bot
        1.0f, -0.5f, 0.5f,    0.0f, 0.0f,  // right top
        -1.0f, -0.5f, 0.5f,   1.0f, 0.0f,  // left top
        1.0f, -1.0f, 0.5f,    0.0f, 1.0f,  // right bot

        // coins:-
        0.5f, 0.2f , 0.2f,    1.0f, 1.0f,  // left bot 
        0.56f, 0.3f, 0.2f,    0.0f, 0.0f,  // right top
        0.5f, 0.3f, 0.2f,     1.0f, 0.0f,  // left top 
        0.56f, 0.2f, 0.2f,    0.0f, 1.0f,  // right bot 

        // logs:-
        0.66f, 0.4f, 0.0f,    1.0f, 1.0f,  // left bot
        0.7f, 0.7f, 0.0f,     0.0f, 0.0f,  // right top
        0.66f, 0.7f, 0.0f,    1.0f, 0.0f,  // left top
        0.7f, 0.4f, 0.0f,     0.0f, 1.0f   // right bot
    };

    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int indices1[] = {
        0, 2, 1,
        0, 1, 3,
        4, 6, 5,
        4, 5, 7,
        8, 10, 9,
        8, 9, 11,
        12, 14, 13,
        12, 13, 15
    };

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    unsigned int texture0;
    glGenTextures(1, &texture0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("../textures/guy.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    unsigned int texture1;
    glGenTextures(1, &texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width1, height1, nrChannels1;
    unsigned char *data1 = stbi_load("../textures/tiles3.png", &width1, &height1, &nrChannels1, 0);
    if (data1)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width1, height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data1);

    unsigned int texture2;
    glGenTextures(1, &texture2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width2, height2, nrChannels2;
    unsigned char *data2 = stbi_load("../textures/coin5.png", &width2, &height2, &nrChannels2, 0);
    if (data2)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data2);

    unsigned int texture3;
    glGenTextures(1, &texture3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture3);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width3, height3, nrChannels3;
    unsigned char *data3 = stbi_load("../textures/obstacle2.png", &width3, &height3, &nrChannels3, 0);
    if (data3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width3, height3, 0, GL_RGBA, GL_UNSIGNED_BYTE, data3);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data3);

    /////////////////////////////////////////////////////////////////////

    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

     // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }
    /////////////////////////////////////////////////////////////////////

    int textureLoc = glGetUniformLocation(shaderProgram, "ourTexture");
    int transLoc = glGetUniformLocation(shaderProgram, "transform");
    int charLoc = glGetUniformLocation(shaderProgram, "moveChar");
    int coinLoc = glGetUniformLocation(shaderProgram, "moveCoin");
    int zapLoc = glGetUniformLocation(shaderProgram, "moveZap");
    int randCoinLoc = glGetUniformLocation(shaderProgram, "randCoin");
    int randObsLoc = glGetUniformLocation(shaderProgram, "randObs");
    int blurLoc = glGetUniformLocation(shaderProgram, "blurr");
    int opaqLoc = glGetUniformLocation(shaderProgram, "opaq");

    glm::vec2 transform = glm::vec2(0.0f);
    glm::vec3 moveChar = glm::vec3(0.0f);
    glm::vec3 moveCoin = glm::vec3(0.0f);
    glm::vec3 moveZap = glm::vec3(0.0f);
    glm::vec3 randCoin = glm::vec3(0.0f);
    glm::vec3 randObs = glm::vec3(0.0f);
    glm::vec3 blurr = glm::vec3(0.0f);
    // float opaq = 1.0f;

    glUniform3fv(coinLoc, 1, glm::value_ptr(moveCoin));
    glUniform3fv(charLoc, 1, glm::value_ptr(moveChar));
    glUniform3fv(zapLoc, 1, glm::value_ptr(moveZap));
    glUniform3fv(randCoinLoc, 1, glm::value_ptr(randCoin));
    glUniform3fv(randObsLoc, 1, glm::value_ptr(randObs));    
    glUniform2fv(transLoc, 1, glm::value_ptr(transform));
    glUniform3fv(blurLoc, 1, glm::value_ptr(blurr));
    glUniform1f(opaqLoc, 1.0f);

    glEnable(GL_DEPTH_TEST);
    // y = -0.5f + ((float)rand()/(float)RAND_MAX);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices1), indices1, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    lvlStartTime = glfwGetTime();
    blurr = glm::vec3(1.0f, 1.0f, 0.0f);

    while (!glfwWindowShouldClose(window))
    {
        // render loop
        //////////////////////////////////////////////////////////////////////////////////////////
        processInput(window, vertices, blurLoc, opaqLoc);
        timeNow = glfwGetTime();

        // rendering commands here
        // ...
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glEnable(GL_DEPTH_TEST);
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        std::string str = "Coins collected: ";
        str.append(std::to_string(numCoins));
        RenderText(shader, str, 20.0f,570.0f, 0.3f, glm::vec3(1.0f, 1.0f, 1.0f));
        str = "Level ";
        str.append(std::to_string(lvlNum));
        RenderText(shader, str, 700.0f,570.0f,0.4f, glm::vec3(092.0f, 046.0f, 023.0f));

        glUseProgram(shaderProgram);

        glBindVertexArray(VAO); 
        // Player
        glUniform1i(textureLoc, 0);

        processInput(window, vertices, blurLoc, opaqLoc);
        moveChar = glm::vec3(0.0f, m, 0.0f);
        glUniform3fv(charLoc, 1, glm::value_ptr(moveChar));
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture0);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glUniform3fv(charLoc, 1, glm::value_ptr(glm::vec3(0.0f,0.0f,0.0f)));
        glUniform1f(opaqLoc, 1.0f);
        glUniform3fv(blurLoc, 1, glm::value_ptr(glm::vec3(0.0f,0.0f,0.0f)));
        // Coins
        glUniform1i(textureLoc, 2);

        moveCoin = glm::vec3(o, 0.0f, 0.0f);
        glUniform3fv(coinLoc, 1, glm::value_ptr(moveCoin));
        glUniform3fv(randCoinLoc, 1, glm::value_ptr(glm::vec3(0.0f,y,0.0f)));

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(12 * sizeof(unsigned int)));

        glUniform3fv(coinLoc, 1, glm::value_ptr(glm::vec3(0.0f,0.0f,0.0f)));
        glUniform3fv(randCoinLoc, 1, glm::value_ptr(glm::vec3(0.0f,0.0f,0.0f)));
        
        // Zapper
        glUniform1i(textureLoc, 3);

        glUniform3fv(blurLoc, 1, glm::value_ptr(blurr));
        moveZap = glm::vec3(th, t, 0.0f);
        glUniform3fv(zapLoc, 1, glm::value_ptr(moveZap));
        glUniform3fv(randObsLoc, 1, glm::value_ptr(glm::vec3(0.0f,x,0.0f)));    
        
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, texture3);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(18*sizeof(unsigned int)));

        glUniform3fv(blurLoc, 1, glm::value_ptr(glm::vec3(0.0f)));
        glUniform3fv(zapLoc, 1, glm::value_ptr(glm::vec3(0.0f,0.0f,0.0f)));
        glUniform3fv(randObsLoc, 1, glm::value_ptr(glm::vec3(0.0f,0.0f,0.0f)));    
        
        // Platform
        glUniform1i(textureLoc, 1);

        transform = glm::vec2(n, 0.0f);
        glUniform2fv(transLoc, 1, glm::value_ptr(transform));

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6*sizeof(unsigned int)));

        transform = glm::vec2(0.0f);
        glUniform2fv(transLoc, 1, glm::value_ptr(transform));

        //////////////////////////////////////////////////////////////////////////////////////////        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        shaderBlur.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            shaderBlur.setInt("image", 0);
            shaderBlur.setInt("horizontal", horizontal);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
            glBindVertexArray(quadVAO);
           //////////////////////////////////
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
            //////////////////////////////////
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);        
        screenShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        screenShader.setInt("scene", 0);
        screenShader.setInt("bloomBlur", 1);
        // screenShader.setInt("hdrBuffer", 6);
        // screenShader.setInt("hdr", 1);
        // screenShader.setInt("exposure", 1);
        screenShader.setInt("bloom", 1);
        // screenShader.setInt("screenTexture", 6);
        screenShader.setFloat("exposure", 3.0f);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        // glActiveTexture(GL_TEXTURE6);
        glBindVertexArray(quadVAO);
        // glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);	// use the color attachment texture as the texture of the quad plane
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        //////////////////////////////////////////////////////////////////////////////////////////
        glfwSwapBuffers(window);
        // glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwPollEvents();
        if(gameOver) break;
    }

    while(!glfwWindowShouldClose(window))
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        std::string str = "Coins collected: ";
        str.append(std::to_string(numCoins));
        RenderText(shader, str, 250.0f,400.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f));
        if(wonGame == false)
        {
            str = "You Lose";
            // str.append(std::to_string(lvlNum));
            RenderText(shader, str, 250.0f,500.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
            str = "Level Reached ";
            str.append(std::to_string(lvlNum));
            RenderText(shader, str, 250.0f,300.0f, 1.0f, glm::vec3(0.0f, 1.0f, 1.0f));
        }
        else
        {
            str = "You WON";
            // str.append(std::to_string(lvlNum));
            RenderText(shader, str, 250.0f,500.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            str = "Level Reached ";
            str.append(std::to_string(lvlNum));
            RenderText(shader, str, 250.0f,300.0f, 1.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        }
        if(glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
            // break;
        }
        // std::cout << "ENDLESS LOOP BIATCH!!!" << std::endl;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();

    std::cout << numCoins << std::endl;

    return 0;
}

void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO_C);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO_C);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}