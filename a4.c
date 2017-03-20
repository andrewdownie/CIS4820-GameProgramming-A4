//// CIS*4280 A3
//// Andrew Downie - 0786342



/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

/* Frames per second code taken from : */
/* http://www.lighthouse3d.com/opengl/glut/index.php?fps */



///
/// Includes ----------------------------------------------
///
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "graphics.h"



///
/// Wall and floor settings -------------------------------
///
#define CHANGE_WALLS_TIME_MS 1500
#define AUTO_CHANGE_WALLS 1
#define TARGET_WALL_COUNT 25
#define MAX_WALL_COUNT 21

#define OUTER_WALL_COLOUR 7
#define INNER_WALL_COLOUR 1
#define PILLAR_COLOUR 2
#define FLOOR_COLOUR 3

#define WALL_COUNT_X 6
#define WALL_COUNT_Z 6
#define WALL_LENGTH 5
#define WALL_HEIGHT 2


///
/// Minimap settings
///
#define MINIMAP_SIZE_RATIO 0.33f
#define FULLMAP_SIZE_RATIO 1.0f 

#define MINIMAP_TRANSPARENCY 1
#define FULLMAP_TRANSPARENCY 0.6f



///
/// Runtime generated "Constants" -------------------------
///
int MAP_SIZE_X;
int MAP_SIZE_Z;



///
/// Player settings ---------------------------------------
///
#define HEART_BLINKS_UPON_HIT 20
#define INVULNERBILITY_TIME 3
#define GRAVITY_RATE 9.8f
#define PLAYER_HEIGHT 2
#define MAX_HEALTH 6
int currentHealth = MAX_HEALTH;
// How many milliseconds since the player got hit by a projectile
int lastHeartBlinkTime;
int heartBlinkCount = 0;
// Wheather or not the player can currenlty take damage
int invulnerble = 0;




///
/// Collision constants -----------------------------------
///
#define NOT_WALKABLE 0
#define EMPTY_PIECE 0
#define WALKABLE 1


void Minimap_Mob(float x, float z, int pixelDim, int startLeft, int startBottom);

///
/// Delta time --------------------------------------------
///       Used to record the last time an event occured.
///
int lastWallChangeTime;
int lastGravityTime;
int lastUpdateTime;

///
/// Player shooting
///
#define PROJECTILE_SPAWN_DISTANCE 0.2f 
#define PROJECTILE_MOVE_SPEED 5.0f
#define PROJECTILE_LIFE_MILI 6000
#define MAX_PROJECTILES 3 
void Shoot();
int HitPlayer(Projectile *projectile);
Projectile projectiles[MAX_PROJECTILES];



///
/// Pillars -----------------------------------------------
///         A 2D array of all the pillars, each pillar having references to the
///           walls that touch it.
///
Pillar pillars[WALL_COUNT_X - 1][WALL_COUNT_Z - 1];

///
/// Mobs
///
#define MOB_COUNT 4
#define MOB_PIXEL_COUNT 9
#define MOB_HEIGHT 4
#define MOB_COLOR 6
#define MOB_FRAME_TIME 300 
Projectile mobProjectiles[MOB_COUNT];

Mob mobs[MOB_COUNT];
int mobFrameTimePassed;

/// MOB: X-ER
#define XER_FRAME_COUNT 4
int XER_ANIMATION[XER_FRAME_COUNT][MOB_PIXEL_COUNT] = {
    {
        1, 0, 1,
        0, 1, 0,
        1, 0, 1
    },
    {
        0, 1, 0,
        0, 1, 0,
        0, 1, 0
    },
    {
        1, 0, 1,
        0, 1, 0,
        1, 0, 1
    },
    {
        0, 0, 0,
        1, 1, 1,
        0, 0, 0
    }
};

/// MOB: Circler
#define CIRCLER_FRAME_COUNT 4
int CIRCLER_ANIMATION[CIRCLER_FRAME_COUNT][MOB_PIXEL_COUNT] = {
    {
        1, 0, 1,
        0, 1, 0,
        1, 0, 1
    },
    {
        0, 1, 0,
        1, 1, 1,
        0, 1, 0
    },
    {
        1, 1, 1,
        1, 0, 1,
        1, 1, 1
    },
    {
        0, 1, 0,
        1, 1, 1,
        0, 1, 0
    }

};
void MoveMob(Mob *mob);
void EraseMob(Mob *mob);
void DrawMob(Mob *mob);
void MobShoot(int projectileID);

///
/// Wall and floor manipulation forward declarations ------
///
int PreventCollision();
void SetupWall(Wall **targetWall, Wall **adjacentWall, GenerationInfo *genInfo);
void ChangeWalls();
void SetupWalls();
void FreeWalls();

void PlaceHorizontalWall(Wall *wall, int wallX, int wallZ, int deltaTime);
void PlaceVerticalWall(Wall *wall, int wallX, int wallZ, int deltaTime);
void PlaceWalls(int deltaTime);

void BuildWorldShell();



///
/// Utility function forward delcarations -----------------
///
float Clamp(float value, float minVal, float maxVal);
float DeltaGravity(int timeSinceLastCollision);

void PrintWallGeneration();

int WalkablePiece(int x, int y, int z);
int PercentChance(float chance);

int Pillar_WallCount();
int CountAllWalls();

void ItemIndexToWorld(int x, int z, int *worldX, int * worldZ);


///
/// Engine extern declarations ----------------------------
///
/* mouse function called by GLUT when a button is pressed or released */
void mouse(int, int, int, int);

/* initialize graphics library */
extern void graphicsInit(int *, char **);

/* lighting control */
extern void setLightPosition(GLfloat, GLfloat, GLfloat);
extern GLfloat* getLightPosition();

/* viewpoint control */
extern void setViewPosition(float, float, float);
extern void getViewPosition(float *, float *, float *);
extern void getOldViewPosition(float *, float *, float *);
extern void setViewOrientation(float, float, float);
extern void getViewOrientation(float *, float *, float *);

/* add cube to display list so it will be drawn */
extern int addDisplayList(int, int, int);

/* mob controls */
extern void createMob(int, float, float, float, float);
extern void setMobPosition(int, float, float, float, float);
extern void hideMob(int);
extern void showMob(int);

/* player controls */
extern void createPlayer(int, float, float, float, float);
extern void setPlayerPosition(int, float, float, float, float);
extern void hidePlayer(int);
extern void showPlayer(int);

/* 2D drawing functions */
extern void  draw2Dline(int, int, int, int, int);
extern void  draw2Dbox(int, int, int, int);
extern void  draw2Dtriangle(int, int, int, int, int, int);
extern void  set2Dcolour(float []);


/* flag which is set to 1 when flying behaviour is desired */
extern int flycontrol;
/* flag used to indicate that the test world should be used */
extern int testWorld;
/* flag to print out frames per second */
extern int fps;
/* flag to indicate the space bar has been pressed */
extern int space;
/* flag indicates the program is a client when set = 1 */
extern int netClient;
/* flag indicates the program is a server when set = 1 */
extern int netServer;
/* size of the window in pixels */
extern int screenWidth, screenHeight;
/* flag indicates if map is to be printed */
extern int displayMap;

/* frustum corner coordinates, used for visibility determination  */
extern float corners[4][3];

/* determine which cubes are visible e.g. in view frustum */
extern void ExtractFrustum();
extern void tree(float, float, float, float, float, float, int);



