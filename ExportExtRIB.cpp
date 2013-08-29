//---------------------------------------------------------------------------
//
//   ExportRIB
//
//          Copyright(C) 1999-2012, tetraface Inc.
//
//      Sample of Export plug-in for exporting a RIB (RenderMan) file.
//      Created DLL must be installed in "Plugins\Export" directory.
//
//    　RIB(RenderMan)形式を出力するエクスポートプラグインのサンプル。
//    　作成したDLLは"Plugins\Export"フォルダに入れる必要がある。
//    　RenderManには詳しくないので、このサンプルではあまり多くの情報を出
//　　力していない。
//
//---------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "MQPlugin.h"
#include "FileLib.h"

#include <vector>
#include <cmath>
#include <cassert>
#include <algorithm>


#define PRODUCT_NUMBER 0xca9884d1
#define ID_NUMBER 0x1fc8


#ifndef PI
#define PI 3.1415926536f
#endif

#if defined(_MSC_VER)
#	if(_MSC_VER < 1400) //msc_ver < vc2005
#		define SNPRINTF _snprintf
#	else
#		define SNPRINTF _snprintf_s
#	endif
#else
#		define SNPRINTF snprintf
#endif


BOOL SaveRIB(const char *filename, MQDocument doc);


//---------------------------------------------------------------------------
//  DllMain
//---------------------------------------------------------------------------
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	//プラグインとしては特に必要な処理はないので、何もせずにTRUEを返す
    return TRUE;
}

//---------------------------------------------------------------------------
//  MQGetPlugInID
//    プラグインIDを返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT void MQGetPlugInID(DWORD *Product, DWORD *ID)
{
	// プロダクト名(制作者名)とIDを、全部で64bitの値として返す
	// 値は他と重複しないようなランダムなもので良い
	*Product = PRODUCT_NUMBER;
	*ID      =      ID_NUMBER;
}

//---------------------------------------------------------------------------
//  MQGetPlugInName
//    プラグイン名を返す。
//    この関数は[プラグインについて]表示時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT const char *MQGetPlugInName(void)
{
	// プラグイン名
	return "Extended RIB Exporter";
}

//---------------------------------------------------------------------------
//  MQGetPlugInType
//    プラグインのタイプを返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT int MQGetPlugInType(void)
{
	// 出力用プラグインである
	return MQPLUGIN_TYPE_EXPORT;
}

//---------------------------------------------------------------------------
//  MQEnumFileType
//    入力または出力可能なファイルタイプを返す。
//    ファイルタイプは別名保存時のダイアログに表示される。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT const char *MQEnumFileType(int index)
{
	switch(index){
	case 0: return "Extended RIB (*.rib)";
	}
	return NULL;
}

//---------------------------------------------------------------------------
//  MQEnumFileExt
//    入力または出力可能な拡張子を返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT const char *MQEnumFileExt(int index)
{
	switch(index){
	case 0: return "rib";
	}
	return NULL;
}

//---------------------------------------------------------------------------
//  MQExportFile
//    [別名で保存]で保存するときに呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT BOOL MQExportFile(int index, const char *filename, MQDocument doc)
{
	switch(index){
	case 0: return SaveRIB(filename, doc);
	}
	return FALSE;
}

static 
std::string ArrayToString(const float ar[], int n)
{
	char buffer[512];

	std::string sRet;
	sRet.reserve(n*32);
	
	sRet += "[";
	for(int i = 0;i<n;i++)
	{
		SNPRINTF(
			buffer,
			sizeof(buffer),
			"%f",
			ar[i]
		);

		sRet += buffer;
		if(i != n-1){
			sRet += " ";
		}
	}
	sRet += "]";
	return sRet;
}

static
std::string MatrixToString(const float m[16])
{
	return ArrayToString(m,16);
}

struct MeshGeom
{
	std::vector<int> materials;//face
	std::vector<int> nvertices;
	std::vector<int> vertices;
	std::vector<float> P;
	std::vector<float> N;	//facevarying
	std::vector<float> st;	//facevarying
};

