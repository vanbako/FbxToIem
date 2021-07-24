#pragma once
#include "fbxsdk.h"
#include "Mesh.h"

namespace Ion
{
	namespace Convert
	{
		class FbxMgr;

		class File final
		{
		public:
			explicit File(FbxMgr* pFbxMgr, const std::string& name);
			~File();
			File(const File& other) = delete;
			File(File&& other) noexcept = delete;
			File& operator=(const File& other) = delete;
			File& operator=(File&& other) noexcept = delete;

			void WriteMeshes() const;
			const std::vector<Convert::Mesh>& GetMeshes() const;
		private:
			FbxMgr* mpFbxMgr;
			std::string mName;
			std::vector<Convert::Mesh> mMeshes;

			void ProcessNode(fbxsdk::FbxNode* pNode);
		};
	}
}
