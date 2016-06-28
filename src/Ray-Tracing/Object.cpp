#include <algorithm>
#include "bmp.h"
#include "Object.h"
#include "Vector3.h"
#include <assert.h>
#include <cmath>
#include <iostream>

using namespace std;
const double pi = 3.1415;
const double epsilon = 1e-5;

Object::Object()
{
	material = new Material;
}


Object::~Object()
{
}

bool Object::objectRead(std::string flag, std::ifstream & fin)
{
	if (flag == "picture=")
	{
		fin >> flag;
		if (material->bmp) delete material->bmp;
		material->bmp = new Bmp;
		material->bmp->Input(flag);
	}
	else if (flag == "color=")
	{
		fin >> material->color.b >> material->color.g >> material->color.r;
	}
	else if (flag == "refractRatio=")
	{
		fin >> material->refractRatio;
	}
	else if (flag == "reflectEnRatio=")
	{
		fin >> material->reflectEnRatio;
	}
	else if (flag == "difReflectEnRatio=")
	{
		fin >> material->difReflectEnRatio;
	}
	else if (flag == "difSpecEnRatio=")
	{
		fin >> material->difSpecEnRatio;
	}
	else if (flag == "refractEnRatio=")
	{
		fin >> material->refractEnRatio;
	}
	else if (flag == "end")
	{
		return false;
	}
	return true;
}

Sphere::Sphere()

{
}


Sphere::~Sphere()
{
}

void Sphere::read(std::ifstream & fin)
{
	char cbuffer[50]; string buffer;
	do
	{
		 fin >> cbuffer; buffer = string(cbuffer);
		if (buffer == "x=")
		{
			center.read(fin);
		}
		else if (buffer == "radius=")
		{
			fin >> radius;
		}
	} while (objectRead(buffer, fin));
}

Color Sphere::GetMaterialColor()
{
	if (material->bmp)
	{
		double u = (atan2((inter.interPosi.y - center.y) / radius, (inter.interPosi.x - center.x) / radius)  + pi) / (pi);
		double v = (asin((inter.interPosi.z - center.z) / radius) + pi / 2) / (pi);
		Color ret = material->bmp->GetSmoothColor(u, v) / 2;
		return ret;
	}
	else
	{
		return material->color;
	}
}

bool Sphere::CalIntersect(const Ray & ray)
{
	
	Vector3 connect = center - ray.launchPoint;
		double centerVerticalFootDist = connect.DotProduct(ray.rayPath);
		Vector3 centerVerticalFoot = ray.launchPoint + ray.rayPath * centerVerticalFootDist;
		double connectrayDist = center.GetDist(centerVerticalFoot);
		if (centerVerticalFootDist > epsilon && connectrayDist < radius)
		{
			double backDist = sqrt(pow(radius, 2) - pow(connectrayDist, 2));
			if (fabs(ray.launchPoint.GetDist(center) - radius) < epsilon) //���ߵ���Բ��
				backDist = -backDist;
			inter.interDist = centerVerticalFootDist - backDist;
			if (fabs(inter.interDist) < epsilon)
			{
				inter.hasInterSection = false;
			}
			else
			{
				Vector3 nearestIntersection = ray.launchPoint + ray.rayPath * (centerVerticalFootDist - backDist);
				inter.SetInterPosi(nearestIntersection, ray);
				inter.axis = (inter.interPosi - center).GetRegulation();
			}
		}
		else
			inter.hasInterSection = false;
	return inter.hasInterSection;
}

Material::Material()
{
	refractRatio = 0;
	reflectEnRatio = difReflectEnRatio = difSpecEnRatio = refractEnRatio = 0;
}


Material::~Material()
{
}

Inter::Inter()
{
}


Inter::~Inter()
{
}

void Inter::SetInterPosi(const Vector3 & posi, const Ray & ray)
{
	interPosi = posi;
	interDist = interPosi.GetDist(ray.launchPoint);
	hasInterSection = true;
}

Plane::Plane()
{
}


Plane::~Plane()
{
}

void Plane::read(std::ifstream & fin)
{
	char cbuffer[50]; string buffer;
	do
	{
		fin >> cbuffer; buffer = string(cbuffer);
		if (buffer == "x=")
		{
			angle.read(fin); fin >> d;
			angle = angle.GetRegulation();
			inter.axis = angle;
		}
		else if (buffer == "picAxis=")
		{
			material->bmpAxis1.read(fin);
			material->bmpAxis2.read(fin);
		}
	} while (objectRead(buffer, fin));
}

