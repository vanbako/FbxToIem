#include "pch.h"
#include "Mesh.h"
#include "FbxMgr.h"
#include "BinOfstream.h"
#include "MeshDataType.h"

using namespace Ion;

Convert::Mesh::Mesh(FbxMgr* pFbxMgr, fbxsdk::FbxNode* pNode)
	: mpFbxMgr{ pFbxMgr }
	, mpNode{ pNode }
	, mpMesh{ pNode->GetMesh() }
	, mName{}
	, mFullName{}
	, mNormals{}
	, mBinormals{}
	, mTangents{}
	, mUVs{}
	, mColors{}
{
	mName = std::string{ pNode->GetName() } + std::string{ mpMesh->GetName() };
	mFullName = "../Resources/Model/" + mName + ".iem";
	GetNormals();
	GetBinormals();
	GetTangents();
	GetUVs();
	GetColors();
}

Convert::Mesh::~Mesh()
{
}

const std::string Convert::Mesh::GetName() const
{
	return mName;
}

void Convert::Mesh::WriteIem() const
{
	Core::BinOfstream file{ mFullName };
	// Major & Minor version
	file.Write<unsigned char>(unsigned char(1u));
	file.Write<unsigned char>(unsigned char(0x3du));
	// Header
	file.Write<char>(char(MeshDataType::HEADER));
	file.Write<unsigned int>(unsigned int(sizeof(unsigned char) + mName.length() + sizeof(unsigned int) + sizeof(unsigned int)));
	file.WriteByteLenString(mName);
	file.Write<unsigned int>(unsigned int(mpMesh->GetControlPointsCount()));
	file.Write<unsigned int>(unsigned int(mpMesh->GetPolygonCount() * 3));
	// Vertices
	file.Write<char>(char(MeshDataType::POSITIONS));
	file.Write<unsigned int>(mpMesh->GetControlPointsCount() * 3 * sizeof(float));
	fbxsdk::FbxVector4* pVector4{ mpMesh->GetControlPoints() };
	for (int i{ 0 }; i < mpMesh->GetControlPointsCount(); ++i)
	{
		file.Write<float>(float(pVector4[i].mData[0]));
		file.Write<float>(float(pVector4[i].mData[1]));
		file.Write<float>(float(pVector4[i].mData[2]));
	}
	// Indices
	file.Write<char>(char(MeshDataType::INDICES));
	file.Write<unsigned int>(mpMesh->GetPolygonCount() * 3 * sizeof(int));
	for (int i{ 0 }; i < mpMesh->GetPolygonCount(); ++i)
	{
		if (mpMesh->GetPolygonSize(i) != 3)
			throw std::runtime_error("Triangulation must have failed\n");
		for (int j{ 0 }; j < mpMesh->GetPolygonSize(i); ++j)
			file.Write<int>(int(mpMesh->GetPolygonVertex(i, j)));
	}
	// Normals
	if (!mNormals.empty())
	{
		file.Write<char>(char(MeshDataType::NORMALS));
		file.Write<unsigned int>(mpMesh->GetControlPointsCount() * 3 * sizeof(float));
		for (auto& normal : mNormals)
		{
			file.Write<float>(float(normal[0]));
			file.Write<float>(float(normal[1]));
			file.Write<float>(float(normal[2]));
		}
	}
	// Binormals
	if (!mBinormals.empty())
	{
		file.Write<char>(char(MeshDataType::BINORMALS));
		file.Write<unsigned int>(mpMesh->GetControlPointsCount() * 3 * sizeof(float));
		for (auto& binormal : mBinormals)
		{
			file.Write<float>(float(binormal[0]));
			file.Write<float>(float(binormal[1]));
			file.Write<float>(float(binormal[2]));
		}
	}
	// Tangents
	if (!mTangents.empty())
	{
		file.Write<char>(char(MeshDataType::TANGENTS));
		file.Write<unsigned int>(mpMesh->GetControlPointsCount() * 3 * sizeof(float));
		for (auto& tangent : mTangents)
		{
			file.Write<float>(float(tangent[0]));
			file.Write<float>(float(tangent[1]));
			file.Write<float>(float(tangent[2]));
		}
	}
	// UVs
	if (!mUVs.empty())
	{
		file.Write<char>(char(MeshDataType::TEXCOORDS));
		file.Write<unsigned int>(mpMesh->GetControlPointsCount() * 2 * sizeof(float));
		file.Write<USHORT>(USHORT(1));
		for (auto& uv : mUVs)
		{
			file.Write<float>(float(uv[0]));
			file.Write<float>(float(uv[1]));
		}
	}
	// Colors
	if (!mColors.empty())
	{
		file.Write<char>(char(MeshDataType::COLORS));
		file.Write<unsigned int>(mpMesh->GetControlPointsCount() * 4 * sizeof(float));
		for (auto& color : mColors)
		{
			file.Write<float>(float(color.mRed));
			file.Write<float>(float(color.mGreen));
			file.Write<float>(float(color.mBlue));
			file.Write<float>(float(color.mAlpha));
		}
	}
	// End
	file.Write<char>(char(MeshDataType::END));
}

