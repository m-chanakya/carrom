#include <iostream>
#include <stdlib.h>
#include <cmath>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
using namespace std;

#define PI 3.141592653589
#define EPS 0.01
#define DEG2RAD(deg) (deg * PI / 180)
#define RED {1, 0, 0}
#define GREEN {0, 1, 0}
#define BLUE {0, 1, 1}
#define BLACK {0, 0, 0}

void printtext(double x, double y, string String)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(1,0,0);
    int w = glutGet(GLUT_SCREEN_WIDTH);
    int h = glutGet(GLUT_SCREEN_HEIGHT);
    int WindowWidth = w * 2 / 3; int WindowHeight = h * 2 / 3;
    double scalex = 5.9;
    double scaley = 3.3;
    x+=scalex; y+=scaley;
    x*=1.0*WindowWidth/11.8;
    y*=1.0*WindowHeight/6.6;
    glOrtho(0, WindowWidth, 0, WindowHeight, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW); glPushMatrix();
    glLoadIdentity(); glPushAttrib(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    glRasterPos2i(x,y);
    for (int i=0; i<String.size(); i++)
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, String[i]);
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

class ScoreBoard
{
private:
    char scoreString[10];
    void getScore()
    {
        int temp = score, i=0;
        int beg = 0;
        if (temp < 0)
        {
            temp *= -1;
            scoreString[beg++] = '-';
            i++;
        }
        while (temp!=0)
        {
            int digit = temp % 10;
            temp /= 10;
            scoreString[i++] = digit + '0';
        }
        scoreString[i] = '\0';
        int end = i-1;
        while(beg<end)
        {
            int t = scoreString[beg];
            scoreString[beg] = scoreString[end];
            scoreString[end] = t;
            beg++; end--;
        }
    }
public:
    int score;
    ScoreBoard()
    {
        score = 30;
    }
    void draw(double x, double y, double a, double b)
    {
        getScore();
        printtext(x, y, "SCORE");
        printtext(a, b, scoreString);
    }
};

ScoreBoard score, score_copy1, score_copy2, score_temp;

class Vector
{
public:
    double x, y;
    Vector()
    {
        x = 0;
        y = 0;
    }
    Vector (double _x, double _y)
    {
        x = _x;
        y = _y;
    }
    double dot(Vector B)
    {
        return x*B.x + y*B.y;
    }
};

void drawCircle(Vector pos, double z, double radius, int mode)
{
    glTranslatef(pos.x, pos.y, z);
    if(mode==0)glBegin(GL_TRIANGLE_FAN);
    else if(mode==1)glBegin(GL_TRIANGLE_STRIP);
    for(double i=0; i<360; i+=1)
        glVertex2f(radius * cos(DEG2RAD(i)), radius * sin(DEG2RAD(i)));
    glEnd();
    glTranslatef(-pos.x, -pos.y, -z);
}

void drawLine(Vector start, Vector end, double z)
{
    glTranslatef(0, 0, z);
    glBegin(GL_LINES);
    glVertex2f(start.x, start.y);
    glVertex2f(end.x,  end.y);
    glEnd();
    glTranslatef(0, 0, -z);
}

void drawBox(double len)
{
    glBegin(GL_QUADS);
    glVertex2f(-len / 2, -len / 2);
    glVertex2f(len / 2, -len / 2);
    glVertex2f(len / 2, len / 2);
    glVertex2f(-len / 2, len / 2);
    glEnd();
}

void drawPolygon(double len, double rad)
{
    glBegin(GL_POLYGON);
    double tx,ty,tz;
    tx = len; ty = len+rad;
    glVertex2f(tx, ty);
    glVertex2f(-tx, ty);
    
    glVertex2f(-ty, tx);
    glVertex2f(-ty, -tx);
    
    glVertex2f(-tx, -ty);
    glVertex2f(tx, -ty);
    
    glVertex2f(ty, -tx);
    glVertex2f(ty, tx);
    glEnd();
}

