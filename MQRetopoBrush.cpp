#pragma warning(disable:4244)
#pragma warning(disable:4267)

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <float.h>
#include "MQBasePlugin.h"
#include "MQHandleObject.h"
#include "MQSelectOperation.h"
#include "MQWidget.h"
#include "MQ3DLib.h"
#include "resource.h"
#include <limits>
#include <algorithm>
#include <iostream>
#include <type_traits>
#include "MQGeometry.h"

HINSTANCE g_hInstance;


std::vector<int>  MakeQuad(const std::vector<int>& quad, const std::vector< MQPoint >& points, const std::vector< MQPoint >& coords, const MQPoint& pivot)
{

	//	angle = acos(x / sqrt(x*x + y * y));
	//	angle = angle * 180.0 / PI;
	//	if (y<0)angle = 360.0 - angle;
	std::vector< std::pair< int, float > > temp;
	temp.reserve(quad.size());
	for (auto q : quad)
	{
		auto p = coords[q] - pivot;
		float angle = acos(p.x / sqrt(p.x * p.x + p.y * p.y));
		angle = angle * 180.0f / PI;
		if (p.y < 0)angle = 360.0f - angle;
		temp.push_back(std::pair< int, float >(q, angle));
	}


	std::sort(
		temp.begin(),
		temp.end(),
		[](const auto& lhs, const auto& rhs) { return lhs.second > rhs.second; }
	);

	std::vector<int> ret;
	ret.reserve(quad.size());
	for (const auto& t : temp)
	{
		ret.push_back(t.first);
	}

	return ret;
}


class MQRetopoBrush : public MQCommandPlugin
{
	friend class MQAutoQuadWindow;

	typedef std::pair<int, float> BrushVert;
	typedef std::vector<BrushVert> BrushVerts;

public:
	// コンストラクタ
	MQRetopoBrush();

	// プラグインIDを返す。
	virtual void GetPlugInID(DWORD* Product, DWORD* ID);
	// プラグイン名を返す。
	virtual const char* GetPlugInName(void);
	// ポップアップメニューに表示される文字列を返す。
	virtual const wchar_t* EnumString(void);
	// アプリケーションの初期化
	virtual BOOL Initialize();
	// アプリケーションの終了
	virtual void Exit();
	// 表示・非表示切り替え要求
	virtual BOOL Activate(MQDocument doc, BOOL flag);
	// 描画時の処理
	virtual void OnDraw(MQDocument doc, MQScene scene, int width, int height);

	// 左ボタンが押されたとき
	virtual BOOL OnLeftButtonDown(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state);
	// 左ボタンが押されながらマウスが移動したとき
	virtual BOOL OnLeftButtonMove(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state);
	// 左ボタンが離されたとき
	virtual BOOL OnLeftButtonUp(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state);
	// マウスが移動したとき
	virtual BOOL OnMouseMove(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state);
	virtual BOOL OnMouseWheel(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state);

	BrushVerts FindVerts(MQDocument doc, MQScene scene, const MQPoint& mouse_pos, bool ignore_border);

	void Smooth(MQScene scene, MQObject obj, const BrushVerts& verts, float strength, int iteration);

	void clear(bool isGeom, bool isScene, bool isSnap)
	{
		if (isGeom) { mqGeom.Clear(); }
		if (isScene || isGeom) sceneCache.Clear();
		if (isSnap) mqSnap = MQSnap();
		if (isGeom || isScene) border.Clear();

		Verts.clear();
	}

	// アンドゥ実行時
	virtual BOOL OnUndo(MQDocument doc, int undo_state) { Trace("Undo\n"); clear(true, true, true); return FALSE; }
	// リドゥ実行時
	virtual BOOL OnRedo(MQDocument doc, int redo_state) { Trace("Redo\n"); clear(true, true, true); return FALSE; }
	// アンドゥ状態更新時
	virtual void OnUpdateUndo(MQDocument doc, int undo_state, int undo_size) {}
	// オブジェクトの編集時
	virtual void OnObjectModified(MQDocument doc) { Trace("OnObjectModified\n"); clear(true, true, false); }
	// カレントオブジェクトの変更時
	virtual void OnUpdateObjectList(MQDocument doc) { clear(true, true, false); }
	// シーン情報の変更時
	virtual void OnUpdateScene(MQDocument doc, MQScene scene) { clear(false, true, false); }

