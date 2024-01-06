//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul VIII - 08_04_obiecte3D.cpp|
// ========================================
// 
//	Program ce deseneaza cu ajutorul de buffere diferite (pentru varfuri, culori, indici) 2 obiecte 3D (un cub si o sfera), folosindu-se tehnicile MODERN OpenGL;
// 
// 
// 
//	Biblioteci
#include <windows.h>        //	Utilizarea functiilor de sistem Windows (crearea de ferestre, manipularea fisierelor si directoarelor);
#include <stdlib.h>         //  Biblioteci necesare pentru citirea shaderelor;
#include <stdio.h>
#include <math.h>			//	Biblioteca pentru calcule matematice;
#include <GL/glew.h>        //  Definește prototipurile functiilor OpenGL si constantele necesare pentru programarea OpenGL moderna; 
#include <GL/freeglut.h>    //	Include functii pentru: 
							//	- gestionarea ferestrelor si evenimentelor de tastatura si mouse, 
							//  - desenarea de primitive grafice precum dreptunghiuri, cercuri sau linii, 
							//  - crearea de meniuri si submeniuri;
#include "loadShaders.h"	//	Fisierul care face legatura intre program si shadere;
#include "glm/glm.hpp"		//	Bibloteci utilizate pentru transformari grafice;
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"


//  Identificatorii obiectelor de tip OpenGL;

float const PI = 3.141592f;
float const U_MIN_SFERA = -PI / 2, U_MAX_SFERA = PI / 2, V_MIN_SFERA = 0, V_MAX_SFERA = 2 * PI;
int const NR_PARR_SFERA = 10, NR_MERID_SFERA = 10;
float step_u_sfera = (U_MAX_SFERA - U_MIN_SFERA) / NR_PARR_SFERA, step_v_sfera = (V_MAX_SFERA - V_MIN_SFERA) / NR_MERID_SFERA;

float const U_MIN_CILINDRU = 0, U_MAX_CILINDRU = 2 * PI, V_MIN_CILINDRU = -20 * PI, V_MAX_CILINDRU = 20 * PI;
int const NR_PARR_CILINDRU = 30, NR_MERID_CILINDRU = 20;
float step_u_cilindru = (U_MAX_CILINDRU - U_MIN_CILINDRU) / NR_PARR_CILINDRU, step_v_cilindru = (V_MAX_CILINDRU - V_MIN_CILINDRU) / NR_MERID_CILINDRU;

int codCol;
float radius_sf = 70;
float radius_cl = 30;
float height_cl = V_MIN_CILINDRU + (NR_MERID_CILINDRU - 2) * step_v_cilindru;

int const NUM_COPACI = 500; 

int const NUM_RANDURI = 5; // numarul de randuri pe fiecare parte

float const LATIME_DRUM = 360;
float const LUNGIME_DRUM = NUM_COPACI / 4. / NUM_RANDURI * 2 * radius_sf;

int const START_X_RIGHT = LATIME_DRUM / 2;
int const END_X_RIGHT = START_X_RIGHT + 2 * radius_sf * NUM_RANDURI;
int const START_X_LEFT = -END_X_RIGHT;
int const END_X_LEFT = -START_X_RIGHT;

int const START_Y = -LUNGIME_DRUM;
int const END_Y = LUNGIME_DRUM;

int const DIF = 2 * radius_sf;

int const NOT_SPAWN_CHANCE = 4; // 1 din acest numar de copaci nu o sa se spawneze (e probabilistic, nu exact)

int ACTUAL_NUM_COPACI;





// alte variabile



GLuint
	VaoId[NUM_COPACI * 2],
	EboId[NUM_COPACI * 2],
	VboId[NUM_COPACI * 2],
	ProgramId,
	viewLocation,
	projLocation,
	codColLocation;

// DACA VREM MAI MULTI COPACI AICI SCHIMBAM

/*int NUM_RANDURI = 6;
int NUM_COPACI_PE_RAND = 15;
int START_X_RIGHT = 80;
int END_X_RIGHT = 200;
int START_X_LEFT = -200;
int END_X_LEFT = -80;
int DIF_X = 60;
int START_Y = -400;
int END_Y = 400;
int DIF_Y = 60;
int ROAD_WIDTH = 160;
int ROAD_LENGHT = 800;
*/
int index, index_aux;

