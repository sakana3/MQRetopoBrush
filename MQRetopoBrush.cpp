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

class MQRetopoWindow;


class MQRetopoBrush : public MQCommandPlugin
{
	friend class MQRetopoWindow;

	enum MODE
	{
		NONE = -1,
		SMOOTH = 0,
		SMOOTH_WITH_BORDER = 1,
		TWEAK = 2,
		SHRINK_WRAP = 3,
	};

	enum FALLOFF
	{
		CURVE,
		CONSTANT,
		LINER,
	};


	friend class MQRetopoWindow;

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

	BOOL Update(MQDocument doc, MQScene scene, const MQPoint& mouse_pos, bool updateVert);
	BrushVerts FindVerts(MQDocument doc, MQScene scene, const MQPoint& mouse_pos, bool ignore_border);
	void Smooth(MQScene scene, MQObject obj, const BrushVerts& verts, float strength, int iteration);
	void Tweak(MQScene scene, MQObject obj, const BrushVerts& verts, float strength, const MQPoint& move, bool fix);
	void FixPositions(MQScene scene, MQObject obj, const BrushVerts& verts, const std::vector<MQPoint>& positions,  bool fix = true);
	void ShrinkWrap(MQScene scene, MQObject obj, const BrushVerts& verts);
	float getFalloff(float val);

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
	MQRetopoWindow* m_Window = NULL;
	HCURSOR m_MoveCursor = NULL;

	BrushVerts Verts;
	std::vector<int> Mirror;
	MQGeom mqGeom;
	MQSnap mqSnap;
	MQSceneCache sceneCache;
	MQBorderComponent border;

	float brush_radius = 200.0f;
	float brush_power = 1.0f;

	MQPoint cursor_pos;
	MQPoint start_cursor_pos;
	MODE mode;
	FALLOFF falloff = FALLOFF::CURVE;

	MODE mainBrush = MQRetopoBrush::MODE::SMOOTH;
	MODE shiftBrush = MQRetopoBrush::MODE::SMOOTH_WITH_BORDER;
	MODE ctrlBrush = MQRetopoBrush::MODE::TWEAK;
	MODE altBrush = MQRetopoBrush::MODE::SHRINK_WRAP;

	bool DoBrush = false;

};


class MQRetopoWindow : public MQWindow
{
public:
	MQRetopoBrush* m_Plugin;
	MQDoubleSpinBox* radius_spinbox;
	MQSlider* radius_slider;
	MQDoubleSpinBox* power_spinbox;
	MQSlider* power_slider;

	struct FallOffTbl
	{
		std::wstring name;
		MQRetopoBrush::FALLOFF type;
		MQButton* button = NULL;
	} falloff[3] = { 
		{L"Curve",MQRetopoBrush::FALLOFF::CURVE},
		{L"Const",MQRetopoBrush::FALLOFF::CONSTANT},
		{L"Liner",MQRetopoBrush::FALLOFF::LINER}
	};

	struct MainBrushTbl
	{
		std::wstring name;
		MQRetopoBrush::MODE type;
	} brushTbl[4] = {
		{L"Smooth",MQRetopoBrush::MODE::SMOOTH},
		{L"SmoothWithBorder",MQRetopoBrush::MODE::SMOOTH_WITH_BORDER},
		{L"Tweak",MQRetopoBrush::MODE::TWEAK},
		{L"Shrink Wrap",MQRetopoBrush::MODE::SHRINK_WRAP}
	};