class Coin
{
public:
    Vector pos, vel, acc;
    static double cor;
    static double colors[5][3];
    static double mass[5];
    static double radius[5];
    int type;
    bool POCKET;
    Coin () {}
    Coin (Vector _pos, int _type)
    {
        pos = _pos;
        type = _type;
        POCKET = false;
    }
    void draw()
    {
        if (POCKET)
            return;
        int r = colors[type][0];
        int g = colors[type][1];
        int b = colors[type][2];
        glColor3f(r, g, b);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawCircle(pos, 0.0f, radius[type], 0);
    }
    void move(double friction)
    {
        if (POCKET)
            return;
        double den = sqrt(vel.x*vel.x + vel.y*vel.y);
        if (den <= EPS)
            acc = vel = Vector(0, 0);
        else if (den)
            acc = Vector(friction*vel.x/den, friction*vel.y/den);
        vel = Vector (vel.x + acc.x, vel.y + acc.y);
        pos = Vector (pos.x + vel.x,
                      pos.y + vel.y);
    }
};

double Coin::cor = 0.5;
double Coin::colors[5][3] = {GREEN, BLACK, RED, BLUE, RED};
double Coin::mass[5] = {10, 10, 10, 20, 20};
double Coin::radius[5] = {0.1, 0.1, 0.1, 0.15, 0.15};

class Board
{
private:
    void updateWallCollision(Coin &c)
    {
        if (c.POCKET) return;
        double rad = Coin::radius[c.type];
        if( (c.pos.x + rad > len/2 && c.vel.x > 0)  || (c.pos.x - rad < -len/2 && c.vel.x < 0) )
            c.vel.x *= -cor;
        if( (c.pos.y + rad > len/2 && c.vel.y > 0) || (c.pos.y - rad < -len/2 && c.vel.y < 0) )
            c.vel.y *= -cor;
    }
    void updateCoinCollision(Coin &c1, Coin &c2, int i, int j)
    {
        if (c1.POCKET || c2.POCKET) return;
        double a = c1.pos.x - c2.pos.x;
        double b = c1.pos.y - c2.pos.y;
        double den = sqrt(a*a + b*b);
        if(checkCoinCollision(c1, c2))
        {
            Vector u1, u2;
            double v1, v2;
            double Sin,Cos; Sin = b/den; Cos = a/den;
            u1 = Vector(c1.vel.x*Cos + c1.vel.y*Sin, c1.vel.x*(-Sin) + c1.vel.y*Cos);
            u2 = Vector(c2.vel.x*Cos + c2.vel.y*Sin, c2.vel.x*(-Sin) + c2.vel.y*Cos);
            double m1, m2; m1 = c1.mass[c1.type]; m2 = Coin::mass[c2.type];
            double e = Coin::cor;
            v1 = (m1*u1.x + m2*u2.x + m2*e*(u2.x-u1.x))/(m1+m2);
            v2 = (m1*u1.x + m2*u2.x + m1*e*(u1.x-u2.x))/(m1+m2);
            u1.x = v1;
            u2.x = v2;
            c1.vel.x = u1.x*Cos + u1.y*(-Sin);
            c1.vel.y = u1.x*Sin + u1.y*Cos;
            c2.vel.x = u2.x*Cos + u2.y*(-Sin);
            c2.vel.y = u2.x*Sin + u2.y*Cos;
            
            den = c1.radius[c1.type] + c2.radius[c2.type] - den;
            den /= 2; den+=EPS;
            c1.pos.x += den*Cos; c1.pos.y += den*Sin;
            c2.pos.x -= den*Cos; c2.pos.y -= den*Sin;
        }
    }
    
public:
    double len;
    double width;
    double friction;
    double cor;
    Coin carroms[8], queen, striker;
    double striker_delta;
    
    bool END, PAUSE, MOVE;
    