///
/// collisionResponse -------------------------------------
///
void collisionResponse() {
/// Performs collision detection and response,
///          sets new xyz  to position of the viewpoint after collision.

    ///
    /// Variables
    ///
    int curIndex_x, curIndex_y, curIndex_z;
    int oldIndex_x, oldIndex_y, oldIndex_z;

    float curPos_x, curPos_y, curPos_z;
    float oldPos_x, oldPos_y, oldPos_z;

    float rotX, rotY, rotZ;

    float deltaGravity;
    int previousPiece;
    int currentPiece;
    int floorLevel;


    ///
    /// Clamp camera rotation
    ///
    getViewOrientation(&rotX, &rotY, &rotZ);

    rotX = remainderf(rotX, 360);

    if(rotX < -180){
        rotX = rotX + 180;
    }


    if(rotX > -100 && rotX < -40){
        rotX = -40;
    }


    if(rotX > 30){
        rotX = 30;
    }

    

    setViewOrientation(rotX, rotY, rotZ);




    deltaGravity = DeltaGravity(lastGravityTime);


    ///
    /// Initial Setup
    ///
    getOldViewPosition(&oldPos_x, &oldPos_y, &oldPos_z);
    getViewPosition(&curPos_x, &curPos_y, &curPos_z);

    oldIndex_x = (int)oldPos_x * -1;
    oldIndex_y = (int)oldPos_y * -1;
    oldIndex_z = (int)oldPos_z * -1;

    curIndex_x = (int)curPos_x * -1;
    curIndex_y = (int)curPos_y * -1;
    curIndex_z = (int)curPos_z * -1;

    previousPiece = WalkablePiece(oldIndex_x, oldIndex_y, oldIndex_z);
    currentPiece = WalkablePiece(curIndex_x, curIndex_y, curIndex_z);


    ///
    /// PLAYER MOVEMENT: Collision with walls and floors and ceiling
    ///

    /// Handle: camera moving down into blocks below
    if(currentPiece == NOT_WALKABLE){

        if(oldIndex_y > curIndex_y){
            curPos_y = (curIndex_y + 1) * -1;
            curIndex_y = (int)curPos_y * -1;
            currentPiece = WalkablePiece(curIndex_x, curIndex_y, curIndex_z);
        }

    }

    /// Handle: camera moving outside of the gamearea
      if(curIndex_y >= WORLDY - 1){
          curPos_y = (WORLDY - 1) * -1;
          curIndex_y = curPos_y;
          currentPiece = WalkablePiece(curIndex_x, curIndex_y, curIndex_z);
      }

      if(curIndex_x < 1){
          curPos_x = -1;
          curIndex_x = curPos_x;
          currentPiece = WalkablePiece(curIndex_x, curIndex_y, curIndex_z);
      }
      else if(curIndex_x >= MAP_SIZE_X - 2){
          curPos_x = (MAP_SIZE_X - 2) * -1;
          curIndex_x = curPos_x * -1;
          currentPiece = WalkablePiece(curIndex_x, curIndex_y, curIndex_z);
      }

      if(curIndex_z < 1){
          curPos_z = -1;
          curIndex_z = curPos_z;
          currentPiece = WalkablePiece(curIndex_x, curIndex_y, curIndex_z);
      }
      else if(curIndex_z >= MAP_SIZE_Z - 2){
            curPos_z = (MAP_SIZE_Z - 2) * -1;
            curIndex_z = curPos_z;
            currentPiece = WalkablePiece(curIndex_x, curIndex_y, curIndex_z);
      }


    /// Handle: camera moving sideways into walls
    if(currentPiece == NOT_WALKABLE){

        if(curIndex_x != oldIndex_x || curIndex_z != oldIndex_z){
            curPos_x = oldPos_x;
            curPos_z = oldPos_z;
        }

    }



    ///
    /// GRAVITY: collision with walls and floors
    ///
    oldIndex_x = curIndex_x;
    oldIndex_y = curIndex_y;
    oldIndex_z = curIndex_z;
    oldPos_x = curPos_x;
    oldPos_y = curPos_y;
    oldPos_z = curPos_z;


    if(flycontrol == 0){
        for(floorLevel = curIndex_y; floorLevel > 0; floorLevel--){

            if(WalkablePiece(curIndex_x, floorLevel, curIndex_z) == NOT_WALKABLE){
                break;
            }
        }
        floorLevel = floorLevel + PLAYER_HEIGHT;

        if(floorLevel >= (curPos_y * -1) - deltaGravity){
            curPos_y = floorLevel * -1;
        }
        else{
            curPos_y = curPos_y + deltaGravity;
        }

    }

    ///
    /// Prevent climbing walls higher than one block tall
    ///
    curIndex_y = (int)curPos_y * -1;
    oldIndex_y = (int)oldPos_y * -1;

    if(curIndex_y > oldIndex_y + 1){
        curPos_x = oldPos_x;
        curPos_y = oldPos_y;
        curPos_z = oldPos_z;
    }


    ///
    /// Finish
    ///
    setViewPosition(curPos_x, curPos_y, curPos_z);
    lastGravityTime = glutGet(GLUT_ELAPSED_TIME);
}



///
/// draw2D
///
void draw2D() {
    ///
    ///     Color
    ///
    GLfloat orange[] = {1.0, 0.5, 0.0, MINIMAP_TRANSPARENCY};
    GLfloat green[] = {0.0, 0.5, 0.0, MINIMAP_TRANSPARENCY};
    GLfloat blue[] = {0.0, 0.0, 0.5, MINIMAP_TRANSPARENCY};
    GLfloat red[] = {0.5, 0.0, 0.0, MINIMAP_TRANSPARENCY};
    GLfloat heart_red[] = {0.5, 0.0, 0.0, 1.0};
    GLfloat white[] = {1, 1, 1, 1};
    GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
    GLfloat yellow[] = {1, 0, 1, 1.0};

    GLfloat gold[] = {1.0, 0.84, 0.0, 1.0};
    GLfloat grey[] = {0.5, 0.5, 0.5, 0.5};

    ///
    ///     Mimimap variables 
    ///
    int widthRatio, heightRatio;
    int pixelDim, playerDim;
    int startBottom;
    int startLeft;

    float player_x, player_y, player_z;
    float projectile_x, projectile_z;

    int curX, curZ;
    int x, z;
    int i;

    float map_ratio;
    int projectileCount;


    ///
    /// Health variables
    ///
    int heartStartX;
    int heartStartY;




    ///
    /// Minimap display mode: hidden, top right, full screen
    ///
    if(displayMap == 0){
        return;
    }
    else if(displayMap == 1){
       map_ratio = MINIMAP_SIZE_RATIO; 

    }
    else if(displayMap == 2){
        map_ratio = FULLMAP_SIZE_RATIO;
        red[3] = FULLMAP_TRANSPARENCY;
        green[3] = FULLMAP_TRANSPARENCY;
        blue[3] = FULLMAP_TRANSPARENCY;
        orange[3] = FULLMAP_TRANSPARENCY;
    }




    ///
    /// Calculate scaling for the Minimap
    ///
    widthRatio = ceil(screenWidth / MAP_SIZE_X * map_ratio);
    heightRatio = ceil(screenHeight / MAP_SIZE_Z * map_ratio);

    if(widthRatio < heightRatio){
        pixelDim = widthRatio;
    }
    else{
        pixelDim = heightRatio;
    }

    playerDim = pixelDim / 2;
    if(playerDim < 1){
        playerDim = 1;
    }

    if(displayMap == 1){
        startLeft = screenWidth - (pixelDim * MAP_SIZE_X);
        startBottom = screenHeight - (pixelDim * MAP_SIZE_Z);
    }
    else{
        startLeft = screenWidth / 2 - (pixelDim * (MAP_SIZE_X - 1)) / 2;
        startBottom = screenHeight / 2 - (pixelDim * (MAP_SIZE_Z - 1)) / 2;
    }

    ///
    /// Draw the player
    ///
    getViewPosition(&player_x, &player_y, &player_z);
    player_x = player_x * -1;
    player_z = player_z * -1;
    player_z = MAP_SIZE_Z - player_z;

    set2Dcolour(black);
    Minimap_Mob(player_x, player_z, pixelDim, startLeft, startBottom);

    ///
    /// Draw the projectiles
    ///
    set2Dcolour(yellow);
    
    for(i = 0; i < MAX_PROJECTILES; i++){
        if(projectiles[i].enabled){
            projectile_x = projectiles[i].x * -1;
            projectile_z = projectiles[i].z * -1;
            projectile_z = MAP_SIZE_Z - projectile_z;
            Minimap_Mob(projectile_x, projectile_z, pixelDim, startLeft, startBottom);
        }
    }
    


    ///
    /// Draw the map 
    ///
    for(z = 0; z < MAP_SIZE_Z - 1; z++){
        for(x = 0; x < MAP_SIZE_X - 1; x++){
            curX = startLeft + pixelDim * x;
            curZ = startBottom + pixelDim * (MAP_SIZE_Z - z  - 2);


            if(world[x][1][z] == OUTER_WALL_COLOUR){
                set2Dcolour(orange);
            }
            else if(world[x][1][z] == INNER_WALL_COLOUR || world[x][2][z] == INNER_WALL_COLOUR){
                set2Dcolour(green);
            }
            else if(world[x][1][z] == PILLAR_COLOUR){
                set2Dcolour(blue);
            }
            else if(world[x][1][z] == MOB_COLOR){
                set2Dcolour(yellow);
            }
            else{
                set2Dcolour(red);
            }

            draw2Dbox(curX, curZ, curX + pixelDim, curZ + pixelDim);
        }
    }



    ///
    /// Draw player ammo
    ///
    projectileCount = 0;
    for(i = 0; i < MAX_PROJECTILES; i++){
        if(projectiles[i].enabled == 0){
            projectileCount++;
        }
    }

    for(i = 0; i < MAX_PROJECTILES; i++){
        if(i < projectileCount){
            set2Dcolour(gold);
        }
        else{
            set2Dcolour(grey);
        }

        draw2Dbox( 5 + i * 2 * pixelDim, 5, 2 * pixelDim + i * 2 * pixelDim, 5 + pixelDim * 5);
    }

    ///
    /// Draw Player Hearts
    ///
    if(invulnerble && glutGet(GLUT_ELAPSED_TIME) - lastHeartBlinkTime > 100){
        heartBlinkCount++;
        lastHeartBlinkTime = glutGet(GLUT_ELAPSED_TIME);
        if(heartBlinkCount >= HEART_BLINKS_UPON_HIT){
            invulnerble = 0;
        }
    }

    if(invulnerble && heartBlinkCount % 2 != 0){
        set2Dcolour(black);
    }
    else{
        set2Dcolour(heart_red);
    }

    heartStartY = screenHeight - 10;
    for(i = 0; i < currentHealth; i++){
        heartStartX = 10 + (i * 45);

        //Top left piece
        draw2Dtriangle(heartStartX + 10, heartStartY, heartStartX, heartStartY - 10, heartStartX + 20, heartStartY - 10);
        //Top right piece
        draw2Dtriangle(heartStartX + 30, heartStartY, heartStartX + 20, heartStartY - 10, heartStartX + 40, heartStartY - 10);
        //Bottom piece
        draw2Dtriangle(heartStartX, heartStartY - 10, heartStartX + 20, heartStartY - 40, heartStartX + 40, heartStartY - 10);
        
    }



}


