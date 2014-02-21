#include "leveleditor.h"

#include <application.h>
#include <profiler.h>
#include <cvar.h>

#include <textures/materiallibrary.h>

#include <glgui/picturebutton.h>
#include <glgui/tree.h>
#include <glgui/menu.h>
#include <glgui/textfield.h>
#include <glgui/checkbox.h>

#include <game/gameserver.h>
#include <renderer/game_renderer.h>
#include <renderer/game_renderingcontext.h>

#include "../workbench.h"

#include "editorpanel.h"
#include "createentitypanel.h"

REGISTER_WORKBENCH_TOOL(LevelEditor);

CLevelEditor* CLevelEditor::s_pLevelEditor = nullptr;
	
CLevelEditor::CLevelEditor()
{
	s_pLevelEditor = this;

	m_hEditorPanel = glgui::RootPanel()->AddControl(new CEditorPanel());
	m_hEditorPanel->SetVisible(false);
	m_hEditorPanel->SetBackgroundColor(Color(0, 0, 0, 150));
	m_hEditorPanel->SetBorder(glgui::CPanel::BT_SOME);

	m_hCreateEntityButton = glgui::RootPanel()->AddControl(new glgui::CPictureButton("Create", CMaterialLibrary::AddMaterial("editor/create-entity.mat")), true);
	m_hCreateEntityButton->SetPos(glgui::CRootPanel::Get()->GetWidth()/2-m_hCreateEntityButton->GetWidth()/2, 20);
	m_hCreateEntityButton->SetClickedListener(this, CreateEntity);
	m_hCreateEntityButton->SetTooltip("Create Entity Tool");

	m_hCreateEntityPanel = glgui::CControl<CCreateEntityPanel>((new CCreateEntityPanel())->GetHandle()); // Adds itself.
	m_hCreateEntityPanel->SetBackgroundColor(Color(0, 0, 0, 255));
	m_hCreateEntityPanel->SetHeaderColor(Color(100, 100, 100, 255));
	m_hCreateEntityPanel->SetBorder(glgui::CPanel::BT_SOME);
	m_hCreateEntityPanel->SetVisible(false);

	m_flCreateObjectDistance = 10;
}

CLevelEditor::~CLevelEditor()
{
	glgui::CRootPanel::Get()->RemoveControl(m_hEditorPanel);

	glgui::CRootPanel::Get()->RemoveControl(m_hCreateEntityButton);
}

void CLevelEditor::LoadLevel(const CHandle<CLevel>& pLevel)
{
	m_pLevel = pLevel;

	EditorPhysics()->RemoveAllEntities();

	if (!m_pLevel.Get())
		return;

	for (size_t i = 0; i < m_pLevel->GetEntityData().size(); i++)
	{
		CLevelEntity* pEntity = &m_pLevel->GetEntityData()[i];
		EditorPhysics()->AddEntity(pEntity, CT_STATIC_MESH);
	}
}

void CLevelEditor::Think()
{
	BaseClass::Think();

	int x, y;
	Application()->GetMousePosition(x, y);

	if (ShouldRenderOrthographic())
	{
		Vector vecPosition1 = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));
		Vector vecPosition0 = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 0));

		m_iHoverEntity = TraceLine(Ray(vecPosition0, (vecPosition1-vecPosition0).Normalized()));
	}
	else
	{
		Vector vecCamera = GameServer()->GetRenderer()->GetCameraPosition();
		Vector vecPosition = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));

		m_iHoverEntity = TraceLine(Ray(vecCamera, (vecPosition-vecCamera).Normalized()));
	}
}

void CLevelEditor::RenderEntity(size_t i)
{
	CLevelEntity* pEntity = &m_pLevel->GetEntityData()[i];

	if (m_hEditorPanel->m_hEntities->GetSelectedNodeId() == i)
		RenderEntity(pEntity, true, false, Manipulator()->GetTransform(true, false));
	else
		RenderEntity(pEntity, m_hEditorPanel->m_hEntities->GetSelectedNodeId() == i, i == m_iHoverEntity);
}

