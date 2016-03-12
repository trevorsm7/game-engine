#include "GlfwRenderer.h"
#include "GlfwTexture.h"

#include <iostream>

bool GlfwRenderer::init()
{
    //glDisable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.1, 0.0, 0.0, 0.0);

    // TODO: quick hack to get minimum viable GL display working

    const char* vertexShader =
    "#version 410\n"
    "uniform vec2 u_cameraScale;\n"
    "uniform vec2 u_cameraOffset;\n"
    "uniform vec2 u_modelScale;\n"
    "uniform vec2 u_modelOffset;\n"
    "uniform vec2 u_textureScale;\n"
    "uniform vec2 u_textureOffset;\n"
    "layout(location = 0) in vec2 a_vertex;\n"
    "out vec2 v_texCoord;\n"
    "void main() {\n"
    "v_texCoord = a_vertex * u_textureScale + u_textureOffset;\n"
    "gl_Position = vec4((a_vertex * u_modelScale + u_modelOffset - u_cameraOffset) * 2 / u_cameraScale - vec2(1, 1), 0.0, 1.0);\n"
    "}\n";

    const char* fragmentShader =
    "#version 410\n"
    "uniform vec3 u_color;\n"
    "uniform sampler2D u_texture;\n"
    "in vec2 v_texCoord;\n"
    "out vec4 f_color;\n"
    "void main() {\n"
    "f_color = vec4(u_color * texture(u_texture, v_texCoord).rgb, 1.0);\n"
    "}\n";

    GLuint program = glCreateProgram();
    glAttachShader(program, loadShader(vertexShader, GL_VERTEX_SHADER));
    glAttachShader(program, loadShader(fragmentShader, GL_FRAGMENT_SHADER));
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE)
    {
        // Get the length of the error log
        GLint logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        // Get the error log and print
        std::unique_ptr<char[]> errorLog(new char [logLength]);
        glGetProgramInfoLog(program, logLength, &logLength, errorLog.get());
        std::cerr << errorLog.get() << std::endl;

        // Exit with failure
        glDeleteProgram(program);
        return false;
    }

    glUseProgram(program);

    m_modelScale = glGetUniformLocation(program, "u_modelScale");
    m_modelOffset = glGetUniformLocation(program, "u_modelOffset");
    m_cameraScale = glGetUniformLocation(program, "u_cameraScale");
    m_cameraOffset = glGetUniformLocation(program, "u_cameraOffset");
    m_textureScale = glGetUniformLocation(program, "u_textureScale");
    m_textureOffset = glGetUniformLocation(program, "u_textureOffset");
    m_color = glGetUniformLocation(program, "u_color");
    glUniform1i(glGetUniformLocation(program, "u_texture"), 0);

    // TODO: import some real mesh loading code

    struct {float x, y;} vertices[4] =
    {
        {0.f, 0.f},
        {1.f, 0.f},
        {1.f, 1.f},
        {0.f, 1.f}
    };

    GLuint indices[4] = {0, 1, 3, 2};

    // Generate vertex array object
    glGenVertexArrays(1, &m_spriteVAO);
    glBindVertexArray(m_spriteVAO);

    // Buffer the vertex data
    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Enable vertex attribute pointer
    //GLint a_vertex = program.getAttribLocation("a_vertex");
    GLint a_vertex = 0;
    glEnableVertexAttribArray(a_vertex);
    glVertexAttribPointer(a_vertex, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Buffer the index data
    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    return true;
}

GLuint GlfwRenderer::loadShader(const char* shaderCode, GLenum shaderType)
{
    // Compile the shader file
    GLuint shader = glCreateShader(shaderType);
    const char* sourceArray[] = {shaderCode};
    glShaderSource(shader, 1, sourceArray, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE)
    {
        // Get the length of the error log
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        // Get the error log and print
        std::unique_ptr<char[]> errorLog(new char [logLength]);
        glGetShaderInfoLog(shader, logLength, &logLength, errorLog.get());
        std::cerr << errorLog.get() << std::endl;

        // Exit with failure
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

void GlfwRenderer::preRender()
{
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GlfwRenderer::postRender()
{
    // Update window and events
    glfwSwapBuffers(m_window);
}

void GlfwRenderer::pushModelTransform(Transform& transform)
{
    m_model = transform;
    glUniform2f(m_modelOffset, transform.getX(), transform.getY());
    glUniform2f(m_modelScale, transform.getW(), transform.getH());
}

void GlfwRenderer::pushCameraTransform(Transform& transform)
{
    glUniform2f(m_cameraOffset, transform.getX(), transform.getY());
    glUniform2f(m_cameraScale, transform.getW(), transform.getH());
}

void GlfwRenderer::setColor(float red, float green, float blue)
{
    glUniform3f(m_color, red, green, blue);
}

void GlfwRenderer::drawSprite(const std::string& name)
{
    // Get texture resource
    GlfwTexturePtr texture = GlfwTexture::loadTexture(m_resources, name);
    if (!texture)
        return; // TODO: we should at least have a placeholder instead of null; assert here?

    // Bind the texture resource
    texture->bind();
    glUniform2f(m_textureOffset, 0.f, 0.f);
    glUniform2f(m_textureScale, 1.f, 1.f);

    // Bind arrays and send draw command
    glBindVertexArray(m_spriteVAO);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void GlfwRenderer::drawTiles(const TileIndex& index, const TileMap& tiles)
{
    // Get texture resource
    GlfwTexturePtr texture = GlfwTexture::loadTexture(m_resources, index.name);
    if (!texture)
        return; // TODO: we should at least have a placeholder instead of null; assert here?

    // Bind the texture and the sprite vao
    texture->bind();
    glBindVertexArray(m_spriteVAO);

    // Compute and set texture size of one tile
    const float tileW = 1.f / index.w;
    const float tileH = 1.f / index.h;
    glUniform2f(m_textureScale, tileW, tileH);

    // Compute and set model size of one tile
    const float modelW = m_model.getW() / tiles.w;
    const float modelH = m_model.getH() / tiles.h;
    glUniform2f(m_modelScale, modelW, modelH);

    const float originY = m_model.getY() + m_model.getH() - modelH;

    // Iterate over tile (x, y) indices
    int i = 0;
    for (int y = 0; y < tiles.h; ++y)
    {
        for (int x = 0; x < tiles.w; ++x)
        {
            const int tile = tiles.tiles[i++];//y * tiles.w + x];

            // Index tiles from top-left
            const float tileX = (tile % index.w) * tileW;
            const float tileY = 1.f - ((tile / index.w) + 1) * tileH;
            glUniform2f(m_textureOffset, tileX, tileY);

            // Draw tilemap from top-left
            const float modelX = m_model.getX() + x * modelW;
            const float modelY = originY - y * modelH;
            glUniform2f(m_modelOffset, modelX, modelY);

            // Draw the tile
            glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
        }
    }

    glBindVertexArray(0);
}

void GlfwRenderer::popModelTransform()
{
    // TODO: do we need a transform stack?
}

void GlfwRenderer::popCameraTransform()
{

}
