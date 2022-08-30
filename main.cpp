#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

uint32_t VBO;
uint32_t VAO;
uint32_t EBO;
unsigned int shaderProgram;

const char* GetGLErrorStr(GLenum err)
{
    switch (err)
    {
    case GL_NO_ERROR:          return "No error";
    case GL_INVALID_ENUM:      return "Invalid enum";
    case GL_INVALID_VALUE:     return "Invalid value";
    case GL_INVALID_OPERATION: return "Invalid operation";
    case GL_STACK_OVERFLOW:    return "Stack overflow";
    case GL_STACK_UNDERFLOW:   return "Stack underflow";
    case GL_OUT_OF_MEMORY:     return "Out of memory";
    default:                   return "Unknown error";
    }
}

float camxr = M_PI_2;
float camyr = 0.0f;
const float camSpeed = 0.1f;
const float camRotSpeed = 0.008f;
double lmx = 0.0f;
double lmy = 0.0f;
char firstMouse = 1;

void mouse_callback(GLFWwindow* window, double dmposx, double dmposy) {
        if (firstMouse) {
            lmx = dmposx;
            lmy = dmposy;
            firstMouse = 0;
        }
    
    float mposx=((float)dmposx-lmx)*camRotSpeed;
    float mposy=((float)dmposy-lmy)*camRotSpeed;
    lmx = dmposx;    
    lmy = dmposy;

    camxr += mposx;
    if (camxr > M_PI) {camxr -= M_PI*2;}
    if (camxr < -M_PI) {camxr += M_PI*2;}

    camyr -= mposy;
    if (camyr > M_PI_2) {camyr=M_PI_2*0.99;}
    if (camyr < -M_PI_2) {camyr=-M_PI_2*.99;}
}

int main() {
    uint32_t width=600;
    uint32_t height=600;

    

    const char* vertexShaderSrc = "#version 330 core\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    //"uniform float time;\n"
    "layout (location=0) in vec3 pos;\n"
    "void main(){\n"
    "   gl_Position=projection * view * model * vec4(pos,1);}\0";

    const char* fragmentShaderSrc = "#version 330 core\n"
    "out vec4 outColor;\n"
    "uniform vec3 color;\n"
    "void main(){outColor=vec4(color,1);}\0";

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width,height,"test",NULL,NULL);
    if(window==NULL){
        printf("no window :(");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    if(GLEW_OK!=glewInit()){
        printf("glew init error :(");
        return 1;
    }

    glViewport(0,0,600,600);

    int success;
    char infoLog[512];
    uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader,1,&vertexShaderSrc,NULL);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader,GL_COMPILE_STATUS,&success);
    if(!success){
        glGetShaderInfoLog(vertexShader,512,NULL,infoLog);
        printf(infoLog);
        return 1;
    }

    uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader,1,&fragmentShaderSrc,NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader,GL_COMPILE_STATUS,&success);
    if(!success){
        glGetShaderInfoLog(fragmentShader,512,NULL,infoLog);
        printf(infoLog);
        return 1;
    }



    shaderProgram=glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram,GL_LINK_STATUS,&success);
    
    
    if(!success){
        glGetProgramInfoLog(shaderProgram,512,NULL,infoLog);
        printf(infoLog);
        return 1;
    }


    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
    glfwSetCursorPosCallback(window, mouse_callback);  

    float squarePoints[] = {
        -1,1,1,
        1,1,1,
        1,1,-1,
        -1,1,-1,
        -1,-1,1,
        1,-1,1,
        -1,-1,-1,
        1,-1,-1
    };

    unsigned int squareInds[] = {
        0,1,2,
        0,3,2,
        0,3,6,
        0,4,6,
        3,2,7,
        3,6,7,
        2,1,5,
        2,7,5,
        1,0,4,
        1,5,4,
        4,5,7,
        4,6,7
    };

    int modelLocation = glGetUniformLocation(shaderProgram, "model");
    int viewLocation = glGetUniformLocation(shaderProgram, "view");
    int projectionLocation = glGetUniformLocation(shaderProgram, "projection");
    int colorLocation = glGetUniformLocation(shaderProgram, "color");
    //int timeLocation = glGetUniformLocation(shaderProgram, "time");

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    float color[] = {1.0f,0.0f,0.0f};

    projection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 100.0f);

    glm::vec3 camPos = glm::vec3(0.0f,0.0f,-10.0f);
    glm::vec3 camDir = glm::vec3(0.0f,0.0f,1.0f);
    glm::vec3 camUp = glm::vec3(0.0f,1.0f,0.0f);

    //shader stuff
    glUseProgram(shaderProgram);
    
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glGenBuffers(1,&EBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squarePoints), squarePoints, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareInds), squareInds, GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camPos += camDir * camSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camPos -= glm::normalize(glm::cross(camDir, camUp)) * camSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camPos -= camDir * camSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camPos += glm::normalize(glm::cross(camDir, camUp)) * camSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            camPos.y -= camSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            camPos.y += camSpeed;
        }

        
        camDir.x = cos(camxr) * cos(camyr);
        camDir.y = sin(camyr);
        camDir.z = sin(camxr) * cos(camyr);
        camDir = glm::normalize(camDir);

        view = glm::lookAt(camPos, camPos + camDir, camUp);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        
        /*struct timeval tv;
        gettimeofday(&tv, NULL);
        float time = (float)tv.tv_usec + (float)(1000000 * (tv.tv_sec%(60*60*24)));*/
        

        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(colorLocation, color[0],color[1],color[2]);
        //glUniform1f(timeLocation, (float)sin(time/1000000.0f)/10.0f);

        //draw
        glClearColor(0.1f, 0.1f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        usleep(1000000/100); //100fps
    }


    return 0;
}