static
float dot(const MQPoint& lhs, const MQPoint& rhs)
{
	return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

static
MQPoint cross(const MQPoint& lhs, const MQPoint& rhs)
{
	return MQPoint(
            lhs.y*rhs.z - lhs.z*rhs.y, //xyzzy
            lhs.z*rhs.x - lhs.x*rhs.z, //yzxxz
            lhs.x*rhs.y - lhs.y*rhs.x  //zxyyx
        );
}

static 
MQPoint normalize(const MQPoint& p)
{
	MQPoint tmp(p);
	tmp.normalize();
	return tmp;
}

static
MeshGeom* ConvertMeshGeom(MQObject obj)
{
	assert(obj);
	if(!obj)return NULL;
	
	int face_count = obj->GetFaceCount();
	int vert_count = obj->GetVertexCount();

	if(face_count<=0||vert_count<=0)return NULL;

	std::vector<int> nvertices(face_count);
	size_t total_vert=0;
	for(int i=0; i<face_count; i++)
	{
		nvertices[i]=obj->GetFacePointCount(i);
		total_vert += nvertices[i];
	}

	std::vector<int> vertices;
	vertices.reserve(total_vert);
	for(int i=0; i<face_count; i++)
	{
		int vert_index[4];
		obj->GetFacePointArray(i, vert_index);
		int count = nvertices[i];
		for(int vert=0; vert<count; vert++)
		{
			vertices.push_back(vert_index[vert]);
		}
	}

	std::vector<float> P(3*vert_count);
	for(int i=0;i<vert_count;i++)
	{
		MQPoint p = obj->GetVertex(i);
		P[3*i+0]=p.x;
		P[3*i+1]=p.y;
		P[3*i+2]=p.z;
	}
			
	std::vector<float> N(3*total_vert);
	{//normalize
		bool bSmooth = false;
		if(obj->GetShading()==MQOBJECT_SHADE_FLAT){
			bSmooth = false; 
		}else{
			bSmooth = true;
		}

		float a = obj->GetSmoothAngle();
		float limit_angle =  obj->GetSmoothAngle()/180.0f*PI;//
		float limit_cos   =  std::cos(limit_angle);
		std::vector<MQPoint> faceN(face_count);
		std::vector<MQPoint> vertN(vert_count);
		memset(&faceN[0],0,sizeof(MQPoint)*faceN.size());
		memset(&vertN[0],0,sizeof(MQPoint)*vertN.size());
		for(int i=0;i<face_count;i++)
		{
			int vert_index[4];
			obj->GetFacePointArray(i, vert_index);
			int count = nvertices[i];

			MQPoint fN = MQPoint(0,0,0);
			if(count == 3)
			{
				MQPoint p0 = obj->GetVertex(vert_index[0]);
				MQPoint p1 = obj->GetVertex(vert_index[1]);
				MQPoint p2 = obj->GetVertex(vert_index[2]);
				fN = normalize(cross(p1-p0,p2-p0));
			}
			else
			{//count == 4
				static const int WRAP[] = {0,1,2,3,0,1,2,3}; 
				MQPoint points[] = 
				{
					obj->GetVertex(vert_index[0]), 
					obj->GetVertex(vert_index[1]), 
					obj->GetVertex(vert_index[2]), 
					obj->GetVertex(vert_index[3]), 
				};
				for(int j=0;j<4;j++)
				{
					MQPoint p0 = obj->GetVertex(vert_index[WRAP[j+0]]);
					MQPoint p1 = obj->GetVertex(vert_index[WRAP[j+1]]);
					MQPoint p2 = obj->GetVertex(vert_index[WRAP[j+2]]);
					fN += normalize(cross(p1-p0,p2-p0));
				}
				fN = normalize(fN);
			}
			for(int vert=0; vert<count; vert++)
			{
				int idx = vert_index[vert];
				vertN[idx] += fN;
			}
			faceN[i] = fN;
		}
		for(int i=0;i<vert_count;i++)
		{
			vertN[i] = normalize(vertN[i]);
		}

		std::vector<MQPoint> NN(total_vert);
		int index = 0;
		for(int i=0;i<face_count;i++)
		{
			int vert_index[4];
			obj->GetFacePointArray(i, vert_index);
			int count = nvertices[i];
			MQPoint fN = faceN[i];
			for(int vert=0; vert<count; vert++)
			{
				if(bSmooth){
					MQPoint vN = vertN[vert_index[vert]];

					float cos_  = dot(fN, vN);

					MQPoint n =  (cos_<=limit_cos)?fN:vN;

					NN[index+vert] = n;
				}else{
					NN[index+vert] = fN;
				}
			}
			index += count;
		}

		for(int i=0;i<total_vert;i++)
		{
			N[3*i+0] = NN[i].x;
			N[3*i+1] = NN[i].y;
			N[3*i+2] = NN[i].z;
		}
	}

	std::vector<float> st(2*total_vert);
	{
		int index = 0;
		for(int i=0;i<face_count;i++)
		{
			MQCoordinate coords[4];
			obj->GetFaceCoordinateArray(i, coords);
			int count = nvertices[i];
			for(int vert=0; vert<count; vert++)
			{
				MQCoordinate c = coords[vert];
				st[2*index+0] = c.u;
				st[2*index+1] = c.v;
				index++;
			}
		}
	}

	std::vector<int> materials(face_count);
	for(int i=0;i<face_count;i++)
	{
		materials[i] = obj->GetFaceMaterial(i);
	}


	MeshGeom* mg = new MeshGeom();

	mg->nvertices.swap(nvertices);
	mg->vertices.swap(vertices);
	mg->P.swap(P);
	mg->N.swap(N);
	mg->st.swap(st);
	mg->materials.swap(materials);

	return mg;
}

static
void SplitMeshGeom(std::vector<MeshGeom*>& geoms, MeshGeom* mg)
{
	assert(mg);
	int face_count = mg->materials.size();
	bool bSingle = true;
	{
		int prevMaterial = mg->materials[0];
		for(int i=1;i<face_count;i++){
			if(prevMaterial!=mg->materials[i]){
				bSingle = false;
				break;
			}
		}
	}
	if(bSingle){
		geoms.push_back(mg);
	}else{
		int vertex_count = (int)(mg->P.size()/3);
		std::vector<int> materials = mg->materials;
		std::sort(materials.begin(), materials.end());
		materials.erase(std::unique(materials.begin(), materials.end()),materials.end());
		int split_count = (int)materials.size();
		std::vector<MeshGeom> tg(split_count);
		for(int k=0;k<split_count;k++)
		{
			int mat_id = materials[k];//

			std::vector<int> vrefs(vertex_count);
			memset(&vrefs[0], 0, sizeof(int)*vertex_count);

			{
				int index = 0;
				for(int i=0;i<face_count;i++){
					int count = mg->nvertices[i];
					if(mg->materials[i] == mat_id)
					{
						for(int j=0;j<count;j++)
						{
							int idx = mg->vertices[index+j];
							vrefs[idx]++;
						}
					}
					index += count;
				}
			}
			{
				int index = 0;
				for(int i=0;i<vertex_count;i++){
					if(vrefs[i]!=0)
					{
						vrefs[i] = index;
						index++;
					}else{
						vrefs[i] = -1;//
					}
				}
			}
			//
			{
				std::vector<int> nvertices;
				std::vector<int> vertices;
				std::vector<float> P;

				std::vector<float> N;
				std::vector<float> st;

				for(int i=0;i<vertex_count;i++){
					if(vrefs[i]>=0)
					{	
						P.push_back(mg->P[3*i+0]);
						P.push_back(mg->P[3*i+1]);
						P.push_back(mg->P[3*i+2]);
					}
				}

				int index = 0;
				for(int i=0;i<face_count;i++){
					int count = mg->nvertices[i];
					if(mg->materials[i] == mat_id)
					{
						nvertices.push_back(count);
						for(int j=0;j<count;j++)
						{
							int idx = mg->vertices[index+j];
							vertices.push_back(vrefs[idx]);

							int fvidx = index+j;

							N.push_back(mg->N[3*fvidx+0]);
							N.push_back(mg->N[3*fvidx+1]);
							N.push_back(mg->N[3*fvidx+2]);

							st.push_back(mg->st[2*fvidx+0]);
							st.push_back(mg->st[2*fvidx+1]);
						}
					}
					index += count;
				}

				tg[k].nvertices.swap(nvertices);
				tg[k].vertices.swap(vertices);
				tg[k].P.swap(P);
				tg[k].N.swap(N);
				tg[k].st.swap(st);
				tg[k].materials.push_back(mat_id);
			}
		}

		for(int k = 0;k<split_count;k++)
		{
			MeshGeom* g = new MeshGeom();
			g->nvertices.swap(tg[k].nvertices);
			g->vertices.swap(tg[k].vertices);
			g->P.swap(tg[k].P);
			g->N.swap(tg[k].N);
			g->st.swap(tg[k].st);
			g->materials.swap(tg[k].materials);

			geoms.push_back(g);
		}
		delete mg;
		mg = NULL;	
	}
}

static
void PrintMeshGeom(FILE* fh, MQDocument doc, const MeshGeom* mg)
{
	int mat_index = mg->materials[0];
	if(mat_index<0)mat_index=0;

	MQMaterial mat = doc->GetMaterial(mat_index);

	fprintf(fh, "AttributeBegin\n");


	char name[64] = {};
	mat->GetName(name, sizeof(name));
	//fprintf(fh, "#  %s\n", name);

	MQColor color = mat->GetColor();
	fprintf(fh, "Color  [ %.3f %.3f %.3f ]\n", color.r, color.g, color.b);		

	fprintf(fh, "PointsPolygons");
	fprintf(fh, " [");
	for(size_t i=0;i<mg->nvertices.size();i++){
		fprintf(fh, " %d", mg->nvertices[i]);
	}
	fprintf(fh, " ]");
	fprintf(fh, "\n");

	fprintf(fh, " [");
	for(size_t i=0;i<mg->vertices.size();i++){
		fprintf(fh, " %d", mg->vertices[i]);
	}
	fprintf(fh, " ]");
	fprintf(fh, "\n");

	fprintf(fh, " \"P\"");
	fprintf(fh, " [");
	for(size_t i=0;i<mg->P.size();i++){
		fprintf(fh, " %f", mg->P[i]);
	}
	fprintf(fh, " ]");
	fprintf(fh, "\n");


	fprintf(fh, " \"facevarying normal N\"");
	fprintf(fh, " [");
	for(size_t i=0;i<mg->N.size();i++){
		fprintf(fh, " %f", mg->N[i]);
	}
	fprintf(fh, " ]");
	fprintf(fh, "\n");

	fprintf(fh, " \"facevarying float[2] st\"");
	fprintf(fh, " [");
	for(size_t i=0;i<mg->st.size();i++){
		fprintf(fh, " %f", mg->st[i]);
	}
	fprintf(fh, " ]");
	fprintf(fh, "\n");


	fprintf(fh, "AttributeEnd\n");
	fprintf(fh, "\n");


}

// RIB形式で出力
BOOL SaveRIB(const char *filename, MQDocument doc)
{
	int i, face, mat_index;
	MQMaterial mat;
	MQFileDialogInfo dlginfo;
	char path[MAX_PATH], name[64];

	// 座標軸変換用ダイアログの表示
	memset(&dlginfo, 0, sizeof(dlginfo));
	dlginfo.dwSize = sizeof(dlginfo);
	dlginfo.axis_x = MQFILE_TYPE_RIGHT;
	dlginfo.axis_y = MQFILE_TYPE_UP;
	dlginfo.axis_z = MQFILE_TYPE_FRONT;
	dlginfo.softname = "RenderMan";
	//MQ_ShowFileDialog("Extended RIB Exporter", &dlginfo);

	// ファイルを作成・オープンする
	FILE *fh;
	errno_t ret_s = fopen_s(&fh, filename, "w");
	if(ret_s != 0)
		return FALSE;

	// 複数のオブジェクトを１つに合成するために、作業用オブジェクトを作成する
	MQObject obj = MQ_CreateObject();

	// 作業用オブジェクトにすべてのオブジェクトを合成
	int objcount = doc->GetObjectCount();

	std::vector<MQObject> objects;

	for(i=0; i<objcount; i++)
	{
		MQObject thisobj = doc->GetObject(i);
		if(thisobj == NULL) 
			continue;

		DWORD nVisible = thisobj->GetVisible();
		if(!nVisible)
			continue;

		int face_count = thisobj->GetFaceCount();
		if(face_count<=0)
			continue;

		// フリーズなどにより内容が変更されてしまうので、オブジェクトを複製しておく
		thisobj = thisobj->Clone();
		// 曲面などの属性をフリーズ
		thisobj->Freeze(MQOBJECT_FREEZE_ALL);
		objects.push_back(thisobj);
	}

	std::vector<MeshGeom*> geoms;
	{
		std::vector<MeshGeom*> tmp;
		for(int i=0;i<objects.size();i++){
			tmp.push_back(ConvertMeshGeom(objects[i]));
			objects[i]->DeleteThis();
		}
		for(int i=0;i<tmp.size();i++){
			SplitMeshGeom(geoms, tmp[i]);
		}
	}

	float fFov = doc->GetScene(0)->GetFOV() / PI * 180.0f ;//radians
	MQPoint pos = doc->GetScene(0)->GetCameraPosition();
	MQAngle angle = doc->GetScene(0)->GetCameraAngle();//hpb


	float viewMatrix[16];
	doc->GetScene(0)->GetViewMatrix(viewMatrix);

	float projMatrix[16];
	doc->GetScene(0)->GetProjMatrix(projMatrix);

	HWND hw = MQ_GetWindowHandle();
	RECT rect;
	GetClientRect(hw, &rect);
	int w = rect.right-rect.left;
	int h = rect.bottom-rect.top;
	fprintf(fh, "#rect = %d %d \n", w, h);

	
	//MQMatrix m(viewMatrix);
	//m = inverse(m);

	// ファイルの先頭にデータを書き込み
	fprintf(fh, "##RenderMan RIB-Structure 1.0\n");
	fprintf(fh, "version 3.03\n");
	fprintf(fh, "\n");

	
	
	fprintf(fh, "Option \"searchpath\" \"shader\" [\".:&\"]\n");
	fprintf(fh, "PixelSamples 2 2\n");
	GetFileNameFromPath(path, filename);
	fprintf(fh, "Display \"%s.tif\" \"file\" \"rgba\"\n", path);
	fprintf(fh, "Display \"+%s.tif\" \"framebuffer\" \"rgba\"\n", path);
	fprintf(fh, "Projection \"perspective\" \"fov\" [%f]\n", fFov);

	fprintf(fh, "#from %f %f %f\n", pos.x,pos.y, pos.z);
	fprintf(fh, "Transform %s\n", MatrixToString(viewMatrix).c_str());
	fprintf(fh, "#Transform %s\n", MatrixToString(projMatrix).c_str());

	fprintf(fh, "\n");

	fprintf(fh, "WorldBegin\n");
	fprintf(fh, "\n");
	fprintf(fh, "LightSource \"ambientlight\" 1 \"intensity\" 0.08\n");
	fprintf(fh, "\n");
	fprintf(fh, "Declare \"shadows\" \"string\"\n");
	fprintf(fh, "Attribute \"light\" \"shadows\" \"on\"\n");
	fprintf(fh, "LightSource \"distantlight\" 2 \"from\" [0 1 -4] \"to\" [0 0 0] \"intensity\" 0.8\n");
	fprintf(fh, "\n");

	//
	for(int i=0;i<geoms.size();i++)
	{
		PrintMeshGeom(fh, doc, geoms[i]);
		delete geoms[i];
	}

	// ファイルの終端のデータを書き込み
	fprintf(fh, "WorldEnd\n");
	fprintf(fh, "\n");

	// ファイルをクローズ
	fclose(fh);

	return TRUE;
}