void Convert::Mesh::GetNormals()
{
	mpMesh->GenerateNormals(false);
	fbxsdk::FbxGeometryElementNormal* pNormal{ mpMesh->GetElementNormal() };
	if (pNormal == nullptr)
		return;
	mNormals.assign(mpMesh->GetControlPointsCount(), fbxsdk::FbxVectorTemplate3<float>{});
	switch (pNormal->GetMappingMode())
	{
	case fbxsdk::FbxGeometryElement::eByControlPoint:
		for (int vertexIndex{ 0 }; vertexIndex < mpMesh->GetControlPointsCount(); ++vertexIndex)
		{
			int normalIndex{ 0 };
			switch (pNormal->GetReferenceMode())
			{
			case fbxsdk::FbxGeometryElement::eDirect:
				normalIndex = vertexIndex;
				break;
			case fbxsdk::FbxGeometryElement::eIndexToDirect:
				normalIndex = pNormal->GetIndexArray().GetAt(vertexIndex);
				break;
			}
			fbxsdk::FbxVector4 normal{ pNormal->GetDirectArray().GetAt(normalIndex) };
			mNormals[vertexIndex] = fbxsdk::FbxVectorTemplate3<float>{ float(normal[0]), float(normal[1]), float(normal[2]) };
		}
		break;
	case fbxsdk::FbxGeometryElement::eByPolygonVertex:
		int indexByPolygonVertex{ 0 };
		for (int polygonIndex{ 0 }; polygonIndex < mpMesh->GetPolygonCount(); ++polygonIndex)
		{
			if (mpMesh->GetPolygonSize(polygonIndex) != 3)
				throw std::runtime_error("Triangulation must have failed\n");
			for (int i{ 0 }; i < mpMesh->GetPolygonSize(polygonIndex); ++i)
			{
				int normalIndex{ 0 };
				switch (pNormal->GetReferenceMode())
				{
				case fbxsdk::FbxGeometryElement::eDirect:
					normalIndex = indexByPolygonVertex;
					break;
				case fbxsdk::FbxGeometryElement::eIndexToDirect:
					normalIndex = pNormal->GetIndexArray().GetAt(indexByPolygonVertex);
					break;
				}
				fbxsdk::FbxVector4 normal{ pNormal->GetDirectArray().GetAt(normalIndex) };
				mNormals[mpMesh->GetPolygonVertex(polygonIndex, i)] = fbxsdk::FbxVectorTemplate3<float>{ float(normal[0]), float(normal[1]), float(normal[2]) };
				++indexByPolygonVertex;
			}
		}
		break;
	}
}