void CLevelEditor::RenderEntity(CLevelEntity* pEntity, bool bSelected, bool bHover)
{
	RenderEntity(pEntity, bSelected, bHover, pEntity->GetGlobalTransform());
}

void CLevelEditor::RenderEntity(CLevelEntity* pEntity, bool bSelected, bool bHover, const Matrix4x4& mGlobal)
{
	CGameRenderingContext r(GameServer()->GetRenderer(), true);

	// If another context already set this, don't clobber it.
	if (!r.GetActiveFrameBuffer())
		r.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

	r.Transform(mGlobal);

	if (pEntity->ShouldRenderInverted())
		r.SetWinding(!r.GetWinding());

	if (pEntity->ShouldDisableBackCulling())
		r.SetBackCulling(false);

	Vector vecScale = pEntity->GetScale();
	if (bSelected && Manipulator()->IsTransforming())
		vecScale = Manipulator()->GetNewTRS().m_vecScaling;

	float flAlpha = 1;
	if (!pEntity->IsVisible())
	{
		r.SetBlend(BLEND_ALPHA);
		flAlpha = 0.4f;
	}

	if (pEntity->GetModelID() != ~0)
	{
		if (!!(flAlpha < 1) ^ GameServer()->GetRenderer()->IsRenderingTransparent())
			return;

		if (bSelected)
			r.SetColor(Color(255, 0, 0, (char)(255*flAlpha)));
		else
			r.SetColor(Color(255, 255, 255, (char)(255*flAlpha)));

		TPROF("CLevelEditor::RenderEntity()");
		r.RenderModel(pEntity->GetModelID(), nullptr);
	}
	else if (pEntity->GetMaterialModel().IsValid())
	{
		bool bRenderMaterial = true;
		if (!pEntity->ShouldDisableBackCulling())
		{
			if (pEntity->ShouldRenderInverted())
			{
				if ((mGlobal.GetTranslation() - GetCameraPosition()).Dot(mGlobal.GetForwardVector()) > 0)
					bRenderMaterial = false;
			}
			else
			{
				if ((mGlobal.GetTranslation() - GetCameraPosition()).Dot(mGlobal.GetForwardVector()) < 0)
					bRenderMaterial = false;
			}
		}

		if (GameServer()->GetRenderer()->IsRenderingTransparent())
		{
			TPROF("CLevelEditor::RenderModel(Material)");
			r.UseProgram("model");

			Color clrEnt;
			if (bSelected)
				clrEnt = Color(255, 0, 0, (char)(255*flAlpha));
			else
				clrEnt = Color(255, 255, 255, (char)(255*flAlpha));

			Color clrEntHover = clrEnt;
			if (!bRenderMaterial)
				clrEnt.SetAlpha(clrEnt.a()/5);

			if (!bHover)
				clrEntHover.SetAlpha(clrEnt.a()/3);

			r.SetBlend(BLEND_ALPHA);
			r.SetColor(clrEntHover);
			r.SetUniform("vecDiffuse", Vector4D(1, 1, 1, 1));
			r.SetUniform("vecColor", clrEntHover);
			r.SetUniform("bDiffuse", false);

			AABB aabbBounds = pEntity->GetBoundingBox();

			// A bit of a hack to fix up the scale of the AABB to render it properly while dragging the manipulator scale handle
			if (bSelected && Manipulator()->IsTransforming())
			{
				aabbBounds.m_vecMins = aabbBounds.m_vecMins / pEntity->GetScale() * vecScale;
				aabbBounds.m_vecMaxs = aabbBounds.m_vecMaxs / pEntity->GetScale() * vecScale;
			}

			r.RenderWireBox(aabbBounds);

			if (bRenderMaterial)
			{
				r.SetColor(clrEnt);
				r.SetUniform("vecColor", clrEnt);
				r.SetUniform("bDiffuse", true);

				r.Scale(vecScale.x, vecScale.y, vecScale.z);
				r.RenderMaterialModel(pEntity->GetMaterialModel());
			}
		}
	}
	else
	{
		if (!!(flAlpha < 1) ^ GameServer()->GetRenderer()->IsRenderingTransparent())
			return;

		r.UseProgram("model");
		r.SetUniform("vecDiffuse", Vector4D(1, 1, 1, 1));
		if (bSelected)
			r.SetUniform("vecColor", Color(255, 0, 0, (char)(255*flAlpha)));
		else if (bHover)
			r.SetUniform("vecColor", Color(255, 255, 255, (char)(255*flAlpha)));
		else
		{
			r.SetBlend(BLEND_ALPHA);
			r.SetUniform("vecColor", Color(255, 255, 255, (char)(150*flAlpha)));
		}

		r.SetUniform("bDiffuse", false);

		AABB aabbBounds = pEntity->GetBoundingBox();

		// A bit of a hack to fix up the scale of the AABB to render it properly while dragging the manipulator scale handle
		if (bSelected && Manipulator()->IsTransforming())
		{
			aabbBounds.m_vecMins = aabbBounds.m_vecMins / pEntity->GetScale() * vecScale;
			aabbBounds.m_vecMaxs = aabbBounds.m_vecMaxs / pEntity->GetScale() * vecScale;
		}

		r.RenderWireBox(aabbBounds);
	}
}

