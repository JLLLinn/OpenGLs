
#include <vector>
#include <map>
#include <GL/glut.h>
#include <fstream>
#include <iostream>
#include <string>
#include "gfx/vec3.h"
#include "gfx/vec4.h"
#include "math.h"
#include <iostream>
#include <fstream>
#include <algorithm>    // std::find
using namespace std;

#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

class Mesh;
class Face;
class Edge;
class Vertex;

Mesh * mesh;
class Mesh {
public:
	vector<Face *> fl; // facelist
	vector<Vertex *> vl; // vertexlist
};

class Vertex {
public:
	float x, y, z;
	int valence;
	vector<Face *> fl;
	vector<Edge *> el;
	Vertex * newV;
	
	Vertex(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
		valence = 0;
		newV = NULL;
	}
	
	bool equals(Vertex * u) {
		return (x == u->x && y == u->y && z == u->z);
	}
};

class Edge {
public:
	Vertex * v1, * v2;
	Vertex * center;
	Vertex * newE;
	float ID;
	
	Edge(Vertex * v1, Vertex * v2) {
		this->v1 = v1;
		this->v2 = v2;
		center = new Vertex((v1->x + v2->x) / 2,
							(v1->y + v2->y) / 2,
							(v1->z + v2->z) / 2);
		newE = NULL;
		if (v1->x < v2->x || v1->y < v2->y || v1->z < v2->z) {
			ID = 1 * v1->x + 10 * v1->y + 100 * v1->z + 1 * v2->x + 10 * v2->y + 100 * v2->z;
		} else {
			ID = 1 * v2->x + 10 * v2->y + 100 * v2->z + 1 * v1->x + 10 * v1->y + 100 * v1->z;
		}
	}
};

class Face {
public:
	Vertex * v[4];
	Vertex * centroid;
	Edge * e[4];
	
	Face(Vertex * v0, Vertex * v1, Vertex * v2, Vertex * v3) {
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
		v[3] = v3;
		centroid = new Vertex((v0->x + v1->x + v2->x + v3->x) / 4.0,
							  (v0->y + v1->y + v2->y + v3->y) / 4.0,
							  (v0->z + v1->z + v2->z + v3->z) / 4.0);
		v0->fl.push_back(this);
		v1->fl.push_back(this);
		v2->fl.push_back(this);
		v3->fl.push_back(this);
	}
};

struct eComp : public binary_function< Edge *, Edge *, bool >
{
    bool operator()(Edge *e1, Edge *e2) const {
		return e1->ID < e2->ID;
	}
};

