#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include "gfx/vec3.h"
#include <GL/glew.h>
#include <GL/glut.h>

using namespace std;
using namespace gfx;

class Teapot{
    private:
        vector<Vec3f> verticies;
        vector<Vec3f> normals;
        vector<Vec3> faces;
        GLuint texture;
        GLuint mixture;
        
    public:
		GLuint env;
        Teapot(string filename);
        void computeNormals();
        void setTexture(string filename);
        void setMixture(string filename);
        void setEnv(string filename);
        void render(int shader_program);
		void renderskeleton();
		GLuint loadTexture( const char * filename);
};
#endif
