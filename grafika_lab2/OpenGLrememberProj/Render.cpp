#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId1;
GLuint texId2;
GLuint texId3;
//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId1);

	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId1);


	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//массив трехбайтных элементов  (R G B)


	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)

	OpenGL::LoadBMP("texture2.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId2);

	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId2);


	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);






	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);



	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)

	OpenGL::LoadBMP("texture3.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId3);

	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId3);


	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);






	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку" 
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}




double middle(double a1, double a2)
{
	double res;
	res = (a1 + a2) / 2;
	return res;
}

double length(double a1, double b1, double a2, double b2)
{
	double res;
	res = sqrt(((a2 - a1) * (a2 - a1)) + ((b2 - b1) * (b2 - b1)));
	return res;
}

double angle(double a1, double b1, double a2, double b2)
{
	double res = acos((a1 * a2 + b1 * b2) / (sqrt(a1 * a1 + b1 * b1) * sqrt(a2 * a2 + b2 * b2)));
	return res;
}

double normal(double A[], double B[], double C[], int r) 
{
		double vx1 = A[0] - B[0];
		double vy1 = A[1] - B[1];
		double vz1 = A[2] - B[2];
		double vx2 = B[0] - C[0];
		double vy2 = B[1] - C[1];
		double vz2 = B[2] - C[2];


		double wrki = sqrt((vy1 * vz2 - vz1 * vy2) * (vy1 * vz2 - vz1 * vy2) + (vz1 * vx2 - vx1 * vz2) * (vz1 * vx2 - vx1 * vz2) + (vx1 * vy2 - vy1 * vx2) * (vx1 * vy2 - vy1 * vx2));

		double Nx = (vy1 * vz2 - vz1 * vy2)/wrki;
		double Ny = (vz1 * vx2 - vx1 * vz2)/wrki;
		double Nz = (vx1 * vy2 - vy1 * vx2)/wrki;

		if (r == 1)
			return Nx;
		else if (r==2)
			return Ny;
		else if (r==3)
			return Nz;
}

double normalY(double ax, double bz, double bx, double az)
{
	return ((-1)*ax * bz + bx * az);
}

double normalZ(double ax, double by, double bx, double ay)
{
	return (ax * by - bx * ay);
}


