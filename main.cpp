#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>


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
const float camRotSpeed = 0.006f;

double lmx = 0.0f;
double lmy = 0.0f;
char firstMouse = 1;

float typeColors[][3] = {
    {1.0f,1.0f,1.0f},
    {1.0f,0.0f,0.0f}
};

struct Platform {
    float x;    
    float y;
    float z;
    float xr;
    float yr;
    float xl;
    float yl;
    float zl;
    uint8_t type;
    glm::mat4 model;
    float color[3];
};

Platform* platforms[300];
unsigned int numPlatforms = 0;

void addPlatform(float px,float py,float pz,float pxr,float pyr,float pxl,float pyl,float pzl,unsigned char ptype) {
    Platform* a = (Platform*)malloc(sizeof(Platform));
    (*a).x=px;
    (*a).y=py;
    (*a).z=pz;
    (*a).xr=pxr;
    (*a).yr=pyr;
    (*a).xl=pxl;
    (*a).yl=pyl;
    (*a).zl=pzl;
    (*a).type=ptype;
    
    (*a).color[0] = typeColors[ptype][0];
    (*a).color[1] = typeColors[ptype][1];
    (*a).color[2] = typeColors[ptype][2];

    glm::quat quaternion = glm::quat(glm::vec3(pxr,pyr,0));
    glm::mat4 rMatrix = glm::toMat4(quaternion);
    glm::mat4 sMatrix = glm::scale(glm::vec3(pxl,pyl,pzl));
    glm::mat4 tMatrix = glm::translate(glm::vec3(px,py,pz));

    (*a).model = tMatrix * rMatrix * sMatrix * glm::mat4(1.0f);
    
    platforms[numPlatforms] = a;
    numPlatforms+=1; 
}

void popPlatform() {
    numPlatforms-=1;
    free(platforms[numPlatforms]);
}

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

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    float color[] = {1.0f,0.0f,0.0f};

    projection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 500.0f);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

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

    //setup level
    glm::vec3 plPos = glm::vec3(0.0f,3 .0f,0.0f);
    glm::vec3 plVel = glm::vec3(0.0f,0.0f,0.0f);
    bool onGround = true;
    float dragCoefficient = 0.004f;
    float frictionCoefficient = 0.096f;
    float gravityConstant = 0.05f;
    float plSpeed = 0.1f;
    float plStrafeSpeed = 0.02f;

    //coefficient for max speed on ground is movespeed / (dragCoefficient + frictionCoefficient)^2
    
    
    addPlatform(0,-8,-2,0,0,3,1,10,0);
    








    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            if (onGround) {plVel += camDir * plSpeed;}
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            if (onGround) {plVel -= glm::normalize(glm::cross(camDir, camUp)) * plStrafeSpeed;}
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            if (onGround) {plVel -= camDir * plSpeed;}
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            if (onGround) {plVel += glm::normalize(glm::cross(camDir, camUp)) * plStrafeSpeed;}
        }

        float plVelMagnitude = glm::length(plVel);
        plVel.y -= gravityConstant;
        
        //friction / air resistance
        if (onGround) {
            plVel = plVel - plVel*(dragCoefficient+frictionCoefficient)*plVelMagnitude;
        } else {
            plVel = plVel - plVel*dragCoefficient*plVelMagnitude;
        }

        plPos += plVel;
        camPos = plPos;


        camDir.x = cos(camxr) * cos(camyr);
        camDir.y = sin(camyr);
        camDir.z = sin(camxr) * cos(camyr);
        //camDir = glm::normalize(camDir); //probably unnecessary

        view = glm::lookAt(camPos, camPos + camDir, camUp);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        
        /*struct timeval tv;
        gettimeofday(&tv, NULL);
        float time = (float)tv.tv_usec + (float)(1000000 * (tv.tv_sec%(60*60*24)));*/
        
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

        //glUniform1f(timeLocation, (float)sin(time/1000000.0f)/10.0f);


        glClearColor(0.15f, 0.4f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (int i = 0; i < numPlatforms; i++) {
            Platform a = *platforms[i];
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(a.model));
            glUniform3f(colorLocation, a.color[0],a.color[1],a.color[2]);
            
            glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);

        }
        
        //draw
        
        
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        usleep(1000000/100); //100fps
    }


    return 0;
}