Color Plane::GetMaterialColor()
{
	if (material->bmp)
	{
		Color ret;
		ret = material->bmp->GetColor(abs((int)inter.interPosi.DotProduct(material->bmpAxis1) % material->bmp->GetH()), abs((int)inter.interPosi.DotProduct(material->bmpAxis2) % material->bmp->GetW())) / 2;
		return ret;
	}
	else
	{
		return material->color;
	}
}

bool Plane::CalIntersect(const Ray & ray)
{
	double t = -(d + inter.axis.DotProduct(ray.launchPoint)) / (inter.axis.DotProduct(ray.rayPath));
	if (t > epsilon)
	{
		inter.SetInterPosi(ray.launchPoint + t * ray.rayPath, ray);
		inter.hasInterSection = true;
	}
	else
		inter.hasInterSection = false;
	return inter.hasInterSection;
}

Vector3 Object::GetIntersection()
{
	return inter.interPosi;
}

Ray Object::GetRefracion(const Ray & ray) const
{
	return Ray(inter.interPosi, ray.rayPath.GetRefraction(inter.axis, material->refractRatio));
}

Ray Object::GetReflection(const Ray & ray) const
{
	return Ray(inter.interPosi, ray.rayPath.GetReflection(inter.axis));
}


Mesh::Mesh()
{
}


Mesh::~Mesh()
{
}

Mesh::Mesh(const Mesh & mesh)
	//:Object(mesh)
{
	(*this) = mesh;
}

void Mesh::read(std::ifstream & fin)
{

}

Color Mesh::GetMaterialColor()
{
	return Color();
}

bool Mesh::CalIntersect(const Ray & ray)
{
	double t = -(d + inter.axis.DotProduct(ray.launchPoint)) / (inter.axis.DotProduct(ray.rayPath));
	if (t > epsilon)
	{
		inter.SetInterPosi(ray.launchPoint + t * ray.rayPath, ray);
		double sign[3];
		for (int i = 0; i < 3; ++i)
		{
			sign[i] = ((inter.interPosi - points[i])*(edges[i])).DotProduct(inter.axis);
			//���ڱ�����ֱ����
			if (sign[i] > -epsilon && sign[i] < epsilon)
			{
				//�㲻�ڱ��ϣ�ͬ����
				if ((inter.interPosi[0] - points[i][0])*(inter.interPosi[0] - points[i][0]) > epsilon)
				{
					inter.hasInterSection = false;
					return inter.hasInterSection;
				}
			}
		}
		//������Ƿ�����Ƭ���������������ڲ�
		int a = 1, b = 1;
		for (int i = 0; i < 3; ++i)
		{
			a *= (sign[i] > epsilon);
			b *= (sign[i] < -epsilon);
		}
		inter.hasInterSection = (a || b);
	}
	else
		inter.hasInterSection = false;
	return inter.hasInterSection;
}

Model::Model()
{

}


Model::~Model()
{
}

void Model::read(std::ifstream & fin)
{
	char cbuffer[50]; string buffer;
	do
	{
		fin >> cbuffer; buffer = string(cbuffer);
		if (buffer == "x=")
		{
			offset.read(fin);
		}
		else if (buffer == "largeRatio=")
		{
			fin >> largeRatio;
		}
		else if (buffer == "obj=")
		{
			string fileName; 
			fin >> fileName; GetMehes(fileName);
			buildKDTree();
		}
	} while (objectRead(buffer, fin));
}

void Model::GetMehes(std::string fileName)
{
	meshes.clear();
	vector<Vector3> points;
	ifstream fin(fileName.c_str());
	char flag;
	while (!fin.eof())
	{
		fin >> flag; if (fin.eof()) break; 
		if (flag == 'v')
		{
			//��ȡ������
			Vector3 point;
			point.read(fin);
			point *= largeRatio;
			point += offset;
			points.push_back(point);
		}
		else if (flag == 'f')
		{
			//��ȡ��Ƭ
			Mesh mesh;
			for (int i = 0; i < 3; i++)
			{
				int index; fin >> index;
				mesh.points.push_back(points[index - 1]);
			}
			//cout << endl;
			//����Ƭ������
			mesh.cg = Vector3(0, 0, 0);
			for (int i = 0; i < 3; ++i)
			{
				mesh.cg += mesh.points[i];
			}
			mesh.cg /= 3;
			//����Ƭ�ı�
			for (int i = 0; i < 3; ++i)
			{
				mesh.edges[i] = mesh.points[(i + 1) % 3] - mesh.points[i];
			}
			//����Ƭ��������
			Vector3 edge_1 = mesh.points[1] - mesh.points[0];
			Vector3 edge_2 = mesh.points[2] - mesh.points[0];
			mesh.inter.axis = (edge_1 * edge_2).GetRegulation();
			//����Ƭ��ƫ��
			mesh.d = -mesh.inter.axis.DotProduct(mesh.points[0]);
			meshes.push_back(mesh);
		}
	}
}

