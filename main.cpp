#include <windows.h>
#include <mmsystem.h>                    
#pragma comment(lib, "winmm.lib")       

#include "imageloader.h"   
#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h> 

// --- Animation Variables ---
float viewAngle = 0.0f;
bool hasAutoSpun = false;

float armAngleForwardX = 0.0f;
float armSwingZ = 0.0f;
float legAngle = 0.0f;
float patrickZ = 0.0f;
float patrickX = 0.0f;
float patrickRotation = 0.0f;

float bubbleY = -150.0f;

// Camera movement
float mouseRotateY = 0.0f;
float mouseRotateX = 0.0f;
int lastMouseX = 0;
int lastMouseY = 0;
bool dragging = false;

bool keys[256] = { false };

bool isWalkingActive = false;
bool armSwingingForward = true;
bool legSwingingForward = true;

// --- State Variables (Interactivity) ---
bool isLightingEnabled = true;

// --- Texture Variables ---
GLuint sandTexture;
GLuint pantTexture;
GLuint pLegLTexture;
GLuint pLegRTexture;

// --- Lighting Setup ---
void setupLighting() {
    GLfloat light_ambient[] = { 0.35f, 0.35f, 0.45f, 1.0f }; // Slightly bluer ambient light for underwater vibe
    GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_position[] = { 100.0f, 300.0f, 100.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    if (isLightingEnabled) {
        glEnable(GL_LIGHTING);
    }
    else {
        glDisable(GL_LIGHTING);
    }

    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);
}

// --- Texture Loader Function ---
GLuint loadTexture(Image * image) {
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
    return textureId;
}

// --- Primitive Helpers ---
void drawUprightCone(float base, float top, float height) {
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    GLUquadricObj* quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, GLU_FILL);
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluCylinder(quad, base, top, height, 30, 30);
    gluDeleteQuadric(quad);
    glPopMatrix();
}

void drawTaperedCylinder(float base, float top, float height) {
    GLUquadricObj* quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, GLU_FILL);
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluCylinder(quad, base, top, height, 30, 30);
    gluDeleteQuadric(quad);
}

void drawTexturedSphere(float radius, int slices, int stacks) {
    GLUquadricObj* quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, GLU_FILL);
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluQuadricTexture(quad, GL_TRUE);
    gluSphere(quad, radius, slices, stacks);
    gluDeleteQuadric(quad);
}

void drawTexturedTaperedCylinder(float base, float top, float height) {
    GLUquadricObj* quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, GLU_FILL);
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluQuadricTexture(quad, GL_TRUE);
    gluCylinder(quad, base, top, height, 30, 30);
    gluDeleteQuadric(quad);
}