    double pointer_length;
    int pointer_angle;
    int pointer_angle_delta;
    
    int power;
    int power_delta;
    double power_meter_len;
    double power_meter_width;
    double power_meter_x;
    double power_meter_y;
    
    double hole_size;
    double hole_pos;
    double inner_hole;
    Vector hole[4];
    double baseline;
    
    int player_coin;
    int no_of_coins;
    
    Board(double _len)
    {
        len = _len;
        width = 0.5;
        friction = -0.0007;
        cor = 0.95;
        hole_size = 0.2;
        inner_hole = 0.1;
        hole_pos = len/2 - hole_size;
        hole[0] = Vector(hole_pos, hole_pos);
        hole[1] = Vector(-hole_pos, hole_pos);
        hole[2] = Vector(-hole_pos, -hole_pos);
        hole[3] = Vector(hole_pos, -hole_pos);
        END = PAUSE = MOVE = false;
        no_of_coins = 0;
        
        int j=0;
        double init = 2*Coin::radius[0] + 0.09;
        for(double i=22.5; j<4; i+=90, j++)
            carroms[j] = Coin(Vector(init * cos(DEG2RAD(i)), init * sin(DEG2RAD(i))), 0);
        
        for(double i=-22.5; j<8; i+=90, j++)
            carroms[j] = Coin(Vector(init * cos(DEG2RAD(i)), init * sin(DEG2RAD(i))), 1);
        queen = Coin(Vector(0,0), 2);
        
        baseline = len/2-1.2*width-inner_hole;
        
        striker_delta = 0.05;
        
        pointer_length = Coin::radius[3] + 0.4;
        pointer_angle = 90;
        pointer_angle_delta = 3;
        
        power = 0;
        power_delta = 5;
        power_meter_x = len/2 + 2*width;
        power_meter_y = -len/2;
        power_meter_len = len/2;
        power_meter_width = width*3/4;
        
        striker = Coin(Vector(0, -baseline), 3);
    }
    
    void initMove();
    
    bool checkCoinCollision(Coin c1, Coin c2)
    {
        double a = c1.pos.x - c2.pos.x;
        double b = c1.pos.y - c2.pos.y;
        double den = sqrt(a*a + b*b);
        if(den <= Coin::radius[c1.type] + Coin::radius[c2.type] + EPS) return true;
        return false;
    }
    
    void checkCollisions()
    {
        for(int i=0; i<8; i++)
            updateWallCollision(carroms[i]);
        updateWallCollision(queen);
        updateWallCollision(striker);
        for(int i=0; i<8; i++)
        {
            for(int j=0; j<i; j++)
                updateCoinCollision(carroms[i],carroms[j],i,j);
            updateCoinCollision(carroms[i],queen,i,8);
            updateCoinCollision(carroms[i],striker,i,9);
        }
        updateCoinCollision(queen,striker,8,9);
    }
    
    void updateCoinPocket(Coin &c, Vector hole)
    {
        if (c.POCKET) return;
        double a = c.pos.x-hole.x;
        double b = c.pos.y-hole.y;
        double dist = sqrt(a*a + b*b);
        if (dist <= EPS*10)
        {
            if (no_of_coins == 0 && c.type != 2 && c.type != 3)
            {
                score.score += 10;
                player_coin = c.type;
                no_of_coins++;
            }
            else if (c.type == player_coin)
            {
                score.score += 10;
                no_of_coins++;
            }
            else if (c.type == 2)
                score.score += 50;
            else
                score.score -= 5;
            c.POCKET = true;
        }
    }
    
    void checkCoinPocket()
    {
        for(int j=0; j<4; j++)
        {
            for(int i=0; i<8; i++)
                updateCoinPocket(carroms[i], hole[j]);
            updateCoinPocket(queen, hole[j]);
            updateCoinPocket(striker, hole[j]);
        }
        int flag = 0;
        if (!queen.POCKET) flag=1;
        else
        {
            if (no_of_coins == 0)
                flag=1;
            else
            {
                for(int i=0; i<8; i++)
                    if(carroms[i].type == player_coin && !carroms[i].POCKET) flag=1;
            }
        }
        if(flag == 0)
            END = true;
    }
    
