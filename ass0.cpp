#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bits/stdc++.h>
#include <unistd.h>

const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 transform;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = transform * vec4(aPos, 1.0);\n"
    "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 redCol;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0,0.0,0.0,1.0);\n"
    "}\n\0";

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

int main()
{
    int sides = 0;
    float pi = 3.14159265359;
    std::cout << "Enter the number of sides of the prism: ";
    std::cin >> sides; 

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearningOpenGL", NULL, NULL);
    if(window == NULL)
    {
        std::cout << "Failed to Create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800,600);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infolog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infolog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infolog << std::endl;
    }

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infolog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infolog << std::endl;
    }

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if(!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infolog);
        std::cout << "ERROR::PROGRAM::VERTEX::COMPILATION_FAILED\n" << infolog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices1[3*(sides+1)], vertices2[3*(sides+1)];
    // float vertices3[6*sides];
    vertices1[0] = 0.0f;
    vertices1[1] = 0.0f;
    vertices1[2] = 0.0f;
    vertices2[0] = 0.0f;
    vertices2[1] = 0.0f;
    vertices2[2] = -0.5f;

    int k = 0, ff = 0;

    for(int i=3;i<3*(sides+1);i+=3)
    {
        vertices1[i] = (float)(cos(2*(k)*(pi/sides)))/2;
        // vertices3[ff++] = vertices1[i];
        vertices1[i+1] = (float)(sin(2*(k)*(pi/sides)))/2;
        // vertices3[ff++] = vertices1[i+1];
        vertices1[i+2] = 0.0f;
        // vertices3[ff++] = vertices1[i+2];
        k++;
    }

    for(int i=3;i<3*(sides+1);i+=3)
    {
        vertices2[i] = vertices1[i];
        // vertices3[ff++] = vertices2[i];
        vertices2[i+1] = vertices1[i+1];
        // vertices3[ff++] = vertices2[i+1];
        vertices2[i+2] = -0.5f;
        // vertices3[ff++] = vertices2[i+2];
    }

    // k=0;
    // for(int i=3;i<3*(sides+1);i++)
    // {
    //     vertices3[k] = vertices1[i];
    //     k++;
    // }
    // for(int i=3;i<3*(sides+1);i++)
    // {
    //     vertices3[k] = vertices2[i];
    //     k++;
    // }
    //////////////////////////////////////////////////////////////////////////

    // for(int i=0;i<(3*sides+1);i++)
    // {
    //     std::cout << "vertices1: " << vertices1[i] << " ";
    // }
    // std::cout << "\n";
    // for(int i=0;i<(3*sides+1);i++)
    // {
    //     std::cout << "vertices2: " << vertices2[i] << " ";
    // }
    // std::cout << "\n";
    // for(int i=0;i<6*sides;i++)
    // {
    //     std::cout << "vertices3: " << vertices3[i] << " ";
    // }

    /////////////////////////////////////////////////////////////////////////
    unsigned int VBO, EBO[3], VAO;
    glGenBuffers(1, &VBO);
    glGenBuffers(3, EBO);
    glGenVertexArrays(1, &VAO);

    unsigned int indices1[3*sides], indices2[3*sides]; //indices3[6*sides];
    
    k=1;
    for(int i=0;i<3*sides;i+=3)
    {
        indices1[i] = 0;
        indices1[i+1] = k;
        indices1[i+2] = k+1;
        k++;
    }
    indices1[(3 * sides)-1] = 1;

    k=1;
    for(int i=0;i<3*sides;i+=3)
    {
        indices2[i] = 0;
        indices2[i+1] = k;
        indices2[i+2] = k+1;
        k++;
    }
    indices2[(3 * sides)-1] = 1;

    // k=0;
    // for(int i=0;i<6*sides;i+=6)
    // {
    //     indices3[i] = k;
    //     indices3[i+1] = 3*sides+k;
    //     indices3[i+2] = k+1;

    //     indices3[i+3] = 3*sides+k+1;
    //     indices3[i+4] = 3*sides+k;
    //     indices3[i+5] = k+1;

    //     k++;
    // }

    // render loop

    glEnable(GL_DEPTH_TEST);

    glBindVertexArray(VAO);
    while(!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // rendering commands here
        // ...
        glClearColor(1.0f, 1.0f, 1.0f, 0.43f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        int transformLoc = glGetUniformLocation(shaderProgram, "transform");

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glUniformMatrix4fv(transformLoc,1,GL_FALSE,glm::value_ptr(transform));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices1), indices1, GL_DYNAMIC_DRAW);
        
        glDrawElements(GL_TRIANGLES, 3*sides, GL_UNSIGNED_INT, 0);

        // glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_DYNAMIC_DRAW);

        // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        // glEnableVertexAttribArray(0);

        glDrawElements(GL_TRIANGLES, 3*sides, GL_UNSIGNED_INT, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[2]);

        for(int i=1;i<=sides;i++)
        {
            unsigned int indices[6];
            indices[0] = i;
            indices[1] = i+1; 
            indices[2] = 3*sides+i;

            indices[3] = 3*sides+i+1;
            indices[4] = 3*sides+i;
            indices[5] = i+1;

            if(i == sides)
            {
                indices[0] = i;
                indices[1] = 1;
                indices[2] = 3*sides+i;

                indices[3] = 3*sides;
                indices[4] = 3*sides+i;
                indices[5] = 1;
            }

            // k++;
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1)+sizeof(vertices2), (vertices1,vertices2), GL_DYNAMIC_DRAW);            
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        // glBindVertexArray(0);
        // check and call events and swap buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    
    return 0;
}