// variabile pentru matricea de vizualizare
float Refx = 0.0f, Refy = 0.0f, Refz = 0.0f;
float alpha = 0.0f, beta = 0.0f, dist = 300.0f;
float Obsx, Obsy, Obsz;
float Vx = 0.0f, Vy = 0.0f, Vz = -1.0f;
float incr_alpha1 = 0.01f, incr_alpha2 = 0.01f;

// variabile pentru matricea de proiectie
float width = 800, height = 600, znear = 1, fov = 30;

// pentru fereastra de vizualizare 
GLint winWidth = 1000, winHeight = 600;

// vectori
glm::vec3 Obs, PctRef, Vert;

// matrice utilizate
glm::mat4 view, projection;

void ProcessNormalKeys(unsigned char key, int x, int y)
{
	switch (key) {			//	Apasarea tastelor `+` si `-` schimba pozitia observatorului (se departeaza / aproprie);
	case '-':
		dist -= 5.0;
		break;
	case '+':
		dist += 5.0;
		break;
	}
	if (key == 27)
		exit(0);
}
void ProcessSpecialKeys(int key, int xx, int yy)
{
	switch (key)						//	Procesarea tastelor 'LEFT', 'RIGHT', 'UP', 'DOWN';
	{									//	duce la deplasarea observatorului pe axele Ox si Oy;
	case GLUT_KEY_LEFT:
		beta -= 0.01f;
		break;
	case GLUT_KEY_RIGHT:
		beta += 0.01f;
		break;
	case GLUT_KEY_UP:
		alpha += incr_alpha1;
		if (abs(alpha - PI / 2) < 0.05)
		{
			incr_alpha1 = 0.f;
		}
		else
		{
			incr_alpha1 = 0.01f;
		}
		break;
	case GLUT_KEY_DOWN:
		alpha -= incr_alpha2;
		if (abs(alpha + PI / 2) < 0.05)
		{
			incr_alpha2 = 0.f;
		}
		else
		{
			incr_alpha2 = 0.01f;
		}
		break;
	}
}

//  Crearea si compilarea obiectelor de tip shader;
//	Trebuie sa fie in acelasi director cu proiectul actual;
//  Shaderul de varfuri / vertex shader - afecteaza geometria scenei;
//  Shaderul de fragment / Fragment shader - afecteaza culoarea pixelilor;
void CreateShaders(void)
{
	ProgramId = LoadShaders("main.vert", "main.frag");
	glUseProgram(ProgramId);
}

