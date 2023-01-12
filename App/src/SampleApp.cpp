#include "SampleApp.h"
#include "FileUtil.h"
#include "CommonStates.h"
#include "DirectXHelpers.h"
#include "Logger.h"
#include "InlineUtil.h"

namespace {
	struct Transform {
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX proj;
	};
}

SampleApp::SampleApp(uint32_t width, uint32_t height)
	: App(width, height)
{
}

SampleApp::~SampleApp()
{
}

bool SampleApp::OnInit()
{
	// メッシュをロード
	{
		std::wstring path;

		// ファイルパスを検索
		if (!SearchFilePath(L"res/teapot/teapot.obj", path)) {
			ELOG("Error : File Not Found.");
			return false;
		}

		std::wstring dir = GetDirectoryPath(path.c_str());

		std::vector<ResMesh> resMesh;
		std::vector<ResMaterial> resMaterial;

		// メッシュリソースをロード
		if (!LoadMesh(path.c_str(), resMesh, resMaterial)) {
			ELOG("Error : Load Mesh Failed. filepath = %ls", path.c_str());
			return false;
		}

		// メモリを予約
		for (size_t i = 0; i < resMesh.size(); i++)
		{
			// メッシュ生成
			auto mesh = new (std::nothrow) Mesh();

			// チェック
			if (mesh == nullptr) {
				ELOG("Error : Out of memory");
				return false;
			}

			// 初期化処理
			if (!mesh->Init(m_pDevice.Get(), resMesh[i])) {
				ELOG("Error :  Mesh Initialize Failed.");
				delete mesh;
				return false;
			}

			// 成功したら登録
			m_pMesh.push_back(mesh);
		}

		// メモリ最適化
		m_pMesh.shrink_to_fit();

		// マテリアル初期化
		if (!m_material.Init(m_pDevice.Get(), m_pPools[POOL_TYPE_RES], 0, resMaterial.size())) {
			ELOG("Error : Material::Init() Failed.");
			return false;
		}

		// リソースバッチを用意
		DirectX::ResourceUploadBatch batch(m_pDevice.Get());

		// バッチ開始
		batch.Begin();

		// テクスチャ設定
		for (size_t i = 0; i < resMaterial.size(); i++)
		{
			std::wstring path = dir + resMaterial[i].diffuseMap;
			m_material.SetTexture(i, TU_DIFFUSE, path, batch);
		}

		// バッチ終了
		auto future = batch.End(m_pQueue.Get());

		// バッチ完了を待機
		future.wait();
	}

	// ルートシグネチャ生成
	{
		auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		// ディスクリプタレンジを設定
		D3D12_DESCRIPTOR_RANGE range = {};
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range.NumDescriptors = 1;
		range.BaseShaderRegister = 0;
		range.RegisterSpace = 0;
		range.OffsetInDescriptorsFromTableStart = 0;

		// ルートパラメータの設定
		D3D12_ROOT_PARAMETER param[2] = {};
		param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param[0].Descriptor.ShaderRegister = 0;
		param[0].Descriptor.RegisterSpace = 0;
		param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param[1].DescriptorTable.NumDescriptorRanges = 1;
		param[1].DescriptorTable.pDescriptorRanges = &range;
		param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// スタティックサンプラーの設定
		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.MipLODBias = D3D12_DEFAULT_MIP_LOD_BIAS;
		sampler.MaxAnisotropy = 1;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = -D3D12_FLOAT32_MAX;
		sampler.MaxLOD = +D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// ルートシグネチャの設定
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.NumParameters = 2;
		desc.NumStaticSamplers = 1;
		desc.pParameters = param;
		desc.pStaticSamplers = &sampler;
		desc.Flags = flag;

		ComPtr<ID3DBlob> pBlob;
		ComPtr<ID3DBlob> pErrorBlob;

		// シリアライズ
		auto hr = D3D12SerializeRootSignature(
			&desc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			pBlob.GetAddressOf(),
			pErrorBlob.GetAddressOf());
		if (FAILED(hr)) {
			return false;
		}

		// ルートシグネチャ生成
		hr = m_pDevice->CreateRootSignature(
			0,
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			IID_PPV_ARGS(m_pRootSig.GetAddressOf()));
		if (FAILED(hr)) {
			ELOG("Error : Root Sginature Create Failed. retcode = 0x%s", hr);
			return false;
		}
	}

	// パイプラインステート生成
	{
		std::wstring vsPath;
		std::wstring psPath;

		// 頂点シェーダを検索
		if (!SearchFilePath(L"SimpleTexVS.cso", vsPath)) {
			ELOG("Error : Vertex Shader Not Found.");
			return false;
		}

		// ピクセルシェーダを検索
		if (!SearchFilePath(L"SimpleTexPS.cso", psPath)) {
			ELOG("Error : Pixel Shader Not Found.");
			return false;
		}

		ComPtr<ID3DBlob> pVSBlob;
		ComPtr<ID3DBlob> pPSBlob;

		// 頂点シェーダを読み込む
		auto hr = D3DReadFileToBlob(vsPath.c_str(), pVSBlob.GetAddressOf());
		if (FAILED(hr)) {
			ELOG("Error : D3DReadFileToBlob() Failed. path = %s", vsPath.c_str());
			return false;
		}

		// ピクセルシェーダを読み込む
		hr = D3DReadFileToBlob(psPath.c_str(), pPSBlob.GetAddressOf());
		if (FAILED(hr)) {
			ELOG("Error : D3DReadFileToBlob() Failed. path = %s", psPath.c_str());
			return false;
		}

		// グラフィクスパイプラインの設定
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.InputLayout = MeshVertex::InputLayout;
		desc.pRootSignature = m_pRootSig.Get();
		desc.VS = { pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize() };
		desc.PS = { pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize() };
		desc.RasterizerState = DirectX::CommonStates::CullNone;
		desc.BlendState = DirectX::CommonStates::Opaque;
		desc.DepthStencilState = DirectX::CommonStates::DepthDefault;
		desc.SampleMask = UINT_MAX;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = m_colorTargets[0].GetViewDesc().Format;
		desc.DSVFormat = m_depthTarget.GetViewDesc().Format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		// パイプラインステートを生成
		hr = m_pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_pPSO.GetAddressOf()));
		if (FAILED(hr)) {
			ELOG("Error : m_pDevice->CreateGraphicsPipelineState() Failed. retcode = 0x%x", hr);
			return false;
		}
	}

	// 変換行列用の定数バッファの生成
	{
		m_transform.reserve(FrameCount);

		for (size_t i = 0u; i < FrameCount; i++)
		{
			auto pCB = new(std::nothrow) ConstantBuffer();
			if (pCB == nullptr) {
				ELOG("Error : Out of memory.");
				return false;
			}

			// 定数バッファ初期化
			if (!pCB->Init(m_pDevice.Get(), m_pPools[POOL_TYPE_RES], sizeof(Transform) * 2)) {
				ELOG("Error : ConstantBuffer::Init() Failed.");
				return false;
			}

			// カメラ設定
			auto eyePos = DirectX::XMVectorSet(0.0f, 1.0f, 2.0f, 0.0f);
			auto targetPos = DirectX::XMVectorZero();
			auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

			// 垂直が核とアスペクト比の設定
			auto fovy = DirectX::XMConvertToRadians(37.5f);
			auto aspect = static_cast<float>(m_width) / static_cast<float>(m_height);

			// 変換行列を設定
			auto ptr = pCB->GetPtr<Transform>();
			ptr->world = DirectX::XMMatrixIdentity();
			ptr->view = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
			ptr->proj = DirectX::XMMatrixPerspectiveFovRH(fovy, aspect, 1.0f, 1000.0f);

			m_transform.push_back(pCB);
		}

		m_rotateAngle = 0.0f;
	}

	return true;
}

