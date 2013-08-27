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

#define PRODUCT_NUMBER 0xca9884d1
#define ID_NUMBER 0x1fc8


#ifndef PI
#define PI 3.1415926536f
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
	case 0: return "RenderMan (*.rib)";
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
	dlginfo.axis_z = MQFILE_TYPE_BACK;
	dlginfo.softname = "RenderMan";
	MQ_ShowFileDialog("Extended RIB Exporter", &dlginfo);

	// ファイルを作成・オープンする
	FILE *fh;
	errno_t ret_s = fopen_s(&fh, filename, "w");
	if(ret_s != 0)
		return FALSE;

	// 複数のオブジェクトを１つに合成するために、作業用オブジェクトを作成する
	MQObject obj = MQ_CreateObject();

	// 作業用オブジェクトにすべてのオブジェクトを合成
	int objcount = doc->GetObjectCount();
	for(i=0; i<objcount; i++)
	{
		MQObject thisobj = doc->GetObject(i);
		if(thisobj == NULL) 
			continue;
		// フリーズなどにより内容が変更されてしまうので、オブジェクトを複製しておく
		thisobj = thisobj->Clone();
		// 曲面などの属性をフリーズ
		thisobj->Freeze(MQOBJECT_FREEZE_ALL);
		// 作業用オブジェクトに合成する
		obj->Merge(thisobj);
		// 複製したオブジェクトはもう不要なので破棄
		thisobj->DeleteThis();
	}

		
	/*
			MQPoint GetCameraPosition();
			MQAngle GetCameraAngle();
			MQPoint GetLookAtPosition();
			MQPoint GetRotationCenter();
			float GetFOV();
	*/

	float fFov = doc->GetScene(0)->GetFOV() / PI * 180.0f ;//radians
	MQPoint pos = doc->GetScene(0)->GetCameraPosition();
	MQAngle angle = doc->GetScene(0)->GetCameraAngle();//hpb




	// 面と頂点の数を取得
	int face_count = obj->GetFaceCount();
	int vert_count = obj->GetVertexCount();

	// ファイルの先頭にデータを書き込み
	fprintf(fh, "##RenderMan RIB-Structure 1.0\n");
	fprintf(fh, "version 3.03\n");
	fprintf(fh, "\n");

	fprintf(fh, "Option \"searchpath\" \"shader\" [\".:&\"]\n");
	GetFileNameFromPath(path, filename);
	fprintf(fh, "Display \"%s.tif\" \"file\" \"rgba\"\n", path);
	fprintf(fh, "Projection \"perspective\" \"fov\" [%f]\n", 45.0);


	fprintf(fh, "Translate %f %f %f\n", 0, 0, 1000);
	//fprintf(fh, "Rotate 1 0 0 %f\n", angle.head);
	//fprintf(fh, "Rotate 0 1 0 %f\n", angle.pitch);
	//fprintf(fh, "Rotate 0 0 1 %f\n", angle.bank);

	fprintf(fh, "\n");

	fprintf(fh, "WorldBegin\n");
	fprintf(fh, "\n");
	fprintf(fh, "LightSource \"ambientlight\" 1 \"intensity\" 0.08\n");
	fprintf(fh, "\n");
	fprintf(fh, "Declare \"shadows\" \"string\"\n");
	fprintf(fh, "Attribute \"light\" \"shadows\" \"on\"\n");
	fprintf(fh, "LightSource \"distantlight\" 1 \"from\" [0 1 -4] \"to\" [0 0 0] \"intensity\" 0.8\n");
	fprintf(fh, "\n");

	// それぞれの材質がいくつの面に使われているかどうかを調べておく
	int mat_count = doc->GetMaterialCount();
	int *mat_used = (int *)malloc(sizeof(int) * (mat_count+1));
	for(i=0; i<=mat_count; i++)
		mat_used[i] = 0;
	for(face=0; face<face_count; face++)
	{
		mat_index = obj->GetFaceMaterial(face);
		mat_used[(mat_index>=0) ? mat_index : mat_count]++; // 未着色面の場合は配列の最後にデータをセット
	}

	// 材質ごとにAttributeとしてポリゴン情報を記録する
	for(mat_index=0; mat_index<=mat_count; mat_index++)
	{
		int mi;

		// この材質が使われていなければパス
		if(mat_used[mat_index] == 0)
			continue;

		if(mat_index == mat_count) {
			mat = MQ_CreateMaterial(); // 未着色面の場合は一時的に材質を作成
			mi = -1;
		} else {
			mat = doc->GetMaterial(mat_index);
			mi = mat_index;
		}
	
		fprintf(fh, "AttributeBegin\n");

		// コメントとして材質名を記録しておく
		mat->GetName(name, sizeof(name));
		fprintf(fh, "#  %s\n", name);

		MQColor color = mat->GetColor();
		fprintf(fh, "  Color  [ %.3f %.3f %.3f ]\n", color.r, color.g, color.b);		

		std::vector<int> nvertices(face_count);
		for(face=0; face<face_count; face++)
		{
			int nv = obj->GetFacePointCount(face);
			nvertices[face]=nv;
		}
		std::vector<int> vertices;
		for(face=0; face<face_count; face++)
		{
			int vert_index[4];
			obj->GetFacePointArray(face, vert_index);
			int count = nvertices[face];
			for(int vert=0; vert<count; vert++)
			{
				vertices.push_back(vert_index[vert]);
			}
		}

		std::vector<float> P;
		int nv = obj->GetVertexCount();
		P.reserve(3*nv);
		for(int i=0;i<nv;i++)
		{
			MQPoint p = obj->GetVertex(i);
			MQ_ExportAxis(&dlginfo, &p, 1);
			P.push_back(p.x);
			P.push_back(p.y);
			P.push_back(p.z);
		}
		std::vector<float> N;

		//fprintf(fh, "Color [1 0 1]\n");
		//fprintf(fh, "Surface \"plastic\"\n");


		fprintf(fh, "PointsPolygons");
		fprintf(fh, " [");
		for(size_t i=0;i<nvertices.size();i++){
			fprintf(fh, " %d", nvertices[i]);
		}
		fprintf(fh, " ]");
		//fprintf(fh, "\n");

		fprintf(fh, " [");
		for(size_t i=0;i<vertices.size();i++){
			fprintf(fh, " %d", vertices[i]);
		}
		fprintf(fh, " ]");
		//fprintf(fh, "\n");

		fprintf(fh, " \"P\"");
		fprintf(fh, " [");
		for(size_t i=0;i<P.size();i++){
			fprintf(fh, " %f", P[i]);
		}
		fprintf(fh, " ]");
		//fprintf(fh, "\n");


		fprintf(fh, "AttributeEnd\n");
		fprintf(fh, "\n");

		// 未着色面用に一時的に作った材質を削除
		if(mat_index == mat_count)
			mat->DeleteThis();
	}

	// ファイルの終端のデータを書き込み
	fprintf(fh, "WorldEnd\n");
	fprintf(fh, "\n");

	// ファイルをクローズ
	fclose(fh);

	// 不要になったデータを破棄
	free(mat_used);
	obj->DeleteThis();

	return TRUE;
}

