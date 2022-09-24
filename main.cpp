void _chk_fail() {
    2+2;
}
void* __chk_fail = 0;

#include <glad/glad.h>
//#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <math.h>

//glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>

//-I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/harfbuzz -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/sysprof-4 -pthread -lfreetype 

/*fonts
#include <ft2build.h>
#include FT_FREETYPE_H  */

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
    /*case GL_STACK_OVERFLOW:    return "Stack overflow";
    case GL_STACK_UNDERFLOW:   return "Stack underflow";*/
    case GL_OUT_OF_MEMORY:     return "Out of memory";
    default:                   return "Unknown error";
    }
}

float camxr = M_PI_2;
float camyr = 0.0f;
const float camRotSpeed = 0.003f;

double lmx = 0.0f;
double lmy = 0.0f;
char firstMouse = 1;

float typeColors[][6] = {
    {1.0f,1.0f,1.0f, 0.8f,0.8f,0.8f},
    {0.9f,0.2f,0.2f, 1.0f,0.0f,0.0f},
    {0.0f,1.0f,0.0f, 0.0f,0.0f,0.0f}
};

glm::vec3 plPos = glm::vec3(0.05f,0.05f,0.05f);
glm::vec3 plVel = glm::vec3(0.0f,0.0f,0.0f);
bool onGround = true;
float dragCoefficient = 0.02f;
float frictionCoefficient = 0.5f;
float gravityConstant = 0.001f;
float plSpeed = 0.018f;
float plStrafeSpeed = plSpeed/2;
float plAirSpeed = plSpeed/8;
float plHeight = 2.0f;
float plJumpForce = 0.08f;
bool haxMode = false;

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
    glm::vec3 collisionPoint5;
    glm::vec3 collisionPoint6;
    glm::vec3 collisionPoint7;    
    float collisionProduct1,collisionProduct2,collisionProduct3,collisionProduct4,collisionProduct5,collisionProduct6;
    glm::quat quaternion;
    float sinPitch;
    float distNum;

} Platform;

typedef struct Projectile {
    glm::vec3 pos;
    glm::vec3 vel;
    glm::mat4 model;
    Projectile* next;
} Projectile;

int numProjectiles = 0;
Projectile* firstProjectile;
float projectileSize = 0.1;
float projectileSpeed = 1.2f;
float projectileDrag = 0.001;
float projectileOffset = 1;
int firingTime = 0;
int firingRate = 10; //frames
glm::vec3 camPos = glm::vec3(0.0f,0.0f,-10.0f);
glm::vec3 camDir = glm::vec3(0.0f,0.0f,1.0f);
glm::vec3 camUp = glm::vec3(0.0f,1.0f,0.0f);
glm::mat4 projectileSMatrix = glm::scale(glm::vec3(projectileSize,projectileSize,projectileSize)) * glm::mat4(1.0f);

void addProjectile(float x,float y,float z,glm::vec3 vel) {
    Projectile* a = (Projectile*)malloc(sizeof(Projectile));
    (*a).pos = glm::vec3(x,y,z) + glm::normalize(vel);
    (*a).vel = vel;
    if (numProjectiles > 0) {
        (*a).next = firstProjectile;
    } else {
        a->next = NULL;
    }

    //there has to be a better way to get a rotation matrix
    glm::vec3 temp = glm::normalize(glm::vec3(vel.x,0,vel.z));

    //glm::quat quaternion = glm::quat(glm::vec3(asin(temp.x),0,acos(temp.z)));
    //glm::mat4 rMatrix = glm::toMat4(quaternion);
    glm::mat4 tMatrix = glm::translate(a->pos);

    (*a).model = tMatrix * projectileSMatrix;// * glm::mat4(1.0f);
    
    firstProjectile = a;
    numProjectiles+=1;
}