void Minimap_Mob(float x, float z, int pixelDim, int startLeft, int startBottom){
    int start_x, end_x, start_y, end_y;
    int playerDim;
    
    playerDim = pixelDim / 2;
    if(playerDim < 1){
        playerDim = 0;
    }
    

    start_x = startLeft + round(x * pixelDim) - playerDim; 
    start_y = startBottom + round((z - 2) * pixelDim) + playerDim;
    end_x = startLeft + round((x + 1) * pixelDim) - playerDim;
    end_y = startBottom + round((z - 1) * pixelDim) + playerDim;

    draw2Dbox(start_x, start_y, end_x, end_y);
}



///
/// update()
///
void update() {
/// Background process, it is called when there are no other events.
    int i;
    /* sample animation for the test world, don't remove this code */
    /* -demo of animating mobs */
    if (testWorld) {

        /* sample of rotation and positioning of mob */
        /* coordinates for mob 0 */
        static float mob0x = 50.0, mob0y = 25.0, mob0z = 52.0;
        static float mob0ry = 0.0;
        static int increasingmob0 = 1;
        /* coordinates for mob 1 */
        static float mob1x = 50.0, mob1y = 25.0, mob1z = 52.0;
        static float mob1ry = 0.0;
        static int increasingmob1 = 1;

        /* move mob 0 and rotate */
        /* set mob 0 position */
        setMobPosition(0, mob0x, mob0y, mob0z, mob0ry);

        /* move mob 0 in the x axis */
        if (increasingmob0 == 1)
        mob0x += 0.2;
        else
        mob0x -= 0.2;
        if (mob0x > 50) increasingmob0 = 0;
        if (mob0x < 30) increasingmob0 = 1;

        /* rotate mob 0 around the y axis */
        mob0ry += 1.0;
        if (mob0ry > 360.0) mob0ry -= 360.0;

        /* move mob 1 and rotate */
        setMobPosition(1, mob1x, mob1y, mob1z, mob1ry);

        /* move mob 1 in the z axis */
        /* when mob is moving away it is visible, when moving back it */
        /* is hidden */
        if (increasingmob1 == 1) {
            mob1z += 0.2;
            showMob(1);
        } else {
            mob1z -= 0.2;
            hideMob(1);
        }
        if (mob1z > 72) increasingmob1 = 0;
        if (mob1z < 52) increasingmob1 = 1;

        /* rotate mob 1 around the y axis */
        mob1ry += 1.0;
        if (mob1ry > 360.0) mob1ry -= 360.0;
        /* end testworld animation */




    } else {
        int currentElapsedTime, deltaWallChangeTime;
        int deltaTime;
        float deltaX, deltaY, deltaZ;
        float curX, curY, curZ;
        int intX, intY, intZ;
        int mobID;


        ///
        /// Move walls
        ///
        if(AUTO_CHANGE_WALLS){
            currentElapsedTime = glutGet(GLUT_ELAPSED_TIME);
            deltaWallChangeTime = currentElapsedTime - lastUpdateTime;

            lastWallChangeTime += deltaWallChangeTime;

            if(lastWallChangeTime >= CHANGE_WALLS_TIME_MS){
                lastWallChangeTime = 0;
                ChangeWalls();

            }

            PlaceWalls(deltaWallChangeTime);


        }

        for(i = 0; i < MOB_COUNT; i++){
            DrawMob(&(mobs[i]));
        }


        ///
        /// Move player projectiles
        ///
        deltaTime = glutGet(GLUT_ELAPSED_TIME) - lastUpdateTime;
        for(i = 0; i < MAX_PROJECTILES; i++){
            if(projectiles[i].enabled){
                deltaX = projectiles[i].moveX * deltaTime / 1000;
                deltaY = projectiles[i].moveY * deltaTime / 1000;
                deltaZ = projectiles[i].moveZ * deltaTime / 1000; 
                curX = projectiles[i].x - deltaX;
                curY = projectiles[i].y - deltaY;
                curZ = projectiles[i].z - deltaZ;
                mobID = projectiles[i].mobID;

                projectiles[i].x = curX;
                projectiles[i].y = curY;
                projectiles[i].z = curZ;



                setMobPosition(mobID, projectiles[i].x * -1, projectiles[i].y * - 1, projectiles[i].z * -1, 0);

                projectiles[i].timeEnabled += deltaTime;

                if(projectiles[i].timeEnabled >= PROJECTILE_LIFE_MILI){
                    projectiles[i].enabled = 0;
                    hideMob(projectiles[i].mobID);
                }


                intX = (int)(curX * -1 + 0.5f);
                intY = (int)(curY * -1 + 0.5f);
                intZ = (int)(curZ * -1 + 0.5f); 
                if(intX >= 0 && intX < WORLDX && intY >= 0 && intY < WORLDY && intZ >= 0 && intZ < WORLDZ){
                    if(world[intX][intY][intZ] != EMPTY_PIECE){
                        projectiles[i].enabled = 0;
                        hideMob(projectiles[i].mobID);
                        if(world[intX][intY][intZ] == INNER_WALL_COLOUR){

                            world[intX][intY][intZ] = 0;
                        }
                    }
                }


                
               /*if(HitPlayer(&(projectiles[i]))){
                   printf(">> A PROJECTILE HIT THE PLAYER!!!\n");
                    projectiles[i].enabled = 0;
                    hideMob(projectiles[i].mobID);
               }*/
            }
        }

        ///
        /// Move mob projectiles
        ///
        for(i = 0; i < MOB_COUNT; i++){
            if(mobProjectiles[i].enabled){
                deltaX = mobProjectiles[i].moveX * deltaTime / 1000;
                deltaY = mobProjectiles[i].moveY * deltaTime / 1000;
                deltaZ = mobProjectiles[i].moveZ * deltaTime / 1000; 
                curX = mobProjectiles[i].x - deltaX;
                curY = mobProjectiles[i].y - deltaY;
                curZ = mobProjectiles[i].z - deltaZ;
                mobID = mobProjectiles[i].mobID;

                mobProjectiles[i].x = curX;
                mobProjectiles[i].y = curY;
                mobProjectiles[i].z = curZ;
                


                setMobPosition(i + MAX_PROJECTILES, mobProjectiles[i].x * -1, mobProjectiles[i].y * - 1, mobProjectiles[i].z * -1, 0);

                mobProjectiles[i].timeEnabled += deltaTime;

                if(mobProjectiles[i].timeEnabled >= PROJECTILE_LIFE_MILI){
                    mobProjectiles[i].enabled = 0;
                    hideMob(mobID + MAX_PROJECTILES);
                }

                
               intX = (int)(curX * -1 + 0.5f);
               intY = (int)(curY * -1 + 0.5f);
               intZ = (int)(curZ * -1 + 0.5f); 
               if(intX >= 0 && intX < WORLDX && intY >= 0 && intY < WORLDY && intZ >= 0 && intZ < WORLDZ){
                    if(world[intX][intY][intZ] != EMPTY_PIECE){
                        mobProjectiles[i].enabled = 0;
                        hideMob(i + MAX_PROJECTILES);
                        if(world[intX][intY][intZ] == INNER_WALL_COLOUR){

                            world[intX][intY][intZ] = 0;
                        }
                    }
               }

               if(HitPlayer(&(mobProjectiles[i]))){
                   printf(">> A PROJECTILE HIT THE PLAYER!!!\n");
                   invulnerble = 1;
                   lastHeartBlinkTime = 0;
                   heartBlinkCount = 0;
                   currentHealth--;
                    mobProjectiles[i].enabled = 0;
                    hideMob(i + MAX_PROJECTILES);
               }


                
            }
        }





        mobFrameTimePassed += glutGet(GLUT_ELAPSED_TIME) - lastUpdateTime;
        if(mobFrameTimePassed > MOB_FRAME_TIME){
            for(i = 0; i < MOB_COUNT; i++){
                EraseMob(&(mobs[i]));
                mobs[i].frame++;
                if(mobs[i].frame >= XER_FRAME_COUNT){
                    mobs[i].frame = 0;

                    // SHOOT
                    MobShoot(i);
                }

                MoveMob(&(mobs[i]));
                
                DrawMob(&(mobs[i]));
            }



            mobFrameTimePassed = 0;
        }





        lastUpdateTime = glutGet(GLUT_ELAPSED_TIME);
        collisionResponse();
    }
}