void CLevelEditor::RenderCreateEntityPreview()
{
	CLevelEntity oRenderEntity;
	oRenderEntity.SetClass(m_hCreateEntityPanel->m_hClass->GetText());

	PopulateLevelEntityFromPanel(&oRenderEntity, m_hCreateEntityPanel->m_hPropertiesPanel);

	oRenderEntity.SetParameterValue("Visible", "no"); // Make it semitransparent
	oRenderEntity.SetParameterValue("Name", m_hCreateEntityPanel->m_hNameText->GetText());
	oRenderEntity.SetParameterValue("Model", m_hCreateEntityPanel->m_hModelText->GetText());
	oRenderEntity.SetGlobalTransform(Matrix4x4(EAngle(0, 0, 0), PositionFromMouse()));

	RenderEntity(&oRenderEntity, true);
}

Vector CLevelEditor::PositionFromMouse()
{
	TAssert(m_hCreateEntityPanel->IsVisible() && m_hCreateEntityPanel->m_bReadyToCreate);

	int x, y;
	Application()->GetMousePosition(x, y);
	Vector vecPosition = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));
	Vector vecCamera = GameServer()->GetRenderer()->GetCameraPosition();

	Vector vecCameraDirection = GameServer()->GetRenderer()->GetCameraDirection();
	if (vecCameraDirection.Dot(vecPosition-vecCamera) < 0)
		vecPosition = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, -1));

	tstring sEntity = m_hCreateEntityPanel->m_hClass->GetText();

	CTraceResult tr;
	EditorPhysics()->TraceLine(tr, vecCamera, vecCamera + (vecPosition-vecCamera)*100);

	if (tr.m_flFraction < 1)
	{
		AABB aabbBoxSize;
		CSaveData* pData = CBaseEntity::FindSaveDataByHandle(("C" + sEntity).c_str(), "BoundingBox");
		if (pData && pData->m_bDefault)
			aabbBoxSize = *(AABB*)pData->m_oDefault;

		Vector vecHit = tr.m_vecHit;

		for (size_t i = 0; i < 3; i++)
		{
			Vector vecPushout;

			vecPushout[i] = aabbBoxSize.m_vecMins[i];
			float flDot = vecPushout.Dot(tr.m_vecNormal);
			if (flDot < 0)
				vecHit -= vecPushout;

			vecPushout[i] = aabbBoxSize.m_vecMaxs[i];
			flDot = vecPushout.Dot(tr.m_vecNormal);
			if (flDot < 0)
				vecHit -= vecPushout;
		}

		return vecHit;
	}
	else
		return vecCamera + (vecPosition - vecCamera).Normalized() * m_flCreateObjectDistance;
}

Vector CLevelEditor::DirectionFromMouse()
{
	int x, y;
	Application()->GetMousePosition(x, y);
	Vector vecPosition = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));
	Vector vecCamera = GameServer()->GetRenderer()->GetCameraPosition();

	Vector vecCameraDirection = GameServer()->GetRenderer()->GetCameraDirection();
	if (vecCameraDirection.Dot(vecPosition-vecCamera) < 0)
		vecPosition = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, -1));

	return (vecPosition - vecCamera).Normalized();
}