void CreateVAO1(float x, float y, float z, int i)
{
	// SFERA
	// Matricele pentru varfuri, culori, indici
	glm::vec4 Vertices1[(NR_PARR_SFERA + 1) * NR_MERID_SFERA];
	glm::vec3 Colors1[(NR_PARR_SFERA + 1) * NR_MERID_SFERA];
	GLushort Indices1[2 * (NR_PARR_SFERA + 1) * NR_MERID_SFERA + 4 * (NR_PARR_SFERA + 1) * NR_MERID_SFERA];

//	printf("%f", z + V_MIN_CILINDRU + (NR_MERID_CILINDRU - 2) * step_v_cilindru);

	for (int merid = 0; merid < NR_MERID_SFERA; merid++)
	{
		for (int parr = 0; parr < NR_PARR_SFERA + 1; parr++)
		{
			// implementarea reprezentarii parametrice 
			float u = U_MIN_SFERA + parr * step_u_sfera; // valori pentru u si v
			float v = V_MIN_SFERA + merid * step_v_sfera;
			float x_vf = x + radius_sf * cosf(u) * cosf(v); // coordonatele varfului corespunzator lui (u,v)
			float y_vf = y + radius_sf * cosf(u) * sinf(v);
			float z_vf = z + V_MIN_CILINDRU + (NR_MERID_CILINDRU - 2) * step_v_cilindru + radius_sf + radius_sf * sinf(u);

			// identificator ptr varf; coordonate + culoare + indice la parcurgerea meridianelor
			index = merid * (NR_PARR_SFERA + 1) + parr;
			Vertices1[index] = glm::vec4(x_vf, y_vf, z_vf, 1.0);
			Colors1[index] = glm::vec3(0, 0.5, 0);
			Indices1[index] = index;

			// indice ptr acelasi varf la parcurgerea paralelelor
			index_aux = parr * (NR_MERID_SFERA)+merid;
			Indices1[(NR_PARR_SFERA + 1) * NR_MERID_SFERA + index_aux] = index;

			// indicii pentru desenarea fetelor, pentru varful curent sunt definite 4 varfuri
			if ((parr + 1) % (NR_PARR_SFERA + 1) != 0) // varful considerat sa nu fie Polul Nord
			{
				int AUX = 2 * (NR_PARR_SFERA + 1) * NR_MERID_SFERA;
				int index1 = index; // varful v considerat
				int index2 = index + (NR_PARR_SFERA + 1); // dreapta lui v, pe meridianul urmator
				int index3 = index2 + 1;  // dreapta sus fata de v
				int index4 = index + 1;  // deasupra lui v, pe acelasi meridian
				if (merid == NR_MERID_SFERA - 1)  // la ultimul meridian, trebuie revenit la meridianul initial
				{
					index2 = index2 % (NR_PARR_SFERA + 1);
					index3 = index3 % (NR_PARR_SFERA + 1);
				}
				Indices1[AUX + 4 * index] = index1;  // unele valori ale lui Indices, corespunzatoare Polului Nord, au valori neadecvate
				Indices1[AUX + 4 * index + 1] = index2;
				Indices1[AUX + 4 * index + 2] = index3;
				Indices1[AUX + 4 * index + 3] = index4;
			}
		}
	};

	// generare VAO/buffere
	glGenVertexArrays(1, &VaoId[i]);
	glBindVertexArray(VaoId[i]);
	glGenBuffers(1, &VboId[i]); // atribute
	glGenBuffers(1, &EboId[i]); // indici

	// legare+"incarcare" buffer
	glBindBuffer(GL_ARRAY_BUFFER, VboId[i]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1) + sizeof(Colors1), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices1), Vertices1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices1), sizeof(Colors1), Colors1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId[i]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices1), Indices1, GL_STATIC_DRAW);

	// atributele; 
	glEnableVertexAttribArray(0); // atributul 0 = pozitie
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0));
	glEnableVertexAttribArray(1); // atributul 1 = culoare
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)(sizeof(Vertices1)));

	//CILINDRU
	glm::vec4 Vertices[(NR_PARR_CILINDRU + 1) * NR_MERID_CILINDRU];
	glm::vec3 Colors[(NR_PARR_CILINDRU + 1) * NR_MERID_CILINDRU];
	GLushort Indices[2 * (NR_PARR_CILINDRU + 1) * NR_MERID_CILINDRU + 4 * (NR_PARR_CILINDRU + 1) * NR_MERID_CILINDRU];
	for (int merid = 0; merid < NR_MERID_CILINDRU; merid++)
	{
		for (int parr = 0; parr < NR_PARR_CILINDRU + 1; parr++)
		{
			// implementarea reprezentarii parametrice 
			float u = U_MIN_CILINDRU + parr * step_u_cilindru; // valori pentru u si v
			float v = V_MIN_CILINDRU + merid * step_v_cilindru;
			float x_vf = x + radius_cl * cosf(u); // coordonatele varfului corespunzator lui (u,v)
			float y_vf = y + radius_cl * sinf(u);
			float z_vf = z + v;

			// identificator ptr varf; coordonate + culoare + indice la parcurgerea meridianelor
			index = merid * (NR_PARR_CILINDRU + 1) + parr;
			Vertices[index] = glm::vec4(x_vf, y_vf, z_vf, 1.0);
			Colors[index] = glm::vec3(139 / 255., 69 / 255., 19 / 255.);
			Indices[index] = index;

			// indice ptr acelasi varf la parcurgerea paralelelor
			index_aux = parr * (NR_MERID_CILINDRU)+merid;
			Indices[(NR_PARR_CILINDRU + 1) * NR_MERID_CILINDRU + index_aux] = index;

			// indicii pentru desenarea fetelor, pentru varful curent sunt definite 4 varfuri
			if ((parr + 1) % (NR_PARR_CILINDRU + 1) != 0) // varful considerat sa nu fie Polul Nord
			{
				int AUX = 2 * (NR_PARR_CILINDRU + 1) * NR_MERID_CILINDRU;
				int index1 = index; // varful v considerat
				int index2 = index + (NR_PARR_CILINDRU + 1); // dreapta lui v, pe meridianul urmator
				int index3 = index2 + 1;  // dreapta sus fata de v
				int index4 = index + 1;  // deasupra lui v, pe acelasi meridian
				if (merid == NR_MERID_CILINDRU - 1)  // la ultimul meridian, trebuie revenit la meridianul initial
				{
					index2 = index2 % (NR_PARR_CILINDRU + 1);
					index3 = index3 % (NR_PARR_CILINDRU + 1);
				}
				Indices[AUX + 4 * index] = index1;  // unele valori ale lui Indices, corespunzatoare Polului Nord, au valori neadecvate
				Indices[AUX + 4 * index + 1] = index2;
				Indices[AUX + 4 * index + 2] = index3;
				Indices[AUX + 4 * index + 3] = index4;
			}
		}
	};

	// generare VAO/buffere
	glGenVertexArrays(1, &VaoId[i + 1]);
	glBindVertexArray(VaoId[i + 1]);
	glGenBuffers(1, &VboId[i + 1]); // atribute
	glGenBuffers(1, &EboId[i + 1]); // indici

	// legare+"incarcare" buffer
	glBindBuffer(GL_ARRAY_BUFFER, VboId[i + 1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(Colors), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices), sizeof(Colors), Colors);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId[i + 1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	// atributele; 
	glEnableVertexAttribArray(0); // atributul 0 = pozitie
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0));
	glEnableVertexAttribArray(1); // atributul 1 = culoare
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)(sizeof(Vertices)));
}