void SampleApp::OnTerm()
{
	// メッシュ破棄
	for (size_t i = 0; i < m_pMesh.size(); i++)
	{
		SafeTerm(m_pMesh[i]);
	}
	m_pMesh.clear();
	m_pMesh.shrink_to_fit();

	// マテリアル破棄
	m_material.Term();

	// 変換バッファ破棄
	for (size_t i = 0; i < m_transform.size(); i++)
	{
		SafeTerm(m_transform[i]);
	}
	m_transform.clear();
	m_transform.shrink_to_fit();
}

void SampleApp::OnRender()
{
	// 更新処理
	{
		m_rotateAngle += 0.025f;

		auto pTransform = m_transform[m_frameIndex]->GetPtr<Transform>();
		pTransform->world = DirectX::XMMatrixRotationY(m_rotateAngle);
	}

	// コマンドリストの記録開始
	auto pCmd = m_commandList.Reset();

	// 書き込み用リソースバリア設定
	DirectX::TransitionResource(pCmd,
		m_colorTargets[m_frameIndex].GetResource(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	// ディスクリプタ取得
	auto handleRTV = m_colorTargets[m_frameIndex].GetHandleRTV();
	auto handleDSV = m_depthTarget.GetHandleDSV();

	// レンダーターゲット設定
	pCmd->OMSetRenderTargets(1, &handleRTV->handleCPU, FALSE, &handleDSV->handleCPU);

	// クリアカラー
	float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };

	// レンダーターゲットをクリア
	pCmd->ClearRenderTargetView(handleRTV->handleCPU, clearColor, 0, nullptr);

	// 深度ステンシルビューをクリア
	pCmd->ClearDepthStencilView(handleDSV->handleCPU, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// 描画処理
	{
		ID3D12DescriptorHeap* const pHeaps[] = {
			m_pPools[POOL_TYPE_RES]->GetHeap()
		};

		pCmd->SetGraphicsRootSignature(m_pRootSig.Get());
		pCmd->SetDescriptorHeaps(1, pHeaps);
		pCmd->SetGraphicsRootConstantBufferView(0, m_transform[m_frameIndex]->GetAddress());
		pCmd->SetPipelineState(m_pPSO.Get());
		pCmd->RSSetViewports(1, &m_viewport);
		pCmd->RSSetScissorRects(1, &m_scissor);

		for (size_t i = 0; i < m_pMesh.size(); i++)
		{
			// マテリアルIDを取得
			auto id = m_pMesh[i]->GetMaterialId();

			// テクスチャ設定
			pCmd->SetGraphicsRootDescriptorTable(1, m_material.GetTextureHandle(id, TU_DIFFUSE));

			// メッシュを描画
			m_pMesh[i]->Draw(pCmd);
		}
	}

	// 表示用リソースバリア設定
	DirectX::TransitionResource(pCmd,
		m_colorTargets[m_frameIndex].GetResource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);

	// お万度リストの記録終了
	pCmd->Close();

	// コマンドリスト実行
	ID3D12CommandList* pLists[] = { pCmd };
	m_pQueue->ExecuteCommandLists(1, pLists);

	// 画面に表示
	Present(1);
}