void killProjectileAfter(Projectile* a) {
   if (a == NULL) {
    Projectile* b = firstProjectile->next;
    free(firstProjectile);
    firstProjectile = b;
   } else {
    Projectile* b = a->next->next;
    free((*a).next);
    a->next = b;
   }

   numProjectiles -= 1;
   

}

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

    (*a).distNum = glm::length2(collisionPoint1);


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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && firingTime == 0) {
       firingTime = firingRate;
       glm::vec3 a = camDir * projectileSpeed + plVel;
       addProjectile(plPos.x,plPos.y,plPos.z, a);
    }
}

int main() {
    uint32_t width=800;
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

    /*if(GLEW_OK!=glewInit()){
        printf("glew init error :(");
        return 1;
    }*/

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        printf("Failed to initialize GLAD\n");
        return -1;
    }    

    glViewport(0,0,width,height);

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
    glfwSetMouseButtonCallback(window, mouse_button_callback);

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

    //int textProjectionLocation = glGetUniformLocation(textShaderProgram, "orthoProjection");
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    //glm::mat4 textProjection = glm::mat4(1.0f);
    float color[] = {1.0f,0.0f,0.0f};

    projection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 50.0f);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    //Projection = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
    //glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(textProjection));

    

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
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    glEnable(GL_DEPTH_TEST);







    addPlatform(0,-5,5,0,0,4,1,20,0);

    addPlatform(15,-5,20,M_PI_2,0,4,1,20,0);

    addPlatform(40,-5,20,M_PI_2/2,0,4,1,20,1);

    addPlatform(80,-5,40,M_PI_2/2,0,4,1,20,0);

    addPlatform(100,-5,80,M_PI_2,0,4,1,4,1);
    addPlatform(130,-3,90,M_PI_2,0,3,1,3,1);
    addPlatform(160,-2,100,M_PI_2,0,2,1,2,1);

    addPlatform(190,-1,95,0,  M_PI_2/2,3,1,3,0);
    addPlatform(210,-1,105,0,-M_PI_2/2,3,1,3,0);
    addPlatform(230,0,90,0,          0,3,1,3,1);
    addPlatform(270,-1,110,0,-M_PI_2/4,3,1,3,1);

    //stairs
    addPlatform(270,6,120,0,-M_PI_2/2,3,1,3,0);
    addPlatform(270,15,130,0,-M_PI_2/2,2.5,1,3,0);
    addPlatform(270,24,140,0,-M_PI_2/2,2,1,3,0);
    addPlatform(270,33,150,0,-M_PI_2/2,1.5,1,3,0);
    
    addPlatform(270,35,160,0,0,3,1,3,1);

    //arrow
    addPlatform(270,35,200,0,0,           6,20,6,1);
    addPlatform(265,29.5,200, M_PI_2, M_PI_2/2,   6,3,10,1);
    addPlatform(275,29.5,200,M_PI_2,-M_PI_2/2,    6,3,10,1);

    addPlatform(255,-30,190,0,0,   1,1,1,1);
    addPlatform(220,-37,190,0,0,   1,1,1,1);
    addPlatform(185,-44,190,0,0,   0.75f,1,0.75f,1);
    addPlatform(155,-51,190,0,0,   0.75f,1,0.75f,1);
    addPlatform(135,-60,190,0,0,   0.5,0.5,0.5,2);

    float sinPitch = 0;
    bool shouldClose = false;
    int defeatedPlatforms = 0;

    while (!(glfwWindowShouldClose(window) || shouldClose)) {
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
            
            if (onGround || (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)) {
                plVel += glm::vec3(0, 1, 0) * plJumpForce;
                plPos += plVel;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            shouldClose = true;
        }
        

        if (plPos.y < -75) {
            shouldClose = true;
        }
        
        //check on ground
        onGround = false;
        sinPitch = 0;
        for (int i = 0; i < numPlatforms; i++) {
            Platform* platform = platforms[i];
            glm::vec3 groundTestPoint = plPos - platform->pos;
            groundTestPoint.y -= plHeight;
            //groundTestPoint = (platform->rotationThing * glm::length(groundTestPoint));
            
            //groundTestPoint =  groundTestPoint + platform->halfLen;
            
            //printf("%f\n%f\n%f\n\n",groundTestPoint.x,groundTestPoint.y,groundTestPoint.z);

            float dot = glm::dot(groundTestPoint, platform->collisionPoint5);
            //printf("%f", dot);
            if (dot < platform->collisionProduct1 && dot > platform->collisionProduct2) {            
                float dot = glm::dot(groundTestPoint, platform->collisionPoint6);
                if (dot < platform->collisionProduct3 && dot > platform->collisionProduct4) {                
                    float dot = glm::dot(groundTestPoint, platform->collisionPoint7);
                    if (dot < platform->collisionProduct5 && dot > platform->collisionProduct6) {                    
                        if (platform->type == 1 || platform->type == 2) {
                            shouldClose = true;
                        }
                        
                        onGround = true;
                        plVel.y = 0;
                        sinPitch = platform->sinPitch * -cos(platform->yaw - atan2(plVel.x, plVel.z));
                    }
                }
            }
        }

        {
            Projectile* a = firstProjectile;
            Projectile* b = NULL;
            loop: while (a != NULL) {
                if (a->pos.y < -75) {
                    a=a->next;
                    killProjectileAfter(b);
                    continue;
                }
                
                //check collisions
                for (int i = 0; i < numPlatforms; i++) {
                    Platform* platform = platforms[i];
                    glm::vec3 groundTestPoint = a->pos - platform->pos;
                    if (glm::length2(groundTestPoint) > platform->distNum) {
                        continue;
                    }
                    
                    
                    float dot = glm::dot(groundTestPoint, platform->collisionPoint5);
                    //printf("%f", dot);
                    if (dot < platform->collisionProduct1 && dot > platform->collisionProduct2) {            
                        float dot = glm::dot(groundTestPoint, platform->collisionPoint6);
                        if (dot < platform->collisionProduct3 && dot > platform->collisionProduct4) {                
                            float dot = glm::dot(groundTestPoint, platform->collisionPoint7);
                            if (dot < platform->collisionProduct5 && dot > platform->collisionProduct6) {                    
                                if (platform->type == 1) {
                                    defeatedPlatforms += 1;
                                    platform->type = 0;
                                }
                                
                                a=a->next;
                                killProjectileAfter(b);
                                goto loop;
                            }
                        }
                    }
                }
                
                a->vel.y -= gravityConstant;
                a->vel = a->vel - a->vel * (projectileDrag) * glm::length(a->vel);
                a->pos += a->vel;
                
                glm::mat4 tMatrix = glm::translate(a->pos);
                (*a).model = tMatrix * projectileSMatrix;

                b = a;
                a = a->next;
                
                
            }
        }
        //plVel = plVel - plVel*(dragCoefficient+frictionCoefficient)*plVelMagnitude;
        
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
        //printf("%f\n", plVel.y);

        camDir.x = cos(camxr) * cos(camyr);
        camDir.y = sin(camyr);
        camDir.z = sin(camxr) * cos(camyr);
        //camDir = glm::normalize(camDir); //probably unnecessary

        view = glm::lookAt(camPos, camPos + camDir, camUp);
        if (firingTime>0) {
            firingTime--;
        }



        //draw shit
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        
        /*struct timeval tv;
        gettimeofday(&tv, NULL);
        float time = (float)tv.tv_usec + (float)(1000000 * (tv.tv_sec%(60*60*24)));*/
        
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

        //glUniform1f(timeLocation, (float)sin(time/1000000.0f)/10.0f);


        glClearColor(0.20f, 0.5f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (int i = 0; i < numPlatforms; i++) {
            Platform a = *platforms[i];
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(a.model));
            
            //glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
            glUniform3f(colorLocation, typeColors[a.type][0],typeColors[a.type][1],typeColors[a.type][2]);

            glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
        }
        
        
        Projectile* a = firstProjectile;
        for (int i = 0; i < numProjectiles; i++) {
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(a->model));
            glUniform3f(colorLocation, 0.9f,0.95f,0.3f);

            glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
            
            a = a->next;
        }
        
        
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        usleep(1000000/100); //100fps
    }


    return 0;
}