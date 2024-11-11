#include "Camera.h"
#include "Application.h"
#include "Wrapper.h"
#include "Pera.h"

using namespace DirectX;

void Camera::SetCBVToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs)
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	auto handle = heap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += _dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * numDescs;
	cbvDesc.BufferLocation = _sceneTransBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(_sceneTransBuff->GetDesc().Width);

	_dx->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
}

bool Camera::Init()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneTransMatrix) + 0xff) & ~0xff);

	_dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_sceneTransBuff.ReleaseAndGetAddressOf())
	);
	auto result = _sceneTransBuff->Map(0, nullptr, (void**)&_sceneTransMatrix);
	if (FAILED(result)) return false;

	SetCBVToHeap(_pera->GetHeap(), 8);
	return true;
}

void Camera::CalcSceneTrans()
{
	_sceneTransMatrix->view = XMMatrixLookAtLH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up));;

	_sceneTransMatrix->projection = XMMatrixPerspectiveFovLH(
		XM_PIDIV4,
		static_cast<float>(Application::GetWindowSize().cx) / static_cast<float>(Application::GetWindowSize().cy),
		1.0f,
		800.0f);
	XMVECTOR det;
	_sceneTransMatrix->invProjection = XMMatrixInverse(&det, _sceneTransMatrix->view * _sceneTransMatrix->projection);
	XMFLOAT4 planeVec = XMFLOAT4(0, 1, 0, 0);
	_sceneTransMatrix->shadow = XMMatrixShadow(
		XMLoadFloat4(&planeVec),
		XMLoadFloat3(&lightVec));
	_sceneTransMatrix->shadowOffsetY = XMMatrixTranslation(0, 15, 0);

	_sceneTransMatrix->invShadowOffsetY = XMMatrixInverse(&det, _sceneTransMatrix->shadowOffsetY);
	_sceneTransMatrix->lightVec = lightVec;
	_sceneTransMatrix->eye = eye;
	auto lightPos = XMLoadFloat3(&target) +
		XMVector3Normalize(XMLoadFloat3(&lightVec)) *
		XMVector3Length(XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&eye))).m128_f32[0];
	_sceneTransMatrix->lightCamera =
		XMMatrixLookAtLH(lightPos, XMLoadFloat3(&target), XMLoadFloat3(&up)) *
		XMMatrixOrthographicLH(150, 150, 1.0f, 800.0f);
	_sceneTransMatrix->lightView = XMMatrixLookAtLH(lightPos, XMLoadFloat3(&target), XMLoadFloat3(&up));
}


Camera::Camera(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera) : _dx(dx), _pera(pera),
eye(40, 20, 150),
rotate(0, 0, 0),
target(20, 30, 30),
up(0, 1, 0),
lightVec(30, 30, 60),
_sceneTransMatrix(nullptr)
{
}

Camera::~Camera()
{
}