size_t CLevelEditor::TraceLine(const Ray& vecTrace)
{
	CLevel* pLevel = GetLevel();
	if (!pLevel)
		return ~0;

	size_t iNearest = ~0;
	float flNearestSqr;

	float flTraceDistance = 100;
	CTraceResult tr;
	EditorPhysics()->TraceLine(tr, vecTrace.m_vecPos, vecTrace.m_vecPos + vecTrace.m_vecDir*flTraceDistance);

	if (tr.m_flFraction < 1)
	{
		for (size_t i = 0; i < pLevel->GetEntityData().size(); i++)
		{
			if (pLevel->GetEntityData()[i].GetHandle() == tr.m_iHit)
			{
				iNearest = i;
				float flDistance = tr.m_flFraction * flTraceDistance;
				flNearestSqr = flDistance*flDistance;
				break;
			}
		}
	}

	for (size_t i = 0; i < pLevel->GetEntityData().size(); i++)
	{
		CLevelEntity* pEnt = &pLevel->GetEntityData()[i];

		Vector vecCenter = pEnt->GetGlobalTransform().GetTranslation();

		AABB aabbLocalBounds = pEnt->GetBoundingBox();
		aabbLocalBounds.m_vecMaxs += Vector(0.01f, 0.01f, 0.01f);	// Make sure it's not zero size.

		AABB aabbGlobalBounds = aabbLocalBounds;
		aabbGlobalBounds += vecCenter;

		if (!GameServer()->GetRenderer()->IsSphereInFrustum(vecCenter, aabbGlobalBounds.Size().Length()/2))
			continue;

		if (pEnt->GetMaterialModel().IsValid() && !pEnt->ShouldDisableBackCulling())
		{
			if (pEnt->ShouldRenderInverted())
			{
				if (vecTrace.m_vecDir.Dot(pEnt->GetGlobalTransform().GetForwardVector()) > 0)
					continue;
			}
			else
			{
				if (vecTrace.m_vecDir.Dot(pEnt->GetGlobalTransform().GetForwardVector()) < 0)
					continue;
			}
		}

		Matrix4x4 mRotation = pEnt->GetGlobalTRS().GetMatrix4x4(true, false);
		Matrix4x4 mInverseRotation = mRotation.InvertedRT();

		Ray vecLocalTrace = mInverseRotation * vecTrace;

		if (EditorPhysics()->IsEntityAdded(pEnt))
			continue;

		Vector vecIntersection;
		if (!RayIntersectsAABB(vecLocalTrace, aabbLocalBounds, vecIntersection))
			continue;

		float flDistanceSqr = (vecLocalTrace.m_vecPos-vecIntersection).LengthSqr();

		if (iNearest == ~0)
		{
			iNearest = i;
			flNearestSqr = flDistanceSqr;
			continue;
		}

		if (flDistanceSqr >= flNearestSqr)
			continue;

		iNearest = i;
		flNearestSqr = flDistanceSqr;
	}

	return iNearest;
}

void CLevelEditor::EntitySelected()
{
	if (!m_pLevel)
		return;

	size_t iSelected = m_hEditorPanel->m_hEntities->GetSelectedNodeId();
	auto& aEntities = m_pLevel->GetEntityData();

	if (iSelected >= aEntities.size())
		return;

	Vector vecCamera = GameServer()->GetRenderer()->GetCameraPosition();
	m_flCreateObjectDistance = (vecCamera - aEntities[iSelected].GetGlobalTransform().GetTranslation()).Length();
}

