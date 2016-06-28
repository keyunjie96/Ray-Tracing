#include <fstream>
#include <iostream>
#include <string>
#include "Camera.h"
#include "Color.h"
#include "Light.h"
#include "Object.h"
#include "RayTrace.h"
#include "Scene.h"
#include "Vector3.h"
#include "RayTrace.h"

using namespace std;

const double infRay = 0.01;
const double epsilon = 1e-5;	

RayTrace::RayTrace()
{
	scene = new Scene;
}


RayTrace::~RayTrace()
{
	delete scene;
}

Color RayTrace::Trace(const Ray & ray, int depth)
{
	scene->isCalNearestObj = false;
	Object * nearestObj = scene->GetNearestObject(ray);

	Inter inter; Color color;
	if (nearestObj == NULL)
	{
		color  *= scene->GetBackgroundColor(ray);
	}
	else
	{
		//����һ�ݽ��㣬��������ߺ���л�ԭ
		color *= nearestObj->GetMaterialColor() * scene->GetLightColor();
		/******************************������������*******************************/
		if (depth < maxDepth)
		{
			/************************���㾵�淴����*************************/
			Ray reflRay = nearestObj->GetReflection(ray);
			if (nearestObj->material->reflectEnRatio > epsilon)
			{
				color += Trace(reflRay, depth + 1) * nearestObj->material->reflectEnRatio;
			}
			/**************************����͸����***********************************/
			if (nearestObj->material->refractEnRatio > epsilon)
			{
				Ray refrRay = nearestObj->GetRefracion(ray);
				color += Trace(refrRay, depth + 1) * nearestObj->material->refractEnRatio;
			}
		}
		return color;
	}
}
void RayTrace::Run()
{
	RunRange(0, scene->camera->Image_H, 0, scene->camera->Image_W, true);
}

void RayTrace::RunRange(int H0, int H1, int W0, int W1, bool output)
{
	for (int i = H0; i < H1; ++i)
	{
		cout << "sampling" << (int)output << "��" << i + 1 << endl;
		for (int j = W0; j < W1; ++j)
		{
			scene->camera->Picture[i][j] = Color(0, 0, 0);
			for (double ii = 0; ii < 1 - epsilon; ii += 1 / sampling)
			{
				for (double jj = 0; jj < 1 - epsilon; jj += 1 / sampling)
				{
					Color color = Trace(scene->camera->EmitRay(i + ii, j + jj), 0);
					scene->camera->Picture[i][j] += color;
				}
			}
			scene->camera->Picture[i][j] /= (sampling * sampling);
		}
	}

}

void RayTrace::Read(std::string fileName)
{
	ifstream fin(fileName.c_str());
	do 
	{
		char cbuffer[50]; fin >> cbuffer; string buffer(cbuffer);
		if (buffer == "Camera") scene->camera->read(fin);
		else if (buffer == "Object") scene->readObj(fin);
		else if (buffer == "Light") scene->readLight(fin);
		else if (buffer == "RayTrace")
		{
			while (true)
			{
				char cbuffer[50]; fin >> cbuffer; string buffer(cbuffer);
				if (buffer == "depth=")
				{
					fin >> maxDepth;
				}
				else if (buffer == "sampling=")
				{
					fin >> sampling;
				}
				else if (buffer == "end")
				{
					break;
				}
			}
		}
	} while (!fin.eof());
}

void RayTrace::Write(std::string fileName)
{
	scene->camera->write();
	scene->camera->bmp->Output(fileName);
}

Ray::Ray()
{
}

Ray::Ray(const Vector3 & lp, const Vector3 & rp)
	:launchPoint(lp), rayPath(rp.GetRegulation())
{
}

Ray::~Ray()
{
}

Ray Ray::GetNewTrace(const Vector3 & newTrace) const
{
	return Ray(launchPoint, newTrace);
}

std::ostream & operator<<(std::ostream & os, const Ray & rhs)
{
	os << "RayPath: " << rhs.rayPath;
	os << "LaunchPoint: " << rhs.launchPoint;
	return os;
}