///
/// HitPlayer 
///         Check whether a projectile hit the player
int HitPlayer(Projectile *projectile){
    float plX, plY, plZ;//Player xyz
    float prX, prY, prZ;//Projectile xyz 
    float dX, dY, dZ;//Delta xyz

    if(invulnerble){
        return 0;
    }

    getViewPosition(&plX, &plY, &plZ);

    prX = projectile->x;
    prY = projectile->y;
    prZ = projectile->z;

    dX = abs(plX - prX);
    dY = abs(plY - prY);
    dZ = abs(plZ - prZ);

    if(dX < 0.5f && dY < 0.5f && dZ < 0.5f){
        return 1;
    }

    return 0;
}



///
/// Mouse
///
void mouse(int button, int state, int x, int y) {
/// called by GLUT when a mouse button is pressed or released.

    if (button == GLUT_LEFT_BUTTON){
        if(state == GLUT_DOWN){
            Shoot();
        }
    }
    else if (button == GLUT_MIDDLE_BUTTON){
        printf("middle button - ");
    }
    else{
        printf("right button - ");
    }

    if (state == GLUT_UP){

    }
    else{

    }

}

void ItemIndexToWorld(int x, int z, int *worldX, int *worldZ){
    int xHalf, zHalf;
    int outX, outZ;

    xHalf = x / 2;
    zHalf = z / 2;

    outX = 4 * xHalf + 2 * xHalf; 
    outZ = 4 * zHalf + 2 * zHalf;


    if(x % 2 != 0){
        outX += 4;
    }

    if(z % 2 != 0){
        outZ += 4;
    }

    outX++;
    outZ++;

    (*worldX) = outX;
    (*worldZ) = outZ;

}