void Figure1(double i, double j, double r, double g, double b)// создает фигуру по заданию
{
	double A1[] = { 0, 0, i };
	double B1[] = { 6, 12, i };
	double C1[] = { 10,7, i };
	double D1[] = { 13,11,i };
	double E1[] = { 14,3, i };
	double F1[] = { 9,6, i };
	double G1[] = { 13,0, i };
	
	double A2[] = { 0, 0, j };
	double B2[] = { 6, 12, j };
	double C2[] = { 10,7, j };
	double D2[] = { 13,11,j };
	double E2[] = { 14,3, j };
	double F2[] = { 9,6, j };
	double G2[] = { 13,0, j };


	glColor4d(0.2, 0.2, 1,0.8);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId3);
	glBegin(GL_TRIANGLES);

	glNormal3d(0, 0, -1);
	glTexCoord2d(A1[0], A1[1]);
	glVertex3dv(A1);
	glTexCoord2d(B1[0], B1[1]);
	glVertex3dv(B1);
	glTexCoord2d(C1[0], C1[1]);
	glVertex3dv(C1);
	
	glTexCoord2d(C1[0], C1[1]);
	glVertex3dv(C1);
	glTexCoord2d(D1[0], D1[1]);
	glVertex3dv(D1);
	glTexCoord2d(E1[0], E1[1]);
	glVertex3dv(E1);
	
	glTexCoord2d(E1[0], E1[1]);
	glVertex3dv(E1);
	glTexCoord2d(C1[0], C1[1]);
	glVertex3dv(C1);
	glTexCoord2d(F1[0], F1[1]);
	glVertex3dv(F1);
	
	//glVertex3dv(F1);
	//glVertex3dv(G1);
	//glVertex3dv(A1);
	glTexCoord2d(A1[0], A1[1]);
	glVertex3dv(A1);
	glTexCoord2d(C1[0], C1[1]);
	glVertex3dv(C1);
	glTexCoord2d(F1[0], F1[1]);
	glVertex3dv(F1);
	glEnd();
	glDisable(GL_TEXTURE_2D);



	glColor4d(1, 0.2, 1,0.8);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId1);

	glBegin(GL_TRIANGLES);
	
	glTexCoord2d(A2[0], A2[1]);
	glNormal3d(0, 0, 1);
	glVertex3dv(A2);
	glTexCoord2d(B2[0], B2[1]);
	glVertex3dv(B2);
	glTexCoord2d(C2[0], C2[1]);
	glVertex3dv(C2);
	
	
	glTexCoord2d(C2[0], C2[1]);
	glVertex3dv(C2);
	glTexCoord2d(D2[0], D2[1]);
	glVertex3dv(D2);
	glTexCoord2d(E2[0], E2[1]);
	glVertex3dv(E2);

	glTexCoord2d(E2[0], E2[1]);
	glVertex3dv(E2);
	glTexCoord2d(C2[0], C2[1]);
	glVertex3dv(C2);
	glTexCoord2d(F2[0], F2[1]);
	glVertex3dv(F2);


	//glVertex3dv(F2);
	//glVertex3dv(G2);
	//glVertex3dv(A2);
	glTexCoord2d(A2[0], A2[1]);
	glVertex3dv(A2);
	glTexCoord2d(C2[0], C2[1]);
	glVertex3dv(C2);
	glTexCoord2d(F2[0], F2[1]);
	glVertex3dv(F2);

	glEnd();
	glDisable(GL_TEXTURE_2D);
	


	//glBegin(GL_QUADS);
	//glColor4d(r, g, b, 0.8);
	//glNormal3d(normal(A1,A2,B2,1), normal(A1, A2, B2, 2), normal(A1, A2, B2, 3));
	//glVertex3dv(A1);
	//glVertex3dv(A2);
	//glVertex3dv(B2);
	//glVertex3dv(B1);
	//glEnd();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId2);
	glBegin(GL_QUADS);
	glNormal3d(normal(B1, B2, C2, 1), normal(B1, B2, C2, 2), normal(B1, B2, C2, 3));
	glColor4d(r, g, b, 0.8);
	glTexCoord2d(B1[0]*2, 0);
	glVertex3dv(B1);
	glTexCoord2d(B1[0]*2, 1);
	glVertex3dv(B2);
	glTexCoord2d(0, 1);
	glVertex3dv(C2);
	glTexCoord2d(0, 0);
	glVertex3dv(C1);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3d(normal(C1, C2, D2, 1), normal(C1, C2, D2, 2), normal(C1, C2, D2, 3));
	glColor4d(r, g, b, 0.8);
	glTexCoord2d(C1[0] * 2, 0);
	glVertex3dv(C1);
	glTexCoord2d(C1[0] * 2, 1);
	glVertex3dv(C2);
	glTexCoord2d(0, 1);
	glVertex3dv(D2);
	glTexCoord2d(0, 0);
	glVertex3dv(D1);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3d(normal(D1, D2, E2, 1), normal(D1, D2, E2, 2), normal(D1, D2, E2, 3));
	glTexCoord2d(D1[0] * 2, 0);
	glVertex3dv(D1);
	glTexCoord2d(D1[0] * 2, 1);
	glVertex3dv(D2);
	glTexCoord2d(0, 1);
	glVertex3dv(E2);
	glTexCoord2d(0, 0);
	glVertex3dv(E1);
	glEnd();

	glBegin(GL_QUADS);
	glColor4d(r, g, b, 0.8);
	glNormal3d(normal(E1, E2, F2, 1), normal(E1, E2, F2, 2), normal(E1, E2, F2, 3));
	glTexCoord2d(E1[0] * 2, 0);
	glVertex3dv(E1);
	glTexCoord2d(E1[0] * 2,1);
	glVertex3dv(E2);
	glTexCoord2d(0, 1);
	glVertex3dv(F2);
	glTexCoord2d(0, 0);
	glVertex3dv(F1);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3d(normal(F1, F2, G2, 1), normal(F1, F2, G2, 2), normal(F1, F2, G2, 3));
	glTexCoord2d(F1[0] * 2, 0);
	glVertex3dv(F1);
	glTexCoord2d(F1[0] * 2, 1);
	glVertex3dv(F2);
	glTexCoord2d(0, 1);
	glVertex3dv(G2);
	glTexCoord2d(0, 0);
	glVertex3dv(G1);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//glBegin(GL_QUADS);
	//glNormal3d(normal(G1, G2, A2, 1), normal(G1, G2, A2, 2), normal(G1, G2, A2, 3));
	//glVertex3dv(G1);
	//glVertex3dv(G2);
	//glVertex3dv(A2);
	//glVertex3dv(A1);
	//glEnd();

}

