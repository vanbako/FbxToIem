#pragma once
#include "fbxsdk.h"

namespace Ion
{
	namespace Convert
	{
		class FbxMgr;

		class Mesh final
		{
		public:
			explicit Mesh(FbxMgr* pFbxMgr, fbxsdk::FbxNode* pNode);
			~Mesh();
			Mesh(const Mesh& other) = default;
			Mesh(Mesh&& other) noexcept = default;
			Mesh& operator=(const Mesh& other) = default;
			Mesh& operator=(Mesh&& other) noexcept = default;

			const std::string GetName() const;
			void WriteIem() const;
		private:
			FbxMgr* mpFbxMgr;
			fbxsdk::FbxNode* mpNode;
			fbxsdk::FbxMesh* mpMesh;
			std::string mName;
			std::string mFullName;
			std::vector<fbxsdk::FbxVectorTemplate3<float>> mNormals;
			std::vector<fbxsdk::FbxVectorTemplate3<float>> mBinormals;
			std::vector<fbxsdk::FbxVectorTemplate3<float>> mTangents;
			std::vector<fbxsdk::FbxVectorTemplate2<float>> mUVs;
			std::vector<fbxsdk::FbxColor> mColors;

			void GetNormals();
			void GetBinormals();
			void GetTangents();
			void GetUVs();
			void GetColors();
		};
	}
}
