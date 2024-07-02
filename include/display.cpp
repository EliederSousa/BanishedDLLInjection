#include "Core.cpp"
#include "texture.cpp"
#include "rapidcsv.h"
#include <GL/gl.h>
#include <SDL.h>
#include <string.h>
#include <array>
#include <map>
#include <iterator> // Is it needed?

namespace {
namespace Display {

typedef void (*_drawText)(float a1,
    float a2,
    float a3,
    float a4,
    unsigned int a5,
    const char *a6,
    float a7,
    unsigned __int8 a8);

typedef void (*_showAINodes)();

SDL_Window* window;
SDL_GLContext original_context;
SDL_GLContext user_context;

SDL_Surface* image;
GLuint textureID;

std::map<std::string, SDL_Surface*> imageList;
std::map<std::string, GLuint> texturesID;
std::string actualTrackName;
int imageCount = 0;

rapidcsv::Document font1Data("minimaps/font/font1data.csv", rapidcsv::LabelParams(0,0));

int width;
int height;
int oldWidth;
int oldHeight; // Usados para recalcular o tamanho do contexto OpenGL
int centerX;
int centerY;
float aspectRatio;
int pixels;
std::string bindedTexture;

// Variables to draw circles
int numVertices = 10;
double delta_angle = 2 * 3.14159265 / numVertices;

Address widthPointer = Core::rvglBaseAddress + Core::ADD_displayWidth;
Address heightPointer = Core::rvglBaseAddress + Core::ADD_displayHeight;

namespace Screen {
    int getWidth() {
        width = Memory::Internal::read<int>(widthPointer);
        return width;
    }

    int getHeight() {
        height = Memory::Internal::read<int>(heightPointer);
        return height;
    }

    int getCenterX() {
        centerX = width/2;
        return centerX;
    }

    int getCenterY() {
        centerY = height/2;
        return centerY;
    }

    int getAspectRatio() {
        aspectRatio = width/height;
        return aspectRatio;
    }

    int getTotalPixels() {
        pixels = width * height;
        return pixels;
    }
}

void drawText( float x, float y, float width, float height, unsigned int color, const char* text ) {
    _drawText draw = (_drawText)(Core::rvglBaseAddress + 0x0091F30); // Arrumar o endereço para ser dinâmico
    draw(x, y, width, height, color, text, 0.0, 1);
}

void displayAINodes() {
    _showAINodes showAINodes = (_showAINodes)(Core::rvglBaseAddress + 0x0014A30); // Arrumar o endereço para ser dinâmico
    showAINodes();
}

void glSetupOrtho() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, viewport[2], viewport[3]);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, viewport[2], viewport[3], 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
}

constexpr void glRestoreGLAndFlush() {
    glPopMatrix();
    glPopAttrib();
    glFlush();
}

constexpr void glDrawFilledRect( float x, float y, float width, float height, GLubyte color[4] ) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(SDL_GL_MULTISAMPLESAMPLES);    // Anti-alising
    glColor4ub( color[0], color[1], color[2], color[3] );
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

constexpr void glDrawOutlineRect( float x, float y, float width, float height, float lineWidth, const GLubyte color[4] ) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(SDL_GL_MULTISAMPLESAMPLES);    // Anti-alising
    glLineWidth(lineWidth);
    glColor4ub(color[0], color[1], color[2], color[3]);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x - 0.5f, y - 0.5f);
    glVertex2f(x + width + 0.5f, y - 0.5f);
    glVertex2f(x + width + 0.5f, y + height + 0.5f);
    glVertex2f(x - 0.5f, y + height + 0.5f);
    glVertex2f(x - 0.5f, y - 0.5f);
    glEnd();
}

constexpr void glDrawFilledCircle( float x, float y, float radius, GLubyte color[4] ) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(SDL_GL_MULTISAMPLESAMPLES);    // Anti-alising
    glColor4ub( color[0], color[1], color[2], color[3] );
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= numVertices; i++) {
        double xx = cos(delta_angle * i) * radius;
        double yy = sin(delta_angle * i) * radius;
        glVertex2d(x + xx, y + yy);
    }
    glEnd();
}