Color Model::GetMaterialColor()
{
	return material->color;
}

bool Model::CalIntersect(const Ray & ray)
{
	std::vector<int> suspectedMeshesIndex;
	searchNode(0, ray, suspectedMeshesIndex);
	//����
	sort(suspectedMeshesIndex.begin(), suspectedMeshesIndex.end());
	auto unique_end = unique(suspectedMeshesIndex.begin(), suspectedMeshesIndex.end());
	suspectedMeshesIndex.erase(unique_end, suspectedMeshesIndex.end());
	//TODO�����ڶ����Ƭ�������㣬�������ֵ
	int nearestIndex = -1; double nearestDist = INT_MAX;
	for (auto index : suspectedMeshesIndex)
	{
		if (meshes[index].CalIntersect(ray))
		{
			if (nearestDist > meshes[index].inter.interDist)
			{
				nearestIndex = index; nearestDist = meshes[index].inter.interDist;
			}
		}
	}
	if (nearestIndex >= 0 && meshes[nearestIndex].inter.interDist > epsilon)
	{
		inter = meshes[nearestIndex].inter;
	}
	else
	{
		inter.hasInterSection = false;
	}
	return inter.hasInterSection;
	//HINT���б�Ҫ�������Ը���ģ�ͣ���֤������Ƭ�����������ȷ�ԣ����Ž���KDTree�Ĳ���

}

void Model::buildKDTree()
{
	nodes.clear();
	Node root; nodes.push_back(root);
	for (int i = 0; i < meshes.size(); ++i)
		nodes[0].meshesIndex.push_back(i);
	//ȷ��������Χ�д�С
	Vector3 lbOffset(INT_MAX, INT_MAX, INT_MAX), rtOffset(-INT_MAX, -INT_MAX, -INT_MAX);
	for (int i = 0; i < nodes[0].meshesIndex.size(); ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				if (meshes[nodes[0].meshesIndex[i]].points[j][k] < lbOffset[k])
					lbOffset[k] = meshes[nodes[0].meshesIndex[i]].points[j][k];
				if (meshes[nodes[0].meshesIndex[i]].points[j][k] > rtOffset[k])
					rtOffset[k] = meshes[nodes[0].meshesIndex[i]].points[j][k];
			}
		}
	}
	nodes[0].box = Box(lbOffset, rtOffset - lbOffset);
	buildSubKDTree(0, 0, nodes[0].meshesIndex, meshes);
}

void Model::buildSubKDTree(int rootIndex, int axis, std::vector<int>& meshesIndex, std::vector<Mesh> & meshes)
{
	//Ѱ����λ��
	nth_element(meshesIndex.begin(), meshesIndex.begin() + meshesIndex.size() / 2, meshesIndex.end(), [axis, &meshes](int lhs, int rhs) {return meshes[lhs].cg[axis] < meshes[rhs].cg[axis]; });
	std::vector<int> lhsMeshIndex; 	std::vector<int> rhsMeshIndex;
	bool allSame = true;
	Node lhsNode, rhsNode;
	bool flag = false;
	for (int i = 0; i < meshesIndex.size() / 2; ++i)
	{
		lhsMeshIndex.push_back(meshesIndex[i]);
		if (meshesIndex[i] == 4572) {
		}
		//�����Ƭ���������Χ�е������Ҫ�����е��ڸ���ķ�����Ӧ��С����λ������ڸ���ķ���
		for (int j = 0; j < 3; ++j)
		{
			if (meshes[meshesIndex[i]].points[j][axis] >= meshes[meshesIndex[meshesIndex.size() / 2]].cg[axis])
			{
				rhsMeshIndex.push_back(meshesIndex[i]);
				break;
			}
			else
			{
				allSame = false;
			}
		}
	}
	for (int i = meshesIndex.size() / 2; i < meshesIndex.size(); ++i)
	{
		rhsMeshIndex.push_back(meshesIndex[i]);
		for (int j = 0; j < 3; ++j)
		{
			if (meshes[meshesIndex[i]].points[j][axis] <= meshes[meshesIndex[meshesIndex.size() / 2]].cg[axis])
			{
				lhsMeshIndex.push_back(meshesIndex[i]);
				break;
			}
		}
	}
	//������ҽ����ͬ���ٻ���
	//WARNING������ܹ��ֵø�ϸ��Ҳ��������Ч�ʣ������ܻ���ڲ��ֽ�������
	if (allSame || lhsMeshIndex.size() == meshesIndex.size() || rhsMeshIndex.size() == meshesIndex.size())
	{
		return;
	}
	else
	{
		//if(flag) cout << endl;
		//�������� ȷ���������ӹ�ϵ
		Node lhsNode, rhsNode;
		lhsNode.parent = rootIndex;  rhsNode.parent = rootIndex;
		nodes[rootIndex].leftChild = nodes.size();
		nodes[rootIndex].rightChild = nodes.size() + 1;
		lhsNode.meshesIndex = lhsMeshIndex; rhsNode.meshesIndex = rhsMeshIndex;
		//ȷ�����Ӱ�Χ�еĴ�С
		lhsNode.box = rhsNode.box = nodes[rootIndex].box;
		rhsNode.box.offset[axis] = meshes[meshesIndex[meshesIndex.size() / 2]].cg[axis];
		lhsNode.box.length[axis] = meshes[meshesIndex[meshesIndex.size() / 2]].cg[axis] - lhsNode.box.offset[axis]; rhsNode.box.length[axis] -= lhsNode.box.length[axis];
		nodes.push_back(lhsNode); nodes.push_back(rhsNode);
		buildSubKDTree(nodes[rootIndex].leftChild, (axis + 1) % 3, lhsMeshIndex, meshes);
		buildSubKDTree(nodes[rootIndex].rightChild, (axis + 1) % 3, rhsMeshIndex, meshes);
	}

}