void CreateVAO2(float z)
{
	// varfurile 
	// (4) Matricele pentru varfuri, culori, indici
	glm::vec4 Vertices[(NR_PARR_CILINDRU + 1) * NR_MERID_CILINDRU];
	glm::vec3 Colors[(NR_PARR_CILINDRU + 1) * NR_MERID_CILINDRU];
	GLushort Indices[2 * (NR_PARR_CILINDRU + 1) * NR_MERID_CILINDRU + 4 * (NR_PARR_CILINDRU + 1) * NR_MERID_CILINDRU];
	for (int merid = 0; merid < NR_MERID_CILINDRU; merid++)
	{
		for (int parr = 0; parr < NR_PARR_CILINDRU + 1; parr++)
		{
			// implementarea reprezentarii parametrice 
			float u = U_MIN_CILINDRU + parr * step_u_cilindru; // valori pentru u si v
			float v = V_MIN_CILINDRU + merid * step_v_cilindru;
			float x_vf = radius_cl * cosf(u); // coordonatele varfului corespunzator lui (u,v)
			float y_vf = radius_cl * sinf(u);
			float z_vf = z + v;

			// identificator ptr varf; coordonate + culoare + indice la parcurgerea meridianelor
			index = merid * (NR_PARR_CILINDRU + 1) + parr;
			Vertices[index] = glm::vec4(x_vf, y_vf, z_vf, 1.0);
			Colors[index] = glm::vec3(139 / 255., 69 / 255., 19 / 255.);
			Indices[index] = index;

			// indice ptr acelasi varf la parcurgerea paralelelor
			index_aux = parr * (NR_MERID_CILINDRU)+merid;
			Indices[(NR_PARR_CILINDRU + 1) * NR_MERID_CILINDRU + index_aux] = index;

			// indicii pentru desenarea fetelor, pentru varful curent sunt definite 4 varfuri
			if ((parr + 1) % (NR_PARR_CILINDRU + 1) != 0) // varful considerat sa nu fie Polul Nord
			{
				int AUX = 2 * (NR_PARR_CILINDRU + 1) * NR_MERID_CILINDRU;
				int index1 = index; // varful v considerat
				int index2 = index + (NR_PARR_CILINDRU + 1); // dreapta lui v, pe meridianul urmator
				int index3 = index2 + 1;  // dreapta sus fata de v
				int index4 = index + 1;  // deasupra lui v, pe acelasi meridian
				if (merid == NR_MERID_CILINDRU - 1)  // la ultimul meridian, trebuie revenit la meridianul initial
				{
					index2 = index2 % (NR_PARR_CILINDRU + 1);
					index3 = index3 % (NR_PARR_CILINDRU + 1);
				}
				Indices[AUX + 4 * index] = index1;  // unele valori ale lui Indices, corespunzatoare Polului Nord, au valori neadecvate
				Indices[AUX + 4 * index + 1] = index2;
				Indices[AUX + 4 * index + 2] = index3;
				Indices[AUX + 4 * index + 3] = index4;
			}
		}
	};
	// generare VAO/buffere
	glGenVertexArrays(1, &VaoId[0]);
	glBindVertexArray(VaoId[0]);
	glGenBuffers(1, &VboId[0]); // atribute
	glGenBuffers(1, &EboId[0]); // indici

	// legare+"incarcare" buffer
	glBindBuffer(GL_ARRAY_BUFFER, VboId[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(Colors), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices), sizeof(Colors), Colors);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	// atributele; 
	glEnableVertexAttribArray(0); // atributul 0 = pozitie
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0));
	glEnableVertexAttribArray(1); // atributul 1 = culoare
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)(sizeof(Vertices)));
}

