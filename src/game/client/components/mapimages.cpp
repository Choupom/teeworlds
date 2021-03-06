/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/map.h>
#include <engine/storage.h>
#include <game/client/component.h>
#include <game/mapitems.h>

#include "mapimages.h"

CMapImages::CMapImages()
{
	m_Count = 0;
	m_MenuCount = 0;
}

void CMapImages::OnMapLoad()
{
	IMap *pMap = Kernel()->RequestInterface<IMap>();

	// unload all textures
	for(int i = 0; i < m_Count; i++)
	{
		Graphics()->UnloadTexture(m_aTextures[i]);
		m_aTextures[i] = IGraphics::CTextureHandle();
	}
	m_Count = 0;

	int Start;
	pMap->GetType(MAPITEMTYPE_IMAGE, &Start, &m_Count);
	m_Count = clamp(m_Count, 0, int(MAX_TEXTURES));

	// load new textures
	for(int i = 0; i < m_Count; i++)
	{
		int TextureFlags = 0;
		bool FoundQuadLayer = false;
		bool FoundTileLayer = false;
		for(int k = 0; k < Layers()->NumLayers(); k++)
		{
			const CMapItemLayer * const pLayer = Layers()->GetLayer(k);
			if(!FoundQuadLayer && pLayer->m_Type == LAYERTYPE_QUADS && ((const CMapItemLayerQuads * const)pLayer)->m_Image == i)
				FoundQuadLayer = true;
			if(!FoundTileLayer && pLayer->m_Type == LAYERTYPE_TILES && ((const CMapItemLayerTilemap * const)pLayer)->m_Image == i)
				FoundTileLayer = true;
		}
		if(FoundTileLayer)
			TextureFlags = FoundQuadLayer ? IGraphics::TEXLOAD_MULTI_DIMENSION : IGraphics::TEXLOAD_ARRAY_256;

		CMapItemImage *pImg = (CMapItemImage *)pMap->GetItem(Start+i, 0, 0);
		if(pImg->m_External || (pImg->m_Version > 1 && pImg->m_Format != CImageInfo::FORMAT_RGB && pImg->m_Format != CImageInfo::FORMAT_RGBA))
		{
			char Buf[256];
			char *pName = (char *)pMap->GetData(pImg->m_ImageName);
			str_format(Buf, sizeof(Buf), "mapres/%s.png", pName);
			m_aTextures[i] = Graphics()->LoadTexture(Buf, IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, TextureFlags);
		}
		else
		{
			void *pData = pMap->GetData(pImg->m_ImageData);
			m_aTextures[i] = Graphics()->LoadTextureRaw(pImg->m_Width, pImg->m_Height, pImg->m_Version == 1 ? CImageInfo::FORMAT_RGBA : pImg->m_Format, pData, CImageInfo::FORMAT_RGBA, TextureFlags);
			pMap->UnloadData(pImg->m_ImageData);
		}
	}
}

void CMapImages::OnMenuMapLoad(IMap *pMap)
{
	// unload all textures
	for(int i = 0; i < m_MenuCount; i++)
	{
		Graphics()->UnloadTexture(m_aMenuTextures[i]);
		m_aMenuTextures[i] = IGraphics::CTextureHandle();
	}
	m_MenuCount = 0;

	int Start;
	pMap->GetType(MAPITEMTYPE_IMAGE, &Start, &m_MenuCount);
	m_MenuCount = clamp(m_MenuCount, 0, int(MAX_TEXTURES));

	CLayers MenuLayers;
	MenuLayers.Init(Kernel(), pMap);

	// load new textures
	for(int i = 0; i < m_MenuCount; i++)
	{
		int TextureFlags = 0;
		bool FoundQuadLayer = false;
		bool FoundTileLayer = false;
		for(int k = 0; k < MenuLayers.NumLayers(); k++)
		{
			const CMapItemLayer * const pLayer = MenuLayers.GetLayer(k);
			if(!FoundQuadLayer && pLayer->m_Type == LAYERTYPE_QUADS && ((const CMapItemLayerQuads * const)pLayer)->m_Image == i)
				FoundQuadLayer = true;
			if(!FoundTileLayer && pLayer->m_Type == LAYERTYPE_TILES && ((const CMapItemLayerTilemap * const)pLayer)->m_Image == i)
				FoundTileLayer = true;
		}
		if(FoundTileLayer)
			TextureFlags = FoundQuadLayer ? IGraphics::TEXLOAD_MULTI_DIMENSION : IGraphics::TEXLOAD_ARRAY_256;
	
		CMapItemImage *pImg = (CMapItemImage *)pMap->GetItem(Start+i, 0, 0);
		if(pImg->m_External || (pImg->m_Version > 1 && pImg->m_Format != CImageInfo::FORMAT_RGB && pImg->m_Format != CImageInfo::FORMAT_RGBA))
		{
			char Buf[256];
			char *pName = (char *)pMap->GetData(pImg->m_ImageName);
			str_format(Buf, sizeof(Buf), "mapres/%s.png", pName);
			m_aMenuTextures[i] = Graphics()->LoadTexture(Buf, IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, TextureFlags);
		}
		else
		{
			void *pData = pMap->GetData(pImg->m_ImageData);
			m_aMenuTextures[i] = Graphics()->LoadTextureRaw(pImg->m_Width, pImg->m_Height, pImg->m_Version == 1 ? CImageInfo::FORMAT_RGBA : pImg->m_Format, pData, CImageInfo::FORMAT_RGBA, TextureFlags);
			pMap->UnloadData(pImg->m_ImageData);
		}
	}
}

IGraphics::CTextureHandle CMapImages::Get(int Index) const
{
	if(Client()->State() == IClient::STATE_ONLINE || Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return m_aTextures[clamp(Index, 0, m_Count)];
	return m_aMenuTextures[clamp(Index, 0, m_MenuCount)];
}

int CMapImages::Num() const
{
	if(Client()->State() == IClient::STATE_ONLINE || Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return m_Count;
	return m_MenuCount;
}