// --- Environment ---
void drawBikiniBottomHouses() {
    glDisable(GL_TEXTURE_2D);

    // 1. Patrick's Rock (Left)
    glPushMatrix();
    glTranslatef(-220.0f, -120.0f, -150.0f);

    // Main Rock Dome
    glColor3f(0.35f, 0.24f, 0.19f); // Dark brown
    glPushMatrix();
    glScalef(1.0f, 0.8f, 1.0f); // Flatten the sphere into a dome
    glutSolidSphere(80.0, 30, 30);
    glPopMatrix();

    // Bamboo weather vane
    glColor3f(0.7f, 0.7f, 0.3f);
    glPushMatrix();
    glTranslatef(0.0f, 60.0f, 0.0f);
    drawUprightCone(2.0f, 2.0f, 30.0f);
    glPopMatrix();
    glPopMatrix();

    // 2. Squidward's Easter Island Head (Middle)
    glPushMatrix();
    glTranslatef(0.0f, -120.0f, -220.0f);
    glColor3f(0.2f, 0.25f, 0.35f); // Deep blue-grey

    // Main head cylinder
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    drawTaperedCylinder(45.0f, 35.0f, 140.0f);
    GLUquadricObj* caps = gluNewQuadric();
    gluQuadricDrawStyle(caps, GLU_FILL);
    gluQuadricNormals(caps, GLU_SMOOTH);
    glTranslatef(0.0f, 0.0f, 140.0f);
    gluDisk(caps, 0.0, 35.0f, 30, 1);
    glPopMatrix();

    // Nose
    glPushMatrix();
    glTranslatef(0.0f, 60.0f, 40.0f);
    glRotatef(10.0f, 1.0f, 0.0f, 0.0f);
    glScalef(1.0f, 3.0f, 1.0f);
    glutSolidSphere(12.0, 20, 20);
    glPopMatrix();

    // Ears
    glPushMatrix(); glTranslatef(-45.0f, 60.0f, 0.0f); glutSolidSphere(10.0, 15, 15); glPopMatrix();
    glPushMatrix(); glTranslatef(45.0f, 60.0f, 0.0f); glutSolidSphere(10.0, 15, 15); glPopMatrix();
    glPopMatrix();

    // 3. SpongeBob's Pineapple (Right)
    glPushMatrix();
    glTranslatef(220.0f, -50.0f, -150.0f);

    // Pineapple Body
    glColor3f(1.0f, 0.65f, 0.1f); // Orange-yellow
    glPushMatrix();
    glScalef(1.0f, 1.4f, 1.0f);
    glutSolidSphere(55.0, 30, 30);
    glPopMatrix();

    // Pineapple Leaves
    glColor3f(0.2f, 0.8f, 0.2f);
    glTranslatef(0.0f, 75.0f, 0.0f);
    for (int i = 0; i < 5; i++) {
        glPushMatrix();
        glRotatef(i * 72.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
        drawUprightCone(8.0f, 0.0f, 50.0f);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawGround() {
    glPushMatrix();
    glTranslatef(0.0f, -120.0f, 0.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, sandTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    glTexCoord2f(0.0f, 0.0f); glVertex3f(-500.0f, 0.0f, 500.0f);
    glTexCoord2f(10.0f, 0.0f); glVertex3f(500.0f, 0.0f, 500.0f);
    glTexCoord2f(10.0f, 10.0f); glVertex3f(500.0f, 0.0f, -500.0f);
    glTexCoord2f(0.0f, 10.0f); glVertex3f(-500.0f, 0.0f, -500.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawSeaPlants() {
    glDisable(GL_LIGHTING);
    glColor3f(0.65f, 0.0f, 0.7f);

    glPushMatrix();
    glTranslatef(-400.0f, -120.0f, -150.0f);
    GLUquadric* quad = gluNewQuadric();

    glPushMatrix(); glRotatef(-5, 0, 0, 1); gluCylinder(quad, 8, 8, 180, 20, 20); glTranslatef(0, 0, 180); glScalef(2.0f, 2.8f, 1.0f); glutSolidSphere(8, 20, 20); glPopMatrix();
    glPushMatrix(); glTranslatef(0, 0, 100); glRotatef(-45, 1, 0, 0); gluCylinder(quad, 6, 6, 70, 20, 20); glTranslatef(0, 0, 70); glScalef(2.0f, 2.8f, 1.0f); glutSolidSphere(7, 20, 20); glPopMatrix();
    glPushMatrix(); glTranslatef(0, 0, 140); glRotatef(40, 1, 0, 0); gluCylinder(quad, 6, 6, 80, 20, 20); glTranslatef(0, 0, 80); glScalef(2.0f, 2.8f, 1.0f); glutSolidSphere(7, 20, 20); glPopMatrix();

    gluDeleteQuadric(quad);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(350.0f, -120.0f, -100.0f);
    quad = gluNewQuadric();

    glPushMatrix(); glRotatef(3, 0, 0, 1); gluCylinder(quad, 8, 8, 220, 20, 20); glTranslatef(0, 0, 220); glScalef(2.0f, 3.0f, 1.0f); glutSolidSphere(8, 20, 20); glPopMatrix();
    glPushMatrix(); glTranslatef(0, 0, 120); glRotatef(-45, 1, 0, 0); gluCylinder(quad, 6, 6, 90, 20, 20); glTranslatef(0, 0, 90); glScalef(2.0f, 3.0f, 1.0f); glutSolidSphere(7, 20, 20); glPopMatrix();
    glPushMatrix(); glTranslatef(0, 0, 170); glRotatef(50, 1, 0, 0); gluCylinder(quad, 6, 6, 90, 20, 20); glTranslatef(0, 0, 90); glScalef(2.0f, 3.0f, 1.0f); glutSolidSphere(7, 20, 20); glPopMatrix();

    gluDeleteQuadric(quad);
    glPopMatrix();

    if (isLightingEnabled) glEnable(GL_LIGHTING);
}

void drawSingleSkyFlower(float r, float g, float b, float dotR, float dotG, float dotB, float lineWidth) {
    glColor3f(r, g, b);
    glLineWidth(lineWidth);
    glBegin(GL_LINE_LOOP);

    for (int i = 0; i < 720; i++) {
        float t = i * 3.1415926f / 360.0f;
        float radius = 30.0f + 12.0f * sin(5.0f * t) - 4.0f * sin(10.0f * t) + 1.5f * sin(17.0f * t);
        float x = radius * cos(t);
        float y = radius * sin(t);
        glVertex3f(x, y, 0.0f);
    }
    glEnd();

    glColor3f(dotR, dotG, dotB);
    glPushMatrix();
    glutSolidSphere(2.5f, 20, 20);
    glPopMatrix();
}

void drawSkyFlowers() {
    glDisable(GL_LIGHTING);

    glPushMatrix(); glTranslatef(-280.0f, 180.0f, 300.0f); glScalef(3.2f, 3.2f, 3.2f); glRotatef(15.0f, 0, 0, 1);
    drawSingleSkyFlower(0.2f, 0.6f, 1.0f, 0.0f, 0.2f, 0.8f, 6.0f); glPopMatrix();

    glPushMatrix(); glTranslatef(280.0f, 260.0f, 350.0f); glScalef(3.5f, 3.5f, 3.5f); glRotatef(-25.0f, 0, 0, 1);
    drawSingleSkyFlower(1.0f, 0.5f, 0.2f, 0.8f, 0.3f, 0.0f, 6.0f); glPopMatrix();

    glPushMatrix(); glTranslatef(-150.0f, 220.0f, 120.0f); glScalef(2.0f, 2.0f, 2.0f); glRotatef(30.0f, 0, 0, 1);
    drawSingleSkyFlower(0.6f, 0.3f, 0.8f, 0.3f, 0.1f, 0.5f, 5.0f); glPopMatrix();

    glPushMatrix(); glTranslatef(170.0f, 180.0f, 100.0f); glScalef(1.8f, 1.8f, 1.8f); glRotatef(-20.0f, 0, 0, 1);
    drawSingleSkyFlower(0.5f, 0.9f, 0.2f, 0.2f, 0.6f, 0.1f, 4.0f); glPopMatrix();

    glPushMatrix(); glTranslatef(-50.0f, 300.0f, 50.0f); glScalef(1.6f, 1.6f, 1.6f);
    drawSingleSkyFlower(0.9f, 0.8f, 0.2f, 0.9f, 0.5f, 0.1f, 4.0f); glPopMatrix();

    glPushMatrix(); glTranslatef(220.0f, 330.0f, 80.0f); glScalef(1.5f, 1.5f, 1.5f);
    drawSingleSkyFlower(0.3f, 0.8f, 1.0f, 0.1f, 0.4f, 0.8f, 5.0f); glPopMatrix();

    glPushMatrix(); glTranslatef(-320.0f, 350.0f, -250.0f); glScalef(0.9f, 0.9f, 0.9f);
    drawSingleSkyFlower(0.9f, 0.9f, 0.2f, 0.8f, 0.6f, 0.1f, 3.0f); glPopMatrix();

    if (isLightingEnabled) glEnable(GL_LIGHTING);
}

void drawBubble() {
    glDisable(GL_LIGHTING);
    glColor3f(0.8f, 0.9f, 1.0f);

    // Bubble 1 (Main)
    glPushMatrix();
    glTranslatef(120.0f, bubbleY, 50.0f);
    glutWireSphere(8.0f, 12, 12);
    glPopMatrix();

    // Bubble 2 (Smaller, trails behind)
    glPushMatrix();
    glTranslatef(95.0f, (bubbleY * 0.85f) - 40.0f, 65.0f);
    glutWireSphere(5.0f, 10, 10);
    glPopMatrix();

    // Bubble 3 (Faster, rises ahead)
    glPushMatrix();
    glTranslatef(145.0f, (bubbleY * 1.15f) + 30.0f, 35.0f);
    glutWireSphere(6.5f, 12, 12);
    glPopMatrix();

    if (isLightingEnabled) glEnable(GL_LIGHTING);
}

void drawPatrick3D() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0.0, 50.0, 450.0,
        0.0, 0.0, 0.0,
        0.0, 1.0, 0.0);

    setupLighting();

    glRotatef(10.0f + mouseRotateX, 1.0f, 0.0f, 0.0f);
    glRotatef(viewAngle + mouseRotateY, 0.0f, 1.0f, 0.0f);

    drawGround();
    drawBikiniBottomHouses();
    drawSeaPlants();
    drawSkyFlowers();
    drawBubble();

    glPushMatrix();
    glScalef(1.6f, 1.6f, 1.6f);
    glTranslatef(patrickX, 0.0f, patrickZ);
    glRotatef(patrickRotation, 0.0f, 1.0f, 0.0f);

    // LEVEL 1 (ROOT): PELVIS / WAIST (PANTS)
    glPushMatrix();
    glTranslatef(0.0f, -30.0f, 0.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, pantTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();
    glTranslatef(0.0f, 15.0f, 0.0f);
    drawTexturedSphere(43.3, 30, 30);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    glPushMatrix();
    glColor3f(0.55f, 0.95f, 0.15f);
    glTranslatef(0.0f, 19.0f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidTorus(3.0f, 42.0f, 16.0f, 40.0f);
    glPopMatrix();

    // LEVEL 2 UPPER TORSO & HEAD
    glPushMatrix();
    glTranslatef(0.0f, 15.0f, 0.0f);
    glColor3f(0.96f, 0.68f, 0.69f);

    glPushMatrix();
    glTranslatef(0.0f, 5.0f, 0.0f);
    glScalef(1.0f, 1.1f, 1.0f);
    glutSolidSphere(43.0, 30, 30);
    glPopMatrix();

    glColor3f(0.95f, 0.66f, 0.66f);
    glPushMatrix();
    glTranslatef(0.0f, 11.0f, 43.0f);
    glutSolidTorus(0.5f, 3.0f, 10, 24);
    glPopMatrix();

    glColor3f(0.96f, 0.68f, 0.69f);
    glPushMatrix();
    glTranslatef(0.0f, 10.0f, 0.0f);
    drawUprightCone(36.0f, 8.0f, 115.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 124.0f, 0.0f);
    glutSolidSphere(8.0, 20, 20);
    glPopMatrix();

    // LEVEL 3: FACE
    // EYES
    glPushMatrix();
    glTranslatef(0.0f, 65.0f, 23.5f);

    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix(); glTranslatef(-6.6f, 5.0f, -3.0f); glScalef(0.9f, 1.2f, 0.7f); glutSolidSphere(8.7, 20, 20); glPopMatrix();
    glPushMatrix(); glTranslatef(6.6f, 5.0f, -3.0f); glScalef(0.9f, 1.2f, 0.7f); glutSolidSphere(8.7, 20, 20); glPopMatrix();

    glColor3f(0.0f, 0.0f, 0.0f);
    glPushMatrix(); glTranslatef(-5.5f, 3.0f, 2.8f); glScalef(1.0f, 1.2f, 0.5f); glutSolidSphere(2.4, 10, 10); glPopMatrix();
    glPushMatrix(); glTranslatef(5.5f, 3.0f, 2.8f); glScalef(1.0f, 1.2f, 0.5f); glutSolidSphere(2.4, 10, 10); glPopMatrix();

    // EYEBROWS
    glLineWidth(3.5f);
    glBegin(GL_LINES);
    glVertex3f(-12.0f, 20.0f, -8.0f); glVertex3f(-4.0f, 23.0f, -6.0f);
    glVertex3f(-12.0f, 22.0f, -8.0f); glVertex3f(-4.0f, 25.0f, -6.0f);
    glVertex3f(12.0f, 20.0f, -8.0f); glVertex3f(4.0f, 23.0f, -6.0f);
    glVertex3f(12.0f, 22.0f, -8.0f); glVertex3f(4.0f, 25.0f, -6.0f);
    glEnd();

    // MOUTH
    glColor3f(0.15f, 0.0f, 0.0f);
    for (float angle = 210; angle <= 330; angle += 1.5f) {
        float rad = angle * 3.14159f / 180.0f;
        float centerOffset = (angle - 270.0f) / 60.0f;
        float curve = 1.0f - (centerOffset * centerOffset);
        float zDepth = -2.5f + (curve * 5.0f);

        glPushMatrix();
        glTranslatef(16.0f * cos(rad), -2.0f + 10.0f * sin(rad), zDepth);
        glutSolidSphere(1.0, 10, 10);
        glPopMatrix();
    }

    // CHEEK
    // Left
    glPushMatrix();
    glColor3f(0.96f, 0.68f, 0.69f);
    glTranslatef(-16.0f, -7.0f, -5.0f);
    glScalef(3.2f, 2.2f, 2.2f);
    glutSolidSphere(2.0, 15, 15);
    glPopMatrix();

    // Right
    glPushMatrix();
    glTranslatef(16.0f, -7.0f, -5.0f);
    glScalef(3.2f, 2.2f, 2.2f);
    glutSolidSphere(2.0, 15, 15);
    glPopMatrix();

    glPopMatrix(); // End Face

    // RIGHT ARM
    glPushMatrix();
    glTranslatef(25.0f, 30.0f, 0.0f);
    glRotatef(armSwingZ, 0.0f, 1.0f, 0.0f);
    glRotatef(-armAngleForwardX, 0.0f, 0.0f, 1.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glColor3f(0.96f, 0.68f, 0.69f);
    drawTaperedCylinder(13.0f, 4.0f, 55.0f);
    glTranslatef(0.0f, 0.0f, 55.0f);
    glutSolidSphere(4.0, 20, 20);
    glPopMatrix();

    // LEFT ARM
    glPushMatrix();
    glTranslatef(-25.0f, 30.0f, 0.0f);
    glRotatef(-armSwingZ, 0.0f, 1.0f, 0.0f);
    glRotatef(armAngleForwardX, 0.0f, 0.0f, 1.0f);
    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
    glColor3f(0.96f, 0.68f, 0.69f);
    drawTaperedCylinder(13.0f, 4.0f, 55.0f);
    glTranslatef(0.0f, 0.0f, 55.0f);
    glutSolidSphere(4.0, 20, 20);
    glPopMatrix();

    glPopMatrix(); // End Torso

    // LEVEL 2: RIGHT LEG
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, pLegRTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();
    glTranslatef(20.0f, -10.0f, 0.0f);
    glRotatef(-legAngle, 1.0f, 0.0f, 0.0f);

    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    drawTexturedTaperedCylinder(17.0f, 17.0f, 18.0f);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    glPushMatrix();
    glTranslatef(0.0f, -12.0f, 0.0f);
    glColor3f(0.96f, 0.68f, 0.69f);
    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    drawTaperedCylinder(17.0f, 10.0f, 15.0f);
    glPopMatrix();
    glTranslatef(0.0f, -12.8f, 0.0f);
    glutSolidSphere(10.0, 20, 20);
    glPopMatrix();

    glPopMatrix();

    // LEVEL 2: LEFT LEG
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, pLegLTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();
    glTranslatef(-20.0f, -10.0f, 0.0f);
    glRotatef(legAngle, 1.0f, 0.0f, 0.0f);

    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    drawTexturedTaperedCylinder(17.0f, 17.0f, 18.0f);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    glPushMatrix();
    glTranslatef(0.0f, -12.0f, 0.0f);
    glColor3f(0.96f, 0.68f, 0.69f);
    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    drawTaperedCylinder(17.0f, 10.0f, 15.0f);
    glPopMatrix();
    glTranslatef(0.0f, -12.8f, 0.0f);
    glutSolidSphere(10.0, 20, 20);
    glPopMatrix();

    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    glutSwapBuffers();
}

void menuAction(int option) {
    switch (option) {
    case 1:
        isLightingEnabled = !isLightingEnabled;
        break;
    case 2:
        mciSendString(TEXT("stop \"C:\\Users\\joey1\\Downloads\\old laptop\\y2s2\\FCG\\FCG-Project\\spongebob.mp3\" "), NULL, 0, NULL);
        break;
    case 3:
        mciSendString(TEXT("play \"C:\\Users\\joey1\\Downloads\\old laptop\\y2s2\\FCG\\FCG-Project\\spongebob.mp3\" repeat"), NULL, 0, NULL);
        break;
    case 4:
        exit(0);
        break;
    }
    glutPostRedisplay();
}

void init() {
    glClearColor(0.3f, 0.75f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    Image* sandImage = loadBMP("C:\\Users\\joey1\\Downloads\\old laptop\\y2s2\\FCG\\FCG-Project\\sand.bmp");
    sandTexture = loadTexture(sandImage);
    delete sandImage;

    Image* pantImage = loadBMP("C:\\Users\\joey1\\Downloads\\old laptop\\y2s2\\FCG\\FCG-Project\\pants.bmp");
    pantTexture = loadTexture(pantImage);
    delete pantImage;

    Image* pLegRImage = loadBMP("C:\\Users\\joey1\\Downloads\\old laptop\\y2s2\\FCG\\FCG - Project\\leg1.bmp");
    pLegRTexture = loadTexture(pLegRImage);
    delete pLegRImage;

    Image* pLegLImage = loadBMP("C:\\Users\\joey1\\Downloads\\old laptop\\y2s2\\FCG\\FCG-Project\\leg2.bmp");
    pLegLTexture = loadTexture(pLegLImage);
    delete pLegLImage;

    glutCreateMenu(menuAction);
    glutAddMenuEntry("Toggle Lighting", 1);
    glutAddMenuEntry("Stop Music", 2);
    glutAddMenuEntry("Start Music", 3);
    glutAddMenuEntry("Exit", 4);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    mciSendString(TEXT("play \"C:\\Users\\joey1\\Downloads\\old laptop\\y2s2\\FCG\\FCG-Project\\spongebob.mp3\" repeat"), NULL, 0, NULL);
}

void updateAnimation(int value) {
    if (!hasAutoSpun) {
        viewAngle += 5.0f;
        if (viewAngle >= 360.0f) {
            viewAngle = 0.0f;
            hasAutoSpun = true;
        }
    }

    bubbleY += 2.0f;

    if (bubbleY > 350.0f)
        bubbleY = -150.0f;

    if (keys['w']) { patrickZ -= 5.0f; patrickRotation = 180.0f; }
    if (keys['s']) { patrickZ += 5.0f; patrickRotation = 0.0f; }
    if (keys['a']) { patrickX -= 5.0f; patrickRotation = -90.0f; }
    if (keys['d']) { patrickX += 5.0f; patrickRotation = 90.0f; }

    if (patrickZ < -300.0f) patrickZ = -300.0f;
    if (patrickZ > 300.0f) patrickZ = 300.0f;
    if (patrickX < -300.0f) patrickX = -300.0f;
    if (patrickX > 300.0f) patrickX = 300.0f;

    isWalkingActive = (keys['w'] || keys['s'] || keys['a'] || keys['d']);

    if (isWalkingActive) {
        if (armSwingingForward) {
            armSwingZ += 1.5f;
            if (armSwingZ > 30.0f) armSwingingForward = false;
        }
        else {
            armSwingZ -= 1.5f;
            if (armSwingZ < -30.0f) armSwingingForward = true;
        }

        if (legSwingingForward) {
            legAngle += 1.2f;
            if (legAngle > 20.0f) legSwingingForward = false;
        }
        else {
            legAngle -= 1.2f;
            if (legAngle < -20.0f) legSwingingForward = true;
        }
    }
    else {
        armSwingZ = 0.0f;
        legAngle = 0.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(16, updateAnimation, 0);
}

void handleKeyboardInputs(unsigned char key, int x, int y) {
    keys[tolower(key)] = true;

    switch (tolower(key)) {
    case 'q':
        armAngleForwardX += 4.0f;
        if (armAngleForwardX > 20.0f) armAngleForwardX = 20.0f;
        break;
    case 'e':
        armAngleForwardX -= 4.0f;
        if (armAngleForwardX < -15.0f) armAngleForwardX = -15.0f;
        break;
    }
    glutPostRedisplay();
}

void handleKeyboardUpInputs(unsigned char key, int x, int y) {
    keys[tolower(key)] = false;
}

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            dragging = true;
            lastMouseX = x;
            lastMouseY = y;
        }
        else {
            dragging = false;
        }
    }
}

void mouseMotion(int x, int y) {
    if (dragging) {
        mouseRotateY += (x - lastMouseX);
        mouseRotateX += (y - lastMouseY);

        if (mouseRotateX > 80.0f) mouseRotateX = 80.0f;
        if (mouseRotateX < -30.0f) mouseRotateX = -30.0f;

        lastMouseX = x;
        lastMouseY = y;

        glutPostRedisplay();
    }
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (float)w / (float)h;

    gluPerspective(60.0f, aspect, 1.0f, 1000.0f);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("3D Hierarchical Patrick (Final Project)");
    init();
    glutDisplayFunc(drawPatrick3D);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeyboardInputs);
    glutKeyboardUpFunc(handleKeyboardUpInputs);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutTimerFunc(0, updateAnimation, 0);
    glutMainLoop();
    return 0;
}