//	Schimba inaltimea/latimea scenei in functie de modificarile facute de utilizator ferestrei (redimensionari);
void ReshapeFunction(GLint newWidth, GLint newHeight)
{
	glViewport(0, 0, newWidth, newHeight);
	winWidth = newWidth;
	winHeight = newHeight;
	width = winWidth / 10, height = winHeight / 10;
}

// Elimina obiectele de tip shader dupa rulare;
void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}

//  Eliminarea obiectelor de tip VBO dupa rulare;
void DestroyVBO(void)
{
	//  Eliberarea atributelor din shadere (pozitie, culoare, texturare etc.);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	//  Stergerea bufferelor pentru VARFURI (Coordonate, Culori), INDICI;
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	for (int i = 0; i <= NUM_COPACI; i++) {
		glDeleteBuffers(1, &VboId[i]);
		glDeleteBuffers(1, &EboId[i]);
		glDeleteBuffers(1, &VboId[i]);
		glDeleteBuffers(1, &EboId[i]);
	}
}

//  Functia de eliberare a resurselor alocate de program;
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

//  Setarea parametrilor necesari pentru fereastra de vizualizare;
void Initialize(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	int i = 0;

	// LA DREAPTA DRUMULUI
	for (int x = START_X_RIGHT; x < END_X_RIGHT; x += DIF)
	{
		for (int y = START_Y; y < END_Y ; y += DIF)
		{
			if (rand() % NOT_SPAWN_CHANCE != 0) 
			{
				CreateVAO1(x + rand() % (int)radius_cl * 2, y + rand() % (int)radius_cl, 0, i);
				i += 2;
			}
		}
	}

	//LA STANGA DRUMULUI
	for (int x = START_X_LEFT; x < END_X_LEFT; x += DIF)
	{
		for (int y = START_Y; y < END_Y; y += DIF)
		{
			if (rand() % NOT_SPAWN_CHANCE != 0)
			{
				CreateVAO1(x - rand() % (int)radius_cl * 2, y + rand() % (int)radius_cl, 0, i);
				i += 2;
			}
		}
	}

	printf("COPACI: %d", i / 2);
	ACTUAL_NUM_COPACI = i / 2;

	//CreateVAO2(0);
	CreateShaders();
	viewLocation = glGetUniformLocation(ProgramId, "viewShader");
	projLocation = glGetUniformLocation(ProgramId, "projectionShader");
	codColLocation = glGetUniformLocation(ProgramId, "codCol");
}

//	Functie utilizata in setarea matricelor de vizualizare si proiectie;
void SetMVP(void)
{
	//pozitia observatorului
	Obsx = Refx + dist * cos(alpha) * cos(beta);
	Obsy = Refy + dist * cos(alpha) * sin(beta);
	Obsz = Refz + dist * sin(alpha);

	// reperul de vizualizare
	glm::vec3 Obs = glm::vec3(Obsx, Obsy, Obsz);   // se schimba pozitia observatorului	
	glm::vec3 PctRef = glm::vec3(Refx, Refy, Refz); // pozitia punctului de referinta
	glm::vec3 Vert = glm::vec3(Vx, Vy, Vz); // verticala din planul de vizualizare 
	view = glm::lookAt(Obs, PctRef, Vert);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

	// matricea de proiectie 
	projection = glm::infinitePerspective(fov, GLfloat(width) / GLfloat(height), znear);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);
}

