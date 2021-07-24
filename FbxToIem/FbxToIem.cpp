#include "pch.h"
#include "Application.h"
#include "Factory.h"
#include "MeshVC.h"
#include "ModelVC.h"
#include "TriangleVC.h"
#include "InstancedModelVC.h"
#include "AnimatedModelVC.h"
#include "Object.h"
#include "FileLogger.h"
#include "BinOfstream.h"
#include "FbxMgr.h"
#include "File.h"
#include "Mesh.h"
#include <iostream>

using namespace Ion;

// Supported files formats: FBX 5/6/7 Binary & ASCII, Collada, DXF, OBJ, 3DS

Core::Object* AddIem(size_t pos, const std::string& name, Core::Scene* pScene, Core::Canvas* pCanvas);
Core::Object* AddCamera(Core::Scene* pScene, Core::Canvas* pCanvas);

int main()
{
	{
		Convert::FbxMgr fbxMgr{};
		Convert::File file{ &fbxMgr, "Bicycle.fbx" };
		file.WriteMeshes();
		RECT rectangle{ 0, 0, 1280, 720 };
		Core::Application application{};

		// Services
		Core::ServiceLocator& serviceLocator{ application.GetServiceLocator() };
		Core::FileLogger fileLogger{ "FbxToIem.log" };
		serviceLocator.RegisterLoggerService(&fileLogger);

		Core::Scene* pScene{ application.AddScene() };

		application.Initialize();

		Core::Window* pWindow{ application.AddWindow(L"Render Window", rectangle) };
		Core::Canvas* pCanvas{ pWindow->AddCanvas(rectangle) };
		pScene->AddCanvas(pCanvas);
		pCanvas->Initialize();

		auto& meshes{ file.GetMeshes() };
		std::vector<Core::Object*> pObjects{};
		for (size_t i{ 0 }; i < meshes.size(); ++i)
			pObjects.push_back(AddIem(i, meshes[i].GetName(), pScene, pCanvas));
		Core::Object* pCamera{ AddCamera(pScene, pCanvas) };
		(pCamera);

		pScene->Initialize();

		for (auto& pObject : pObjects)
			pObject->SetIsActive(true, true);
		pCamera->SetIsActive(true, true);

		pScene->SetIsActive(true);
#ifdef _DEBUG
		std::cout << "App is running" << std::endl;
#endif
		application.Run();
		pScene->SetIsActive(false);
#ifdef _DEBUG
		std::cout << "App is shutting down" << std::endl;
#endif
		pScene->SetIsEnd(true);

	}
#ifdef _DEBUG
	std::cout << "Done" << std::endl;
#endif
}

Core::Object* AddIem(size_t pos, const std::string& name, Core::Scene* pScene, Core::Canvas* pCanvas)
{
	Core::Object* pIem{ pScene->AddObject(false) };
	Core::TransformMC* pTransformMC{ pIem->AddModelC<Core::TransformMC>(false) };
	pTransformMC->SetPosition(DirectX::XMFLOAT4{ 10.f * float(pos), 0.f, 0.f, 0.f });
	Core::ModelVC* pModelVC{ pIem->AddViewC<Core::ModelVC>(name, "iem", "PosNormTanTex_A", false, Core::Winding::CW, Core::CoordSystem::RightHanded) };
	pModelVC->AddTexture(Core::TextureType::Albedo, "SM_Bicycle.png");
	pModelVC->AddCanvas(pCanvas);
	return pIem;
}

Core::Object* AddCamera(Core::Scene* pScene, Core::Canvas* pCanvas)
{
	Core::Object* pCamera{ Core::Factory::AddCamera(pScene) };
	Core::TransformMC* pCameraTransform{ pCamera->GetModelC<Core::TransformMC>() };
	pCameraTransform->SetPosition(DirectX::XMFLOAT4{ 0.f, 20.f, -200.f, 0.f });
	pCameraTransform->SetRotation(DirectX::XMFLOAT3{ 0.f, 0.f, 0.f }, Core::AngleUnit::Degree);
	pCanvas->SetCamera(pCamera);
	return pCamera;
}
