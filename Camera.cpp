#include "Camera.h"
#include "Application.h"
#include "Wrapper.h"
#include "Pera.h"

using namespace DirectX;

//カメラクラス
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

//カメラ情報の初期化
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

//カメラ情報の更新
void Camera::CalcSceneTrans()
{
	//視点情報
		//視点位置
	_sceneTransMatrix->eye = eye;
		//ビュー
	_sceneTransMatrix->view = XMMatrixLookAtLH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up));;
		//プロジェクション
	_sceneTransMatrix->projection = XMMatrixPerspectiveFovLH(
		XM_PIDIV4,
		static_cast<float>(Application::GetWindowSize().cx)
		/ static_cast<float>(Application::GetWindowSize().cy),
		1.0f,
		3000.0f);
		//プロジェクションの逆行列
	XMVECTOR det;
	_sceneTransMatrix->invProjection = XMMatrixInverse(
		&det, _sceneTransMatrix->view * _sceneTransMatrix->projection
	);


	//光源情報
		//光源の位置
	_sceneTransMatrix->lightVec = lightVec;

		//光源ベクトル
	auto lightPos = XMLoadFloat3(&target) +
		XMVector3Normalize(XMLoadFloat3(&lightVec)) *
		XMVector3Length(XMVectorSubtract(XMLoadFloat3(&target),
			XMLoadFloat3(&eye))).m128_f32[0];
		//ライトビュープロジェクション
	_sceneTransMatrix->lightCamera =
		XMMatrixLookAtLH(lightPos, XMLoadFloat3(&target), XMLoadFloat3(&up)) *
		XMMatrixOrthographicLH(200, 200, 1.0f, 100.0f);
		//ライトビュー
	_sceneTransMatrix->lightView = XMMatrixLookAtLH(lightPos, XMLoadFloat3(&target), XMLoadFloat3(&up));
}