//	Functia de desenare a graficii pe ecran;
void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	SetMVP();

	// SFERA
	for (int i = 0; i < ACTUAL_NUM_COPACI * 2; i+=2) {
		glBindVertexArray(VaoId[i]);
		codCol = 0;
		glUniform1i(codColLocation, codCol);
		for (int patr = 0; patr < (NR_PARR_SFERA + 1) * NR_MERID_SFERA; patr++)
		{
			if ((patr + 1) % (NR_PARR_SFERA + 1) != 0) // nu sunt considerate fetele in care in stanga jos este Polul Nord
				glDrawElements(
					GL_QUADS,
					4,
					GL_UNSIGNED_SHORT,
					(GLvoid*)((2 * (NR_PARR_SFERA + 1) * (NR_MERID_SFERA)+4 * patr) * sizeof(GLushort)));
		}

		glBindVertexArray(VaoId[i + 1]);
		glUniform1i(codColLocation, codCol);
		for (int patr = 0; patr < (NR_PARR_CILINDRU + 1) * NR_MERID_CILINDRU; patr++)
		{
			if ((patr + 1) % (NR_PARR_CILINDRU + 1) != 0) // nu sunt considerate fetele in care in stanga jos este Polul Nord
				glDrawElements(
					GL_QUADS,
					4,
					GL_UNSIGNED_SHORT,
					(GLvoid*)((2 * (NR_PARR_CILINDRU + 1) * (NR_MERID_CILINDRU)+4 * patr) * sizeof(GLushort)));
		}
	}
	
	// CUBUL
	/*
	codCol = 0;
	glBindVertexArray(VaoId[0]);
	glUniform1i(codColLocation, codCol);
	for (int patr = 0; patr < (NR_PARR_CILINDRU + 1) * NR_MERID_CILINDRU; patr++)
	{
		if ((patr + 1) % (NR_PARR_CILINDRU + 1) != 0) // nu sunt considerate fetele in care in stanga jos este Polul Nord
			glDrawElements(
				GL_QUADS,
				4,
				GL_UNSIGNED_SHORT,
				(GLvoid*)((2 * (NR_PARR_CILINDRU + 1) * (NR_MERID_CILINDRU)+4 * patr) * sizeof(GLushort)));
	}
	*/

	glutSwapBuffers();
	glFlush();
}

//	Punctul de intrare in program, se ruleaza rutina OpenGL;
int main(int argc, char* argv[])
{
	//  Se initializeaza GLUT si contextul OpenGL si se configureaza fereastra si modul de afisare;
	//printf("variabile: %f %f %d %d %d %d %d %d %d ", LATIME_DRUM, LUNGIME_DRUM, START_X_RIGHT, END_X_RIGHT, START_X_LEFT, END_X_LEFT, START_Y, END_Y, DIF);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);							//	Se folosesc 2 buffere pentru desen (unul pentru afisare si unul pentru randare => animatii cursive) si culori RGB + 1 buffer pentru adancime;
	glutInitWindowSize(winWidth, winHeight);											//  Dimensiunea ferestrei;
	glutInitWindowPosition(100, 100);													//  Pozitia initiala a ferestrei;
	glutCreateWindow("Masina prin padure");		//	Creeaza fereastra de vizualizare, indicand numele acesteia;

	//	Se initializeaza GLEW si se verifica suportul de extensii OpenGL modern disponibile pe sistemul gazda;
	//  Trebuie initializat inainte de desenare;

	glewInit();

	Initialize();							//  Setarea parametrilor necesari pentru fereastra de vizualizare; 
	glutReshapeFunc(ReshapeFunction);		//	Schimba inaltimea/latimea scenei in functie de modificarile facute de utilizator ferestrei (redimensionari);
	glutDisplayFunc(RenderFunction);		//  Desenarea scenei in fereastra;
	glutIdleFunc(RenderFunction);			//	Asigura rularea continua a randarii;
	glutKeyboardFunc(ProcessNormalKeys);	//	Functii ce proceseaza inputul de la tastatura utilizatorului;
	glutSpecialFunc(ProcessSpecialKeys);
	glutCloseFunc(Cleanup);					//  Eliberarea resurselor alocate de program;

	//  Bucla principala de procesare a evenimentelor GLUT (functiile care incep cu glut: glutInit etc.) este pornita;
	//  Prelucreaza evenimentele si deseneaza fereastra OpenGL pana cand utilizatorul o inchide;

	glutMainLoop();

	return 0;
}