	EDIT_OPTION option()
	{
		EDIT_OPTION option;
		GetEditOption(option);
		return option;
	}


private:
	bool m_bActivated;
	HCURSOR m_MoveCursor = NULL;

	BrushVerts Verts;
	std::vector<int> Mirror;
	MQGeom mqGeom;
	MQSnap mqSnap;
	MQSceneCache sceneCache;
	MQBorderComponent border;

	float cursor_radius = 200.0f;
	MQPoint cursor_pos;

	bool DoBrush = false;

};


//---------------------------------------------------------------------------
//  SingleMovePlugin::SingleMovePlugin
//    コンストラクタ
//---------------------------------------------------------------------------
MQRetopoBrush::MQRetopoBrush()
{
	m_bActivated = false;
}

//---------------------------------------------------------------------------
//  SingleMovePlugin::GetPlugInID
//    プラグインIDを返す。
//---------------------------------------------------------------------------
void MQRetopoBrush::GetPlugInID(DWORD* Product, DWORD* ID)
{
	// プロダクト名(制作者名)とIDを、全部で64bitの値として返す
	// 値は他と重複しないようなランダムなもので良い
	*Product = 0xA0E090AD;
	*ID = 0x12AD137E;
}


//---------------------------------------------------------------------------
//  SingleMovePlugin::GetPlugInName
//    プラグイン名を返す。
//---------------------------------------------------------------------------
const char* MQRetopoBrush::GetPlugInName(void)
{
	return "MQ Retopo Brush      by sakana3";
}


//---------------------------------------------------------------------------
//  SingleMovePlugin::EnumString
//    ボタンに表示される文字列を返す。
//---------------------------------------------------------------------------
const wchar_t* MQRetopoBrush::EnumString(void)
{
	return L"Retopo Brush";
}

//---------------------------------------------------------------------------
//  SingleMovePlugin::Initialize
//    アプリケーションの初期化
//---------------------------------------------------------------------------
BOOL MQRetopoBrush::Initialize()
{
	if (m_MoveCursor == NULL) {
		m_MoveCursor = LoadCursor(g_hInstance, MAKEINTRESOURCE(IDC_CURSOR1));
	}

	return TRUE;
}

//---------------------------------------------------------------------------
//  SingleMovePlugin::Exit
//    アプリケーションの終了
//---------------------------------------------------------------------------
void MQRetopoBrush::Exit()
{
}

//---------------------------------------------------------------------------
//  SingleMovePlugin::Activate
//    表示・非表示切り替え要求
//---------------------------------------------------------------------------
BOOL MQRetopoBrush::Activate(MQDocument doc, BOOL flag)
{
	if (flag)
	{
		mqSnap.Update(doc);
		mqGeom.Clear();
		SetMouseCursor(m_MoveCursor);
	}
	else
	{
		clear(true, true, true);
		SetMouseCursor(GetResourceCursor(MQCURSOR_DEFAULT));
	}

	m_bActivated = flag ? true : false;
	// It returns 'flag' as it is.
	// そのままflagを返す
	return flag;
}

void DrawFace(MQScene scene, MQObject draw, MQObject obj, const std::vector<int>& quad, int iMat0)
{
	std::vector<int> pts; pts.reserve(quad.size());
	for (auto q : quad)
	{
		auto v = obj->GetVertex(q);
		pts.push_back(draw->AddVertex(v));
	}

	auto iFace = draw->AddFace((int)pts.size(), pts.data());
	draw->SetFaceMaterial(iFace, iMat0);

	std::reverse(pts.begin(), pts.end());
	iFace = draw->AddFace((int)pts.size(), pts.data());
	draw->SetFaceMaterial(iFace, iMat0);
}


