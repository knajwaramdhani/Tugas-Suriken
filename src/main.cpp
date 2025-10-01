#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

// Vertex Shader
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(aPos, 1.0);
    ourColor = aColor;
}
)";

// Fragment Shader
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 ourColor;

void main()
{
    FragColor = vec4(ourColor, 1.0);
}
)";

// Callback untuk resize window
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Callback untuk input keyboard
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Fungsi untuk membuat transform matrix rotasi
void createRotationMatrix(float angle, float* matrix) {
    float c = cos(angle);
    float s = sin(angle);
    
    matrix[0] = c;  matrix[1] = s;   matrix[2] = 0;  matrix[3] = 0;
    matrix[4] = -s; matrix[5] = c;   matrix[6] = 0;  matrix[7] = 0;
    matrix[8] = 0;  matrix[9] = 0;   matrix[10] = 1; matrix[11] = 0;
    matrix[12] = 0; matrix[13] = 0;  matrix[14] = 0; matrix[15] = 1;
}

int main() {
    // Inisialisasi GLFW
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Buat window
    GLFWwindow* window = glfwCreateWindow(800, 800, "Shuriken OpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load OpenGL function pointers dengan GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Compile Vertex Shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Compile Fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link Shader Program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Data vertices untuk shuriken
    float size = 0.3f;
    float bladeLength = 0.5f;
    
    // Vertices: posisi (x, y, z) dan warna (r, g, b)
    // Gradasi Pink ke Putih
    float vertices[] = {
        // Persegi tengah (2 segitiga siku-siku) - Pink sedang
        // Segitiga 1
        -size, -size, 0.0f,  1.0f, 0.4f, 0.7f,  // kiri bawah - pink
         size, -size, 0.0f,  1.0f, 0.4f, 0.7f,  // kanan bawah - pink
        -size,  size, 0.0f,  1.0f, 0.4f, 0.7f,  // kiri atas - pink
        
        // Segitiga 2
         size, -size, 0.0f,  1.0f, 0.4f, 0.7f,  // kanan bawah - pink
         size,  size, 0.0f,  1.0f, 0.4f, 0.7f,  // kanan atas - pink
        -size,  size, 0.0f,  1.0f, 0.4f, 0.7f,  // kiri atas - pink
        
        // Segitiga atas - gradasi pink ke putih
        -size, size, 0.0f,  1.0f, 0.5f, 0.75f,  // pink terang
         size, size, 0.0f,  1.0f, 0.5f, 0.75f,  // pink terang
         0.0f, size + bladeLength, 0.0f,  1.0f, 0.95f, 0.98f,  // hampir putih
        
        // Segitiga bawah - gradasi pink ke putih
        -size, -size, 0.0f,  1.0f, 0.5f, 0.75f,  // pink terang
         size, -size, 0.0f,  1.0f, 0.5f, 0.75f,  // pink terang
         0.0f, -size - bladeLength, 0.0f,  1.0f, 0.95f, 0.98f,  // hampir putih
        
        // Segitiga kanan - gradasi pink ke putih
        size, -size, 0.0f,  1.0f, 0.5f, 0.75f,  // pink terang
        size,  size, 0.0f,  1.0f, 0.5f, 0.75f,  // pink terang
        size + bladeLength, 0.0f, 0.0f,  1.0f, 0.95f, 0.98f,  // hampir putih
        
        // Segitiga kiri - gradasi pink ke putih
        -size, -size, 0.0f,  1.0f, 0.5f, 0.75f,  // pink terang
        -size,  size, 0.0f,  1.0f, 0.5f, 0.75f,  // pink terang
        -size - bladeLength, 0.0f, 0.0f,  1.0f, 0.95f, 0.98f,  // hampir putih
    };

    // Buat VAO dan VBO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Atribut posisi
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atribut warna
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Variabel untuk animasi rotasi
    float angle = 0.0f;

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Background
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Gunakan shader program
        glUseProgram(shaderProgram);

        // Update rotasi (diperlambat)
        angle += 0.002f;
        if (angle > 2 * 3.14159f) angle -= 2 * 3.14159f;

        // Buat matrix transformasi
        float transform[16];
        createRotationMatrix(angle, transform);

        // Kirim matrix ke shader
        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform);

        // Render shuriken
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 18);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}