constexpr void glDrawOutlineCircle( float x, float y, float radius, float lineWidth, GLubyte color[4] ) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(SDL_GL_MULTISAMPLESAMPLES);    // Anti-alising
    glColor4ub( color[0], color[1], color[2], color[3] );
    glLineWidth(lineWidth);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= numVertices; i++) {
        double xx = cos(delta_angle * i) * radius;
        double yy = sin(delta_angle * i) * radius;
        glVertex2d(x + xx, y + yy);
    }
    glEnd();
}

void glDrawScreenCoordinates(float size) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(SDL_GL_MULTISAMPLESAMPLES);    // Anti-alising
    glLineWidth(1);
    glColor4ub(255, 255, 255, 128);
    glBegin(GL_LINES);
    for( int x=0; x<500; x++ ) {
        if( x % 10 == 0 ) {
            glColor4ub(255, 80, 80, 80); 
        } else {
            glColor4ub(255, 255, 255, 32);
        }
        glVertex2f(x * size, 0);
        glVertex2f(x * size, 9999);
    }
    for( int y=0; y<500; y++ ) {
        if( y % 10 == 0 ) {
            glColor4ub(255, 80, 80, 80); 
        } else {
            glColor4ub(255, 255, 255, 32);
        }
        glVertex2f(0, y * size);
        glVertex2f(9999, y * size);
    }
    glEnd();
}

constexpr void glDrawRotatedImage( std::string textureName, float x, float y, float angle, float size, int alpha) {
    glPushMatrix();
    glTranslatef(x, y, 0); //Translate back
    glRotatef(angle, 0, 0, 1); //Rotate
    glTranslatef(-x, -y, 0);

    glColor4ub( 255, 255, 255, alpha );
    glEnable(GL_TEXTURE_2D);
    if( texturesID[textureName] == NULL ) glGenTextures(1, &texturesID[textureName]);
    glBindTexture(GL_TEXTURE_2D, texturesID[textureName]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageList[textureName]->w, imageList[textureName]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageList[textureName]->pixels);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);

    if( imageList[textureName]->w >= imageList[textureName]->h ) {
        float aspectRatio = (float)imageList[textureName]->h / (float)imageList[textureName]->w;
        float height = aspectRatio * size;
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(x, y);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(x + size, y);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(x + size, y + height);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(x, y + height);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    } else {
        float aspectRatio = (float)imageList[textureName]->w / (float)imageList[textureName]->h;
        float width = aspectRatio * size;
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(x, y);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(x + width, y);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(x + width, y + size);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(x, y + size);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }
    glPopMatrix();
}

constexpr void glDrawImage( std::string textureName, float x, float y, float size, int alpha ) {
    glColor4ub( 255, 255, 255, alpha );
    glEnable(GL_TEXTURE_2D);
    if( texturesID[textureName] == NULL ) glGenTextures(1, &texturesID[textureName]);
    if( bindedTexture != textureName ) {
        glBindTexture(GL_TEXTURE_2D, texturesID[textureName]);
        bindedTexture = textureName;
    }
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, texturesID[textureName]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageList[textureName]->w, imageList[textureName]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageList[textureName]->pixels);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);

    if( imageList[textureName]->w >= imageList[textureName]->h ) {
        float aspectRatio = (float)imageList[textureName]->h / (float)imageList[textureName]->w;
        float height = aspectRatio * size;
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(x, y);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(x + size, y);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(x + size, y + height);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(x, y + height);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    } else {
        float aspectRatio = (float)imageList[textureName]->w / (float)imageList[textureName]->h;
        float width = aspectRatio * size;
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(x, y);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(x + width, y);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(x + width, y + size);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(x, y + size);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }
}

