#include "pch.h"
#include "File.h"
#include "FbxMgr.h"

using namespace Ion;

Convert::File::File(FbxMgr* pFbxMgr, const std::string& name)
	: mpFbxMgr{ pFbxMgr }
	, mName{ name }
{
	fbxsdk::FbxImporter* pImporter{ fbxsdk::FbxImporter::Create(mpFbxMgr->GetFbxManager(), "MyImporter") };
	pImporter->Initialize(("../Resources/Original/" + mName).c_str(), -1, mpFbxMgr->GetFbxManager()->GetIOSettings());
	fbxsdk::FbxScene* pScene{ fbxsdk::FbxScene::Create(mpFbxMgr->GetFbxManager(), "myScene") };
	pImporter->Import(pScene);
	pImporter->Destroy();
	fbxsdk::FbxGeometryConverter geometryConverter{ mpFbxMgr->GetFbxManager() };
	fbxsdk::NodeList* pAffectedNodes{ nullptr };
	geometryConverter.RemoveBadPolygonsFromMeshes(pScene, pAffectedNodes);
	geometryConverter.Triangulate(pScene, true, true);
	fbxsdk::FbxNode* pNode{ pScene->GetRootNode() };
	if (pNode)
		for (int i{ 0 }; i < pNode->GetChildCount(); ++i)
			ProcessNode(pNode->GetChild(i));
}

Convert::File::~File()
{
}

void Convert::File::WriteMeshes() const
{
	for (auto& mesh : mMeshes)
		mesh.WriteIem();
}

const std::vector<Convert::Mesh>& Convert::File::GetMeshes() const
{
	return mMeshes;
}

void Convert::File::ProcessNode(fbxsdk::FbxNode* pNode)
{
	for (int i{ 0 }; i < pNode->GetNodeAttributeCount(); ++i)
		if (pNode->GetNodeAttributeByIndex(i)->GetAttributeType() == fbxsdk::FbxNodeAttribute::eMesh)
			mMeshes.emplace_back(mpFbxMgr, pNode);
	for (int i{ 0 }; i < pNode->GetChildCount(); ++i)
		ProcessNode(pNode->GetChild(i));
}
