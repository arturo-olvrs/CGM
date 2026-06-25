#include <fstream>

#include <GLApp.h>

class MyGLApp : public GLApp {
private:
  const float degreePreSecond{ 45.0f };


  Mat4 projection{};
  GLuint program{0};
  GLint modelViewMatrixUniform{-1};
  GLint projectionMatrixUniform{-1};

  constexpr static float sqrt3{ 1.7320508076f };

  const static int numTriangles{ 3 };
  GLuint vbo{0};
  GLuint vaos[numTriangles] = {0};
  Mat4 modelViews[numTriangles];

  constexpr static GLfloat triangles[numTriangles*3*3*2] = {
    // Botton left triangle. Magenta, blue, cyan
    -sqrt3, -1.5, 0.0f,    0.0f, 0.0f, 1.0f,
     0.0f, -1.5, 0.0f,    0.0f, 1.0f, 1.0f,
    -sqrt3/2,   0.0f, 0.0f,    1.0f, 0.0f, 1.0f,

    // Bottom right triangle. Yellow, cyan, green
     0.0f, -1.5, 0.0f,    0.0f, 1.0f, 1.0f,
     sqrt3, -1.5f, 0.0f,    0.0f, 1.0f, 0.0f,
     sqrt3/2,   0.0f, 0.0f,    1.0f, 1.0f, 0.0f,

    // Top triangle. Red, magenta, yellow
    -sqrt3/2,  0.0f, 0.0f,     1.0f, 0.0f, 1.0f,
     sqrt3/2,  0.0f, 0.0f,     1.0f, 1.0f, 0.0f,
     0.0f, 1.5f, 0.0f,     1.0f, 0.0f, 0.0f
  };


public:
  
  MyGLApp()
    : GLApp(800,600,4,"Assignment 11 - Triforce")
  {}
  
  virtual void init() override {
    setupShaders();
    setupGeometry();
    GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    resetAnimation();
    setAnimation(false);
  }

  virtual void animate(double animationTime) override {
    double angle = animationTime * degreePreSecond;
    
    // Triangle 0 -> bottom left -> z-axis, centered at its geometric centroid.
    Vec3 center = (Vec3(-2,-sqrt3,0) + Vec3(0,-sqrt3,0) + Vec3(-1,0,0))/3;
    modelViews[0] = Mat4::translation(center) *
                    Mat4::rotationZ(angle) *
                    Mat4::translation(Vec3()-center);

    // Triangle 1 -> bottom right -> y-axis, centered at half its width
    center = Vec3(1,-sqrt3/2,0);
    modelViews[1] = Mat4::translation(center) *
                    Mat4::rotationY(angle) *
                    Mat4::translation(Vec3()-center);

    // Triangle 2 -> top -> x-axis, centered at half its hight
    center = Vec3(0, sqrt3/2, 0);
    modelViews[2] = Mat4::translation(center) *
                    Mat4::rotationX(angle) *
                    Mat4::translation(Vec3()-center);
  }

  virtual void draw() override {
    GL(glClear(GL_COLOR_BUFFER_BIT));
    GL(glUseProgram(program));

    for (int i = 0; i < numTriangles; ++i){
      GL(glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_TRUE, modelViews[i]));

      GL(glBindVertexArray(vaos[i]));
      GL(glDrawArrays(GL_TRIANGLES, 0, 3));
    }
    
