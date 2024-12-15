#include "Camera.h"
#include "Application.h"
#include "Wrapper.h"
#include "Pera.h"

using namespace DirectX;

//�J�����N���X
Camera::Camera(std::shared_ptr<Wrapper> dx, std::shared_ptr<Pera> pera) : _dx(dx), _pera(pera),
eye(0, 2.5, -10),
rotate(0, 0, 0),
target(0, 2.5, 0),
up(0, 1, 0),
lightVec(-10, 10, -20),
_sceneTransMatrix(nullptr)
{
}

Camera::~Camera()
{
}

//�J�������̏�����
bool Camera::Init()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(
		(sizeof(SceneTransMatrix) + 0xff) & ~0xff);
	auto result = _dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_sceneTransBuff.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) return false;
	result = _sceneTransBuff->Map(
		0, nullptr, (void**)&_sceneTransMatrix
	);
	if (FAILED(result)) return false;
	_pera->SetCBV(_sceneTransBuff);
	CalcSceneTrans();
	return true;
}

//�J�������̍X�V
void Camera::CalcSceneTrans()
{
	//���_���
		//���_�ʒu
	_sceneTransMatrix->eye = eye;
		//�r���[
	_sceneTransMatrix->view = XMMatrixLookAtLH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up));;
		//�v���W�F�N�V����
	_sceneTransMatrix->projection = XMMatrixPerspectiveFovLH(
		XM_PIDIV4,
		static_cast<float>(Application::GetWindowSize().cx)
		/ static_cast<float>(Application::GetWindowSize().cy),
		1.0f,
		3000.0f);
		//�v���W�F�N�V�����̋t�s��
	XMVECTOR det;
	_sceneTransMatrix->invProjection = XMMatrixInverse(
		&det, _sceneTransMatrix->view * _sceneTransMatrix->projection
	);


	//�������
		//�����̈ʒu
	_sceneTransMatrix->lightVec = lightVec;

		//�����x�N�g��
	auto lightPos = XMLoadFloat3(&target) +
		XMVector3Normalize(XMLoadFloat3(&lightVec)) *
		XMVector3Length(XMVectorSubtract(XMLoadFloat3(&target),
			XMLoadFloat3(&eye))).m128_f32[0];
		//���C�g�r���[�v���W�F�N�V����
	_sceneTransMatrix->lightCamera =
		XMMatrixLookAtLH(lightPos, XMLoadFloat3(&target), XMLoadFloat3(&up)) *
		XMMatrixOrthographicLH(200, 200, 1.0f, 100.0f);
		//���C�g�r���[
	_sceneTransMatrix->lightView = XMMatrixLookAtLH(lightPos, XMLoadFloat3(&target), XMLoadFloat3(&up));
}