//---------------------------------------------------------------------------
//  SingleMovePlugin::OnDraw
//    描画時の処理
//---------------------------------------------------------------------------
void MQRetopoBrush::OnDraw(MQDocument doc, MQScene scene, int width, int height)
{
	if (!m_bActivated) return;

	MQObject obj = doc->GetObject(doc->GetCurrentObjectIndex());
	MQColor color = GetSystemColor(MQSYSTEMCOLOR_TEMP);

	MQObject drawPoint = CreateDrawingObject(doc, DRAW_OBJECT_POINT);
	drawPoint->SetColor(MQColor(1, 1, 1));
	drawPoint->SetColorValid(TRUE);
	for (const auto& v : Verts)
	{
		auto p = obj->GetVertex(v.first);
		auto x = drawPoint->AddVertex(p);
		drawPoint->AddFace(1, &x);
	}

	{
		MQObject drawCircle = CreateDrawingObject(doc, DRAW_OBJECT_LINE);
		drawCircle->AddRenderFlag(MQOBJECT_RENDER_ALPHABLEND);
		drawCircle->SetColor(MQColor(1, 1, 1));
		drawCircle->SetColorValid(TRUE);

		int circle[64];
		for (int i = 0; i < 64; i++)
		{
			float ang = (float)i / 64.0f;
			auto x = cosf(ang * 2.0f * PI) * cursor_radius + cursor_pos.x;
			auto y = sinf(ang * 2.0f * PI) * cursor_radius + cursor_pos.y;
			MQPoint p = MQPoint(x, y, scene->GetFrontZ());
			auto v = scene->ConvertScreenTo3D(p);
			circle[i] = drawCircle->AddVertex(v);
		}
		for (int i = 0; i < 64; i++)
		{
			int line[2] = { circle[i] , circle[(i + 1) % 64] };
			drawCircle->AddFace(2, line);
		}
	}
}



//---------------------------------------------------------------------------
//  SingleMovePlugin::OnLeftButtonDown
//    左ボタンが押されたとき
//---------------------------------------------------------------------------
BOOL MQRetopoBrush::OnLeftButtonDown(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	return FALSE;
}



//---------------------------------------------------------------------------
//  SingleMovePlugin::OnLeftButtonUp
//    左ボタンが離されたとき
//---------------------------------------------------------------------------
BOOL MQRetopoBrush::OnLeftButtonUp(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	if (DoBrush)
	{
		UpdateUndo(L"Smooth Brush");
		RedrawScene(scene);
	}
	DoBrush = false;

	return FALSE;
}

