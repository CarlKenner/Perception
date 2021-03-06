/********************************************************************
Vireio Perception: Open-Source Stereoscopic 3D Driver
Copyright (C) 2012 Andres Hernandez

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#include "D3DProxyDeviceSource.h"
#include "StereoViewFactory.h"

D3DProxyDeviceSource::D3DProxyDeviceSource(IDirect3DDevice9* pDevice):D3DProxyDevice(pDevice)
{
	OutputDebugString("D3D ProxyDev Source Created\n");
}

D3DProxyDeviceSource::~D3DProxyDeviceSource()
{
}

void D3DProxyDeviceSource::Init(ProxyHelper::ProxyConfig& cfg)
{
	OutputDebugString("D3D ProxyDev Source Init\n");
	D3DProxyDevice::Init(cfg);
	roll_mode = 1;
}

HRESULT WINAPI D3DProxyDeviceSource::EndScene()
{
	HandleControls();
	HandleTracking();
	ComputeViewTranslation();

	return D3DProxyDevice::EndScene();
}

HRESULT WINAPI D3DProxyDeviceSource::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{
	if(stereoView->initialized && stereoView->stereoEnabled)
	{
		if(eyeShutter > 0)
		{
			stereoView->UpdateEye(StereoView::LEFT_EYE);
		}
		else 
		{
			stereoView->UpdateEye(StereoView::RIGHT_EYE);
		}

		stereoView->Draw();

		eyeShutter *= -1;
	}

	return D3DProxyDevice::Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT WINAPI D3DProxyDeviceSource::SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
	if(stereoView->initialized && Vector4fCount >= 4 && validRegister(StartRegister) && (fabs(pConstantData[12]) + fabs(pConstantData[13]) + fabs(pConstantData[14]) > 0.001f))
	{
		currentMatrix = const_cast<float*>(pConstantData);

		D3DXMATRIX sourceMatrix(currentMatrix);

		D3DXMatrixTranspose(&sourceMatrix, &sourceMatrix);
			
		sourceMatrix = sourceMatrix * matViewTranslation; 

		D3DXMatrixTranspose(&sourceMatrix, &sourceMatrix);

		currentMatrix = (float*)sourceMatrix;

		return D3DProxyDevice::SetVertexShaderConstantF(StartRegister, currentMatrix, Vector4fCount);
	}

	return D3DProxyDevice::SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

bool D3DProxyDeviceSource::validRegister(UINT reg)
{
	switch(game_type)
	{
	case SOURCE_L4D:
		if(reg == 4 || reg == 8 || reg == 51) 
			return true;
		else return false;
		break;
	default:
		return true;
	}
}

int D3DProxyDeviceSource::getMatrixIndex()
{
	switch(game_type)
	{
	case SOURCE_L4D:
		return 0;
		break;
	default:
		return 0;
	}
}