    bool checkCoinMove(Coin c)
    {
        if (c.POCKET) return false;
        else if (c.vel.x != 0 || c.vel.y != 0) return true;
        else return false;
    }
    
    void draw()
    {
        if (END)
        {
            printtext(-0.8, 0.6, "GAME OVER");
            score.draw(-0.7, 0, 0.55, 0);
            printtext(-1.2, -0.6, "PLAY AGAIN? (y/n)");
            return;
        }
        glTranslatef(0.0f, 0.0f, -8.0f);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor3f(0.65f, 0.5f, 0.39f);
        drawBox(len+width);
        
        glColor3f(1.0f, 1.0f, 1.0f);
        //drawBox(len);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawPolygon(hole_pos,hole_size);
        
        double len2 = len/2-1.2*width-inner_hole;
        glColor3f(1.0f, 0.0f, 0.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawLine(Vector(len2, len2+inner_hole), Vector(-len2, len2+inner_hole),0);
        drawLine(Vector(len2, -(len2+inner_hole)), Vector(-len2, -(len2+inner_hole)),0);
        drawLine(Vector(len2+inner_hole, len2), Vector(len2+inner_hole, -len2),0);
        drawLine(Vector(-(len2+inner_hole), len2), Vector(-(len2+inner_hole), -len2),0);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawCircle(Vector(len2, len2), 0.0f, inner_hole, 1);
        drawCircle(Vector(-len2, len2), 0.0f, inner_hole, 1);
        drawCircle(Vector(-len2, -len2), 0.0f, inner_hole, 1);
        drawCircle(Vector(len2, -len2), 0.0f, inner_hole, 1);
        
        drawCircle(Vector(0, 0), 0.0f, 0.5, 1);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor3f(0.0f, 0.0f, 0.0f);
        for(int i=0; i<4; i++)
            drawCircle(hole[i], 0.0f, hole_size, 0);
        
        for(int i=0; i<8; i++)
            carroms[i].draw();
        queen.draw();
        striker.draw();
        
        if(!MOVE && !END)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glColor3f(1.0f, 0.0f, 0.0f);
            Vector pointer(striker.pos.x + pointer_length * cos(DEG2RAD(pointer_angle)),
                           striker.pos.y + pointer_length * sin(DEG2RAD(pointer_angle))
                           );
            drawLine(striker.pos, pointer, 0);
            
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            for(int i=0; i<100; i++)
            {
                double r,g;
                r = (255.0*i)/25500;
                g = (255.0*(100-i))/25500;
                glColor3f(r, g, 0.0f);
                glBegin(GL_QUAD_STRIP);
                glVertex2f(power_meter_x, power_meter_y);
                glVertex2f(power_meter_x + power_meter_width, power_meter_y);
                power_meter_y += power_meter_len/100;
                glVertex2f(power_meter_x, power_meter_y);
                glVertex2f(power_meter_x + power_meter_width, power_meter_y);
                glEnd();
            }
            power_meter_y = -len/2;
            glColor3f(1,1,1);
            glBegin(GL_LINES);
            glVertex2f(power_meter_x - 0.05, power_meter_y + power * power_meter_len/60);
            glVertex2f(power_meter_x + power_meter_width + 0.05, power_meter_y + power * power_meter_len/60);
            glEnd();
        }
        if(PAUSE) printtext(-0.6, 2.8, "PAUSED");
        if(no_of_coins == 1)
        {
            if (player_coin == 1)
                printtext(-1.1, -3, "You are BLACK");
            else
                printtext(-1.1, -3, "You are GREEN");
        }
    }
};

Board board(4.5), board_copy1(4.5), board_copy2(4.5), board_temp(4.5);

