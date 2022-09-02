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

float typeColors[][6] = {
    {1.0f,1.0f,1.0f, 0.8f,0.8f,0.8f},
    {0.8f,0.1f,0.1f, 1.0f,0.0f,0.0f}
};

glm::vec3 plPos = glm::vec3(0.05f,0.05f,0.05f);
glm::vec3 plVel = glm::vec3(0.0f,0.0f,0.0f);
bool onGround = true;
float dragCoefficient = 0.03f;
float frictionCoefficient = 0.5f;
float gravityConstant = 0.001f;
float plSpeed = 0.018f;
float plStrafeSpeed = plSpeed/2;
float plAirSpeed = plSpeed/8;
float plHeight = 2.0f;
float plJumpForce = 0.08f;
//coefficient for max speed on ground is movespeed / (dragCoefficient + frictionCoefficient)^2
    

typedef struct Platform {
    glm::vec3 pos;
    //glm::vec3 halfLen;
    float yaw;
    float pitch;
    float xl;
    float yl;
    float zl;
    uint8_t type;
    glm::mat4 model;
    //float color[3];
    //float color2[3];
    //glm::vec3 collisionPoint1;
    glm::vec3 collisionPoint5;
    glm::vec3 collisionPoint6;
    glm::vec3 collisionPoint7;    
    float collisionProduct1,collisionProduct2,collisionProduct3,collisionProduct4,collisionProduct5,collisionProduct6;
    glm::quat quaternion;
    float sinPitch;

} Platform;

Platform* platforms[300];
unsigned int numPlatforms = 0;

