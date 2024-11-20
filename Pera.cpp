#include "Pera.h"
#include "Wrapper.h"

using namespace std;
using namespace DirectX;

bool Pera::VertexInit()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(pv));
	auto result = _dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(vertexBuffer.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) return false;

	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(pv);
	vbView.StrideInBytes = sizeof(PeraVertex);

	PeraVertex* PeraMap = nullptr;
	vertexBuffer->Map(0, nullptr, (void**)&PeraMap);
	copy(begin(pv), end(pv), PeraMap);
	vertexBuffer->Unmap(0, nullptr);
	return true;
}

bool Pera::HeapInit()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 11;

	auto result = _dx->GetDevice()->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(_peraHeaps.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	return true;
}

void Pera::SetSRV(Microsoft::WRL::ComPtr<ID3D12Resource> buffer, DXGI_FORMAT format)
{
	std::pair< Microsoft::WRL::ComPtr<ID3D12Resource>, DXGI_FORMAT> srvBuff = { buffer, format };
	srvBuffs.emplace_back(srvBuff);
}

void Pera::SetCBV(Microsoft::WRL::ComPtr<ID3D12Resource> buffer)
{
	cbvBuffs.emplace_back(buffer);
}

void Pera::SetViews()
{
	for (int i = 0; i < cbvBuffs.size(); ++i)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = cbvBuffs[i]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = static_cast<UINT>(cbvBuffs[i]->GetDesc().Width);
		auto handle = _peraHeaps->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * i;
		_dx->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
	}
	for (int i = 0; i < srvBuffs.size(); ++i)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = srvBuffs[i].second;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		auto handle = _peraHeaps->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (i + cbvBuffs.size());
		_dx->GetDevice()->CreateShaderResourceView(srvBuffs[i].first.Get(), &srvDesc, handle);
	}
	
}

Pera::Pera(std::shared_ptr<Wrapper> dx) : _dx(dx)
{
}

bool Pera::Init()
{
	if (!VertexInit()) return false;
	if (!HeapInit()) return false;
	return true;
}

void Pera::Draw() const
{
	_dx->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	_dx->GetCommandList()->IASetVertexBuffers(0, 1, &vbView);
	ID3D12DescriptorHeap* heaps[] = { _peraHeaps.Get() };

	_dx->GetCommandList()->SetDescriptorHeaps(1, heaps);
	auto handle = _peraHeaps->GetGPUDescriptorHandleForHeapStart();
	_dx->GetCommandList()->SetGraphicsRootDescriptorTable(
		0,
		handle);
	
	_dx->GetCommandList()->DrawInstanced(4, 1, 0, 0);
}


Pera::~Pera()
{
}