	MQRetopoWindow(int id, MQRetopoBrush* plugin) : MQWindow(id)
	{
		m_Plugin = plugin;

		MQFrame* frame = CreateVerticalFrame(this);
		frame->SetOutSpace(0.2);

		{
			MQFrame* hframe = CreateHorizontalFrame(frame);
			hframe->SetUniformSize(true);
			hframe->SetInSpace(0);

			CreateLabel(hframe, L"Brush Size");
			radius_slider = CreateSlider(hframe);
			radius_slider->SetMin(0.0f);
			radius_slider->SetMax(1000.0f);
			radius_slider->SetPosition(m_Plugin->brush_radius);
			radius_slider->AddChangedEvent(this, &MQRetopoWindow::OnChangeBrushSize);
			radius_slider->AddChangingEvent(this, &MQRetopoWindow::OnChangeBrushSize);

			radius_spinbox = CreateDoubleSpinBox(hframe);
			radius_spinbox->SetMin(0.0f);
			radius_spinbox->SetMax(1000.0f);
			radius_spinbox->SetPosition(m_Plugin->brush_radius);
			radius_spinbox->AddChangedEvent(this, &MQRetopoWindow::OnChangeBrushSize);
			radius_spinbox->AddChangingEvent(this, &MQRetopoWindow::OnChangeBrushSize);
		}

		{
			MQFrame* hframe = CreateHorizontalFrame(frame);
			hframe->SetUniformSize(true);
			hframe->SetInSpace(0);

			CreateLabel(hframe, L"Brush Power");
			power_slider = CreateSlider(hframe);
			power_slider->SetMin(0.0f);
			power_slider->SetMax(2.0f);
			power_slider->SetPosition(m_Plugin->brush_power);
			power_slider->AddChangedEvent(this, &MQRetopoWindow::OnChangeBrushPower);
			power_slider->AddChangingEvent(this, &MQRetopoWindow::OnChangeBrushPower);

			power_spinbox = CreateDoubleSpinBox(hframe);
			power_spinbox->SetMin(0.0f);
			power_spinbox->SetMax(2.0f);
			power_spinbox->SetPosition(m_Plugin->brush_power);
			power_spinbox->AddChangedEvent(this, &MQRetopoWindow::OnChangeBrushPower);
			power_spinbox->AddChangingEvent(this, &MQRetopoWindow::OnChangeBrushPower);
		}


		{
			MQFrame* hframe = CreateHorizontalFrame(frame);
			hframe->SetUniformSize(true);
			hframe->SetInSpace(0);

			CreateLabel(hframe, L"FallofType");
			for ( auto& fo : falloff )
			{
				fo.button = CreateButton(hframe, fo.name);
				fo.button->SetToggle(true);
				fo.button->SetDown(m_Plugin->falloff == fo.type);
				fo.button->SetPaddingX(0);
				fo.button->AddClickEvent(this, &MQRetopoWindow::OnChangeFallOff);
			}
		}

		{
			MQFrame* hframe = CreateHorizontalFrame(frame);
			hframe->SetUniformSize(true);
			CreateLabel(hframe, L"Main Brush");
			auto cb = CreateComboBox(hframe);
			cb->AddChangedEvent(this, &MQRetopoWindow::OnChangeMainBrush);
			for (auto& brush : brushTbl)
			{
				cb->AddItem(brush.name);
			}
			cb->SetCurrentIndex((int)m_Plugin->mainBrush);
		}
		{
			MQFrame* hframe = CreateHorizontalFrame(frame);
			hframe->SetUniformSize(true);
			CreateLabel(hframe, L"Shift Brush");
			auto cb = CreateComboBox(hframe);
			cb->AddChangedEvent(this, &MQRetopoWindow::OnChangeShiftBrush);
			for (auto& brush : brushTbl)
			{
				cb->AddItem(brush.name);
			}
			cb->SetCurrentIndex( (int)m_Plugin->shiftBrush);
		}
		{
			MQFrame* hframe = CreateHorizontalFrame(frame);
			hframe->SetUniformSize(true);
			CreateLabel(hframe, L"Ctrl Brush");
			auto cb = CreateComboBox(hframe);
			cb->AddChangedEvent(this, &MQRetopoWindow::OnChangeCtrlBrush);
			for (auto& brush : brushTbl)
			{
				cb->AddItem(brush.name);
			}
			cb->SetCurrentIndex((int)m_Plugin->ctrlBrush);
		}
		{
			MQFrame* hframe = CreateHorizontalFrame(frame);
			hframe->SetUniformSize(true);
			CreateLabel(hframe, L"Alt Brush");
			auto cb = CreateComboBox(hframe);
			cb->AddChangedEvent(this, &MQRetopoWindow::OnChangeAltBrush);
			for (auto& brush : brushTbl)
			{
				cb->AddItem(brush.name);
			}
			cb->SetCurrentIndex((int)m_Plugin->altBrush);
		}
	}
	~MQRetopoWindow() {}

