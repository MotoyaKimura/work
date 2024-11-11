#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>

class Wrapper;
class Camera
{
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> _sceneTransBuff = nullptr;
	std::shared_ptr<Wrapper> _dx;

	struct SceneTransMatrix {
		DirectX::XMMATRIX view;//ビュー
		DirectX::XMMATRIX projection;//プロジェクション
		DirectX::XMMATRIX invProjection;
		DirectX::XMMATRIX shadow;
		DirectX::XMMATRIX shadowOffsetY;
		DirectX::XMMATRIX invShadowOffsetY;
		DirectX::XMMATRIX lightView;
		DirectX::XMMATRIX lightCamera;
		DirectX::XMFLOAT3 lightVec;
		DirectX::XMFLOAT3 eye;
	};
	SceneTransMatrix* _sceneTransMatrix;

	DirectX::XMFLOAT3 lightVec;
	DirectX::XMFLOAT3 eye;
	DirectX::XMFLOAT3 rotate;
	DirectX::XMFLOAT3 target;
	DirectX::XMFLOAT3 up;

public:
	void CalcSceneTrans();
	bool SceneTransBuffInit();
	void SetCBVToHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, UINT numDescs);
	Microsoft::WRL::ComPtr<ID3D12Resource> GetSceneTransBuff() const { return _sceneTransBuff.Get(); }
	DirectX::XMFLOAT3* GetEyePos() { return &eye; }
	DirectX::XMFLOAT3* GetTargetPos() { return &target; }
	Camera(std::shared_ptr<Wrapper> dx);
	~Camera();
};