void trig(double i, double j)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId3);
	glBegin(GL_TRIANGLE_FAN);
	glColor4d(1, 0.2, 1, 0.8);
	double G[] = { middle(0,6), middle(0, 12), i };
	double x = length(middle(0, 6), middle(0, 12), 6, 12) * cos(18.4 * PI / 180) * (-1);
	double y = length(middle(0, 6), middle(0, 12), 6, 12) * sin(18.4 * PI / 180);
	double u2[] = { -5.99, 3, i };
	double u3[] = { 0, 0, i };
	glNormal3d(0, 0, -1);
	glTexCoord2d(G[0], G[1]);
	glVertex3dv(G);
	u2[0] = 6;
	u2[1] = 12;
	glTexCoord2d(u2[0], u2[1]);
	glVertex3dv(u2);
	for (i = 64; i < 244; i++) {
		u2[0]=  length(middle(0, 6), middle(0, 12), 6, 12) * cos(i* PI / 180)+3 ;
		u2[1]= length(middle(0, 6), middle(0, 12), 6, 12) * sin(i * PI / 180)+6;
		glTexCoord2d(u2[0], u2[1]);
		glVertex3dv(u2);

	}
		
	u2[0] = 0;
	u2[1] = 0;
	glTexCoord2d(u2[0], u2[1]);
	glVertex3dv(u2);

	//for (int g = 0; g < 1080; g = g + 15)
	//{
	//double param = 30.0;
	//	double x = length(middle(0, 6), middle(0, 12), 6, 12) * cos(g * PI / 180);
	//	double y = length(middle(0, 6), middle(0, 12), 6, 12) * sin(g * PI / 180);
	//	double F[] = { x, y, i };
	//	glVertex3dv(F);
	////glVertex3dv(u2);
	////glVertex3dv(u3);
	//
	//}
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId1);
	glBegin(GL_TRIANGLE_FAN);
	glColor4d(0.2,1, 1, 0.8);
	double G11[] = { middle(0,6), middle(0, 12), j };
	double u21[] = { -5.99, 3, j };
	double u31[] = { 0, 0, j };
	glNormal3d(0, 0, 1);
	glTexCoord2d(G11[0], G11[1]);
	glVertex3dv(G11);
	u21[0] = 6;
	u21[1] = 12;
	glTexCoord2d(u21[0], u21[1]);
	glVertex3dv(u21);
	for (i = 64; i < 244; i++) {
		u21[0] = length(middle(0, 6), middle(0, 12), 6, 12) * cos(i * PI / 180) + 3;
		u21[1] = length(middle(0, 6), middle(0, 12), 6, 12) * sin(i * PI / 180) + 6;
		glTexCoord2d(u21[0], u21[1]);
		glVertex3dv(u21);

	}

	u21[0] = 0;
	u21[1] = 0;
	glTexCoord2d(u21[0], u21[1]);
	glVertex3dv(u21);
	//for (int g = 0; g < 1080; g = g + 15)
	//{
	//double param = 30.0;
	//	double x = length(middle(0, 6), middle(0, 12), 6, 12) * cos(g * PI / 180);
	//	double y = length(middle(0, 6), middle(0, 12), 6, 12) * sin(g * PI / 180);
	//	double F[] = { x, y, i };
	//	glVertex3dv(F);
	////glVertex3dv(u2);
	////glVertex3dv(u3);
	//
	//}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	


	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId2);
	glBegin(GL_QUAD_STRIP);
	//glNormal3d(normal(G, u2, u21, 1), normal(G, u2, u21, 2), normal(G, u2, u21, 3));
	glColor4d(0.2, 1,1, 0.8);
	u2[0]  = 6;
	u2[1]  = 12;
	u21[0] = 6;
	u21[1] = 12;
	glTexCoord2d(u2[0] * 2, 0);
	glVertex3dv(u2);
	glTexCoord2d(u2[0] * 2, 1);
	glVertex3dv(u21);
	double K[] = { 6, 12, 0 };
	int chet = 1;
	glNormal3d(normal(K, u2, u21, 1), normal(K, u2, u21, 2), normal(K, u2, u21, 3));
	for (i = 64; i < 244; i++) {

		u2[0] = length(middle(0, 6), middle(0, 12), 6, 12) * cos(i * PI / 180) + 3;
		u2[1] = length(middle(0, 6), middle(0, 12), 6, 12) * sin(i * PI / 180) + 6;
		

		u21[0] = length(middle(0, 6), middle(0, 12), 6, 12) * cos(i * PI / 180) + 3;
		u21[1] = length(middle(0, 6), middle(0, 12), 6, 12) * sin(i * PI / 180) + 6;
		glNormal3d(normal(K, u2, u21, 1), normal(K, u2, u21, 2), normal(K, u2, u21, 3));

			glTexCoord2d(u2[0] * 2, 0);
			glVertex3dv(u2);
			glTexCoord2d(u2[0] * 2, 1);
			glVertex3dv(u21);

		K[0] = u21[0];
		K[1] = u21[1];
		K[2] = u21[2];
	}
	u2[0] = 0;
	u2[1] = 0;
	u21[0] =0;
	u21[1] =0;
	glNormal3d(normal(K, u2, u21, 1), normal(K, u2, u21, 2), normal(K, u2, u21, 3));
	glTexCoord2d(u2[0] * 2, 0);
	glVertex3dv(u2);
	glTexCoord2d(u2[0] * 2, 1);
	glVertex3dv(u21);

	
	
	
	glEnd();
	glDisable(GL_TEXTURE_2D);
}


