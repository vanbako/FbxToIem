#include "pch.h"
#include "FbxMgr.h"

using namespace Ion;

Convert::FbxMgr::FbxMgr()
	: mpManager{ fbxsdk::FbxManager::Create() }
{
	fbxsdk::FbxIOSettings* pIoSettings{ fbxsdk::FbxIOSettings::Create(mpManager, IOSROOT) };
	mpManager->SetIOSettings(pIoSettings);
}

Convert::FbxMgr::~FbxMgr()
{
	mpManager->Destroy();
}

fbxsdk::FbxManager* Ion::Convert::FbxMgr::GetFbxManager()
{
	return mpManager;
}