BOOL MQRetopoBrush::OnMouseWheel(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	if (state.Ctrl)
	{
		cursor_radius = cursor_radius + (cursor_radius / 1000.0f) * state.Wheel;
		RedrawScene(scene);
		return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------------
//  SingleMovePlugin::OnLeftButtonMove
//    左ボタンが押されながらマウスが移動したとき
//---------------------------------------------------------------------------
BOOL MQRetopoBrush::OnLeftButtonMove(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	OnMouseMove(doc, scene, state);

	if (this->Verts.size() > 0)
	{
		MQObject obj = doc->GetObject(doc->GetCurrentObjectIndex());
		if (state.Ctrl)
		{
			Smooth(scene, obj, this->Verts, 0.0f, 1);
		}
		else
		{
			Smooth(scene, obj, this->Verts, 0.01f, 10);
		}
		DoBrush = true;
		RedrawAllScene();
	}

	return FALSE;
}

//---------------------------------------------------------------------------
//  SingleMovePlugin::OnMouseMove
//    マウスが移動したとき
//---------------------------------------------------------------------------
BOOL MQRetopoBrush::OnMouseMove(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	RedrawScene(scene);
	auto mouse_pos = MQPoint((float)state.MousePos.x, (float)state.MousePos.y, 0);
	cursor_pos = mouse_pos;
	MQObject obj = doc->GetObject(doc->GetCurrentObjectIndex());

	if (obj->GetLocking() == TRUE || obj->GetVisible() == FALSE)
	{
		return FALSE;
	}

	mqGeom.Update(obj);
	border.Update(scene, mqGeom.obj);
	mqSnap.Update(doc);

	bool ignore_border = true;
	if (state.Ctrl || state.Shift)
	{
		ignore_border = false;
	}

	this->Verts = FindVerts(doc, scene, mouse_pos, ignore_border);

	return FALSE;
}


void MQRetopoBrush::Smooth(MQScene scene, MQObject obj, const BrushVerts& verts, float strength, int iteration)
{
	std::vector<MQPoint> cos(mqGeom.obj->verts.size());
	auto screen = sceneCache.Get(scene, mqGeom.obj);
	bool isSnap = option().SnapFace;
	bool isSymmetry = option().Symmetry;

	for (int it = 0; it < iteration; it++)
	{
		if (strength > 0.0f)
		{
			for ( const auto& vi : verts)
			{
				auto vert = &mqGeom.obj->verts[vi.first];
				MQVector avg = MQVector(0.0f);
				float div = 0.0f;
				for (const auto edge : vert->link_edges)
				{
					avg += edge->other_vert(vert)->co;
					div += 1.0f;
				}
				avg /= div;

				cos[vi.first] = vert->co.Lerp(avg, strength);
			}
		}
		else
		{
			for (const auto& vi : verts)
			{
				cos[vi.first] = mqGeom.obj->verts[vi.first].co;
			}
		}

		for (const auto& vi: verts)
		{
			auto vert = &mqGeom.obj->verts[vi.first];
			auto co = cos[vi.first];
			if (isSnap)
			{
				co = mqSnap.colsest_point(co).position;
			}

			MQGeom::Vert* mirror = NULL;
			if (isSymmetry)
			{
				mirror = mqGeom.obj->find_mirror(vert);
				if (mirror != NULL)
				{
					if (mirror != vert)
					{
						auto t = std::find_if(
							verts.begin(), verts.end(),
							[&](const auto& o) { return (o.first == mirror->id); });
						if (t != verts.end())
						{
							if (vi.second < t->second)
							{
								mirror = NULL;
							}
						}

					}
					else
					{
						co.x = 0.0f;
					}
				}
			}

			if (mirror != NULL)
			{
				auto mco = MQPoint(-co.x, co.y, co.z);
				mirror->co = mco;
				mqGeom.obj->cos[mirror->id] = mco;
				obj->SetVertex(mirror->id, mco);
				screen->UpdateVert(scene, mirror->id, mco);
			}

			vert->co = co;
			mqGeom.obj->cos[vi.first] = co;
			obj->SetVertex(vi.first, co);
			screen->UpdateVert(scene, vi.first, co);

		}
	}

}


MQRetopoBrush::BrushVerts MQRetopoBrush::FindVerts(MQDocument doc, MQScene scene, const MQPoint& mouse_pos, bool ignore_border)
{
	//スクリーン変換
	auto screen = sceneCache.Get(scene, mqGeom.obj);
	float radius = this->cursor_radius * this->cursor_radius;
	bool isSymmetry = option().Symmetry;

	BrushVerts verts; verts.reserve(100);
	for (auto& vi : mqGeom.obj->verts)
	{
		if (screen->in_screen[vi.id])
		{
			float dx = mouse_pos.x - screen->coords[vi.id].x;
			float dy = mouse_pos.y - screen->coords[vi.id].y;
			float dist = dx * dx + dy * dy;
			if (radius >= dist)
			{
				if (!ignore_border || !vi.is_border())
				{
					//					if ( vi.link_faces.size() > 1 )
					{
						if (mqSnap.check_view(scene, vi.co))
						{
							verts.push_back(BrushVert(vi.id, dist / radius));
							if (isSymmetry)
							{
								auto mirror = mqGeom.obj->find_mirror(&vi);
							}
						}
					}
				}
			}
		}
	}
	return verts;
}


//---------------------------------------------------------------------------
//  GetPluginClass
//    プラグインのベースクラスを返す
//---------------------------------------------------------------------------
MQBasePlugin* GetPluginClass()
{
	static MQRetopoBrush plugin;
	return &plugin;
}



//---------------------------------------------------------------------------
//  DllMain
//---------------------------------------------------------------------------
BOOL APIENTRY DllMain(HINSTANCE hinstDLL,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		g_hInstance = hinstDLL;
	}
	return TRUE;
}