void trig2(double i, double j)
{
	double F1[] = { 9,6, i};
	double F2[] = { 9,6, j };
	double x = length(middle(0, 6), middle(0, 12), 6, 12) * cos(18.4 * PI / 180) * (-1);
	double y = length(middle(0, 6), middle(0, 12), 6, 12) * sin(18.4 * PI / 180);
	double u2[] = { 13, 0 , i };
	double u21[] = { 13, 0 ,j };
	glColor4d(1, 1, 0.2, 0.8);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId3);
	glBegin(GL_TRIANGLE_FAN);
	glNormal3d(0, 0, -1);
	glTexCoord2d(F1[0], F1[1]);
	glVertex3dv(F1);
	glTexCoord2d(u2[0], u2[1]);
	glVertex3dv(u2);
	for (i = 65; i <116; i++) {
		u2[0] = 14.97 * cos(i * PI / 180) + (6.5);
		u2[1] = 14.97 * sin(i * PI / 180) - 13.5;
		if (u2[1] < -0.1) {
			
		}else
			glTexCoord2d(u2[0], u2[1]);
		glVertex3dv(u2);

	}
	u2[0] = 0;
	u2[1] = 0;
	glTexCoord2d(u2[0], u2[1]);
	glVertex3dv(u2);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	u2[0] = 13;
	u2[1] = 0;
	u21[0] = 13;
	u21[1] = 0;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId1);
	glColor4d(1, 1, 0.5, 0.8);

	glBegin(GL_TRIANGLE_FAN);
	glNormal3d(0, 0, 1);
	glTexCoord2d(F2[0], F2[1]);
	glVertex3dv(F2);
	glTexCoord2d(u21[0], u21[1]);
	glVertex3dv(u21);
	for (i = 65; i < 116; i++) {
		u21[0] = 14.97 * cos(i * PI / 180) + (6.5);
		u21[1] = 14.97 * sin(i * PI / 180) - 13.5;
		if (u21[1] < -0.1) {

		} 
		else if (u21[0] < -0.1) {

		}
		else{
			glTexCoord2d(u21[0], u21[1]);
			glVertex3dv(u21);
		}
		

	}
	u21[0] = 0;
	u21[1] = 0;
	glTexCoord2d(u21[0], u21[1]);
	glVertex3dv(u21);
	glEnd();
	glDisable(GL_TEXTURE_2D);


	u2[0] = 13;
	u2[1] = 0;
	u21[0] = 13;
	u21[1] = 0;
	glColor4d(0.2, 0.2, 1, 0.8);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId2);
	glBegin(GL_QUAD_STRIP);
	glTexCoord2d(u2[0] * 2, 0);
	glVertex3dv(u2);
	glTexCoord2d(u2[0] * 2, 1);
	glVertex3dv(u21);
	double K[] = { 0, 0, 0 };
	K[0] = -1 * u21[0];
	K[1] = -1 * u21[1];
	K[2] = -1 * u21[2];
	glNormal3d(normal(K, u2, u21, 1), normal(K, u2, u21, 2), normal(K, u2, u21, 3));
	int chet = 1;
	for (i = 64; i < 116; i++) {
	
		u2[0] = 14.97 * cos(i * PI / 180) + (6.5);
		u2[1] = 14.97 * sin(i * PI / 180) - 13.5;
		u21[0] = 14.97 * cos(i * PI / 180) + (6.5);
		u21[1] = 14.97 * sin(i * PI / 180) - 13.5;
		if (u21[1] < 0) {
	
		}
		else if (u21[1] < 0) {
	
		}
		else{
			 glNormal3d(normal(K, u2, u21, 1), normal(K, u2, u21, 2), normal(K, u2, u21, 3)); 

				 glTexCoord2d(u2[0] * 2, 0);
				 glVertex3dv(u2);
				 glTexCoord2d(u2[0] * 2, 1);
				 glVertex3dv(u21);


		}
		K[0] = -1*u21[0];
		K[1] = -1*u21[1];
		K[2] = -1*u21[2];
	}
	u2[0] =   0;
	u2[1] =   0;
	u21[0] =  0;
	u21[1] =  0;
	glNormal3d(normal(K, u2, u21, 1), normal(K, u2, u21, 2), normal(K, u2, u21, 3));
	glTexCoord2d(u2[0] * 2, 0);
	glVertex3dv(u2);
	glTexCoord2d(u2[0] * 2, 1);
	glVertex3dv(u21);
	


	glEnd();
	glDisable(GL_TEXTURE_2D);
}



