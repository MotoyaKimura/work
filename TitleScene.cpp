#include "TitleScene.h"
#include "Application.h"
#include "GameScene.h"
#include "Wrapper.h"
#include "Pera.h"
#include "Model.h"
#include "Renderer.h"
#include "Keyboard.h"

std::shared_ptr<Pera> TitleScene::_pera = nullptr;
std::shared_ptr<Renderer> TitleScene::_renderer = nullptr;
std::shared_ptr<Model> TitleScene::_model = nullptr;
std::shared_ptr<Model> TitleScene::_model2 = nullptr;
std::shared_ptr<Model> TitleScene::_model3 = nullptr;
std::shared_ptr<Model>TitleScene::_model4 = nullptr;
std::shared_ptr<Keyboard> TitleScene::_keyboard = nullptr;

using namespace DirectX;

void TitleScene::FadeoutUpdate()
{
	_controller.ChangeScene(new GameScene(_controller));
}

bool TitleScene::SceneInit()
{
	if (!CreatePeraRTVAndSRV()) return false;
	if (!RSMBuffInit()) return false;
	if (!DepthBuffInit()) return false;
	if (!CreateDepthHeap()) return false;
	if (!CreateDepthView()) return false;
	if (!LightDepthBuffInit()) return false;
	if (!CreateSSAOBuff()) return false;
	if (!CreateSSAOHeap()) return false;
	if (!CreateSSAORTVAndSRV()) return false;
	if (!SceneTransBuffInit()) return false;
	CalcSceneTrans();

	_pera.reset(new Pera(Application::_dx, *this));
	if (!_pera->Init())
	{
		Application::DebugOutputFormatString("ペラポリゴンの初期化エラー\n ");
		return false;
	}

	_keyboard.reset(new Keyboard(Application::hwnd));

	_renderer.reset(new Renderer(Application::_dx, _pera, _keyboard, *this));
	if (!_renderer->Init())
	{
		Application::DebugOutputFormatString("レンダラー周りの初期化エラー\n ");
		return false;
	}

	_model.reset(new Model(Application::_dx, *this));
	if (!_model->Load("modelData/bunny/bunny.obj")) return false;

	if (!_model->Init())
	{
		Application::DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model->Move(30, 0, 30);

	_renderer->AddModel(_model);

	_model2 = std::make_shared<Model>(Application::_dx, *this);
	if (!_model2->Load("modelData/RSMScene/floor/floor.obj")) return false;
	if (!_model2->Init())
	{
		Application::DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model2->Move(30, 0, 30);
	_renderer->AddModel(_model2);

	_model3 = std::make_shared<Model>(Application::_dx, *this);
	if (!_model3->Load("modelData/RSMScene/wall/wall_red.obj")) return false;
	if (!_model3->Init())
	{
		Application::DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model3->Move(30, 30, 0);
	_renderer->AddModel(_model3);

	_model4 = std::make_shared<Model>(Application::_dx, *this);
	if (!_model4->Load("modelData/RSMScene/wall/wall_green.obj")) return false;
	if (!_model4->Init())
	{
		Application::DebugOutputFormatString("モデルの初期化エラー\n ");
		return false;
	}
	_model4->Move(0, 30, 30);
	_renderer->AddModel(_model4);

	return true;
}

void TitleScene::SceneUpdate(void)
{
	Application::_dx->ResizeBackBuffers();
	_renderer->ResizeBuffers();
}

void TitleScene::SceneRender(void)
{
	_renderer->Move();
	_renderer->Update();
	Update();

	BeginDrawShade();
	_renderer->BeforeDrawShade();
	_renderer->DrawShade();
	EndDrawShade();

	BeginDrawTeapot();
	_renderer->BeforeDrawTeapot();
	_renderer->DrawTeapot();
	EndDrawTeapot();

	BeginDrawSSAO();
	_renderer->BeforeDrawSSAO();
	_renderer->DrawSSAO();
	EndDrawSSAO();

	BeginDrawPera();
	_renderer->BeforeDrawPera();
	_renderer->DrawPera();
	EndDrawPera();
	
	Application::_dx->ExecuteCommand();
	Application::_dx->Flip();
}

void TitleScene::SceneFinal(void)
{
}

const char* TitleScene::GetSceneName(void)
{
	return "TitleScene";
}

bool TitleScene::CreatePeraRTVAndSRV()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = Application::_dx->GetBackBuffer()->GetDesc();
	float clsClr[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(
		DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);
	for (auto& res : _peraBuff) {
		auto result = Application::_dx->GetDevice()->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue,
			IID_PPV_ARGS(res.ReleaseAndGetAddressOf()));
		if (FAILED(result)) return false;
	}

	auto heapDesc = Application::_dx->GetRTVHeap()->GetDesc();
	heapDesc.NumDescriptors = 2;

	auto result = Application::_dx->GetDevice()->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(_peraRTVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	auto handle = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (auto& res : _peraBuff) {
		Application::_dx->GetDevice()->CreateRenderTargetView(
			res.Get(),
			nullptr,
			handle);
		handle.ptr += Application::_dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 9;

	_peraViewMap["_peraBuff"] = peraSRVHeapNum;
	peraSRVHeapNum += _peraBuff.size();
	_peraViewMap["_RSMBuff"] = peraSRVHeapNum;
	peraSRVHeapNum += _RSMBuff.size();
	_peraViewMap["_depthBuff"] = peraSRVHeapNum++;
	_peraViewMap["_lightDepthBuff"] = peraSRVHeapNum++;
	_peraViewMap["_ssaoBuff"] = peraSRVHeapNum++;
	_peraViewMap["_sceneTransBuff"] = peraSRVHeapNum++;

	result = Application::_dx->GetDevice()->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(_peraSRVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = rtvDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	handle = _peraSRVHeap->GetCPUDescriptorHandleForHeapStart();


	int cnt = _peraViewMap["_peraBuff"];
	for (auto& res : _peraBuff) {
		handle.ptr += Application::_dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * cnt++;
		Application::_dx->GetDevice()->CreateShaderResourceView(
			res.Get(),
			&srvDesc,
			handle);
	}
	return true;
}


bool TitleScene::RSMBuffInit()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = Application::_dx->GetBackBuffer()->GetDesc();
	resDesc.Width = shadow_difinition;
	resDesc.Height = shadow_difinition;
	float clsClr[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(
		DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);
	for (auto& res : _RSMBuff) {
		auto result = Application::_dx->GetDevice()->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue,
			IID_PPV_ARGS(res.ReleaseAndGetAddressOf()));
		if (FAILED(result)) return false;
	}

	auto heapDesc = Application::_dx->GetRTVHeap()->GetDesc();
	heapDesc.NumDescriptors = 3;

	auto result = Application::_dx->GetDevice()->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(_RSMRTVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	auto handle = _RSMRTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (auto& res : _RSMBuff) {
		Application::_dx->GetDevice()->CreateRenderTargetView(
			res.Get(),
			nullptr,
			handle);
		handle.ptr += Application::_dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = rtvDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


	int cnt = _peraViewMap["_RSMBuff"];
	for (auto& res : _RSMBuff) {
		handle = _peraSRVHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += Application::_dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * cnt++;
		Application::_dx->GetDevice()->CreateShaderResourceView(
			res.Get(),
			&srvDesc,
			handle);
	}

	return true;
}

bool TitleScene::DepthBuffInit()
{
	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = Application::GetWindowSize().cx;
	depthResDesc.Height = Application::GetWindowSize().cy;
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthResDesc.SampleDesc.Count = 1;
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;


	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	auto result = Application::_dx->GetDevice()->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(_depthBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;



	return true;
}

bool TitleScene::CreateDepthHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 2;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	auto result = Application::_dx->GetDevice()->CreateDescriptorHeap(
		&dsvHeapDesc,
		IID_PPV_ARGS(_dsvHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	return true;
}


bool TitleScene::CreateDepthView()
{

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	Application::_dx->GetDevice()->CreateDepthStencilView(
		_depthBuff.Get(),
		&dsvDesc,
		_dsvHeap->GetCPUDescriptorHandleForHeapStart());


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


	auto handle = _peraSRVHeap->GetCPUDescriptorHandleForHeapStart();

	handle.ptr += Application::_dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * _peraViewMap["_depthBuff"];

	Application::_dx->GetDevice()->CreateShaderResourceView(
		_depthBuff.Get(),
		&srvDesc,
		handle);
	return true;
}





bool TitleScene::LightDepthBuffInit()
{
	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = shadow_difinition;
	depthResDesc.Height = shadow_difinition;
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthResDesc.SampleDesc.Count = 1;
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	auto result = Application::_dx->GetDevice()->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(_lightDepthBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	auto handle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += Application::_dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	Application::_dx->GetDevice()->CreateDepthStencilView(
		_lightDepthBuff.Get(),
		&dsvDesc,
		handle);


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


	handle = _peraSRVHeap->GetCPUDescriptorHandleForHeapStart();


	handle.ptr += Application::_dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * _peraViewMap["_lightDepthBuff"];
	Application::_dx->GetDevice()->CreateShaderResourceView(
		_lightDepthBuff.Get(),
		&srvDesc,
		handle);

	return true;
}

bool TitleScene::CreateSSAOBuff()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = Application::_dx->GetBackBuffer()->GetDesc();
	resDesc.Format = DXGI_FORMAT_R32_FLOAT;
	float clsClr[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(
		DXGI_FORMAT_R32_FLOAT, clsClr);
	auto result = Application::_dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(_ssaoBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	return true;
}

bool TitleScene::CreateSSAOHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	auto result = Application::_dx->GetDevice()->CreateDescriptorHeap(
		&desc,
		IID_PPV_ARGS(_ssaoRTVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	result = Application::_dx->GetDevice()->CreateDescriptorHeap(
		&desc,
		IID_PPV_ARGS(_ssaoSRVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result)) return false;

	return true;
}

bool TitleScene::CreateSSAORTVAndSRV()
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	Application::_dx->GetDevice()->CreateRenderTargetView(
		_ssaoBuff.Get(),
		&rtvDesc,
		_ssaoRTVHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	auto handle = _peraSRVHeap->GetCPUDescriptorHandleForHeapStart();


	handle.ptr += Application::_dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * _peraViewMap["_ssaoBuff"];
	Application::_dx->GetDevice()->CreateShaderResourceView(
		_ssaoBuff.Get(),
		&srvDesc,
		handle);

	return true;
}

bool TitleScene::SceneTransBuffInit()
{

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneTransMatrix) + 0xff) & ~0xff);

	Application::_dx->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_sceneTransBuff.ReleaseAndGetAddressOf())
	);

	auto result = _sceneTransBuff->Map(0, nullptr, (void**)&_sceneTransMatrix);
	if (FAILED(result)) return false;




	auto handle = _peraSRVHeap->GetCPUDescriptorHandleForHeapStart();

	handle.ptr += Application::_dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * _peraViewMap["_sceneTransBuff"];
	cbvDesc.BufferLocation = _sceneTransBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(_sceneTransBuff->GetDesc().Width);

	Application::_dx->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);

	return true;
}

void TitleScene::CalcSceneTrans()
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

void TitleScene::BeginDrawShade()
{
	for (auto& res : _RSMBuff) {
		RSMBuffBarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		RSMBuffBarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		RSMBuffBarrierDesc.Transition.pResource = res.Get();
		RSMBuffBarrierDesc.Transition.Subresource = 0;
		RSMBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		RSMBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		Application::_dx->GetCommandList()->ResourceBarrier(1, &RSMBuffBarrierDesc);
	}
	auto handle = _RSMRTVHeap->GetCPUDescriptorHandleForHeapStart();
	auto rtvIncSize = Application::_dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE RSMRTVHs[3] = {};
	uint32_t offset = 0;
	for (auto& rsmRTVH : RSMRTVHs) {
		rsmRTVH.InitOffsetted(handle, offset);
		offset += rtvIncSize;
	}

	auto dsvH = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
	dsvH.ptr += Application::_dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	Application::_dx->GetCommandList()->OMSetRenderTargets(3, RSMRTVHs, true, &dsvH);
	float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	for (int i = 0; i < _countof(RSMRTVHs); ++i)
	{
		Application::_dx->GetCommandList()->ClearRenderTargetView(RSMRTVHs[i], clearColor, 0, nullptr);
	}

	Application::_dx->GetCommandList()->ClearDepthStencilView(
		dsvH,
		D3D12_CLEAR_FLAG_DEPTH,
		1.0f,
		0,
		0,
		nullptr);

	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.0f, 0.0f, shadow_difinition, shadow_difinition);
	CD3DX12_RECT rc(0, 0, shadow_difinition, shadow_difinition);
	Application::_dx->GetCommandList()->RSSetViewports(1, &vp);
	Application::_dx->GetCommandList()->RSSetScissorRects(1, &rc);
}

void TitleScene::EndDrawShade()
{
	for (auto& res : _RSMBuff) {
		RSMBuffBarrierDesc.Transition.pResource = res.Get();
		RSMBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		RSMBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		Application::_dx->GetCommandList()->ResourceBarrier(1, &RSMBuffBarrierDesc);
	}
}

void TitleScene::BeginDrawTeapot()
{
	for (auto& res : _peraBuff) {
		peraBuffBarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		peraBuffBarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		peraBuffBarrierDesc.Transition.pResource = res.Get();
		peraBuffBarrierDesc.Transition.Subresource = 0;
		peraBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		peraBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		Application::_dx->GetCommandList()->ResourceBarrier(1, &peraBuffBarrierDesc);
	}
	auto handle = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();
	auto rtvIncSize = Application::_dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE peraRTVHs[2] = {};
	uint32_t offset = 0;
	for (auto& peraRTVH : peraRTVHs) {
		peraRTVH.InitOffsetted(handle, offset);
		offset += rtvIncSize;
	}


	auto dsvH = _dsvHeap->GetCPUDescriptorHandleForHeapStart();

	Application::_dx->GetCommandList()->OMSetRenderTargets(
		2,
		peraRTVHs,
		true,
		&dsvH);
	float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	for (int i = 0; i < _countof(peraRTVHs); ++i)
	{
		Application::_dx->GetCommandList()->ClearRenderTargetView(peraRTVHs[i], clearColor, 0, nullptr);
	}

	Application::_dx->GetCommandList()->ClearDepthStencilView(
		dsvH,
		D3D12_CLEAR_FLAG_DEPTH,
		1.0f,
		0,
		0,
		nullptr);

	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.0f, 0.0f, Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	CD3DX12_RECT rc(0, 0, Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	Application::_dx->GetCommandList()->RSSetViewports(1, &vp);
	Application::_dx->GetCommandList()->RSSetScissorRects(1, &rc);
}



void TitleScene::EndDrawTeapot()
{
	for (auto& res : _peraBuff) {
		peraBuffBarrierDesc.Transition.pResource = res.Get();
		peraBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		peraBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		Application::_dx->GetCommandList()->ResourceBarrier(1, &peraBuffBarrierDesc);
	}
}

void TitleScene::BeginDrawSSAO()
{
	ssaoBuffBarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	ssaoBuffBarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	ssaoBuffBarrierDesc.Transition.pResource = _ssaoBuff.Get();
	ssaoBuffBarrierDesc.Transition.Subresource = 0;
	ssaoBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	ssaoBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	Application::_dx->GetCommandList()->ResourceBarrier(1, &ssaoBuffBarrierDesc);
	auto rtvH = _ssaoRTVHeap->GetCPUDescriptorHandleForHeapStart();
	Application::_dx->GetCommandList()->OMSetRenderTargets(
		1,
		&rtvH,
		false,
		nullptr);

	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.0f, 0.0f, Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	CD3DX12_RECT rc(0, 0, Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	Application::_dx->GetCommandList()->RSSetViewports(1, &vp);
	Application::_dx->GetCommandList()->RSSetScissorRects(1, &rc);
}

void TitleScene::EndDrawSSAO()
{
	ssaoBuffBarrierDesc.Transition.pResource = _ssaoBuff.Get();
	ssaoBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	ssaoBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	Application::_dx->GetCommandList()->ResourceBarrier(1, &ssaoBuffBarrierDesc);
}





void TitleScene::BeginDrawPera()
{
	auto backBufferIndex = Application::_dx->GetSwapChain()->GetCurrentBackBufferIndex();

	backBuffBarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	backBuffBarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	backBuffBarrierDesc.Transition.pResource = Application::_dx->GetBackBuffer().Get();
	backBuffBarrierDesc.Transition.Subresource = 0;
	backBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	backBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	Application::_dx->GetCommandList()->ResourceBarrier(1, &backBuffBarrierDesc);

	auto rtvH = Application::_dx->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += backBufferIndex *
		Application::_dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto dsvH = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
	Application::_dx->GetCommandList()->OMSetRenderTargets(
		1,
		&rtvH,
		true,
		&dsvH);
	float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	Application::_dx->GetCommandList()->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.0f, 0.0f, Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	CD3DX12_RECT rc(0, 0, Application::GetWindowSize().cx, Application::GetWindowSize().cy);
	Application::_dx->GetCommandList()->RSSetViewports(1, &vp);
	Application::_dx->GetCommandList()->RSSetScissorRects(1, &rc);

}

void TitleScene::EndDrawPera()
{
	backBuffBarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	backBuffBarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	Application::_dx->GetCommandList()->ResourceBarrier(1, &backBuffBarrierDesc);
}




void TitleScene::Draw()
{
}

void TitleScene::Update()
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

DirectX::XMFLOAT3* TitleScene::GetEyePos()
{
	return &eye;
}

DirectX::XMFLOAT3* TitleScene::GetTargetPos()
{
	return &target;
}

Microsoft::WRL::ComPtr<ID3D12Resource> TitleScene::GetSceneTransBuff()
{
	return _sceneTransBuff;
}

Microsoft::WRL::ComPtr<ID3D12Resource> TitleScene::GetLightDepthBuff()
{
	return _lightDepthBuff;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> TitleScene::GetPeraSRVHeap() const 
{
	return _peraSRVHeap;
}

TitleScene::TitleScene(SceneManager& controller)
	: Scene(controller), _controller(controller),
	eye(40, 20, 150),
	rotate(0, 0, 0),
	target(20, 30, 30),
	up(0, 1, 0),
	lightVec(30, 30, 60),
	_sceneTransMatrix(nullptr)
{
	SceneInit();
}

TitleScene::~TitleScene()
{
}
