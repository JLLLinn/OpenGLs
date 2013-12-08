
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <string>
#include "gfx/vec3.h"
#include "i.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include "sHelper.h"
using namespace gfx;
int FPS = 60;		// frames per second
Teapot teapot("I3f.txt");

GLuint vertexS, fragS, shadeP;
bool skeleton = true;
int rot = 1;

GLuint background,envN, texN, mode;
float eyeY = -0.3f, eyeZ = 2.5f;
string envPic[] = {"img/0e.jpg","img/1e.bmp","img/2e.bmp","img/3e.bmp"};
string texPic[] = {"img/0t.bmp","img/1t.bmp","img/2t.bmp","img/3t.bmp"};
string masPic[] = {"img/0m.bmp","img/1m.bmp","img/2m.bmp","img/3m.bmp"};
char* shader[3] = {"shader/mix.fs","shader/environment.fs","shader/texture.fs"};
char* vertexShader = "shader/vs.vs";

/* Camera interpolation variables */
const float BECON[][3] = {
	{0.0, 0.0, 3.0},
	{-16.0, 20.0, -20.0},
	{16.0, -10.0, 10.0},
	{0.0, 0.0, 3.0},
};


float cam_x = 0.f, cam_y = 3.f, cam_z = 5.f; // initial camera position
float t = 0.0;

// Cubic Bezier Curve
float interpolate(int c) {
	return (1-t)*(1-t)*(1-t) * BECON[0][c] + 
		3 * t * (1-t)*(1-t)  * BECON[1][c] + 
		3 * (1 - t) * t*t    * BECON[2][c] + 
		t * t * t            * BECON[3][c];
}

void moveCamera() {
	t += 0.0015;
	double intpart;
	t = modf(t, &intpart);
	cam_x = interpolate(0);
	cam_y = interpolate(1);
	cam_z = interpolate(2);
}

char * loaderShader(char *fn) {
	FILE *fp;
	char *content = NULL;

	int count=0;

	if (fn != NULL) {
		fp = fopen(fn,"rt");

		if (fp != NULL) {
      
      fseek(fp, 0, SEEK_END);
      count = ftell(fp);
      rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}

void initShaders(char * fsFilename) {
	char *vs = NULL,*fs = NULL;
	
	vertexS = glCreateShader(GL_VERTEX_SHADER);
	fragS = glCreateShader(GL_FRAGMENT_SHADER);	

	vs = loaderShader(vertexShader);
	fs = loaderShader( fsFilename );

	const char * vv = vs;
	const char * ff = fs;

	glShaderSource(vertexS, 1, &vv,NULL);
	glShaderSource(fragS, 1, &ff,NULL);

	free(vs);
	free(fs);

	glCompileShader(vertexS);
	glCompileShader(fragS);

	shadeP = glCreateProgram();
	glAttachShader(shadeP,vertexS);
	glAttachShader(shadeP,fragS);

	glLinkProgram(shadeP);
}

void init(void) 
{
	initSub();
	envN = 1;
	texN = 1;
	mode = 0;
    cout<<"Hello Welcome to my I demo!(Jiaxin Lin)\n"\
		<<"\"z\" to start rotating\n"\
		<<"\"f\" to switch: \n	texture mapping | environment mapping | environment and texture mix\n"\
		<<"\"e\" to change an environment picture\n"\
		
		<<"\"t\" to change texture picture\n"\
		<<"\"s\" to switch between skeleton and the rendered teapot\n"\
		<<"\"up, down\" to let you eye go up and down\n"\
		<<"\"left, right\" to zoom in and out\n"\
		<<"\"d\" to subdivide. All output goes to example.txt\n";

    initShaders(shader[mode]);

    teapot.computeNormals();
	teapot.setEnv(envPic[envN]);
	teapot.setTexture(texPic[texN]);
	teapot.setMixture(masPic[texN]);
	background = teapot.loadTexture(envPic[envN].c_str());

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat white[] = {1,1,1,1.0};
    GLfloat lpos[] = {3.0,3.0,3.0,3.0};

    glLightfv(GL_LIGHT0, GL_POSITION, lpos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, white);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white);

    glClearColor (0.5, 0.5, 0.5, 0.0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);
}

void display(void)
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f (1.0, 1.0, 1.0);

    /* clear the matrix */



//make background
	 glMatrixMode( GL_PROJECTION);
	 glPushMatrix();
	 glOrtho(0, 1, 0, 1, 0, 1);

	 glMatrixMode( GL_MODELVIEW);
	 glPushMatrix();
	 glLoadIdentity();
	
	// No depth buffer writes for background.
	 glDepthMask( false );
	 glActiveTexture(GL_TEXTURE0);
	 glBindTexture(  GL_TEXTURE_2D, background );
	 glBegin(  GL_QUADS ); {
	   glTexCoord2f( 0.0f, 0.0f );
	   glVertex2f( 0, 0 );
	   glTexCoord2f( 0.0f, 1.0f );
	   glVertex2f( 0, 1.0f );
	   glTexCoord2f( 1.0f, 1.0f );
	   glVertex2f( 1.0f, 1.0f );
	   glTexCoord2f( 1.0f, 0.0f );
	   glVertex2f( 1.0f, 0 );
	}  glEnd();

	 glDepthMask( true );

	 glPopMatrix();
	 glMatrixMode( GL_PROJECTION);
	 glPopMatrix();
	 glMatrixMode( GL_MODELVIEW);

//end background
    
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();

    static int time = 0;
    time = glutGet(GLUT_ELAPSED_TIME);


	// setup look at transformation so that 
	// eye is at : (0,eyeY,eyeZ)
	// look at center is at : (0,1,0)
	// up direction is +y axis
	switch (rot){
	case 0:
		gluLookAt(0.f,eyeY,eyeZ,
			0.f,0.f,0.f,
			0.f,1.f,0.f);
		//glRotatef((int)(time/50) % 360 , 0, 1, 0);
		
		break;
	case 1:
		gluLookAt(cam_x, cam_y, cam_z,
			  0.f,0.f,0.f,
			  0.f,1.f,0.f);
		break;

	default:
		gluLookAt(cam_x, cam_y, cam_z,
			  0.f,0.f,0.f,
			  0.f,1.f,0.f);
		break;
	}

	GLfloat amb[] = {.1,0.1,0.1,1.0};
	GLfloat diff[] = {1.0, 1.0, 1.0, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, diff);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 110.0);

	if(skeleton){
		teapot.renderskeleton();
	}else{
		teapot.render(shadeP);
	}

	
	//Sleep(300);
    glutSwapBuffers();
    glFlush ();
    glutPostRedisplay();
}