void CLevelEditor::CreateEntityFromPanel(const Vector& vecPosition)
{
	auto& aEntityData = m_pLevel->GetEntityData();

	size_t iNewEntity = m_pLevel->CreateEntity(m_hCreateEntityPanel->m_hClass->GetText());
	CLevelEntity* pNewEntity = &m_pLevel->GetEntityData()[iNewEntity];

	pNewEntity->SetParameterValue("Name", m_hCreateEntityPanel->m_hNameText->GetText());

	pNewEntity->SetParameterValue("Model", m_hCreateEntityPanel->m_hModelText->GetText());
	pNewEntity->SetParameterValue("Origin", sprintf("%f %f %f", vecPosition.x, vecPosition.y, vecPosition.z));

	PopulateLevelEntityFromPanel(pNewEntity, m_hCreateEntityPanel->m_hPropertiesPanel);

	EditorPhysics()->AddEntity(pNewEntity, CT_STATIC_MESH);

	m_hEditorPanel->Layout();
	m_hEditorPanel->m_hEntities->SetSelectedNode(iNewEntity);
}

void CLevelEditor::PopulateLevelEntityFromPanel(class CLevelEntity* pEntity, CEntityPropertiesPanel* pPanel)
{
	tstring sModel;

	for (size_t i = 0; i < pPanel->m_asPropertyHandle.size(); i++)
	{
		CSaveData oSaveData;
		CSaveData* pSaveData = CBaseEntity::FindSaveDataValuesByHandle(("C" + pEntity->GetClass()).c_str(), pPanel->m_asPropertyHandle[i].c_str(), &oSaveData);
		if (strcmp(pSaveData->m_pszType, "bool") == 0)
		{
			bool bValue = pPanel->m_ahPropertyOptions[i].DowncastStatic<glgui::CCheckBox>()->GetState();

			if (bValue)
				pEntity->SetParameterValue(pPanel->m_asPropertyHandle[i], "1");
			else
				pEntity->SetParameterValue(pPanel->m_asPropertyHandle[i], "0");
		}
		else
		{
			tstring sValue = pPanel->m_ahPropertyOptions[i].DowncastStatic<glgui::CTextField>()->GetText();

			if (pPanel->m_asPropertyHandle[i] == "Model")
				CModelLibrary::AddModel(sValue);

			pEntity->SetParameterValue(pPanel->m_asPropertyHandle[i], sValue);
		}
	}
}

void CLevelEditor::DuplicateSelectedEntity()
{
	if (!m_hEditorPanel->m_hEntities->GetSelectedNode())
		return;

	size_t iNewHandle = m_pLevel->CopyEntity(m_pLevel->GetEntityData()[m_hEditorPanel->m_hEntities->GetSelectedNodeId()]);

	EditorPhysics()->AddEntity(&m_pLevel->GetEntityData()[iNewHandle], CT_STATIC_MESH);

	m_hEditorPanel->Layout();
	m_hEditorPanel->m_hEntities->SetSelectedNode(iNewHandle);
}

void CLevelEditor::CreateEntityCallback(const tstring& sArgs)
{
	m_hCreateEntityPanel->SetPos(glgui::CRootPanel::Get()->GetWidth()/2-m_hCreateEntityPanel->GetWidth()/2, 72);
	m_hCreateEntityPanel->SetVisible(true);
}

void CLevelEditor::SaveLevelCallback(const tstring& sArgs)
{
	if (m_pLevel)
		m_pLevel->SaveToFile();

}

bool CLevelEditor::KeyPress(int c)
{
	if (c == TINKER_KEY_DEL)
	{
		size_t iSelected = m_hEditorPanel->m_hEntities->GetSelectedNodeId();
		auto& aEntities = m_pLevel->GetEntityData();

		m_hEditorPanel->m_hEntities->Unselect();
		if (iSelected < aEntities.size())
		{
			EditorPhysics()->RemoveEntity(&aEntities[iSelected]);
			aEntities.erase(aEntities.begin()+iSelected);
			m_hEditorPanel->Layout();
			return true;
		}
	}

	// ; because my dvorak to qwerty key mapper works against me when the game is open, oh well.
	if ((c == 'S' || c == ';') && Application()->IsCtrlDown())
	{
		if (m_pLevel)
			m_pLevel->SaveToFile();

		return true;
	}

	// H for the same reason, my dvorak to qwerty key mapper
	if ((c == 'D' || c == 'H') && Application()->IsCtrlDown())
	{
		DuplicateSelectedEntity();
		return true;
	}

	return BaseClass::KeyPress(c);
}

