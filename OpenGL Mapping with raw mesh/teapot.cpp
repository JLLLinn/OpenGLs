#include "teapot.h"
#include <fstream>
#include <iostream>
#include <string>
#include <GL/glut.h>
#include <cmath>
#include <SOIL/SOIL.h>

Teapot::Teapot(string filename){
    texture = 0;

    ifstream in;
    in.open(filename.c_str(), ios::in);

    string command;


    while( in ){
        in >> command;
        if( in.eof() ) continue;
        
        if( command == "v" ){
            Vec3f v;
            in >> v[0];
            in >> v[1];
            in >> v[2];
            verticies.push_back(v);
        } else if (command == "f" ){
            Vec3f f;
            in >> f[0];
            in >> f[1];
            in >> f[2];

            // obj file format actually uses 1-indexed faces
            f[0] -= 1;
            f[1] -= 1;
            f[2] -= 1;
            faces.push_back(f);
        } 

        getline(in, command);
    }

    Vec3f zero(0, 0, 0);
    for(unsigned int i=0;i<verticies.size();i++){
        normals.push_back( zero );
    }

}

void Teapot::setTexture(string filename){
    texture = loadTexture(filename.c_str());
}

void Teapot::setMixture(string filename){
    mixture = loadTexture(filename.c_str());
}

void Teapot::setEnv(string filename){
    env = loadTexture(filename.c_str());
}

void Teapot::computeNormals(){
    for(unsigned int i=0;i<faces.size();i++){
        Vec3f one = verticies[ faces[i][0] ];
        Vec3f two = verticies[ faces[i][1] ];
        Vec3f three = verticies[ faces[i][2] ];
        
        Vec3f x = two - one;
        Vec3f y = three - one;
        Vec3f normal = cross(x, y);

        normals[ faces[i][0] ] += normal;
        normals[ faces[i][1] ] += normal;
        normals[ faces[i][2] ] += normal;
    }

    for(unsigned int i=0;i<normals.size();i++){
        unitize( normals[i] );
    }
}

void Teapot::render(int shader_program){

    glUseProgram(shader_program);
    int texCoordLoc = glGetAttribLocation(shader_program,"TextureCoord");
    int texLoc = glGetUniformLocation(shader_program,"texture");
    int envLoc = glGetUniformLocation(shader_program,"env");
    int mixLoc = glGetUniformLocation(shader_program,"mixture");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, env);


    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, mixture);

    glUniform1i(envLoc, 0);
    glUniform1i(texLoc, 2);
    glUniform1i(mixLoc, 4);

    float x, y, z, theta;
    float nx, ny, nz;
    float ymax = .5;

    for(unsigned int i=0;i<faces.size();i++){
        glBegin(GL_TRIANGLES);
            for(int j=0;j<3;j++){
                x = verticies[ faces[i][j] ][0];
                y = verticies[ faces[i][j] ][1];
                z = verticies[ faces[i][j] ][2];
                theta = atan2(x, z);

                float percent = (theta + M_PI) / (2 * M_PI);
                if(percent > .5) percent = 1 - percent;

                nx = normals[ faces[i][j] ][0];
                ny = normals[ faces[i][j] ][1];
                nz = normals[ faces[i][j] ][2];

                glNormal3f(nx, ny, nz);
                glVertexAttrib2f(texCoordLoc, 20* percent, y / ymax);
                glVertex3f(x, y, z);
            }
        glEnd();
    }

    glUseProgram(0);

    
}


void Teapot::renderskeleton(){
	 float x, y, z, theta;
    float nx, ny, nz;
    float ymax = .5;
	glColor3f(1.0,0.0,0.0);
    for(unsigned int i=0;i<faces.size();i++){
        glBegin(GL_LINE_LOOP);
            for(int j=0;j<3;j++){
                x = verticies[ faces[i][j] ][0];
                y = verticies[ faces[i][j] ][1];
                z = verticies[ faces[i][j] ][2];
                theta = atan2(x, z);

                float percent = (theta + M_PI) / (2 * M_PI);
                if(percent > .5) percent = 1 - percent;

                nx = normals[ faces[i][j] ][0];
                ny = normals[ faces[i][j] ][1];
                nz = normals[ faces[i][j] ][2];

                glNormal3f(nx, ny, nz);
                glVertex3f(x, y, z);
            }
        glEnd();
    }
}
GLuint Teapot::loadTexture( const char * filename)
{
    GLuint texture;
    texture = SOIL_load_OGL_texture
		(
			filename,
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

    glBindTexture( GL_TEXTURE_2D, texture );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture( GL_TEXTURE_2D, 0);

    return texture;
}