void subdivide() {
	ofstream myfile ("example.txt");
	if (myfile.is_open())
	{	
		cout<<"yeah I just opened a file!\n";
	}else{
		cout<<":( failed open...\n";
	}
	vector<Vertex *> tempVertex;
	vector<Vec3> tempFace;
	int k = 1;//for testing

	Mesh * newMesh = new Mesh;
	
	int numFaces = mesh->fl.size();
	
	std::map<Edge *, vector<Face *>, eComp> el; // edge to faces map
	
	// iterate over all faces one time to record edges and find valences
	for (int i = 0; i < numFaces; i++) {
		Face * f = mesh->fl[i];
		
		// record this face's edges
		for (int i = 0; i < 4; i++) {
			Vertex * vi = f->v[i];
			Vertex * vj = f->v[(i + 1) % 4];
			
			Edge * e = new Edge(vi, vj);
			
			vector<Face *> faceVec;
			std::map<Edge*, vector<Face *>, eComp >::iterator iter;
			if ((iter = el.find(e)) == el.end()) { // edge not yet recorded
				vi->el.push_back(e);
				vj->el.push_back(e);
				vi->valence++;
				vj->valence++;
				f->e[i] = e;
				faceVec.push_back(f);
				el.insert(make_pair(e, faceVec));
			}
			else { // need to add second face
				f->e[i] = (*iter).first;
				vector<Face *> faceVec = (*iter).second;
				faceVec.push_back(f);
				el[e] = faceVec;
			}
		}
	}
	
	// iterate over all faces second time to compute new faces
	for (int i = 0; i < numFaces; i++) {
		Face * f = mesh->fl[i];

		vector<Vertex *> edgepoints;
		vector<Vertex *> vertexpoints;
		
		// compute edgepoints
		for (int j = 0; j < 4; j++) {	
			Edge * e = f->e[j];
			
			if (e->newE != NULL) {
				edgepoints.push_back(e->newE);
				continue;
			}
			vector<Face *> faceVec;
			map<Edge*, vector<Face *>, eComp >::iterator iter;	
			iter = el.find(e);
			faceVec = (*iter).second;
			
			float xSum = e->v1->x + e->v2->x; float xPts = 2;
			float ySum = e->v1->y + e->v2->y; float yPts = 2;
			float zSum = e->v1->z + e->v2->z; float zPts = 2;
			for (int k = 0; k < faceVec.size(); k++) {
				Face * adjFace = faceVec[k];
				Vertex * cent = adjFace->centroid;
				xSum += cent->x; xPts++;
				ySum += cent->y; yPts++;
				zSum += cent->z; zPts++;
			}
			Vertex * edgepoint = new Vertex(xSum / xPts, ySum / yPts, zSum / zPts);		
			e->newE = edgepoint;
			edgepoints.push_back(edgepoint);
		}
		
		// compute vertexpoints
		for (int j = 0; j < 4; j++) {
			Vertex * v = f->v[j];
			
			if (v->newV != NULL) {
				vertexpoints.push_back(v->newV);
				continue;
			}
			
			float qx = 0.0; float qxpts = 0.0;
			float qy = 0.0; float qypts = 0.0;
			float qz = 0.0; float qzpts = 0.0;
			for (int k = 0; k < v->fl.size(); k++) {
				Face * adjFace = v->fl[k];
				qx += adjFace->centroid->x; qxpts++;
				qy += adjFace->centroid->y; qypts++;
				qz += adjFace->centroid->z; qzpts++;
			}
			Vertex * q = new Vertex(qx / qxpts, qy / qypts, qz / qzpts);
			
			float rx = 0.0; float rxpts = 0.0;
			float ry = 0.0; float rypts = 0.0;
			float rz = 0.0; float rzpts = 0.0;
			for (int m = 0; m < v->el.size(); m++) {
				Edge * adjEdge = v->el[m];
				rx += adjEdge->center->x; rxpts++;
				ry += adjEdge->center->y; rypts++;
				rz += adjEdge->center->z; rzpts++;
			}
			Vertex * r = new Vertex(rx / rxpts, ry / rypts, rz / rzpts);
			
			float n = v->valence;
			Vertex * vertexpoint = new Vertex((q->x / n + 2 * r->x / n + (n - 3) * v->x / n),
											  (q->y / n + 2 * r->y / n + (n - 3) * v->y / n),
											  (q->z / n + 2 * r->z / n + (n - 3) * v->z / n));
			
			v->newV = vertexpoint;
			vertexpoints.push_back(vertexpoint);
		}
		
		Face * f1 = new Face(vertexpoints[0], edgepoints[0], f->centroid, edgepoints[3]);		
		Face * f2 = new Face(edgepoints[0], vertexpoints[1], edgepoints[1], f->centroid);		
		Face * f3 = new Face(f->centroid, edgepoints[1], vertexpoints[2], edgepoints[2]);		
		Face * f4 = new Face(edgepoints[3], f->centroid, edgepoints[2], vertexpoints[3]);
		
		vector<Vertex *>::iterator iter;
		int v[4];

		iter = find(tempVertex.begin(), tempVertex.end(), vertexpoints[0]);
		if (iter !=  tempVertex.end()) {
			v[0] = iter - tempVertex.begin();
		} else {
			v[0] = tempVertex.size();
			tempVertex.push_back(vertexpoints[0]);
		}
		//if (k < 120) cout<<vertexpoints[0]<<" "<<k++<<"\n";

		iter = find(tempVertex.begin(), tempVertex.end(), vertexpoints[1]);
		if (iter !=  tempVertex.end()) {
			v[1] = iter - tempVertex.begin();
		} else {
			v[1] = tempVertex.size();
			tempVertex.push_back(vertexpoints[1]);
		}
		//if (k < 120) cout<<vertexpoints[1]<<" "<<k++<<"\n";
		iter = find(tempVertex.begin(), tempVertex.end(), vertexpoints[2]);
		if (iter !=  tempVertex.end()) {
			v[2] = iter - tempVertex.begin();
		} else {
			v[2] = tempVertex.size();
			tempVertex.push_back(vertexpoints[2]);
		}
		//if (k < 120) cout<<vertexpoints[2]<<" "<<k++<<"\n";


		iter = find(tempVertex.begin(), tempVertex.end(), vertexpoints[3]);
		if (iter !=  tempVertex.end()) {
			v[3] = iter - tempVertex.begin();
		} else {
			v[3] = tempVertex.size();
			tempVertex.push_back(vertexpoints[3]);
		}
		
		//if (k < 120) cout<<vertexpoints[3]<<" "<<k++<<"\n";


		int e[4];
		iter = find(tempVertex.begin(), tempVertex.end(), edgepoints[0]);
		if (iter !=  tempVertex.end()) {
			e[0] = iter - tempVertex.begin();
		} else {
			e[0] = tempVertex.size();
			tempVertex.push_back(edgepoints[0]);
		}
		//if (k < 120) cout<<edgepoints[3]<<" "<<k++<<"\n";

		iter = find(tempVertex.begin(), tempVertex.end(), edgepoints[1]);
		if (iter !=  tempVertex.end()) {
			e[1] = iter - tempVertex.begin();
		} else {
			e[1] = tempVertex.size();
			tempVertex.push_back(edgepoints[1]);
		}
		//if (k < 120) cout<<edgepoints[3]<<" "<<k++<<"\n";

		iter = find(tempVertex.begin(), tempVertex.end(), edgepoints[2]);
		if (iter !=  tempVertex.end()) {
			e[2] = iter - tempVertex.begin();
		} else {
			e[2] = tempVertex.size();
			tempVertex.push_back(edgepoints[2]);
		}
		//if (k < 120) cout<<edgepoints[3]<<" "<<k++<<"\n";

		iter = find(tempVertex.begin(), tempVertex.end(), edgepoints[3]);
		if (iter !=  tempVertex.end()) {
			e[3] = iter - tempVertex.begin();
		} else {
			e[3] = tempVertex.size();
			tempVertex.push_back(edgepoints[3]);
		}
		//if (k < 120) cout<<edgepoints[3]<<" "<<k++<<"\n";

		int cen;
		iter = find(tempVertex.begin(), tempVertex.end(), f->centroid);
		if (iter !=  tempVertex.end()) {
			cen = iter - tempVertex.begin();
		} else {
			cen = tempVertex.size();
			tempVertex.push_back(f->centroid);
		}


		tempFace.push_back(Vec3(v[0],cen,e[3]));
		tempFace.push_back(Vec3(v[0],e[0],cen));

		tempFace.push_back(Vec3(e[0],e[1],cen));
		tempFace.push_back(Vec3(e[0],v[1],e[1]));

		tempFace.push_back(Vec3(cen,v[2],e[2]));
		tempFace.push_back(Vec3(cen,e[1],v[2]));

		tempFace.push_back(Vec3(e[3],e[2],v[3]));
		tempFace.push_back(Vec3(e[3],cen,e[2]));



		newMesh->fl.push_back(f1);
		newMesh->fl.push_back(f2);
		newMesh->fl.push_back(f3);
		newMesh->fl.push_back(f4);
	}
	for (unsigned int i = 0; i < tempVertex.size(); i++) {
		myfile<< "v "<< tempVertex[i]->x<<" "<<tempVertex[i]->y<<" "<< tempVertex[i]->z<<"\n";
	}
	for (unsigned int i = 0; i < tempFace.size(); i++) {
		myfile<< "f "<< tempFace[i][0]<<" "<<tempFace[i][1]<<" "<< tempFace[i][2]<<"\n";	
	}
	myfile.close();
	
	
	mesh = newMesh;
}

void setupVerticesAndFaces(Mesh * m) {
	//taken from mp3
	string filename = "I4f.txt";
	ifstream in;
    in.open(filename.c_str(), ios::in);

    string command;


    while( in ){
        in >> command;
        if( in.eof() ) continue;
        
        if( command == "v" ){
			float x,y,z;
            in >> x;
            in >> y;
            in >> z;
            m->vl.push_back(new Vertex(x, y, z));;
        } else if (command == "f" ){

            int v1, v2, v3, v4;
            in >> v1;
            in >> v2;
            in >> v3;
			in >> v4;
            m->fl.push_back(new Face(m->vl[v1], m->vl[v2], m->vl[v3], m->vl[v4]));
        } 

        getline(in, command);
    }
	
}

void initSub(){
	mesh = new Mesh;
	setupVerticesAndFaces(mesh);
}