void reshape (int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0f,((float)w)/h,0.1f,1000.0f);
    glMatrixMode (GL_MODELVIEW);
}

void arrows(int key, int x, int y){
    switch(key){
        case GLUT_KEY_UP:
			eyeY += 0.1f;
			cout <<eyeY<<"\n"<<eyeZ<<"\n";
            break;
        case GLUT_KEY_DOWN:
			eyeY -= 0.1f;
			cout <<eyeY<<"\n"<<eyeZ<<"\n";
            break;
        case GLUT_KEY_LEFT:
			eyeZ -= 0.1f;
			cout <<eyeY<<"\n"<<eyeZ<<"\n";
            break;
        case GLUT_KEY_RIGHT:
			eyeZ += 0.1f;
			cout <<eyeY<<"\n"<<eyeZ<<"\n";
            break;
    }
}

void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
        case 'f':
			mode = (mode+1)%3;
            initShaders(shader[mode]);
            break;
		case 'e':
			envN = (envN+1)%4;
			teapot.setEnv(envPic[envN]);
			background = teapot.loadTexture(envPic[envN].c_str());
            break;
		case 't':
			texN = (texN+1)%4;
			teapot.setTexture(texPic[texN]);
			teapot.setMixture(texPic[texN]);
			break;
		case 's':
            skeleton = !skeleton;
            break;
		case 'z':
            rot = (rot+1)%2;
            break;
        case 27:
            exit(0);
            break;
		case 'd':
			subdivide();
			teapot= *(new Teapot("example.txt"));
			teapot.computeNormals();
			teapot.setEnv(envPic[envN]);
			teapot.setTexture(texPic[texN]);
			teapot.setMixture(masPic[texN]);
   }
}
void timer(int v) {
	glutPostRedisplay();				// trigger display function again
	glutTimerFunc(1000/FPS,timer,v);	// restart timer again
	moveCamera();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (1000, 500); 
    glutInitWindowPosition (100, 100);
    glutCreateWindow (argv[0]);
    glewInit();


    init ();
    glutDisplayFunc(display); 
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(arrows);
	glutTimerFunc(100, timer, FPS);

    glutMainLoop();


    return 0;
}