void Board::initMove()
{
    if (striker.type == 4) MOVE = false;
    double velocity = (power + 40) * 0.0013;
    striker.vel = Vector(velocity * cos(DEG2RAD(pointer_angle)), velocity * sin(DEG2RAD(pointer_angle)));
    double factor = friction / Coin::mass[striker.type];
    striker.acc = Vector(factor * cos(DEG2RAD(pointer_angle)), factor * sin(DEG2RAD(pointer_angle)));
}


void drawscene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
    
    board.draw();
    if(!board.END) score.draw(3.32, 1.5, 3.67, 1.15);
    
    glPopMatrix();
    glutSwapBuffers();
}

int replay=1;
int DELAY = 10;

void update_board(int value)
{
    if (replay==0)
    {
        board_temp = board;
        board = board_copy2;
        score_temp = score;
        score = score_copy2;
        replay = 2;
        DELAY = 42;
    }
    if(board.MOVE && !board.END && !board.PAUSE)
    {
        board.checkCollisions();
        board.striker.move(board.friction);
        board.queen.move(board.friction);
        for(int i=0; i<8; i++)
            board.carroms[i].move(board.friction);
        board.checkCoinPocket();
        int flag = 0;
        if (board.checkCoinMove(board.striker) || board.checkCoinMove(board.queen)) flag=1;
        else
        {
            for(int i=0; i<8; i++)
                if (board.checkCoinMove(board.carroms[i]))
                    flag = 1;
        }
        if (flag==0)
        {
            board.MOVE = false;
            if (replay==2)
            {
                board = board_temp;
                score = score_temp;
                replay = 1;
                DELAY = 10;
            }
            board.striker = Coin(Vector(0, -board.baseline), 3);
            
            board.initMove();
            board_copy2 = board_copy1;
            board_copy1 = board;
        }
    }
    else
    {
        int flag=0;
        if (board.checkCoinCollision(board.queen, board.striker)) flag=1;
        else
        {
            for(int i=0; i<8; i++)
                if (board.checkCoinCollision(board.carroms[i], board.striker)) flag=1;
        }
        double len2 = board.len/2-1.2*board.width-board.inner_hole;
        if (board.striker.pos.x > len2 || board.striker.pos.x < -len2 || board.pointer_angle < 0 || board.pointer_angle > 180 || flag) board.striker.type = 4;
        else board.striker.type = 3;
    }
    glutTimerFunc(DELAY, update_board, 0);
}

void update_score(int value)
{
    if(!board.END && !board.PAUSE) score.score--;
    glutTimerFunc(1000, update_score, 0);
}

void handleResize(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)w / (float)h, 0.1f, 200.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void handleKeypress1(unsigned char key, int x, int y) 
{
    if (key == 27)
        exit(0);
    if(!board.MOVE && !board.END && !board.PAUSE)
    {
        switch(key)
        {
            case 'a': board.pointer_angle += board.pointer_angle_delta; board.pointer_angle %= 360; break;
            case 'c': board.pointer_angle -= board.pointer_angle_delta; if(board.pointer_angle < 0) board.pointer_angle += 360; break;
            case 'p': board.PAUSE = true; break;
            case 'v': replay = 0; break;
            case 32:  board.MOVE = true;
                board.initMove();
                board_copy2 = board_copy1;
                board_copy1 = board;
                score_copy2 = score_copy1;
                score_copy1 = score;
                break;
        }
    }
    else if(key=='r') board.PAUSE = false;
    else if(board.END)
    {
        switch(key)
        {
            case 'y': board = Board(4.5); break;
            case 'n': exit(0); break;
        }
    }
}