	static MQRetopoWindow* Create(MQRetopoBrush* plugin)
	{
		// Create a window
		// ウィンドウを作成
		MQRetopoWindow* window = new MQRetopoWindow(MQWindow::GetSystemWidgetID(MQSystemWidget::OptionPanel), plugin);
		window->SetTitle(L"RetopoBrushSetting");
		POINT size = window->GetJustSize();
		window->SetWidth(size.x);
		window->SetHeight(size.y);
		window->SetVisible(true);
		return window;
	}

	BOOL OnChangeFallOff(MQWidgetBase* sender, MQDocument doc)
	{
		for (auto& fo : falloff)
		{
			fo.button->SetDown(sender == fo.button);
			if (sender == fo.button) m_Plugin->falloff = fo.type;
		}
		return TRUE;
	}
	BOOL OnChangeMainBrush(MQWidgetBase* sender, MQDocument doc)
	{
		auto idx = ((MQComboBox*)sender)->GetCurrentIndex();
		m_Plugin->mainBrush = brushTbl[idx].type;
		return TRUE;
	}
	BOOL OnChangeShiftBrush(MQWidgetBase* sender, MQDocument doc)
	{
		auto idx = ((MQComboBox*)sender)->GetCurrentIndex();
		m_Plugin->shiftBrush = brushTbl[idx].type;
		return TRUE;
	}
	BOOL OnChangeCtrlBrush(MQWidgetBase* sender, MQDocument doc)
	{
		auto idx = ((MQComboBox*)sender)->GetCurrentIndex();
		m_Plugin->ctrlBrush = brushTbl[idx].type;
		return TRUE;
	}
	BOOL OnChangeAltBrush(MQWidgetBase* sender, MQDocument doc)
	{
		auto idx = ((MQComboBox*)sender)->GetCurrentIndex();
		m_Plugin->altBrush = brushTbl[idx].type;
		return TRUE;
	}
	BOOL OnChangeBrushSize(MQWidgetBase* sender, MQDocument doc)
	{
		auto size = ((MQSlider*)sender)->GetPosition();
		if (m_Plugin->brush_radius != size)
		{
			m_Plugin->brush_radius = size;
			m_Plugin->RedrawAllScene();
			radius_spinbox->SetPosition(size);
			radius_slider->SetPosition(size);
		}
		return TRUE;
	}
	BOOL OnChangeBrushPower(MQWidgetBase* sender, MQDocument doc)
	{
		auto size = ((MQSlider*)sender)->GetPosition();
		if (m_Plugin->brush_power != size)
		{
			m_Plugin->brush_power = size;
			m_Plugin->RedrawAllScene();
			power_spinbox->SetPosition(size);
			power_slider->SetPosition(size);
		}
		return TRUE;
	}

	void Redraw()
	{
		power_spinbox->SetPosition(m_Plugin->brush_power);
		power_slider->SetPosition(m_Plugin->brush_power);
		radius_spinbox->SetPosition(m_Plugin->brush_radius);
		radius_slider->SetPosition(m_Plugin->brush_radius);
		this->Repaint(true);
	}

private:

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
	// Release the window
	// ウィンドウを破棄
	delete m_Window;
	m_Window = NULL;
}