    GL(glBindVertexArray(0));
    GL(glUseProgram(0));
  }

  virtual void resize(const Dimensions winDim, const Dimensions fbDim) override{
    GLApp::resize(winDim, fbDim);

    const float ratio = fbDim.aspect();
    projection = Mat4::ortho(-ratio * 1.5f, ratio * 1.5f, -1.5f, 1.5f, -10.0f, 10.0f);
    GL(glUseProgram(program));
    GL(glUniformMatrix4fv(projectionMatrixUniform, 1, GL_TRUE, projection));
    GL(glUseProgram(0));
  }

  std::string loadFile(const std::string& filename) {
    std::ifstream shaderFile{ filename };
    if (!shaderFile)
    {
      throw GLException{ std::string("Unable to open file ") + filename };
    }
    std::string str;
    std::string fileContents;
    while (std::getline(shaderFile, str)) {
      fileContents += str + "\n";
    }
    return fileContents;
  }
  
  GLuint createShaderFromFile(GLenum type, const std::string& sourcePath) {
    const std::string shaderCode = loadFile(sourcePath);
    const std::string fullSource = GLProgram::getShaderPreamble() + shaderCode;
    const GLchar* c_shaderCode = fullSource.c_str();
    const GLuint s = glCreateShader(type);
    GL(glShaderSource(s, 1, &c_shaderCode, NULL));
    glCompileShader(s); checkAndThrowShader(s);
    return s;
  }
  
  void setupShaders() {
    const std::string vertexSrcPath = "vertexShader.vert";
    const std::string fragmentSrcPath = "fragmentShader.frag";
    const GLuint vertexShader = createShaderFromFile(GL_VERTEX_SHADER, vertexSrcPath);
    const GLuint fragmentShader = createShaderFromFile(GL_FRAGMENT_SHADER, fragmentSrcPath);
    
    program = glCreateProgram();
    GL(glAttachShader(program, vertexShader));
    GL(glAttachShader(program, fragmentShader));
    GL(glLinkProgram(program));
    checkAndThrowProgram(program);
    
    GL(glUseProgram(program));
    modelViewMatrixUniform = glGetUniformLocation(program, "modelViewMatrix");
    projectionMatrixUniform = glGetUniformLocation(program, "projectionMatrix");
    GL(glUseProgram(0));
  }
  
  void setupGeometry() {
    const GLuint vertexPos = GLuint(glGetAttribLocation(program, "vertexPosition"));
    const GLuint vertexCol = GLuint(glGetAttribLocation(program, "vertexColor"));

    GL(glGenVertexArrays(numTriangles, vaos));
    GL(glGenBuffers(1, &vbo));
    GL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL(glBufferData(GL_ARRAY_BUFFER, sizeof(triangles), triangles, GL_STATIC_DRAW));

    for (int i = 0; i < numTriangles; ++i)
    {
      GL(glBindVertexArray(vaos[i]));
      GL(glBindBuffer(GL_ARRAY_BUFFER, vbo));

      GL(glEnableVertexAttribArray(vertexPos));
      GL(glVertexAttribPointer(vertexPos, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(i * 9 * 2 * sizeof(float))));   // 9 floats per triangle, 2 attributes (pos, col)
      
      GL(glEnableVertexAttribArray(vertexCol));
      GL(glVertexAttribPointer(vertexCol, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)((i * 9 * 2 + 3) * sizeof(float))));   // +3 offset for color
      
      GL(glBindVertexArray(0););
    }
  }
  
  virtual void keyboard(int key, int scancode, int action, int mods) override
  {
    if (key == GLENV_KEY_ESCAPE && action == GLENV_PRESS)
      closeWindow();
    
    else if (key == GLENV_KEY_R && action == GLENV_PRESS)
      resetAnimation();

    else if (key == GLENV_KEY_SPACE && action == GLENV_PRESS)
      setAnimation(!getAnimation());
  }
} myApp;

#ifdef _WIN32
#include <Windows.h>

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
    std::vector<std::string> args = getArgsWindows();
#else
int main(int argc, char** argv) {
    std::vector<std::string> args{ argv + 1, argv + argc };
#endif
    try {
        myApp.run();
    }
    catch (const GLException& e) {
        std::stringstream ss;
        ss << "Insufficient OpenGL Support " << e.what();
#ifndef _WIN32
        std::cerr << ss.str().c_str() << std::endl;
#else
        MessageBoxA(
            NULL,
            ss.str().c_str(),
            "OpenGL Error",
            MB_ICONERROR | MB_OK
        );
#endif
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
