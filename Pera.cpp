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

Pera::Pera(std::shared_ptr<Wrapper> dx) : _dx(dx)
{
}

bool Pera::Init()
{
	if (!VertexInit()) return false;
	
	return true;
}

void Pera::Draw()
{
	_dx->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	_dx->GetCommandList()->IASetVertexBuffers(0, 1, &vbView);
	ID3D12DescriptorHeap* heaps[] = { _dx->GetPeraSRVHeap().Get() };

	_dx->GetCommandList()->SetDescriptorHeaps(1, heaps);
	auto handle = _dx->GetPeraSRVHeap()->GetGPUDescriptorHandleForHeapStart();
	_dx->GetCommandList()->SetGraphicsRootDescriptorTable(
		0,
		handle);
	
	_dx->GetCommandList()->DrawInstanced(4, 1, 0, 0);
}

Pera::~Pera()
{
}

