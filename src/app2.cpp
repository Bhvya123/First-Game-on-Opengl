#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#include <GLFW/glfw3.h>
#include <unistd.h>
#include <bits/stdc++.h>

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "layout (location = 1) in vec2 aTex;\n"
                                 "out vec2 TexCoord;\n"
                                 "uniform vec3 moveChar;\n"
                                 "uniform vec3 moveCoin;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos+moveCoin+moveChar, 1.0);\n"
                                 "   TexCoord = aTex;\n"
                                 "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "in vec2 TexCoord;\n"
                                   "uniform vec2 transform;\n"
                                   "uniform sampler2D ourTexture;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = texture(ourTexture, TexCoord + transform);\n"
                                   "}\n\0";

void framebuffer_size_callback(GLFWwindow *, int width, int height)
{
    glViewport(0, 0, width, height);
}
float n = 0.001f;   // Translating textures 
float m = 0.001f;   // For moving the Character
float o = 1.0f;   // For moving coin

float timeNow;

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        (m < 1.29f) ? m += ((float)(glfwGetTime() - timeNow)) * 1.0f : m += 0;
    }
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        (m <= 0.0) ? m = 0.0f : m -= ((float)(glfwGetTime() - timeNow) * 1.0f);
    }
    if(glfwGetKey(window, GLFW_KEY_INSERT) == GLFW_RELEASE)
    {
        n -= (float)(glfwGetTime() - timeNow) * 1.0f;            
        o -= (float)(glfwGetTime() - timeNow) * 1.1f; 
    }
}

void RandomCoin(float *y)
{
    *y = -0.5f + ((float)rand()/(float)RAND_MAX);
    return;
}

// float y = -0.5f + (float)(rand()/((float)((float)RAND_MAX/1.29f+0.5f)));
int main()
{
    time_t start, end;
    srand(time(0));
    int numCoins = 0;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

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

    float vertices[] = {
        // positions       // textures

        // player:-
        -0.7f, -0.5f, 0.0f,   1.0f, 1.0f, // left bot
        -0.6f, -0.3f, 0.0f,   0.0f, 0.0f, // right top
        -0.7f, -0.3f, 0.0f,   1.0f, 0.0f, // left top
        -0.6f, -0.5f, 0.0f,   0.0f, 1.0f, // right bot

        // tiles:-
        -1.0f, -1.0f, 0.5f,   1.0f, 1.0f, // left bot
        1.0f, -0.5f, 0.5f,    0.0f, 0.0f, // right top
        -1.0f, -0.5f, 0.5f,   1.0f, 0.0f, // left top
        1.0f, -1.0f, 0.5f,    0.0f, 1.0f, // right bot

        // coins:-
        0.7f, 0.4f , 0.2f,    1.0f, 1.0f,   // left bot // 41   40
        0.76f, 0.5f, 0.2f,    0.0f, 0.0f,   // right top //46   45
        0.7f, 0.5f, 0.2f,     1.0f, 0.0f,   // left top // 51   50
        0.76f, 0.4f, 0.2f,    0.0f, 1.0f   // right bot // 56   55
    };

    unsigned int indices1[] = {
        0, 2, 1,
        0, 1, 3,
        4, 6, 5,
        4, 5, 7,
        8, 10, 9,
        8, 9, 11
    };

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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
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
    unsigned char *data1 = stbi_load("../textures/tiles.png", &width1, &height1, &nrChannels1, 0);
    if (data1)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width1, height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1);
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data2);

    int textureLoc = glGetUniformLocation(shaderProgram, "ourTexture");
    int transLoc = glGetUniformLocation(shaderProgram, "transform");
    int charLoc = glGetUniformLocation(shaderProgram, "moveChar");
    int coinLoc = glGetUniformLocation(shaderProgram, "moveCoin");

    glm::vec2 transform = glm::vec2(0.0f);
    glm::vec3 moveChar = glm::vec3(0.0f);
    glm::vec3 moveCoin = glm::vec3(0.0f);
    glUniform3fv(coinLoc, 1, glm::value_ptr(moveCoin));
    glUniform3fv(charLoc, 1, glm::value_ptr(moveChar));
    glUniform2fv(transLoc, 1, glm::value_ptr(transform));

    // render loop
    glEnable(GL_DEPTH_TEST);
    float y = -0.5f + ((float)rand()/(float)RAND_MAX);
    bool flag = false;
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);
        timeNow = glfwGetTime();
        // if((int)(end-start)%3 == 0 && flag == false) flag = true;

        // rendering commands here
        // ...
        flag = false;
        if(y < 0)
        {
            vertices[41] = y;
            vertices[46] = y+0.1f;
            vertices[51] = y+0.1f;
            vertices[56] = y;
        }
        else
        {
            vertices[41] = y-0.1f;
            vertices[46] = y;
            vertices[51] = y-0.1f;
            vertices[56] = y;
        }
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        moveCoin = glm::vec3(0.0f);
        glUniform3fv(coinLoc, 1, glm::value_ptr(moveCoin));

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices1), indices1, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glUseProgram(shaderProgram);

        glUniform1i(textureLoc, 0);

        transform = glm::vec2(0.0f);
        glUniform2fv(transLoc, 1, glm::value_ptr(transform));
        moveChar = glm::vec3(0.0f, m, 0.0f);
        glUniform3fv(charLoc, 1, glm::value_ptr(moveChar));
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture0);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glUniform1i(textureLoc, 1);

        transform = glm::vec2(n, 0.0f);
        glUniform2fv(transLoc, 1, glm::value_ptr(transform));
        moveChar = glm::vec3(0.0f);
        glUniform3fv(charLoc, 1, glm::value_ptr(moveChar));

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6*sizeof(unsigned int)));

        transform = glm::vec2(0.0f);
        glUniform2fv(transLoc, 1, glm::value_ptr(transform));
        moveCoin = glm::vec3(o, 0.0f, 0.0f);
        glUniform3fv(coinLoc, 1, glm::value_ptr(moveCoin));

        glUniform1i(textureLoc, 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(12 * sizeof(unsigned int)));

        if((vertices[40] + o >= -0.7f && vertices[40] + o <= -0.6f) || 
           (vertices[45] + o >= -0.7f && vertices[45] + o <= -0.6f) || 
           (vertices[50] + o >= -0.7f && vertices[50] + o <= -0.6f) || 
           (vertices[55] + o >= -0.7f && vertices[55] + o <= -0.6f)   )
        {
            if((-0.5f + m <= vertices[41] && -0.3f + m >= vertices[41]) ||
               (-0.5f + m <= vertices[46] && -0.3f + m >= vertices[46]) ||
               (-0.5f + m <= vertices[51] && -0.3f + m >= vertices[51]) ||
               (-0.5f + m <= vertices[56] && -0.3f + m >= vertices[56])   )
            {
                flag = true;
                std::cout<<"*";
            }
        }

        if(flag)
        {
            o = 1.0f;
            RandomCoin(&y);
            numCoins++;
            std::cout << numCoins << std::endl;
        }
        else
        {
            // o -= (float)(glfwGetTime() - timeNow) * 1.1f; 
            if(o <= -1.7f) 
            {
                o = 1.0f;
                RandomCoin(&y);
            }
        }
        // n -= (float)(glfwGetTime() - timeNow) * 2.0f;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}