bool Model::searchNode(int nodeIndex, const Ray & ray, std::vector<int> & suspectedMeshesIndex)
{
	double lhsDist = INT_MAX, rhsDist = INT_MAX;
	//Ҷ�ӽ�㣬�洢ȫ����Ƭ��Ϣ //WARNING�����ﲻ��Ϊ������Ҷ�ӽ��
	if (nodes[nodeIndex].leftChild == -1 && nodes[nodeIndex].rightChild == -1)
	{
		for (int i = 0; i < nodes[nodeIndex].meshesIndex.size(); ++i)
		{
			suspectedMeshesIndex.push_back(nodes[nodeIndex].meshesIndex[i]);
		}
	}
	//����
		//������ҽ��Ľ������
	if (nodes[nodeIndex].leftChild >= 0 && nodes[nodes[nodeIndex].leftChild].box.CalIntersect(ray))
	{
		lhsDist = nodes[nodes[nodeIndex].leftChild].box.inter.interDist;
	}
	if (nodes[nodeIndex].rightChild >= 0 && nodes[nodes[nodeIndex].rightChild].box.CalIntersect(ray))
	{
		rhsDist = nodes[nodes[nodeIndex].rightChild].box.inter.interDist;
	}
	//WARNING��return�ڵ�һ������������
	if (lhsDist == INT_MAX && rhsDist == INT_MAX)
	{
		return false;
	}
	else
	{
		//WARNING����dist��һ����Ϊ��
		//�󲿽�����������������ߵĽ�㣻����ֹͣ����û���������ұߵ�
		bool flag1 = false, flag2 = false;
		if (lhsDist - rhsDist > -epsilon)
		{
			if (!searchNode(nodes[nodeIndex].rightChild, ray, suspectedMeshesIndex))
			{
				return searchNode(nodes[nodeIndex].leftChild, ray, suspectedMeshesIndex);
			}
			else
			{
				return true;
			}
		}
		if (lhsDist - rhsDist < epsilon)
		{
			if (!searchNode(nodes[nodeIndex].leftChild, ray, suspectedMeshesIndex))
			{
				return searchNode(nodes[nodeIndex].rightChild, ray, suspectedMeshesIndex);
			}
			else
			{
				return true;
			}
		}
		return true;
		//return flag1 || flag2;
		assert(1 == 0);
		//return true;
	}
}

Node::Node()
{
	leftChild = rightChild = parent = -1;
}


Node::~Node()
{
}

Box::Box()
{
}

Box::Box(Vector3 offset_, Vector3 length_)
	:offset(offset_), length(length_)
{
}


Box::~Box()
{
}

void Box::read(std::ifstream & fin)
{
	char cbuffer[50]; string buffer;
	do
	{
		fin >> cbuffer; buffer = string(cbuffer);
		if (buffer == "x=")
		{
			offset.read(fin);
		}
		else if (buffer == "length=")
		{
			length.read(fin);
		}
	} while (objectRead(buffer, fin));
}

Color Box::GetMaterialColor()
{
	return material->color;
}