void glDrawPartImage( std::string textureName, float x, float y, float px, float py, float zoom, int size) {
    glEnable(GL_TEXTURE_2D);
    if( texturesID[textureName] == NULL ) glGenTextures(1, &texturesID[textureName]);
    glBindTexture(GL_TEXTURE_2D, texturesID[textureName]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageList[textureName]->w, imageList[textureName]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageList[textureName]->pixels);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);

    glTexCoord2f(px - zoom, py - zoom);
    glVertex2f(x, y);
    glTexCoord2f(px + zoom, py - zoom);
    glVertex2f(x + size, y);
    glTexCoord2f(px - zoom, py + zoom);
    glVertex2f(x, y + size);
    glTexCoord2f(px + zoom, py + zoom);
    glVertex2f(x + size, y + size);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void glDrawCircleImage( std::string textureName, float x, float y, float size ) {
    glEnable(GL_TEXTURE_2D);
    if( texturesID[textureName] == NULL ) glGenTextures(1, &texturesID[textureName]);
    glBindTexture(GL_TEXTURE_2D, texturesID[textureName]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageList[textureName]->w, imageList[textureName]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageList[textureName]->pixels);

    glEnable(GL_BLEND);
    glEnable(SDL_GL_MULTISAMPLESAMPLES);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_TRIANGLE_FAN);

    int numVertices = 30;
    float radius = 200;
    x = 200;
    y = 200;
    for (int i = 0; i <= 30; i++) {
        double angle = 2 * 3.14159265 * i / 20;
        double xt = (cos(angle) + 1.0) * 0.5;
        double yt = (sin(angle) + 1.0) * 0.5;
        glTexCoord2d(xt, yt);

        double xx = cos(angle) * radius;
        double yy = sin(angle) * radius;
        glVertex2d(x + xx, y + yy);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    // OUTLINE
    glColor4ub( 255.0f, 0.0f, 0.0f, 255.0f );
    glLineWidth(2);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= numVertices; i++) {
        double angle = 2 * 3.14159265 * i / numVertices;
        double xx = cos(angle) * radius;
        double yy = sin(angle) * radius;
        glVertex2d(x + xx, y + yy);
    }
    glEnd();
}

int glDrawChar( std::string character, float x, float y, float size, int alpha ) {
    std::string textureName = "font1";
    glColor4ub( 255, 255, 255, alpha );
    glEnable(GL_TEXTURE_2D);
    if( texturesID[textureName] == NULL ) glGenTextures(1, &texturesID[textureName]);
    glBindTexture(GL_TEXTURE_2D, texturesID[textureName]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageList[textureName]->w, imageList[textureName]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageList[textureName]->pixels);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);

    float xStart = font1Data.GetCell<float>("xStart", character) / (float)imageList[textureName]->w;
    float yStart = font1Data.GetCell<float>("yStart", character) / (float)imageList[textureName]->h;
    float xEnd   = font1Data.GetCell<float>("xEnd", character)   / (float)imageList[textureName]->w;
    float yEnd   = font1Data.GetCell<float>("yEnd", character)   / (float)imageList[textureName]->h;
    int xOffset  = font1Data.GetCell<float>("xOffset", character) * size;
    int yOffset  = font1Data.GetCell<float>("yOffset", character) * size;
    int xAdvance = font1Data.GetCell<float>("xAdvance", character) * size;
    int   width  = font1Data.GetCell<float>("xEnd", character) * size;
    int   height = font1Data.GetCell<float>("yEnd", character) * size;

    glTexCoord2f(xStart, yStart);
    glVertex2f(x + xOffset, y + yOffset);
    glTexCoord2f(xStart + xEnd, yStart);
    glVertex2f(x + xOffset + width, y + yOffset);
    glTexCoord2f(xStart + xEnd, yStart + yEnd);
    glVertex2f(x + xOffset + width, y + yOffset + height);
    glTexCoord2f(xStart, yStart + yEnd);
    glVertex2f(x + xOffset, y + yOffset + height);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    return xAdvance;
}

void glDrawString( std::string text, float x, float y, float size, int alpha, bool reverseMode=0 ) {
    int advance = 0;
    if( reverseMode ) {
        for( int w=text.length()-1; w>-1; w-- ) {
            std::string c = "";
            c += text[w];
            advance += glDrawChar(c, x - advance, y, size, alpha);
        }
    } else {
        for( int w=0; w<text.length(); w++ ) {
            std::string c = "";
            c += text[w];
            advance += glDrawChar(c, x + advance, y, size, alpha);
        }
    }
}

int loadImage( std::string path, std::string name ) {
    imageList[name] = IMG_Load( path.c_str() );
    if (imageList[name] == NULL) {
        return 0;
    } else {
        imageCount++;
        return 1;
    }
}

void freeImage( std::string name ) {
    SDL_FreeSurface( imageList[name] );
    imageList[name] = NULL;
}

unsigned int createColor(unsigned char red, unsigned char green,
                         unsigned char blue, unsigned char alpha) {
    unsigned int color = alpha << 24;
    color += red << 16;
    color += green << 8;
    color += blue;
    return color;
}

void update() {
    Screen::getWidth();
    Screen::getHeight();
}
}
}