void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode) {
		glEnable(GL_TEXTURE_2D);

	}
		

	if (lightMode)
	{
		glEnable(GL_LIGHTING);

	}
		


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	

	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  
	glPushMatrix();
	
	Figure1(1, 4, 0.2, 0.7, 0.7);
	trig2(1, 4);
	//glTranslated(middle(0, 6), middle(0, 12), 0);//Задание №2
	trig(1, 4);								//Выпуклость 
		
	
	glPopMatrix();
	//////Начало рисования квадратика станкина
	//double A[] = { -4, -4,1 };
	//double B[] = { 4, -4,1 };
	//double C[] = { 4, 4,1 };
	//double D[] = { -4, 4,1 };
	//
	//glBindTexture(GL_TEXTURE_2D, texId);
	//
	//glColor3d(0.6, 0.6, 0.6);
	//glBegin(GL_QUADS);
	//
	//glNormal3d(0, 0, 1);
	//glTexCoord2d(0, 0);
	//glVertex3dv(A);
	//glTexCoord2d(1, 0);
	//glVertex3dv(B);
	//glTexCoord2d(1, 1);
	//glVertex3dv(C);
	//glTexCoord2d(0, 1);
	//glVertex3dv(D);
	//
	//glEnd();
	//////конец рисования квадратика станкина


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}