bool Box::CalIntersect(const Ray & ray)
{

	double cmp[2][3];// = { INT_MAX,INT_MAX,INT_MAX };
	bool otherSide[3]; int isInner = 0; 
	//�ҵ�����Ľ���
	for (int i = 0; i < 2; ++i) 
	{
		for (int j = 0; j < 3; ++j)
		{
			if (fabs(ray.rayPath[j]) < epsilon)
			{
				cmp[i][j] = -INT_MAX;
				continue;
			}
			else
			{
				cmp[i][j] = (offset[j] + i * length[j] - ray.launchPoint[j]) / ray.rayPath[j];
			}
		}
	}
	//����Ϊ3ʱ�����ڲ���������Ըı�
	for (int i = 0; i < 3; ++i)
	{
		isInner += cmp[0][i] * cmp[1][i] < 0;
	}
	for (int i = 0; i < 2; ++i) 
	{
		for (int j = 0; j < 3; ++j)
		{
			if(isInner <  3)
			{
				cmp[0][j] = cmp[0][j] > cmp[1][j] ? cmp[1][j] : cmp[0][j];
			}
			else
			{
				cmp[0][j] = cmp[0][j] > epsilon ? cmp[0][j] : cmp[1][j];
			}
		}
	}
	int index; double suspectedLength;
	//���������ҳ����ֵ��Ϊ�����ཻ�ĵ�
	if (isInner < 3)
	{
		suspectedLength = -INT_MAX;
		for (int i = 0; i < 3; ++i)
		{
			if (suspectedLength < cmp[0][i])
			{
				index = i; suspectedLength = cmp[0][i];
			}
		}
	}
	//��������Сֵ�����ڲ����
	else
	{
		suspectedLength = INT_MAX;
		for (int i = 0; i < 3; ++i)
		{
			if (suspectedLength > cmp[0][i])
			{
				index = i; suspectedLength = cmp[0][i];
			}
		}
	}
	//��⽻���Ƿ��ڹ��ߵ�����
	if (suspectedLength < -epsilon)//*
	{
		inter.hasInterSection = false;
	}
	else
	{
		inter.hasInterSection = true;
		//������ܵĽ���
		inter.SetInterPosi(ray.launchPoint + suspectedLength * ray.rayPath,ray);
		int sign_1 = otherSide[index] ? 1 : -1;
		int sign_2 = length[index] > epsilon ? 1 : -1;
		inter.axis = Vector3(0, 0, 0); inter.axis[index] = sign_1 * sign_2;

		//����������Ƿ���Լ��������
		for (int i = 1; i <= 2; ++i) //**
		{
			int ii = (index + i) % 3;
			double value = -offset[ii] + ray.launchPoint[ii] + suspectedLength * ray.rayPath[ii];
			int sign = length[ii] > epsilon ? 1 : -1;
			if (!(value * sign > epsilon && value * sign < length[ii] * sign))
				inter.hasInterSection = false;
		}
	}
	return inter.hasInterSection;
}

Box_::Box_()
{
}

Box_::Box_(Vector3 offset_, Vector3 length_)
	:offset(offset_), length(length_)
{
}

bool Box_::CalIntersect(const Ray & ray)
{
	double cmp[3] = { INT_MAX,INT_MAX,INT_MAX };
	bool otherSide[3];
	//�ҵ�����Ľ���
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			if (fabs(ray.rayPath[j]) < epsilon)
			{
				cmp[j] = -INT_MAX;
				continue;
			}
			else
			{
				double t = (offset[j] + i * length[j] - ray.launchPoint[j]) / ray.rayPath[j];
				if (t < cmp[j])
				{
					otherSide[j] = i;
					cmp[j] = t;
				}
			}
		}
	}
	int index; double maximum = -INT_MAX;
	//���������ҳ����ֵ��Ϊ�����ཻ�ĵ�
	for (int i = 0; i < 3; ++i)
	{
		if (maximum < cmp[i])
		{
			index = i; maximum = cmp[i];
		}
	}
	bool flag = true;
	//��⽻���Ƿ��ڹ��ߵ�����
	if (maximum < -epsilon)//*
	{
		flag = false;
	}
	//����������Ƿ���Լ��������
	for (int i = 0; i <= 2; ++i)
	{
		int ii = (index + i) % 3;
		double value = -offset[ii] + ray.launchPoint[ii] + maximum * ray.rayPath[ii];
		int sign = length[ii] > epsilon ? 1 : -1;
		if (!(value * sign > epsilon && value * sign < length[ii] * sign))
			flag = false;
	}
	return flag;
}