bool CLevelEditor::MouseInput(int iButton, tinker_mouse_state_t iState)
{
	if (iButton == TINKER_KEY_MOUSE_RIGHT)
	{
		if (iState == TINKER_MOUSE_PRESSED)
			CVar::SetCVar("cam_free", "on");
		else
			CVar::SetCVar("cam_free", "off");
	}

	if (iState == TINKER_MOUSE_PRESSED && iButton == TINKER_KEY_MOUSE_LEFT)
	{
		if (m_hCreateEntityPanel->IsVisible() && m_hCreateEntityPanel->m_bReadyToCreate)
		{
			CreateEntityFromPanel(PositionFromMouse());
			return true;
		}

		size_t iSelected;
		if (ShouldRenderOrthographic())
		{
			int x, y;
			Application()->GetMousePosition(x, y);
			Vector vecPosition1 = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));
			Vector vecPosition0 = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 0));

			iSelected = TraceLine(Ray(vecPosition0, (vecPosition1-vecPosition0).Normalized()));
		}
		else
		{
			Vector vecCamera = GameServer()->GetRenderer()->GetCameraPosition();

			iSelected = TraceLine(Ray(vecCamera, DirectionFromMouse()));
		}

		m_hEditorPanel->m_hEntities->SetSelectedNode(iSelected);

		return true;
	}

	return false;
}

void CLevelEditor::Activate()
{
	SetCameraOrientation(GameServer()->GetCameraManager()->GetCameraPosition(), GameServer()->GetCameraManager()->GetCameraDirection());

	LoadLevel(GameServer()->GetLevel(CVar::GetCVarValue("game_level")));

	m_hEditorPanel->SetVisible(true);
	m_hCreateEntityButton->SetVisible(true);

	GetFileMenu()->AddSubmenu("Save", this, SaveLevel);

	BaseClass::Activate();
}

void CLevelEditor::Deactivate()
{
	BaseClass::Deactivate();

	EditorPhysics()->RemoveAllEntities();

	m_hEditorPanel->SetVisible(false);
	m_hCreateEntityButton->SetVisible(false);
	m_hCreateEntityPanel->SetVisible(false);

	if (m_pLevel && m_pLevel->GetEntityData().size())
		GameServer()->RestartLevel();
}

void CLevelEditor::RenderScene()
{
	if (!m_pLevel)
		return;

	TPROF("CLevelEditor::RenderEntities()");

	{
		CRenderingContext c(GameServer()->GetRenderer(), true);

		if (!GameServer()->GetRenderer()->IsDrawingBackground())
			GameServer()->GetRenderer()->DrawBackground(&c);

		c.UseProgram("model");

		c.SetUniform("bDiffuse", false);
		c.SetBlend(BLEND_ALPHA);

		c.SetUniform("vecDiffuse", Vector4D(1, 1, 1, 1));
		c.SetUniform("vecColor", Vector4D(0.7f, 0.2f, 0.2f, 0.7f));
		c.BeginRenderLines();
			c.Vertex(Vector(-10000, 0, 0));
			c.Vertex(Vector(10000, 0, 0));
		c.EndRender();

		c.SetUniform("vecColor", Vector4D(0.2f, 0.7f, 0.2f, 0.7f));
		c.BeginRenderLines();
			c.Vertex(Vector(0, -10000, 0));
			c.Vertex(Vector(0, 10000, 0));
		c.EndRender();

		c.SetUniform("vecColor", Vector4D(0.2f, 0.2f, 0.7f, 0.7f));
		c.BeginRenderLines();
			c.Vertex(Vector(0, 0, -10000));
			c.Vertex(Vector(0, 0, 10000));
		c.EndRender();

		c.SetUniform("vecColor", Vector4D(1.0f, 1.0f, 1.0f, 0.2f));

		int i;

		Vector vecStartX(-10, -10, 0);
		Vector vecEndX(-10, 10, 0);
		Vector vecStartZ(-10, -10, 0);
		Vector vecEndZ(10, -10, 0);

		c.BeginRenderLines();
		for (i = 0; i <= 20; i++)
		{
			if (i != 10)
			{
				c.Vertex(vecStartX);
				c.Vertex(vecEndX);
				c.Vertex(vecStartZ);
				c.Vertex(vecEndZ);
			}

			vecStartX.x += 1;
			vecEndX.x += 1;
			vecStartZ.y += 1;
			vecEndZ.y += 1;
		}
		c.EndRender();

		c.SetUniform("vecColor", Vector4D(1.0f, 1.0f, 1.0f, 1.0f));
	}

	GameServer()->GetRenderer()->SetRenderingTransparent(false);

	auto& aEntityData = m_pLevel->GetEntityData();
	for (size_t i = 0; i < aEntityData.size(); i++)
		RenderEntity(i);

	GameServer()->GetRenderer()->SetRenderingTransparent(true);

	for (size_t i = 0; i < aEntityData.size(); i++)
		RenderEntity(i);

	if (m_hCreateEntityPanel->IsVisible() && m_hCreateEntityPanel->m_bReadyToCreate)
		RenderCreateEntityPreview();
}