void handleKeypress2(int key, int x, int y)
{
    if(!board.MOVE && !board.END && !board.PAUSE)
    {
        switch(key)
        {
            case GLUT_KEY_LEFT: board.striker.pos.x -= board.striker_delta; break;
            case GLUT_KEY_RIGHT: board.striker.pos.x += board.striker_delta; break;
            case GLUT_KEY_UP: board.power += board.power_delta; if(board.power > 60) board.power = 60; break;
            case GLUT_KEY_DOWN: board.power -= board.power_delta; if (board.power < 0) board.power = 0; break;
        }
    }
}

void mouseClickConvert(int x, int y, GLdouble &worldX, GLdouble &worldY, GLdouble &worldZ)
{
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
        
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );
    
    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    winZ = 0;
    
    gluUnProject(winX, winY, winZ, modelview, projection, viewport, &worldX, &worldY, &worldZ);
    worldX *= 78.85; worldY *= 78.85; worldZ *= 78.85;
}

void handleMousetrack(int x, int y)
{
    if (board.MOVE || board.END || board.PAUSE) return;
    GLdouble X, Y, Z;
    mouseClickConvert(x, y, X, Y, Z);
    double realX = X, realY = Y;
    double range = board.len/2-1.2*board.width-board.inner_hole;
    double delta = range/6;
    X -= board.striker.pos.x;
    Y -= board.striker.pos.y;
    if (Y>=0 && realY<=board.len/2  && realX <= board.len/2 && realX >= -board.len/2)
    {
        board.power = sqrt(X*X + Y*Y)/delta * board.power_delta;
        if (board.power >= 60) board.power = 60;
        board.pointer_angle = acos(X/sqrt(X*X + Y*Y))/PI * 180;
    }
}

void handleMousedrag(int x, int y)
{
    if (board.MOVE || board.END || board.PAUSE) return;
    GLdouble X, Y, Z;
    mouseClickConvert(x, y, X, Y, Z);
    double range = board.len/2-1.2*board.width-board.inner_hole;
    double rad = Coin::radius[board.striker.type];
    if (Y >= -board.len/2 && Y <= -board.baseline)
    {
        if (X <= -range) X = -range;
        else if (X >= range) X = range;
        board.striker.pos = Vector(X, -board.baseline);
    }
}

void handleMouseclick(int button, int state, int x, int y) 
{
    if (board.MOVE || board.END) return;
    GLdouble X, Y, Z;
    mouseClickConvert(x, y, X, Y, Z);
    double rad = Coin::radius[board.striker.type];
    if (state == GLUT_DOWN)
    {
        if (button == GLUT_LEFT_BUTTON)
        {
            double len = board.len/2;
            if (Y < -len || Y > len || X < -len || X > len)
                board.PAUSE = true;
            else
            {
                board.PAUSE = false;
                if (Y > -board.baseline && Y <= board.len/2
                    &&  X <= board.len/2 && X >= -board.len/2)
                {
                    board.MOVE = true;
                    board.initMove();
                    board_copy2 = board_copy1;
                    board_copy1 = board;
                    score_copy2 = score_copy1;
                    score_copy1 = score;
                }
            }
        }
    }
}

void initRendering()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,  GL_NICEST);
    glLineWidth(2.0);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	int w = glutGet(GLUT_SCREEN_WIDTH);
	int h = glutGet(GLUT_SCREEN_HEIGHT);
	int windowWidth = w * 2/3;
	int windowHeight = h * 2/3;
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition((w - windowWidth) / 2, (h - windowHeight) / 2);

	//Setup the window
	glutCreateWindow("Carrom");
	initRendering();

	//Register callbacks
	glutDisplayFunc(drawscene);
	glutIdleFunc(drawscene);
	glutKeyboardFunc(handleKeypress1);
	glutSpecialFunc(handleKeypress2);
	glutMouseFunc(handleMouseclick);
	glutReshapeFunc(handleResize);
	glutTimerFunc(10, update_board, 0);
    glutTimerFunc(1000, update_score, 0);
    glutPassiveMotionFunc(handleMousetrack);
    glutMotionFunc(handleMousedrag);
	glutMainLoop();
	return 0;
}