//---------------------------------------------------------------------------
//  SingleMovePlugin::Activate
//    表示・非表示切り替え要求
//---------------------------------------------------------------------------
BOOL MQRetopoBrush::Activate(MQDocument doc, BOOL flag)
{
	if (flag)
	{
		this->m_Window = MQRetopoWindow::Create(this);
		mqSnap.Update(doc);
		mqGeom.Clear();
		SetMouseCursor(m_MoveCursor);
	}
	else
	{
		delete(this->m_Window);
		this->m_Window = NULL;
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

void DrawCircle(MQScene scene, MQObject drawCircle, const MQPoint& cursor_pos , float radius , const MQColorRGBA& color )
{
	drawCircle->AddRenderFlag(MQOBJECT_RENDER_ALPHABLEND);
	drawCircle->AddRenderFlag(MQOBJECT_RENDER_EDGECOLOR);
	drawCircle->AddRenderFlag(MQOBJECT_RENDER_VERTEXCOLOR);
	drawCircle->AddRenderFlag(MQOBJECT_RENDER_VERTEXCOLORPOINT);
	drawCircle->SetColor(MQColor(1,1,1));
	drawCircle->SetColorValid(TRUE);
	DWORD c = ((DWORD)(color.a * 0xff) << 24) + ((DWORD)(color.b * 0xff) << 16) + ((DWORD)(color.g * 0xff) << 8) + ((DWORD)(color.r * 0xff));

	int circle[64];
	for (int i = 0; i < 64; i++)
	{
		float ang = (float)i / 64.0f;
		auto x = cosf(ang * 2.0f * PI) * radius + cursor_pos.x;
		auto y = sinf(ang * 2.0f * PI) * radius + cursor_pos.y;
		MQPoint p = MQPoint(x, y, scene->GetFrontZ());
		auto v = scene->ConvertScreenTo3D(p);
		circle[i] = drawCircle->AddVertex(v);
	}
	for (int i = 0; i < 64; i++)
	{
		int line[2] = { circle[i] , circle[(i + 1) % 64] };
		auto face = drawCircle->AddFace(2, line);
		drawCircle->SetFaceVertexColor(face, 0, c);
	}
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
	drawPoint->AddRenderFlag(MQOBJECT_RENDER_ALPHABLEND);
	drawPoint->AddRenderFlag(MQOBJECT_RENDER_VERTEXCOLOR);
	drawPoint->AddRenderFlag(MQOBJECT_RENDER_VERTEXCOLORPOINT);
	drawPoint->SetColor(MQColor(1, 1, 1));
	drawPoint->SetColorValid(TRUE);
	int iMat;
	auto mat = CreateDrawingMaterial(doc,iMat);
	mat->SetVertexColor(MQMATERIAL_VERTEXCOLOR_DIFFUSE);

	MQColorRGBA c0(1, 0.2f, 0.2f, 0);
	MQColorRGBA c1(1, 1, 1, 1);

	for (const auto& v : Verts)
	{
		auto p = drawPoint->AddVertex(obj->GetVertex(v.first));
		auto face = drawPoint->AddFace(1, &p);
//		float r = getFalloff(v.second);
		MQColorRGBA color = c0;// *(1.0f - r) + c1 * r;
		DWORD c = ((DWORD)(color.a * 0xff) << 24) + ((DWORD)(color.b * 0xff) << 16) + ((DWORD)(color.g * 0xff) << 8) + ((DWORD)(color.r * 0xff));
		drawPoint->SetFaceVertexColor(face, 0, c);
		drawPoint->SetFaceMaterial(face, iMat);
	}

	{
		MQObject drawCircle = CreateDrawingObject(doc, DRAW_OBJECT_LINE);
		DrawCircle(scene, drawCircle, cursor_pos, brush_radius * (brush_power / 2), MQColorRGBA(1,1,1, 0.5f));
	}

	{
		MQObject drawCircle = CreateDrawingObject(doc, DRAW_OBJECT_LINE);
		DrawCircle(scene, drawCircle, cursor_pos, brush_radius, MQColorRGBA(1, 1, 1,1));
	}
}



//---------------------------------------------------------------------------
//  SingleMovePlugin::OnLeftButtonDown
//    左ボタンが押されたとき
//---------------------------------------------------------------------------
BOOL MQRetopoBrush::OnLeftButtonDown(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	MQObject obj = doc->GetObject(doc->GetCurrentObjectIndex());
	if (obj->GetLocking() == TRUE || obj->GetVisible() == FALSE)
	{
		mode = MODE::NONE;
		return FALSE;
	}

	start_cursor_pos = MQPoint((float)state.MousePos.x, (float)state.MousePos.y, 0);
	DoBrush = false;
	mode = mainBrush;
	if (state.Shift)
	{
		mode = shiftBrush;
	}
	else if (state.Ctrl)
	{
		mode = ctrlBrush;
	}
	else if (state.Alt)
	{
		mode = altBrush;
	}

	return TRUE;
}



//---------------------------------------------------------------------------
//  SingleMovePlugin::OnLeftButtonUp
//    左ボタンが離されたとき
//---------------------------------------------------------------------------
BOOL MQRetopoBrush::OnLeftButtonUp(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	switch (mode)
	{
	case MODE::TWEAK:
		MQObject obj = doc->GetObject(doc->GetCurrentObjectIndex());
		cursor_pos = MQPoint((float)state.MousePos.x, (float)state.MousePos.y, 0);
		Tweak(scene, obj, this->Verts, 1.0f, cursor_pos - start_cursor_pos, true);
		break;
	}

	if (DoBrush)
	{
		UpdateUndo(L"Smooth Brush");
		RedrawScene(scene);
	}
	DoBrush = false;
	mode = MODE::NONE;

	return TRUE;
}

BOOL MQRetopoBrush::OnMouseWheel(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	if (state.Shift)
	{
		brush_radius = brush_radius + (brush_radius / 1000.0f) * state.Wheel;
		brush_radius = std::max(std::min(brush_radius, 1000.0f), 10.0f);
		RedrawScene(scene);
		this->m_Window->Redraw();
		return TRUE;
	}
	if (state.Ctrl)
	{
		brush_power = brush_power + 0.001f * state.Wheel;
		brush_power = std::max(std::min(brush_power, 2.0f), 0.0f);
		RedrawScene(scene);
		this->m_Window->Redraw();
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
	MQObject obj = doc->GetObject(doc->GetCurrentObjectIndex());
	cursor_pos = MQPoint((float)state.MousePos.x, (float)state.MousePos.y, 0);

	RedrawScene(scene);

	switch (mode)
	{
	case MODE::SMOOTH :
	case MODE::SMOOTH_WITH_BORDER:
		Update(doc, scene, cursor_pos, true);
		Smooth(scene, obj, this->Verts, (brush_power * brush_power) * 0.01f, 10);
		break;
	case MODE::SHRINK_WRAP:
		Update(doc, scene, cursor_pos, true);
		ShrinkWrap(scene, obj, this->Verts);
		break;
	case MODE::TWEAK:
		Tweak(scene, obj, this->Verts, 1.0f, cursor_pos - start_cursor_pos , false);
		break;
	}

	if (mode != MODE::NONE)
	{
		DoBrush = true;
		RedrawAllScene();
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

//---------------------------------------------------------------------------
//  SingleMovePlugin::OnMouseMove
//    マウスが移動したとき
//---------------------------------------------------------------------------
BOOL MQRetopoBrush::OnMouseMove(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	cursor_pos = MQPoint((float)state.MousePos.x, (float)state.MousePos.y, 0);
	RedrawScene(scene);

	MQObject obj = doc->GetObject(doc->GetCurrentObjectIndex());
	if (obj->GetLocking() == TRUE || obj->GetVisible() == FALSE)
	{
		this->Verts.clear();
		return FALSE;
	}

	Update(doc, scene, cursor_pos, true);
	return FALSE;
}

BOOL MQRetopoBrush::Update(MQDocument doc, MQScene scene, const MQPoint& mouse_pos , bool updateVert )
{
	MQObject obj = doc->GetObject(doc->GetCurrentObjectIndex());

	mqGeom.Update(obj);
	border.Update(scene, mqGeom.obj);
	mqSnap.Update(doc);

	this->Verts = FindVerts(doc, scene, mouse_pos, mode == MODE::SMOOTH);

	return TRUE;
}



void MQRetopoBrush::Smooth(MQScene scene, MQObject obj, const BrushVerts& verts, float strength, int iteration)
{
	std::vector<MQPoint> cos(mqGeom.obj->verts.size());
	auto screen = sceneCache.Get(scene, mqGeom.obj);
	bool isSnap = option().SnapFace;

	for (int it = 0; it < iteration; it++)
	{
		for (const auto& vi : verts)
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

			auto co = vert->co.Lerp(avg, strength * getFalloff(vi.second));
			if (isSnap)
			{
				co = mqSnap.colsest_point(co).position;
			}
			cos[vi.first] = co;
		}

		FixPositions(scene, obj, verts, cos);
	}
}

void MQRetopoBrush::ShrinkWrap(MQScene scene, MQObject obj, const BrushVerts& verts)
{
	std::vector<MQPoint> cos(mqGeom.obj->verts.size());
	for (const auto& vi : verts)
	{
		auto co = mqGeom.obj->verts[vi.first].co;
		cos[vi.first] = mqSnap.colsest_point(co).position;
	}
	FixPositions(scene, obj, verts, cos);
}

void MQRetopoBrush::Tweak(MQScene scene, MQObject obj, const BrushVerts& verts, float strength, const MQPoint& move, bool fix)
{
	std::vector<MQPoint> cos(mqGeom.obj->verts.size());
	auto screen = sceneCache.Get(scene, mqGeom.obj);
	bool isSnap = option().SnapFace;

	for (const auto& vi : verts)
	{
		auto co = mqGeom.obj->verts[vi.first].co;

		auto r = getFalloff( vi.second );

		co = scene->Convert3DToScreen(co);
		co.x = co.x + move.x * r;
		co.y = co.y + move.y * r;
		co = scene->ConvertScreenTo3D(co);
		if (isSnap)
		{
			MQVector screen_pos = scene->Convert3DToScreen(co);
			auto hit = mqSnap.intersect(MQRay(scene, screen_pos) );
			if (hit.is_hit)
			{
				co = hit.position;
			}
		}
		cos[vi.first] = co;
	}

	FixPositions(scene, obj, verts, cos, fix);
}

void MQRetopoBrush::FixPositions(MQScene scene, MQObject obj, const BrushVerts& verts, const std::vector<MQPoint>& positions , bool fix )
{
	auto screen = sceneCache.Get(scene, mqGeom.obj);
	bool isSymmetry = option().Symmetry;

	for (const auto& vi : verts)
	{
		auto vert = &mqGeom.obj->verts[vi.first];
		auto co = positions[vi.first];

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
			if (fix)
			{
				mirror->co = mco;
				mqGeom.obj->cos[mirror->id] = mco;
				screen->UpdateVert(scene, mirror->id, mco);
			}
			obj->SetVertex(mirror->id, mco);
		}

		if (fix)
		{
			vert->co = co;
			mqGeom.obj->cos[vi.first] = co;
			screen->UpdateVert(scene, vi.first, co);
		}
		obj->SetVertex(vi.first, co);
	}
}


MQRetopoBrush::BrushVerts MQRetopoBrush::FindVerts(MQDocument doc, MQScene scene, const MQPoint& mouse_pos, bool ignore_border)
{
	//スクリーン変換
	auto screen = sceneCache.Get(scene, mqGeom.obj);
	float radius = this->brush_radius * this->brush_radius;
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

float MQRetopoBrush::getFalloff(float val)
{
	switch (falloff)
	{
	case FALLOFF::CONSTANT:
		return 1.0f;
	case FALLOFF::CURVE:
		return (1.0f - val) * (1.0f - val);
	case FALLOFF::LINER:
		return (1.0f - val);
	}
	return (1.0f - val);
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