void CLevelEditor::ManipulatorUpdated(const tstring& sArguments)
{
	// Grab this before GetToyToModify since that does a layout and clobbers the list.
	size_t iSelected = m_hEditorPanel->m_hEntities->GetSelectedNodeId();

	tvector<tstring> asTokens;
	strtok(sArguments, asTokens);
	TAssert(asTokens.size() == 2);
	TAssert(stoi(asTokens[1]) == iSelected);
	TAssert(iSelected != ~0);

	if (!GetLevel())
		return;

	if (iSelected >= GetLevel()->GetEntityData().size())
		return;

	CLevelEntity* pEntity = &GetLevel()->GetEntityData()[iSelected];

	Vector vecTranslation = Manipulator()->GetTRS().m_vecTranslation;
	EAngle angRotation = Manipulator()->GetTRS().m_angRotation;
	Vector vecScaling = Manipulator()->GetTRS().m_vecScaling;

	pEntity->SetParameterValue("Origin", pretty_float(vecTranslation.x) + " " + pretty_float(vecTranslation.y) + " " + pretty_float(vecTranslation.z));
	pEntity->SetParameterValue("Angles", pretty_float(angRotation.p) + " " + pretty_float(angRotation.y) + " " + pretty_float(angRotation.r));
	pEntity->SetParameterValue("Scale", pretty_float(vecScaling.x) + " " + pretty_float(vecScaling.y) + " " + pretty_float(vecScaling.z));

	EditorPhysics()->SetEntityTransform(pEntity, pEntity->GetPhysicsTransform());

	m_hEditorPanel->LayoutEntity();
}

void CLevelEditor::DuplicateMove(const tstring& sArguments)
{
	auto& aEntityData = m_pLevel->GetEntityData();

	size_t iSelected = m_hEditorPanel->m_hEntities->GetSelectedNodeId();

	if (iSelected >= aEntityData.size())
		return;

	Vector vecTranslation = Manipulator()->GetNewTRS().m_vecTranslation;
	EAngle angRotation = Manipulator()->GetNewTRS().m_angRotation;
	Vector vecScaling = Manipulator()->GetNewTRS().m_vecScaling;

	size_t iNewObject = m_pLevel->CopyEntity(aEntityData[iSelected]);

	m_hEditorPanel->Layout();

	CLevelEntity* pEntity = &GetLevel()->GetEntityData()[iNewObject];

	pEntity->SetParameterValue("Origin", pretty_float(vecTranslation.x) + " " + pretty_float(vecTranslation.y) + " " + pretty_float(vecTranslation.z));
	pEntity->SetParameterValue("Angles", pretty_float(angRotation.p) + " " + pretty_float(angRotation.y) + " " + pretty_float(angRotation.r));
	pEntity->SetParameterValue("Scale", pretty_float(vecScaling.x) + " " + pretty_float(vecScaling.y) + " " + pretty_float(vecScaling.z));

	EditorPhysics()->AddEntity(pEntity, CT_STATIC_MESH);
	EditorPhysics()->SetEntityTransform(pEntity, pEntity->GetPhysicsTransform());

	m_hEditorPanel->m_hEntities->SetSelectedNode(aEntityData.size()-1);

	m_hEditorPanel->LayoutEntity();
}
