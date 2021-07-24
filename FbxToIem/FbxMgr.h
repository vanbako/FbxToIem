#pragma once
#include "fbxsdk.h"

namespace Ion
{
	namespace Convert
	{
		class FbxMgr final
		{
		public:
			explicit FbxMgr();
			~FbxMgr();
			FbxMgr(const FbxMgr& other) = delete;
			FbxMgr(FbxMgr&& other) noexcept = delete;
			FbxMgr& operator=(const FbxMgr& other) = delete;
			FbxMgr& operator=(FbxMgr&& other) noexcept = delete;

			fbxsdk::FbxManager* GetFbxManager();
		private:
			fbxsdk::FbxManager* mpManager;
		};
	}
}