void addPlatform(float px,float py,float pz,float yaw,float pitch,float pxl,float pyl,float pzl,unsigned char ptype) {
    Platform* a = (Platform*)malloc(sizeof(Platform));
    (*a).pos = glm::vec3(px,py,pz);
    //(*a).halfLen = glm::vec3(pxl,pyl,pzl)/2.0f;
    (*a).yaw=yaw;
    (*a).pitch=pitch;
    (*a).xl=pxl;
    (*a).yl=pyl;
    (*a).zl=pzl;
    (*a).type=ptype;
    
    /*(*a).color[0] = typeColors[ptype*2][0];
    (*a).color[1] = typeColors[ptype*2][1];
    (*a).color[2] = typeColors[ptype*2][2];
    (*a).color2[0] = typeColors[ptype*2+1][0];
    (*a).color2[1] = typeColors[ptype*2+1][1];
    (*a).color2[2] = typeColors[ptype*2+1][2];*/

    glm::quat quaternion = glm::quat(glm::vec3(pitch,yaw,0));
    glm::mat4 rMatrix = glm::toMat4(quaternion);
    glm::mat4 sMatrix = glm::scale(glm::vec3(pxl/2,pyl/2,pzl/2));
    glm::mat4 tMatrix = glm::translate(glm::vec3(px,py,pz));

    (*a).model = tMatrix * rMatrix * sMatrix * glm::mat4(1.0f);
    //pitch += M_PI_2;
    //(*a).rotationThing = glm::vec3(-cos(pitch)*cos(yaw), sin(pitch), - cos(pitch)*sin(yaw));
    (*a).quaternion = quaternion;
    (*a).sinPitch = sin(pitch);

    glm::vec3 collisionPoint1 = quaternion * glm::vec3(-pxl/2,-pyl/2,-pzl/2);
    glm::vec3 collisionPoint2 = quaternion * glm::vec3(pxl/2,-pyl/2,-pzl/2);
    glm::vec3 collisionPoint3 = quaternion * glm::vec3(-pxl/2,pyl/2,-pzl/2);
    glm::vec3 collisionPoint4 = quaternion * glm::vec3(-pxl/2,-pyl/2,pzl/2);
    (*a).collisionPoint5 = collisionPoint1 - collisionPoint2;
    (*a).collisionPoint6 = collisionPoint1 - collisionPoint3;
    (*a).collisionPoint7 = collisionPoint1 - collisionPoint4;

    (*a).collisionProduct1 = glm::dot((*a).collisionPoint5, collisionPoint1);
    (*a).collisionProduct2 = glm::dot((*a).collisionPoint5, collisionPoint2);
    (*a).collisionProduct3 = glm::dot((*a).collisionPoint6, collisionPoint1);
    (*a).collisionProduct4 = glm::dot((*a).collisionPoint6, collisionPoint3);
    (*a).collisionProduct5 = glm::dot((*a).collisionPoint7, collisionPoint1);
    (*a).collisionProduct6 = glm::dot((*a).collisionPoint7, collisionPoint4);




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
    addPlatform(0,-6,0,0,0,10,2,20,0);
    addPlatform(3,-6,35,M_PI_2,0.2,40,2,20,0);
    
















    float sinPitch = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            if (onGround) {
                plVel += glm::normalize(glm::vec3(camDir.x, 0, camDir.z)) * plSpeed;
            } else {
                plVel += glm::normalize(glm::vec3(camDir.x, 0, camDir.z)) * plAirSpeed;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            if (onGround) {
                plVel += glm::normalize(glm::vec3(camDir.z, 0, -camDir.x)) * plStrafeSpeed;
            } else {
                plVel += glm::normalize(glm::vec3(camDir.z, 0, -camDir.x)) * plAirSpeed;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            if (onGround) {
                plVel -= glm::normalize(glm::vec3(camDir.x, 0, camDir.z)) * plStrafeSpeed;
            } else {
                plVel -= glm::normalize(glm::vec3(camDir.x, 0, camDir.z)) * plAirSpeed;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            if (onGround) {
                plVel -= glm::normalize(glm::vec3(camDir.z, 0, -camDir.x)) * plStrafeSpeed;
            } else {
                plVel -= glm::normalize(glm::vec3(camDir.z, 0, -camDir.x)) * plAirSpeed;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (onGround) {
                plVel += glm::vec3(0, 1, 0) * plJumpForce;
                plPos += plVel;
            }
        }

        //check on ground
        onGround = false;
        sinPitch = 0;
        for (int i = 0; i < numPlatforms; i++) {
            Platform platform = *platforms[i];
            glm::vec3 groundTestPoint = plPos - platform.pos;
            groundTestPoint.y -= plHeight;
            //groundTestPoint = (platform.rotationThing * glm::length(groundTestPoint));
            
            //groundTestPoint =  groundTestPoint + platform.halfLen;
            
            //printf("%f\n%f\n%f\n\n",groundTestPoint.x,groundTestPoint.y,groundTestPoint.z);

            float dot = glm::dot(groundTestPoint, platform.collisionPoint5);
            //printf("%f", dot);
            if (dot < platform.collisionProduct1 && dot > platform.collisionProduct2) {            
                float dot = glm::dot(groundTestPoint, platform.collisionPoint6);
                if (dot < platform.collisionProduct3 && dot > platform.collisionProduct4) {                
                    float dot = glm::dot(groundTestPoint, platform.collisionPoint7);
                    if (dot < platform.collisionProduct5 && dot > platform.collisionProduct6) {                    
                        onGround = true;
                        plVel.y = 0;
                        sinPitch = platform.sinPitch * -cos(platform.yaw - atan2(plVel.x, plVel.z));
                    }
                }
            }




        }

        
        if (!onGround) {plVel.y -= gravityConstant;};
        
        float plVelMagnitude = glm::length(plVel);
        //friction / air resistance
        if (onGround) {
            plVel = plVel - plVel*(dragCoefficient+frictionCoefficient)*plVelMagnitude;
            plVel.y = sinPitch * plVelMagnitude;
        } else {
            plVel = plVel - plVel*dragCoefficient*plVelMagnitude;
        }

        plPos += plVel;
        camPos = plPos;
printf("%f\n", plPos.y);

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
            //glUniform3f(colorLocation, a.color2[0],a.color2[1],a.color2[2]);
            
            //glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
            glUniform3f(colorLocation, typeColors[a.type][0],typeColors[a.type][1],typeColors[a.type][2]);

            glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0/*(void*)(6 * sizeof(GLuint))*/);
        }
        
        //draw
        
        
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        usleep(1000000/100); //100fps
    }


    return 0;
}