void Convert::Mesh::GetBinormals()
{
	fbxsdk::FbxGeometryElementBinormal* pBinormal{ mpMesh->GetElementBinormal() };
	if (pBinormal == nullptr)
		return;
	mBinormals.assign(mpMesh->GetControlPointsCount(), fbxsdk::FbxVectorTemplate3<float>{});
	switch (pBinormal->GetMappingMode())
	{
	case fbxsdk::FbxGeometryElement::eByControlPoint:
		for (int vertexIndex{ 0 }; vertexIndex < mpMesh->GetControlPointsCount(); ++vertexIndex)
		{
			int binormalIndex{ 0 };
			switch (pBinormal->GetReferenceMode())
			{
			case fbxsdk::FbxGeometryElement::eDirect:
				binormalIndex = vertexIndex;
				break;
			case fbxsdk::FbxGeometryElement::eIndexToDirect:
				binormalIndex = pBinormal->GetIndexArray().GetAt(vertexIndex);
				break;
			}
			fbxsdk::FbxVector4 binormal{ pBinormal->GetDirectArray().GetAt(binormalIndex) };
			mBinormals[vertexIndex] = fbxsdk::FbxVectorTemplate3<float>{ float(binormal[0]), float(binormal[1]), float(binormal[2]) };
		}
		break;
	case fbxsdk::FbxGeometryElement::eByPolygonVertex:
		int indexByPolygonVertex{ 0 };
		for (int polygonIndex{ 0 }; polygonIndex < mpMesh->GetPolygonCount(); ++polygonIndex)
		{
			int polygonSize{ mpMesh->GetPolygonSize(polygonIndex) };
			for (int i{ 0 }; i < polygonSize; ++i)
			{
				int binormalIndex{ 0 };
				switch (pBinormal->GetReferenceMode())
				{
				case fbxsdk::FbxGeometryElement::eDirect:
					binormalIndex = indexByPolygonVertex;
					break;
				case fbxsdk::FbxGeometryElement::eIndexToDirect:
					binormalIndex = pBinormal->GetIndexArray().GetAt(indexByPolygonVertex);
					break;
				}
				fbxsdk::FbxVector4 binormal{ pBinormal->GetDirectArray().GetAt(binormalIndex) };
				mBinormals[mpMesh->GetPolygonVertex(polygonIndex, i)] = fbxsdk::FbxVectorTemplate3<float>{ float(binormal[0]), float(binormal[1]), float(binormal[2]) };
				++indexByPolygonVertex;
			}
		}
		break;
	}
}

void Convert::Mesh::GetTangents()
{
	mpMesh->GenerateTangentsData(NULL, false);
	fbxsdk::FbxGeometryElementTangent* pTangent{ mpMesh->GetElementTangent() };
	if (pTangent == nullptr)
		return;
	mTangents.assign(mpMesh->GetControlPointsCount(), fbxsdk::FbxVectorTemplate3<float>{});
	switch (pTangent->GetMappingMode())
	{
	case fbxsdk::FbxGeometryElement::eByControlPoint:
		for (int vertexIndex{ 0 }; vertexIndex < mpMesh->GetControlPointsCount(); ++vertexIndex)
		{
			int tangentIndex{ 0 };
			switch (pTangent->GetReferenceMode())
			{
			case fbxsdk::FbxGeometryElement::eDirect:
				tangentIndex = vertexIndex;
				break;
			case fbxsdk::FbxGeometryElement::eIndexToDirect:
				tangentIndex = pTangent->GetIndexArray().GetAt(vertexIndex);
				break;
			}
			fbxsdk::FbxVector4 tangent{ pTangent->GetDirectArray().GetAt(tangentIndex) };
			mTangents[vertexIndex] = fbxsdk::FbxVectorTemplate3<float>{ float(tangent[0]), float(tangent[1]), float(tangent[2]) };
		}
		break;
	case fbxsdk::FbxGeometryElement::eByPolygonVertex:
		int indexByPolygonVertex{ 0 };
		for (int polygonIndex{ 0 }; polygonIndex < mpMesh->GetPolygonCount(); ++polygonIndex)
		{
			int polygonSize{ mpMesh->GetPolygonSize(polygonIndex) };
			for (int i{ 0 }; i < polygonSize; ++i)
			{
				int tangentIndex{ 0 };
				switch (pTangent->GetReferenceMode())
				{
				case fbxsdk::FbxGeometryElement::eDirect:
					tangentIndex = indexByPolygonVertex;
					break;
				case fbxsdk::FbxGeometryElement::eIndexToDirect:
					tangentIndex = pTangent->GetIndexArray().GetAt(indexByPolygonVertex);
					break;
				}
				fbxsdk::FbxVector4 tangent{ pTangent->GetDirectArray().GetAt(tangentIndex) };
				mTangents[mpMesh->GetPolygonVertex(polygonIndex, i)] = fbxsdk::FbxVectorTemplate3<float>{ float(tangent[0]), float(tangent[1]), float(tangent[2]) };
				++indexByPolygonVertex;
			}
		}
		break;
	}
}

