#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
#include <filesystem>
#include <cstdio>

// Jika mau coba path absolut sementara, isi di bawah (atau kosongkan)
// Contoh: "D:/Kuliah/GrafKom/install stb_image/resources/basecolor.png"
#define ABSOLUTE_TEX_PATH ""

// Vertex Shader (ditambah texCoord)
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(aPos, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
}
)";

// Fragment Shader (meng-sample texture dan dikali warna)
// Jika texture gagal, we still multiply with vertex color because shader reads texture (we'll bind a 1x1 white if needed)
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D myTexture;

void main()
{
    vec4 texColor = texture(myTexture, TexCoord);
    FragColor = texColor * vec4(ourColor, 1.0);
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

// Fungsi untuk membuat transform matrix rotasi (kolom-major)
void createRotationMatrix(float angle, float* matrix) {
    float c = cos(angle);
    float s = sin(angle);
    
    matrix[0]  = c;  matrix[1]  = s;   matrix[2]  = 0;  matrix[3]  = 0;
    matrix[4]  = -s; matrix[5]  = c;   matrix[6]  = 0;  matrix[7]  = 0;
    matrix[8]  = 0;  matrix[9]  = 0;   matrix[10] = 1; matrix[11] = 0;
    matrix[12] = 0; matrix[13] = 0;  matrix[14] = 0; matrix[15] = 1;
}

int main() {
    // Inisialisasi GLFW
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // OpenGL 3.3 core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Buat window
    GLFWwindow* window = glfwCreateWindow(800, 800, "Shuriken OpenGL (stb_image debug)", NULL, NULL);
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

    // --- Compile shaders ---
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    {
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    {
        int success;
        char infoLog[512];
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    {
        int success;
        char infoLog[512];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Data vertices untuk shuriken
    float size = 0.3f;
    float bladeLength = 0.5f;
    
    // Vertices: posisi (x,y,z), warna (r,g,b) dan texCoord (u,v)
    // Stride = 8 floats
    float vertices[] = {
        // Persegi tengah (2 segitiga)
        -size, -size, 0.0f,  1.0f, 0.4f, 0.7f,   0.0f, 0.0f,
         size, -size, 0.0f,  1.0f, 0.4f, 0.7f,   1.0f, 0.0f,
        -size,  size, 0.0f,  1.0f, 0.4f, 0.7f,   0.0f, 1.0f,

         size, -size, 0.0f,  1.0f, 0.4f, 0.7f,   1.0f, 0.0f,
         size,  size, 0.0f,  1.0f, 0.4f, 0.7f,   1.0f, 1.0f,
        -size,  size, 0.0f,  1.0f, 0.4f, 0.7f,   0.0f, 1.0f,

        // Segitiga atas
        -size, size, 0.0f,   1.0f, 0.5f, 0.75f,  0.0f, 1.0f,
         size, size, 0.0f,   1.0f, 0.5f, 0.75f,  1.0f, 1.0f,
         0.0f, size + bladeLength, 0.0f,  1.0f, 0.95f, 0.98f,  0.5f, 1.5f,

        // Segitiga bawah
        -size, -size, 0.0f,  1.0f, 0.5f, 0.75f,  0.0f, 0.0f,
         size, -size, 0.0f,  1.0f, 0.5f, 0.75f,  1.0f, 0.0f,
         0.0f, -size - bladeLength, 0.0f,  1.0f, 0.95f, 0.98f,  0.5f, -0.5f,

        // Segitiga kanan
        size, -size, 0.0f,   1.0f, 0.5f, 0.75f,  1.0f, 0.0f,
        size,  size, 0.0f,   1.0f, 0.5f, 0.75f,  1.0f, 1.0f,
        size + bladeLength, 0.0f, 0.0f,  1.0f, 0.95f, 0.98f,  1.5f, 0.5f,

        // Segitiga kiri
        -size, -size, 0.0f,  1.0f, 0.5f, 0.75f,  0.0f, 0.0f,
        -size,  size, 0.0f,  1.0f, 0.5f, 0.75f,  0.0f, 1.0f,
        -size - bladeLength, 0.0f, 0.0f,  1.0f, 0.95f, 0.98f,  -0.5f, 0.5f,
    };

    // Buat VAO dan VBO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Atribut posisi (location = 0) -> 3 floats, offset 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atribut warna (location = 1) -> 3 floats, offset 3*sizeof(float)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Atribut texCoord (location = 2) -> 2 floats, offset 6*sizeof(float)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // --- Setup texture menggunakan stb_image (dengan debug + fallback) ---
    const char* relativeTexPath = "resources/basecolor.png";
    const char* texPath = relativeTexPath;

    if (std::string(ABSOLUTE_TEX_PATH).size() > 0) {
        texPath = ABSOLUTE_TEX_PATH; // gunakan path absolut kalau diset
    }

    std::cout << "[DEBUG] CWD: " << std::filesystem::current_path() << std::endl;
    std::cout << "[DEBUG] Trying texture path: " << texPath << std::endl;

    // cek eksistensi file
    FILE* f = fopen(texPath, "rb");
    if (!f && texPath == relativeTexPath) {
        // coba absolute path based on current folder + relative (just in case)
        std::filesystem::path alt = std::filesystem::current_path() / relativeTexPath;
        std::string altStr = alt.string();
        std::cout << "[DEBUG] Relative file not found. Trying: " << altStr << std::endl;
        f = fopen(altStr.c_str(), "rb");
        if (f) texPath = altStr.c_str();
    }

    if (f) {
        std::cout << "[DEBUG] File exists: " << texPath << std::endl;
        fclose(f);
    } else {
        std::cout << "[DEBUG] File NOT found at: " << texPath << std::endl;
        std::cout << "[SUGGESTION] copy basecolor.png to exe dir or set ABSOLUTE_TEX_PATH or configure debugger cwd." << std::endl;
    }

    // Generate texture object
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set wrapping & filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // IMPORTANT: atur unpack alignment supaya row alignment tidak merusak data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // load image only if file exists
    int width = 0, height = 0, nrChannels = 0;
    unsigned char* data = nullptr;
    if (std::filesystem::exists(texPath)) {
        stbi_set_flip_vertically_on_load(true);
        data = stbi_load(texPath, &width, &height, &nrChannels, 0);
    } else {
        data = nullptr;
    }

    bool textureLoaded = false;
    if (data) {
        std::cout << "[DEBUG] Texture loaded: " << width << "x" << height << " channels:" << nrChannels << std::endl;
        GLenum format = GL_RGB;
        GLenum internalFormat = GL_RGB8;
        if (nrChannels == 1) { format = GL_RED; internalFormat = GL_R8; }
        else if (nrChannels == 3) { format = GL_RGB; internalFormat = GL_RGB8; }
        else if (nrChannels == 4) { format = GL_RGBA; internalFormat = GL_RGBA8; }

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        textureLoaded = true;
    } else {
        std::cout << "[WARN] Failed to load texture at: " << texPath << std::endl;
        std::cout << "[WARN] The program will continue and use vertex colors only." << std::endl;

        // buat fallback 1x1 white texture supaya shader tetap punya sesuatu untuk di-sample
        unsigned char whitePixel[4] = { 255, 255, 255, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
        // no mipmaps needed for 1x1 but generate anyway
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Set uniform sampler di shader (satu kali)
    glUseProgram(shaderProgram);
    int texLoc = glGetUniformLocation(shaderProgram, "myTexture");
    glUniform1i(texLoc, 0); // texture unit 0

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
        if (angle > 2.0f * 3.14159265f) angle -= 2.0f * 3.14159265f;

        // Buat matrix transformasi
        float transform[16];
        createRotationMatrix(angle, transform);

        // Kirim matrix ke shader
        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform);

        // Bind texture ke texture unit 0 sebelum render
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

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
    glDeleteTextures(1, &texture);

    glfwTerminate();
    return 0;
}