int main(int argc, char** argv)
{
    int i, j, k;
    /* initialize the graphics system */
    graphicsInit(&argc, argv);

    /* the first part of this if statement builds a sample */
    /* world which will be used for testing */
    /* DO NOT remove this code. */
    /* Put your code in the else statment below */
    /* The testworld is only guaranteed to work with a world of
    with dimensions of 100,50,100. */
    if (testWorld == 1) {
        /* initialize world to empty */
        for(i=0; i<WORLDX; i++)
        for(j=0; j<WORLDY; j++)
        for(k=0; k<WORLDZ; k++)
        world[i][j][k] = 0;

        /* some sample objects */
        /* build a red platform */
        for(i=0; i<WORLDX; i++) {
            for(j=0; j<WORLDZ; j++) {
                world[i][24][j] = 3;
            }
        }
        /* create some green and blue cubes */
        world[50][25][50] = 1;
        world[49][25][50] = 1;
        world[49][26][50] = 1;
        world[52][25][52] = 2;
        world[52][26][52] = 2;

        /* blue box shows xy bounds of the world */
        for(i=0; i<WORLDX-1; i++) {
            world[i][25][0] = 2;
            world[i][25][WORLDZ-1] = 2;
        }
        for(i=0; i<WORLDZ-1; i++) {
            world[0][25][i] = 2;
            world[WORLDX-1][25][i] = 2;
        }

        /* create two sample mobs */
        /* these are animated in the update() function */
        createMob(0, 50.0, 25.0, 52.0, 0.0);
        createMob(1, 50.0, 25.0, 52.0, 0.0);

        /* create sample player */
        createPlayer(0, 52.0, 27.0, 52.0, 0.0);

    } else {

        flycontrol = 0;

        ///
        /// initialize random
        ///
        srand((unsigned) time(NULL));

        ///
        /// Set lastUpdateTime to zero
        ///
        lastGravityTime = 0;

        MAP_SIZE_X = (WALL_COUNT_X * WALL_LENGTH) + WALL_COUNT_X + 2;
        MAP_SIZE_Z = (WALL_COUNT_Z * WALL_LENGTH) + WALL_COUNT_Z + 2;

        
        ///
        /// Spawn the items on the map
        ///
        int tstX, tstZ;
        ItemIndexToWorld(5, 3, &tstX, &tstZ);
       printf("new x %d, new z %d\n", tstX, tstZ); 


        ///
        /// Build the initial world
        ///
        BuildWorldShell();
        SetupWalls();
        PlaceWalls(0);

        printf("Wall count: %d\n", CountAllWalls());


        ///
        /// Create the player projectiles
        for(i = 0; i < MAX_PROJECTILES; i++){
            createMob(i, i, 5.0, 1.0, 0.0);
            hideMob(i);
            projectiles[i].mobID = i;
        }

        for(i = 0; i < MOB_COUNT; i++){
            createMob(i + MAX_PROJECTILES, i + MAX_PROJECTILES, 5.0, 1.0, 0.0);
            hideMob(i + MAX_PROJECTILES);
            mobProjectiles[i].mobID = i + MAX_PROJECTILES;
        }

        ///
        /// Setup the mobs
        ///
        mobs[0].type = 0;
        mobs[0].startX = 8;
        mobs[0].startZ = 8;
        mobs[0].endX = 8;
        mobs[0].endZ = 8;
        mobs[0].frame = 0;
        mobs[0].projectileIndex = MAX_PROJECTILES;

        mobs[1].type = 0;
        mobs[1].startX = 32;
        mobs[1].startZ = 2;
        mobs[1].endX = 32;
        mobs[1].endZ = 2;
        mobs[1].frame = 0;
        mobs[1].projectileIndex = MAX_PROJECTILES + 1;

        mobs[2].type = 1;
        mobs[2].startX = 2;
        mobs[2].startZ = 32;
        mobs[2].endX = 2;
        mobs[2].endZ = 32;
        mobs[2].frame = 0;
        mobs[2].projectileIndex = MAX_PROJECTILES + 2;

        mobs[3].type = 1;
        mobs[3].startX = 32;
        mobs[3].startZ = 32;
        mobs[3].endX = 32;
        mobs[3].endZ = 32;
        mobs[3].frame = 0;
        mobs[3].projectileIndex = MAX_PROJECTILES + 3;

        mobFrameTimePassed = 0;
        ///
        /// Create the mobs
        ///
        for(i = 0; i < MOB_COUNT; i++){
            DrawMob(&(mobs[i]));
        } 


    }




    /* starts the graphics processing loop */
    /* code after this will not run until the program exits */
    glutMainLoop();
    FreeWalls();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/////
///// Mobs 
/////
void MoveMob(Mob *mob){
    int i = 0;
    int height = 0;

    int randMove[4] = {1, 1, 1, 1};//Left, right, up, down
    int randDir = rand() % 4;
    int playerAbove, playerBelow, playerRight, playerLeft;
    int distX, distZ;
    float playerX, playerY, playerZ;

    int seenByPlayer = MobVisible(mob->startX, mob->startX + 3, mob->startZ, mob->endZ + 3);

    getViewPosition(&playerX, &playerY, &playerZ);

    playerAbove = 0;
    playerBelow = 0;
    playerRight = 0;
    playerLeft = 0; 

    distX = abs(playerX - mob->startX);
    distZ = abs(playerX - mob->startZ);

    if(seenByPlayer){
        if(playerX > mob->startX + 1){
            playerBelow = 1;
        }
        else{
            playerAbove = 1;
        }

        if(playerY > mob->startZ + 1){
            playerRight = 1;
        }
        else{
            playerLeft = 1;
        }
    }

    if(distX > distZ){
        playerRight = 0;
        playerLeft = 0;
    }
    else{
        playerAbove = 0;
        playerBelow = 0;
    }



    ///
    /// Move the mob
    ///
    if(mob->endX > mob->startX){
        mob->startX++;
    } 
    else if(mob->endX < mob->startX){
        mob->startX--;
    }
    else if(mob->endZ > mob->startZ){
        mob->startZ++;
    }
    else if(mob->endZ < mob->startZ){
        mob->startZ--;
    }

    ///
    /// Pick a new destination
    ///
    if(mob->startX != mob->endX || mob->startZ != mob->endZ){
        return;
    }

    //Left move
    if(mob->startX >= 8){
        for(height = 0; height < WALL_HEIGHT; height++){
            for(i = 0; i < 3; i++){
                if(world[mob->startX - 2][height + 1][i + mob->startZ] != 0){
                    randMove[0] = 0;
                }
            }
        }
    }
    else{
        randMove[0] = 0;
    }

    //Right move
    if(mob->startX <= 26){
        for(height = 0; height < WALL_HEIGHT; height++){
            for(i = 0; i < 3; i++){
                if(world[mob->startX + 4][height + 1][i + mob->startZ] != 0){
                    randMove[1] = 0;
                }
            }
        }
    }
    else{
        randMove[1] = 0;
    }

    //Up move
    if(mob->startZ >= 8){
        for(height = 0; height < WALL_HEIGHT; height++){
            for(i = 0; i < 3; i++){
                if(world[mob->startX + i][height + 1][mob->startZ - 2] != 0){
                    randMove[2] = 0;
                }
            }
        }
    }
    else{
        randMove[2] = 0;
    }

    //Down move
    if(mob->startZ <= 26){
        for(height = 0; height < WALL_HEIGHT; height++){
            for(i = 0; i < 3; i++){
                if(world[mob->startX + i][height + 1][mob->startZ + 4] != 0){
                    randMove[3] = 0;
                }
            }
        }
    }
    else{
        randMove[3] = 0;
    }



    if(randMove[0] == 1 && randDir == 0 && !playerLeft){
        mob->endX = mob->startX - 6;
        if(PreventCollision()){
            mob->endX = mob->startX;
        }
        return;
    }
    else if(randMove[1] == 1 && randDir <= 1 && !playerRight){
        mob->endX = mob->startX + 6;
        if(PreventCollision()){
            mob->endX = mob->startX;
        }
        return;
    }
    else if(randMove[2] == 1 && randDir <= 2 && !playerAbove){
        mob->endZ = mob->startZ - 6;
        if(PreventCollision()){
            mob->endZ = mob->startZ;
        }
        return;
    }
    else if(randMove[3] == 1 && randDir <= 3 && !playerBelow){
        mob->endZ = mob->startZ + 6;
        if(PreventCollision()){
            mob->endZ = mob->startZ;
        }
        return;
    }


    if(randMove[0] == 1 && !playerLeft){
        mob->endX = mob->startX - 6;
        if(PreventCollision()){
            mob->endX = mob->startX;
        }
        return;
    }
    else if(randMove[1] == 1 && !playerRight){
        mob->endX = mob->startX + 6;
        if(PreventCollision()){
            mob->endX = mob->startX;
        }
        return;
    }
    else if(randMove[2] == 1 && !playerAbove){
        mob->endZ = mob->startZ - 6;
        if(PreventCollision()){
            mob->endZ = mob->startZ;
        }
        return;
    }
    else if(randMove[3] == 1 && !playerBelow){
        mob->endZ = mob->startZ + 6;
        if(PreventCollision()){
            mob->endZ = mob->startZ;
        }

        return;
    }

    mob->endX = mob->startX;
    mob->endZ = mob->startZ;
}

int PreventCollision(){
    int i, j;

    for(i = 0; i < MOB_COUNT; i++){
        for(j = 0; j < MOB_COUNT; j++){
            if(i != j){
                if(mobs[i].endX == mobs[j].endX && mobs[i].endZ == mobs[j].endZ){
                    return 1;
                }
            }
        }        
    }

    return 0;
}




void EraseMob(Mob *mob){
    int x, z, y;
    int *frame;

    if(mob->type == 0){
        frame = XER_ANIMATION[mob->frame];
    }
    else{
        frame = CIRCLER_ANIMATION[mob->frame];
    }

    for(x = 0; x < 3; x++){
        for(z = 0; z < 3; z++){
            for(y = 0; y < MOB_HEIGHT; y++){

                if(frame[x + z * 3] == 1){
                    world[mob->startX + x][1 + y][mob->startZ + z] = 0;
                }            

            }
        }
    }

}


void DrawMob(Mob *mob){
    int x, z, y;
    int *frame;

    if(mob->type == 0){
        frame = XER_ANIMATION[mob->frame];
    }
    else{
        frame = CIRCLER_ANIMATION[mob->frame];
    }

    for(x = 0; x < 3; x++){
        for(z = 0; z < 3; z++){
            for(y = 0; y < MOB_HEIGHT; y++){

                if(frame[x + z * 3] == 1){
                    world[mob->startX + x][1 + y][mob->startZ + z] = MOB_COLOR;
                }            

            }
        }
    }

}

void MobShoot(int ID){
    float playerX, playerY, playerZ;
    float rotX, rotY, rotZ;

    int i;

    //float mobX, mobZ;
    float moveX, moveY, moveZ;
    int mobID;

    getViewPosition(&playerX, &playerY, &playerZ);
    getViewOrientation(&rotX, &rotY, &rotZ);

    if(mobProjectiles[ID].enabled){
        return;
    }    

    mobProjectiles[ID].enabled = 1;
    mobProjectiles[ID].timeEnabled = 0;


    mobProjectiles[ID].moveX = -(mobs[ID].startX + playerX + 1) / 5;
    mobProjectiles[ID].moveY = -(playerY + MOB_HEIGHT + 2) / 5; 
    mobProjectiles[ID].moveZ = -(mobs[ID].startZ + playerZ + 1) / 5;




    mobProjectiles[ID].x = -mobs[ID].startX - 1;
    mobProjectiles[ID].y = -MOB_HEIGHT - 1;
    mobProjectiles[ID].z = -mobs[ID].startZ - 1;

    setMobPosition(ID + MAX_PROJECTILES, mobProjectiles[ID].x * -1, mobProjectiles[ID].y * -1, mobProjectiles[ID].z * -1, 0);
    showMob(ID + MAX_PROJECTILES);


}


////////////////////////////////////////////////////////////////////////////////
/////
///// World Building Functions =================================================
/////



///
/// BuildWorldShell
///
void BuildWorldShell(){
/// Builds the floor, the outer walls, and the pillars.

    int x, y, z;
    int height;

    ///
    /// Initialize world to empty
    ///
    for(x=0; x<WORLDX; x++){
        for(y=0; y<WORLDY; y++){
            for(z=0; z<WORLDZ; z++){
                world[x][y][z] = 0;
            }
        }
    }


    ///
    /// Build the floor
    ///
    for(x=0; x < MAP_SIZE_X - 1; x++) {
        for(z=0; z < MAP_SIZE_Z - 1; z++) {
            world[x][0][z] = FLOOR_COLOUR;
        }
    }

    ///
    /// Build the outer walls
    ///
    for(height = 0; height < WALL_HEIGHT; height++){

        for(x=0; x < MAP_SIZE_X - 1; x++) {
            world[x][1 + height][0] = OUTER_WALL_COLOUR;
            world[x][1 + height][MAP_SIZE_Z-2] = OUTER_WALL_COLOUR;
        }

        for(z=0; z < MAP_SIZE_Z - 1; z++) {
            world[0][1 + height][z] = OUTER_WALL_COLOUR;
            world[MAP_SIZE_X-2][1 + height][z] = OUTER_WALL_COLOUR;
        }

    }

    ///
    /// Create the pillars
    ///
    for(x = 0; x < WALL_COUNT_X + 1; x++){
        for(z = 0; z < WALL_COUNT_Z + 1; z++){

            for(height = 0; height < WALL_HEIGHT; height++){
                world[x * (WALL_LENGTH + 1)][1 + height][z * (WALL_LENGTH + 1)] = PILLAR_COLOUR;
            }

        }
    }

}



///
/// SetupWalls --------------------------------------------
///
void SetupWalls(){
/// Generates the initial setup of all the walls. Goes through each wall, and
///           calls the function SetupWall()
/// Basically this function acts as for a manager for all the SetupWall calls
///           being made. The result of every SetupWall call is tracked in the
///           "genInfo" variable.
    int wallCount;
    int x, z;

    GenerationInfo genInfo;


    genInfo.spawnChanceModifier = 0;
    genInfo.spawnChance = 0;

    genInfo.creationAttempts = 0;
    genInfo.wallsCreated = 0;

    wallCount = ((WALL_COUNT_X - 1) * WALL_COUNT_Z) + (WALL_COUNT_X * (WALL_COUNT_Z - 1));
    genInfo.spawnChance = (TARGET_WALL_COUNT * 100) / wallCount;


    ///
    /// Set pillars to null
    ///
    for(x = 0; x < WALL_COUNT_X; x++){
        for(z = 0; z < WALL_COUNT_Z; z++){
            pillars[x][z].north = NULL;
            pillars[x][z].south = NULL;
            pillars[x][z].east = NULL;
            pillars[x][z].west = NULL;
        }
    }


    for(x = 0; x < WALL_COUNT_X - 1; x++){
        for(z = 0; z < WALL_COUNT_Z - 1; z++){

            ///
            /// North wall
            ///
            if(z > 0){
                if(Pillar_WallCount( &(pillars[x][z - 1]) ) < 3){
                    SetupWall( &(pillars[x][z].north), &(pillars[x][z - 1].south), &genInfo);
                }
            }
            else{
                SetupWall( &(pillars[x][z].north), NULL, &genInfo);
            }


            ///
            /// South wall
            ///
            if(z < WALL_COUNT_Z){
                if(Pillar_WallCount( &(pillars[x][z + 1]) ) < 3){
                    SetupWall( &(pillars[x][z].south), &(pillars[x][z + 1].north), &genInfo);
                }
            }
            else{
                SetupWall( &(pillars[x][z].south), NULL, &genInfo);
            }


            ///
            /// East Wall
            ///
            if(x < WALL_COUNT_X - 1){
                if(Pillar_WallCount( &(pillars[x + 1][z]) ) < 3){
                    SetupWall( &(pillars[x][z].east), &(pillars[x + 1][z].west), &genInfo);
                }
            }
            else{
                SetupWall( &(pillars[x][z].east), NULL, &genInfo);
            }


            ///
            /// West Wall
            ///
            if(x > 0){
                if(Pillar_WallCount( &(pillars[x - 1][z]) ) < 3){
                    SetupWall( &(pillars[x][z].west), &(pillars[x - 1][z].east), &genInfo);
                }
            }
            else{
                SetupWall( &(pillars[x][z].west), NULL, &genInfo);
            }


        }



    }

    pillars[0][0].west->percentClosed = 0;
    pillars[0][0].west->state = open;

}


///
/// FreeWalls ---------------------------------------------
///
void FreeWalls(){
/// Goes through all the walls, and frees them.

    int x, z;

    for(x = 0; x < WALL_COUNT_X - 1; x++){
        for(z = 0; z < WALL_COUNT_Z - 1; z++){
            if(x == 0){
                free(pillars[z][x].west);
            }
            if(z == 0){
                free(pillars[z][x].north);
            }

            free(pillars[z][x].east);
            free(pillars[z][x].south);
        }
    }

}



///
/// PlaceVerticalWall -------------------------------------
///
void PlaceVerticalWall(Wall *wall, int wallX, int wallZ, int deltaTime){
/// Places blocks starting at "wallX" and "wallZ", and moves along the z-axis.
/// --
/// Uses "*wall" to figure out if the wall is open, opening, closed or closing,
///      also used "*wall" to figure out what direction opening and closing
///      should be moved.
/// Uses "deltaTime" to figure out how much an opening or closing wall should be
///      change in length, which results in the walls appearing to be animated.

    int actualWallLength;
    int yOffset, z, y;
    int deltaPercent;

    float playerXf, playerYf, playerZf;
    int playerX, playerY, playerZ;


    ///
    /// Clear the wall first
    ///
    if(wall->state == open){

        for(z = 0; z < WALL_LENGTH; z++){

            for(yOffset = 0; yOffset < WALL_HEIGHT; yOffset++){
                if(world[wallX][1 + yOffset][wallZ + z] != MOB_COLOR){

                    world[wallX][1 + yOffset][wallZ + z] = 0;
                }
            }

        }
    }


    ///
    /// Update the wall using deltaTime
    ///
    deltaPercent = (((float)deltaTime * 2) / (float)CHANGE_WALLS_TIME_MS) * 100;
    if(wall->state == opening){
        wall->percentClosed -= deltaPercent;
        if(wall->percentClosed < 0){
            wall->percentClosed = 0;
            wall->state = open;
        }
    }
    else if(wall->state == closing){
        wall->percentClosed += deltaPercent;
        if(wall->percentClosed > 100){
            wall->percentClosed = 100;
            wall->state = closed;
        }
    }


    ///
    /// Place the wall
    ///
    actualWallLength = (WALL_LENGTH * wall->percentClosed) / 100;
    if(wall->state == closed && deltaTime != 0){
        return;///////////////////////////////////////////////////////////////////
    }

    for(yOffset = 0; yOffset < WALL_HEIGHT; yOffset++){
        for(z = 0; z < WALL_LENGTH; z++){

            if(deltaTime != 0){
                //z = actualWallLength - 1;
            }


            if(z - 1 < actualWallLength){
                if( (wall->direction == moveNorth && wall->state == closing) ){
                    world[wallX][1 + yOffset][wallZ + z] = INNER_WALL_COLOUR;
                }
                else{
                    world[wallX][1 + yOffset][wallZ + WALL_LENGTH - z - 1] = INNER_WALL_COLOUR;
                }
                getViewPosition(&playerXf, &playerYf, &playerZf);
                playerX = (int)playerXf * -1;
                playerY = (int)playerYf * -1;
                playerZ = (int)playerZf * -1;

                for(y = 0; y < WALL_HEIGHT; y++){
                    if(playerX == wallX && playerZ == wallZ + z && y + 1 == playerY){
                        setViewPosition(playerXf + 1, playerYf, playerZf);
                    }
                }
            }
            else if(world[wallX][1 + yOffset][wallZ + z]){
                world[wallX][1 + yOffset][wallZ + z] = 0;
            }



        }

    }


}


///
/// PlaceHorizontalWall -----------------------------------
///
void PlaceHorizontalWall(Wall *wall, int wallX, int wallZ, int deltaTime){
/// Places blocks starting at "wallX" and "wallZ", and moves along the x-axis.
/// --
/// Uses "*wall" to figure out if the wall is open, opening, closed or closing,
///      also used "*wall" to figure out what direction opening and closing
///      should be moved.
/// Uses "deltaTime" to figure out how much an opening or closing wall should be
///      change in length, which results in the walls appearing to be animated.

    int actualWallLength;
    int yOffset, x, y;
    float deltaPercent;

    float playerXf, playerYf, playerZf;
    int playerX, playerY, playerZ;


    ///
    /// Clear the wall first
    ///
    if(wall->state == open){

        for(x = 0; x < WALL_LENGTH; x++){

            for(yOffset = 0; yOffset < WALL_HEIGHT; yOffset++){
                if(world[wallX + x][1 + yOffset][wallZ] != MOB_COLOR){

                    world[wallX + x][1 + yOffset][wallZ] = 0;
                }
            }

        }
    }



    ///
    /// Update the wall using deltaTime
    ///
    deltaPercent = (((float)deltaTime * 2) / (float)CHANGE_WALLS_TIME_MS) * 100;
    if(wall->state == opening){
        wall->percentClosed -= deltaPercent;

        if(wall->percentClosed <= 0){
            wall->percentClosed = 0;
            wall->state = open;
        }

    }
    else if(wall->state == closing){
        wall->percentClosed += deltaPercent;

        if(wall->percentClosed > 100){
            wall->percentClosed = 100;
            wall->state = closed;
        }

    }


    ///
    /// Place the wall
    ///
    actualWallLength = (WALL_LENGTH * wall->percentClosed) / 100;
    if(wall->state == closed && deltaTime != 0){
        return;
    }

    for(yOffset = 0; yOffset < WALL_HEIGHT; yOffset++){
        for(x = 0; x < WALL_LENGTH; x++){
            if(deltaTime != 0){
                //x = WALL_LENGTH - 1;
            }


            if(x - 1 < actualWallLength){
                if( (wall->direction == moveWest && wall->state == closing) ){
                    world[wallX + x][1 + yOffset][wallZ] = INNER_WALL_COLOUR;
                }
                else{
                    world[wallX + WALL_LENGTH - x - 1][1 + yOffset][wallZ] = INNER_WALL_COLOUR;
                }


                getViewPosition(&playerXf, &playerYf, &playerZf);
                playerX = (int)playerXf * -1;
                playerY = (int)playerYf * -1;
                playerZ = (int)playerZf * -1;

                for(y = 0; y < WALL_HEIGHT; y++){
                    if(playerX == wallX + x && playerZ == wallZ && y + 1 ==  playerY){
                        setViewPosition(playerXf, playerYf, playerZf - 1);
                    }
                }
            }
            else if(world[wallX + x][1 + yOffset][wallZ] != 0){
                world[wallX + x][1 + yOffset][wallZ] = 0;
            }


        }


    }


}

///
/// PlaceWalls --------------------------------------------
///
void PlaceWalls(int deltaTime){
/// Figures out where each wall starts in worldspace. Calls PlaceHorizontalWall,
///         or PlaceVerticalWall to actaully place the wall blocks into the world.
/// "deltaTime": is used to figure out how much the walls being animated
///              should move
    int x, z;

    for(x = 0; x < WALL_COUNT_X - 1; x++){
        for(z = 0; z < WALL_COUNT_Z - 1; z++){
            if(x == 0){
                PlaceHorizontalWall(pillars[x][z].west, 1, (WALL_LENGTH + 1) * (z + 1), deltaTime);
            }
            if(z == 0){
                PlaceVerticalWall(pillars[x][z].north, (WALL_LENGTH + 1) * (x + 1), 1, deltaTime);
            }

            PlaceHorizontalWall(pillars[x][z].east, (x + 1) * (WALL_LENGTH + 1) + 1, (WALL_LENGTH + 1) * (z + 1), deltaTime);
            PlaceVerticalWall(pillars[x][z].south, (WALL_LENGTH + 1) * (x + 1), (z + 1) * (WALL_LENGTH + 1) + 1, deltaTime);
        }
    }


}


///
/// ChangeWalls -------------------------------------------
///
void ChangeWalls(){
/// Picks a wall to open and a wall to close. Sets these walls states so they
///       get animated opening / closing over time.
/// STEPS:
/// 1. Setup variables, clear variables.
/// 2. Pick a random pillar.
/// 3. For the picked pillar: create a closedWall list, and a openWall list.
/// 4. Pick a random closed wall, from the closedWall list.
/// 5. Pick a random open wall, from the openWall list.
/// 6. Set the selected closed wall to open
/// 7. Set the selected open wall to close

    ///
    /// 1. Setup variables, clear variables
    ///
    int randX, randZ;
    int randomPillar;

    MovementDirection adjOpenWallsDir[4], adjClosedWallsDir[4];
    MovementDirection openWallsDir[4], closedWallsDir[4];

    Wall *adjOpenWalls[4], *adjClosedWalls[4];
    Wall *openWalls[4], *closedWalls[4];

    Wall *adjWallToClose, *adjWallToOpen;
    Wall *wallToClose, *wallToOpen;


    Pillar *currentPillar;


    int openWallCount = 0, closedWallCount = 0;
    int randOpenWall = 0, randClosedWall = 0;
    int i;

    for(i = 0; i < 4; i++){
        openWalls[i] = NULL;
        closedWalls[i] = NULL;
    }





    ///
    /// 2. Pick a random pillar, and make sure it has walls that can be moved
    ///      if we happen to get a pillar that can't move walls, pick a new pillar.
    ///
    i = 0;
    while(1){

        randX = rand() % (WALL_COUNT_X - 1);
        randZ = rand() % (WALL_COUNT_Z - 1);

        if(randX >= WALL_COUNT_X - 1){//TODO: this is experimental
            randX--;
        }

        if(randZ >= WALL_COUNT_Z - 1){
            randZ--;
        }

        currentPillar = &(pillars[randX][randZ]);


        if(Pillar_WallCount(currentPillar) != 0 && Pillar_WallCount(currentPillar) != 4){
            break;
        }
        i++;

        if(i > 1000){
            printf("!-!-! ERROR: was unable to randomly pick a valid pillar (within 1000 random picks) for wall movement... Wallcount(%d)\n", Pillar_WallCount(currentPillar));
            return;
        }
    }





    ///
    /// 3. For the current pillar: create a list of open walls, and a list of
    ///        closed walls.
    ///    Track how the adjacent pillars should be affected for each wall on
    ///          each list (as most walls are attached to two pillars).
    ///
    wallToOpen = NULL;
    wallToClose = NULL;
    adjWallToOpen = NULL;
    adjWallToClose = NULL;


    if(currentPillar->north->state == closed){
        closedWalls[closedWallCount] = currentPillar->north;
        closedWallsDir[closedWallCount] = moveSouth;


        if(randZ > 0){
            adjClosedWalls[closedWallCount] = pillars[randX][randZ - 1].south;
            adjClosedWallsDir[closedWallCount] = moveNorth;
        }
        else{
            adjClosedWalls[closedWallCount] = NULL;
        }

        closedWallCount++;
    }
    else{
        openWalls[openWallCount] = currentPillar->north;
        openWallsDir[openWallCount] = moveNorth;

        if(randZ > 0){
            adjOpenWalls[openWallCount] = pillars[randX][randZ - 1].south;
            adjOpenWallsDir[openWallCount] = moveSouth;
        }
        else{
            adjOpenWalls[openWallCount] = NULL;
        }

        openWallCount++;
    }



    if(currentPillar->south->state == closed){
        closedWalls[closedWallCount] = currentPillar->south;
        closedWallsDir[closedWallCount] = moveNorth;

        if(randZ < WALL_COUNT_Z - 2){
            adjClosedWalls[closedWallCount] = pillars[randX][randZ + 1].north;
            adjClosedWallsDir[closedWallCount] = moveSouth;
        }
        else{
            adjClosedWalls[closedWallCount] = NULL;
        }

        closedWallCount++;
    }
    else{
        openWalls[openWallCount] = currentPillar->south;
        openWallsDir[openWallCount] = moveSouth;

        if(randZ < WALL_COUNT_Z - 2){
            adjOpenWalls[openWallCount] = pillars[randX][randZ + 1].north;
            adjOpenWallsDir[openWallCount] = moveNorth;
        }
        else{
            adjOpenWalls[openWallCount] = NULL;
        }

        openWallCount++;
    }



    if(currentPillar->east->state == closed){
        closedWalls[closedWallCount] = currentPillar->east;
        closedWallsDir[closedWallCount] = moveWest;

        if(randX < WALL_COUNT_X - 2){
            adjClosedWalls[closedWallCount] = pillars[randX + 1][randZ].west;
            adjClosedWallsDir[closedWallCount] = moveEast;
        }
        else{
            adjClosedWalls[closedWallCount] = NULL;
        }

        closedWallCount++;
    }
    else{
        openWalls[openWallCount] = currentPillar->east;
        openWallsDir[openWallCount] = moveEast;

        if(randX < WALL_COUNT_X - 2){
            adjOpenWalls[openWallCount] = pillars[randX + 1][randZ].west;
            adjOpenWallsDir[openWallCount] = moveWest;
        }
        else{
            adjOpenWalls[openWallCount] = NULL;
        }

        openWallCount++;
    }


    if(currentPillar->west->state == closed){
        closedWalls[closedWallCount] = currentPillar->west;
        closedWallsDir[closedWallCount] = moveEast;

        if(randX > 0){
            adjClosedWalls[closedWallCount] = pillars[randX - 1][randZ].east;
            adjClosedWallsDir[closedWallCount] = moveWest;
        }
        else{
            adjClosedWalls[closedWallCount] = NULL;
        }

        closedWallCount++;
    }
    else{
        openWalls[openWallCount] = currentPillar->west;
        openWallsDir[openWallCount] = moveWest;

        if(randX > 0){
            adjOpenWalls[openWallCount] = pillars[randX - 1][randZ].east;
            adjOpenWallsDir[openWallCount] = moveEast;
        }
        else{
            adjOpenWalls[openWallCount] = NULL;
        }

        openWallCount++;
    }



    ///
    /// 4. Pick a random closed wall, from the closedWall list
    ///
    if(closedWallCount > 0){
        randClosedWall = rand() % closedWallCount;
    }




    ///
    /// 5. Pick a random open wall from the openWall list
    ///
    if(openWallCount > 0){
        randOpenWall = rand() % openWallCount;
    }



    ///
    /// 6. Set the selected closed wall to open
    ///
    wallToOpen = closedWalls[randClosedWall];
    wallToOpen->direction = closedWallsDir[randClosedWall];
    wallToOpen->state = opening;


    adjWallToOpen = adjClosedWalls[randClosedWall];
    if(adjWallToOpen != NULL){
        adjWallToOpen->state = opening;
        adjWallToOpen->direction = adjClosedWallsDir[randClosedWall];
    }


    ///
    /// 7. Set the selected open wall to close
    ///
    wallToClose = openWalls[randOpenWall];
    wallToClose->direction = closedWallsDir[randClosedWall];
    wallToClose->state = closing;


    adjWallToClose = adjOpenWalls[randOpenWall];
    if(adjWallToClose != NULL){
        adjWallToClose->state = closing;
        adjWallToClose->direction = adjOpenWallsDir[randOpenWall];
    }

}



///
/// SetupWall ---------------------------------------------
///
void SetupWall(Wall **targetWall, Wall **adjacentWall, GenerationInfo *genInfo){
/// Mallocs memory for a wall. Then randomly decides if that wall should be
///         created opened or closed. Tracks whether a wall was created opened
///         or closed using the genInfo struct passed in.

    float currentSpawnPercentage;

    ///
    /// Error checking
    ///
    if(targetWall == NULL){
        printf("!-!-! ERROR: pointer to targetWall-pointer was null\n");
        return;
    }

    ///
    /// Make sure we're not going to spawn a wall where a wall exists already
    ///
    if(*targetWall != NULL){
        return;
    }


    ///
    /// Malloc the new wall
    ///
    Wall *newWall = (Wall*)malloc(sizeof(Wall));
    newWall->direction = notMoving;
    genInfo->creationAttempts++;


    ///
    /// Randomly decide if the wall should be open or closed
    ///
    if(PercentChance(genInfo->spawnChance + genInfo->spawnChanceModifier) && genInfo->wallsCreated < MAX_WALL_COUNT){
        newWall->percentClosed = 100;
        newWall->state = closed;
        genInfo->wallsCreated++;
    }
    else{
        newWall->percentClosed = 0;
        newWall->state = open;
    }

    ///
    /// Use the spawnChanceModifier to push the spawnChance
    ///         towards spawning the target number of walls
    currentSpawnPercentage = (100 * ((float)genInfo->wallsCreated / (float)genInfo->creationAttempts));
    genInfo->spawnChanceModifier = genInfo->spawnChance - currentSpawnPercentage;




    ///
    /// Assign the new wall
    ///
    *targetWall = newWall;

    if(adjacentWall != NULL){
        *adjacentWall = newWall;
    }
}





////////////////////////////////////////////////////////////////////////////////
/////
///// Utility Functions 
/////



///
/// PercentChance -----------------------------------------
///
int PercentChance(float percent){
/// Generates a random boolean in the form of an int with the value 0 or 1.
/// Parameter "percent": is the chance out of 100 that this function will return 1.
/// Setting "percent" to 25, means there should be a 25% chance this function will return 1.

    int rnd;
    rnd = rand() % 100 + 1;

    if(percent > rnd){
        return 1;
    }
    return 0;
}



///
/// WalkablePiece -----------------------------------------
///
int WalkablePiece(int x, int y, int z){
/// Given determines if a block is empty or not.

    int count = 0, height;

    for(height = 0; height < PLAYER_HEIGHT; height++){
        if(world[x][y + height][z] != EMPTY_PIECE){
            count++;
        }
    }

    return count == 0;
}



///
/// Clamp -------------------------------------------------
///
float Clamp(float value, float minVal, float maxVal){
/// Prevents a value from being bigger than a maxVal, and smaller than a minVal

    if(value < minVal){
        return minVal;
    }
    else if(value > maxVal){
        return maxVal;
    }

    return value;
}



///
/// DeltaGravity ------------------------------------------
///
float DeltaGravity(int lastCollisionTime){
/// Given a previous GLUT_ELAPSED_TIME, this function will figure out how much
///       time has passed, and then figure out how far gravity pulled an object
///       down, in the amount of time that has passed.

    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    int deltaTime = currentTime - lastCollisionTime;
    return GRAVITY_RATE * deltaTime / 1000;
}



///
/// PrintWallGeneration -----------------------------------
///
void PrintWallGeneration(){
/// Prints an ascii version of what walls are closed, and what walls are open.

    int x, z;

    ///
    /// This is for debugging purposes only
    ///
    printf(" < PRINT WALL GENERATION > \n");
    for(z = 0; z < WALL_COUNT_Z - 1; z++){

        for(x = 0; x < WALL_COUNT_X - 1; x++){
            putchar(' ');


            if(pillars[x][z].north->state == closed){
                putchar('|');
            }
            else{
                putchar(' ');
            }
            putchar(' ');
        }
        putchar('\n');

        for(x = 0; x < WALL_COUNT_X - 1; x++){
            if(pillars[x][z].west->state == closed){
                putchar('-');
            }
            else{
                putchar(' ');
            }
            putchar('+');
            if(pillars[x][z].east->state == closed){
                putchar('-');
            }
            else{
                putchar(' ');
            }
        }
        putchar('\n');

        for(x = 0; x < WALL_COUNT_X - 1; x++){
            putchar(' ');
            if(pillars[x][z].south->state == closed){
                putchar('|');
            }
            else{
                putchar(' ');
            }
            putchar(' ');
        }
        putchar('\n');
    }

}



///
/// Pillar_WallCount --------------------------------------
///
int Pillar_WallCount(Pillar *pillar){
/// Counts the number of walls on a given pillar that are closed.

    int count = 0;

    if(pillar->north != NULL && pillar->north->state == closed){
        count++;
    }

    if(pillar->east != NULL && pillar->east->state == closed){
        count++;
    }

    if(pillar->south != NULL && pillar->south->state == closed){
        count++;
    }

    if(pillar->west != NULL && pillar->west->state == closed){
        count++;
    }


    return count;
}



///
/// CountAllWalls -----------------------------------------
///
int CountAllWalls(){
/// Goes through each pillar, and counts how many closed walls there are currently
///      closed, or are currently closing on the map.

    Pillar *currentPillar;
    int count, x, z;

    count = 0;

    for(x = 0; x < WALL_COUNT_X - 1; x++){
        for(z = 0; z < WALL_COUNT_Z - 1; z++){
            currentPillar = &(pillars[x][z]);

            if(x == 0 && (currentPillar->west->state == closed || currentPillar->west->state == closing)){
                count++;
            }

            if(z == 0 &&  (currentPillar->north->state == closed || currentPillar->north->state == closing)){
                count++;
            }

            if( (currentPillar->east->state == closed || currentPillar->east->state == closing)){
                count++;
            }

            if( (currentPillar->south->state == closed || currentPillar->south->state == closing)){
                count++;
            }
        }
    }

    return count;
}


void Shoot(){
    float playerX, playerY, playerZ;
    float rotX, rotY, rotZ;

    int projectileInsert = -1;
    int i;

    //float mobX, mobZ;
    float moveX, moveY, moveZ;
    int mobID;


    for(i = 0; i < MAX_PROJECTILES; i++){
        if(projectiles[i].enabled == 0){
            projectileInsert = i;
            break;
        }
    }

    if(projectileInsert == -1){
        //printf("Cant shoot now\n");
        return;
    }

    //printf("Shoot now\n");
    



    getViewPosition(&playerX, &playerY, &playerZ);
    getViewOrientation(&rotX, &rotY, &rotZ);
    


    projectiles[projectileInsert].enabled = 1;
    projectiles[projectileInsert].timeEnabled = 0;
    mobID = projectiles[projectileInsert].mobID;

    moveX = cos((rotY - 90) * 0.0174533f);
    moveY = -sin((rotX) * 0.0174533f);
    moveZ = sin((rotY - 90) * 0.0174533f);
    projectiles[projectileInsert].moveX = moveX * PROJECTILE_MOVE_SPEED;
    projectiles[projectileInsert].moveY = moveY * PROJECTILE_MOVE_SPEED;
    projectiles[projectileInsert].moveZ = moveZ * PROJECTILE_MOVE_SPEED; 

    projectiles[projectileInsert].x = playerX + 0.5f - moveX * PROJECTILE_SPAWN_DISTANCE;
    projectiles[projectileInsert].y = playerY + 0.5f - moveY * PROJECTILE_SPAWN_DISTANCE;
    projectiles[projectileInsert].z = playerZ + 0.5f - moveZ * PROJECTILE_SPAWN_DISTANCE;

    setMobPosition(mobID, projectiles[projectileInsert].x * -1, projectiles[projectileInsert].y * -1, projectiles[projectileInsert].z * -1, 0);
    showMob(mobID);

    projectileInsert++;
}