void Convert::Mesh::GetUVs()
{
	mUVs.assign(mpMesh->GetControlPointsCount(), fbxsdk::FbxVectorTemplate2<float>{});
	fbxsdk::FbxGeometryElementUV* pUV{ mpMesh->GetElementUV() };
	if (pUV == nullptr)
		return;
	switch (pUV->GetMappingMode())
	{
	case fbxsdk::FbxGeometryElement::eByControlPoint:
		for (int vertexIndex{ 0 }; vertexIndex < mpMesh->GetControlPointsCount(); ++vertexIndex)
		{
			int uvIndex{ 0 };
			switch (pUV->GetReferenceMode())
			{
			case fbxsdk::FbxGeometryElement::eDirect:
				uvIndex = vertexIndex;
				break;
			case fbxsdk::FbxGeometryElement::eIndexToDirect:
				uvIndex = pUV->GetIndexArray().GetAt(vertexIndex);
				break;
			}
			fbxsdk::FbxVector4 uv{ pUV->GetDirectArray().GetAt(uvIndex) };
			mUVs[vertexIndex] = fbxsdk::FbxVectorTemplate2<float>{ float(uv[0]), float(uv[1]) };
		}
		break;
	case fbxsdk::FbxGeometryElement::eByPolygonVertex:
		int indexByPolygonVertex{ 0 };
		for (int polygonIndex{ 0 }; polygonIndex < mpMesh->GetPolygonCount(); ++polygonIndex)
		{
			int polygonSize{ mpMesh->GetPolygonSize(polygonIndex) };
			for (int i{ 0 }; i < polygonSize; ++i)
			{
				int uvIndex{ 0 };
				switch (pUV->GetReferenceMode())
				{
				case fbxsdk::FbxGeometryElement::eDirect:
					uvIndex = indexByPolygonVertex;
					break;
				case fbxsdk::FbxGeometryElement::eIndexToDirect:
					uvIndex = pUV->GetIndexArray().GetAt(indexByPolygonVertex);
					break;
				}
				fbxsdk::FbxVector4 uv{ pUV->GetDirectArray().GetAt(uvIndex) };
				mUVs[mpMesh->GetPolygonVertex(polygonIndex, i)] = fbxsdk::FbxVectorTemplate2<float>{ float(uv[0]), float(uv[1]) };
				++indexByPolygonVertex;
			}
		}
		break;
	}
}

void Convert::Mesh::GetColors()
{
	mColors.assign(mpMesh->GetControlPointsCount(), fbxsdk::FbxColor{});
	fbxsdk::FbxGeometryElementVertexColor* pColor{ mpMesh->GetElementVertexColor() };
	if (pColor == nullptr)
		return;
	switch (pColor->GetMappingMode())
	{
	case fbxsdk::FbxGeometryElement::eByControlPoint:
		for (int vertexIndex{ 0 }; vertexIndex < mpMesh->GetControlPointsCount(); ++vertexIndex)
		{
			int colorIndex{ 0 };
			switch (pColor->GetReferenceMode())
			{
			case fbxsdk::FbxGeometryElement::eDirect:
				colorIndex = vertexIndex;
				break;
			case fbxsdk::FbxGeometryElement::eIndexToDirect:
				colorIndex = pColor->GetIndexArray().GetAt(vertexIndex);
				break;
			}
			fbxsdk::FbxColor color{ pColor->GetDirectArray().GetAt(colorIndex) };
			mColors[vertexIndex] = color;
		}
		break;
	case fbxsdk::FbxGeometryElement::eByPolygonVertex:
		int indexByPolygonVertex{ 0 };
		for (int polygonIndex{ 0 }; polygonIndex < mpMesh->GetPolygonCount(); ++polygonIndex)
		{
			int polygonSize{ mpMesh->GetPolygonSize(polygonIndex) };
			for (int i{ 0 }; i < polygonSize; ++i)
			{
				int colorIndex{ 0 };
				switch (pColor->GetReferenceMode())
				{
				case fbxsdk::FbxGeometryElement::eDirect:
					colorIndex = indexByPolygonVertex;
					break;
				case fbxsdk::FbxGeometryElement::eIndexToDirect:
					colorIndex = pColor->GetIndexArray().GetAt(indexByPolygonVertex);
					break;
				}
				fbxsdk::FbxColor color{ pColor->GetDirectArray().GetAt(colorIndex) };
				mColors[mpMesh->GetPolygonVertex(polygonIndex, i)] = color;
				++indexByPolygonVertex;
			}
		}
		break;
	}
}
