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
	// ���b�V�������[�h
	{
		std::wstring path;

		// �t�@�C���p�X������
		if (!SearchFilePath(L"res/teapot/teapot.obj", path)) {
			ELOG("Error : File Not Found.");
			return false;
		}

		std::wstring dir = GetDirectoryPath(path.c_str());

		std::vector<ResMesh> resMesh;
		std::vector<ResMaterial> resMaterial;

		// ���b�V�����\�[�X�����[�h
		if (!LoadMesh(path.c_str(), resMesh, resMaterial)) {
			ELOG("Error : Load Mesh Failed. filepath = %ls", path.c_str());
			return false;
		}

		// ��������\��
		for (size_t i = 0; i < resMesh.size(); i++)
		{
			// ���b�V������
			auto mesh = new (std::nothrow) Mesh();

			// �`�F�b�N
			if (mesh == nullptr) {
				ELOG("Error : Out of memory");
				return false;
			}

			// ����������
			if (!mesh->Init(m_pDevice.Get(), resMesh[i])) {
				ELOG("Error :  Mesh Initialize Failed.");
				delete mesh;
				return false;
			}

			// ����������o�^
			m_pMesh.push_back(mesh);
		}

		// �������œK��
		m_pMesh.shrink_to_fit();

		// �}�e���A��������
		if (!m_material.Init(m_pDevice.Get(), m_pPools[POOL_TYPE_RES], 0, resMaterial.size())) {
			ELOG("Error : Material::Init() Failed.");
			return false;
		}

		// ���\�[�X�o�b�`��p��
		DirectX::ResourceUploadBatch batch(m_pDevice.Get());

		// �o�b�`�J�n
		batch.Begin();

		// �e�N�X�`���ݒ�
		for (size_t i = 0; i < resMaterial.size(); i++)
		{
			std::wstring path = dir + resMaterial[i].diffuseMap;
			m_material.SetTexture(i, TU_DIFFUSE, path, batch);
		}

		// �o�b�`�I��
		auto future = batch.End(m_pQueue.Get());

		// �o�b�`������ҋ@
		future.wait();
	}

	// ���[�g�V�O�l�`������
	{
		auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		// �f�B�X�N���v�^�����W��ݒ�
		D3D12_DESCRIPTOR_RANGE range = {};
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range.NumDescriptors = 1;
		range.BaseShaderRegister = 0;
		range.RegisterSpace = 0;
		range.OffsetInDescriptorsFromTableStart = 0;

		// ���[�g�p�����[�^�̐ݒ�
		D3D12_ROOT_PARAMETER param[2] = {};
		param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param[0].Descriptor.ShaderRegister = 0;
		param[0].Descriptor.RegisterSpace = 0;
		param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param[1].DescriptorTable.NumDescriptorRanges = 1;
		param[1].DescriptorTable.pDescriptorRanges = &range;
		param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// �X�^�e�B�b�N�T���v���[�̐ݒ�
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

		// ���[�g�V�O�l�`���̐ݒ�
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.NumParameters = 2;
		desc.NumStaticSamplers = 1;
		desc.pParameters = param;
		desc.pStaticSamplers = &sampler;
		desc.Flags = flag;

		ComPtr<ID3DBlob> pBlob;
		ComPtr<ID3DBlob> pErrorBlob;

		// �V���A���C�Y
		auto hr = D3D12SerializeRootSignature(
			&desc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			pBlob.GetAddressOf(),
			pErrorBlob.GetAddressOf());
		if (FAILED(hr)) {
			return false;
		}

		// ���[�g�V�O�l�`������
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

	// �p�C�v���C���X�e�[�g����
	{
		std::wstring vsPath;
		std::wstring psPath;

		// ���_�V�F�[�_������
		if (!SearchFilePath(L"SimpleTexVS.cso", vsPath)) {
			ELOG("Error : Vertex Shader Not Found.");
			return false;
		}

		// �s�N�Z���V�F�[�_������
		if (!SearchFilePath(L"SimpleTexPS.cso", psPath)) {
			ELOG("Error : Pixel Shader Not Found.");
			return false;
		}

		ComPtr<ID3DBlob> pVSBlob;
		ComPtr<ID3DBlob> pPSBlob;

		// ���_�V�F�[�_��ǂݍ���
		auto hr = D3DReadFileToBlob(vsPath.c_str(), pVSBlob.GetAddressOf());
		if (FAILED(hr)) {
			ELOG("Error : D3DReadFileToBlob() Failed. path = %s", vsPath.c_str());
			return false;
		}

		// �s�N�Z���V�F�[�_��ǂݍ���
		hr = D3DReadFileToBlob(psPath.c_str(), pPSBlob.GetAddressOf());
		if (FAILED(hr)) {
			ELOG("Error : D3DReadFileToBlob() Failed. path = %s", psPath.c_str());
			return false;
		}

		// �O���t�B�N�X�p�C�v���C���̐ݒ�
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

		// �p�C�v���C���X�e�[�g�𐶐�
		hr = m_pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_pPSO.GetAddressOf()));
		if (FAILED(hr)) {
			ELOG("Error : m_pDevice->CreateGraphicsPipelineState() Failed. retcode = 0x%x", hr);
			return false;
		}
	}

	// �ϊ��s��p�̒萔�o�b�t�@�̐���
	{
		m_transform.reserve(FrameCount);

		for (size_t i = 0u; i < FrameCount; i++)
		{
			auto pCB = new(std::nothrow) ConstantBuffer();
			if (pCB == nullptr) {
				ELOG("Error : Out of memory.");
				return false;
			}

			// �萔�o�b�t�@������
			if (!pCB->Init(m_pDevice.Get(), m_pPools[POOL_TYPE_RES], sizeof(Transform) * 2)) {
				ELOG("Error : ConstantBuffer::Init() Failed.");
				return false;
			}

			// �J�����ݒ�
			auto eyePos = DirectX::XMVectorSet(0.0f, 1.0f, 2.0f, 0.0f);
			auto targetPos = DirectX::XMVectorZero();
			auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

			// �������j�ƃA�X�y�N�g��̐ݒ�
			auto fovy = DirectX::XMConvertToRadians(37.5f);
			auto aspect = static_cast<float>(m_width) / static_cast<float>(m_height);

			// �ϊ��s���ݒ�
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
	// ���b�V���j��
	for (size_t i = 0; i < m_pMesh.size(); i++)
	{
		SafeTerm(m_pMesh[i]);
	}
	m_pMesh.clear();
	m_pMesh.shrink_to_fit();

	// �}�e���A���j��
	m_material.Term();

	// �ϊ��o�b�t�@�j��
	for (size_t i = 0; i < m_transform.size(); i++)
	{
		SafeTerm(m_transform[i]);
	}
	m_transform.clear();
	m_transform.shrink_to_fit();
}

void SampleApp::OnRender()
{
	// �X�V����
	{
		m_rotateAngle += 0.025f;

		auto pTransform = m_transform[m_frameIndex]->GetPtr<Transform>();
		pTransform->world = DirectX::XMMatrixRotationY(m_rotateAngle);
	}

	// �R�}���h���X�g�̋L�^�J�n
	auto pCmd = m_commandList.Reset();

	// �������ݗp���\�[�X�o���A�ݒ�
	DirectX::TransitionResource(pCmd,
		m_colorTargets[m_frameIndex].GetResource(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	// �f�B�X�N���v�^�擾
	auto handleRTV = m_colorTargets[m_frameIndex].GetHandleRTV();
	auto handleDSV = m_depthTarget.GetHandleDSV();

	// �����_�[�^�[�Q�b�g�ݒ�
	pCmd->OMSetRenderTargets(1, &handleRTV->handleCPU, FALSE, &handleDSV->handleCPU);

	// �N���A�J���[
	float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };

	// �����_�[�^�[�Q�b�g���N���A
	pCmd->ClearRenderTargetView(handleRTV->handleCPU, clearColor, 0, nullptr);

	// �[�x�X�e���V���r���[���N���A
	pCmd->ClearDepthStencilView(handleDSV->handleCPU, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// �`�揈��
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
			// �}�e���A��ID���擾
			auto id = m_pMesh[i]->GetMaterialId();

			// �e�N�X�`���ݒ�
			pCmd->SetGraphicsRootDescriptorTable(1, m_material.GetTextureHandle(id, TU_DIFFUSE));

			// ���b�V����`��
			m_pMesh[i]->Draw(pCmd);
		}
	}

	// �\���p���\�[�X�o���A�ݒ�
	DirectX::TransitionResource(pCmd,
		m_colorTargets[m_frameIndex].GetResource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);

	// �����x���X�g�̋L�^�I��
	pCmd->Close();

	// �R�}���h���X�g���s
	ID3D12CommandList* pLists[] = { pCmd };
	m_pQueue->ExecuteCommandLists(1, pLists);

	// ��ʂɕ\��
	Present(1);
}
