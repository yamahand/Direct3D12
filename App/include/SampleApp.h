#pragma once

#include <App.h>
#include <ConstantBuffer.h>
#include <Material.h>

class SampleApp : public App {
public:
	SampleApp(uint32_t width, uint32_t height);

	virtual ~SampleApp();

private:
	bool OnInit() override;

	void OnTerm() override;

	void OnRender() override;

private:
	std::vector<Mesh*>	m_pMesh;
	std::vector<ConstantBuffer*>	m_transform;
	Material	m_material;
	ComPtr<ID3D12PipelineState>	m_pPSO;
	ComPtr<ID3D12RootSignature>	m_pRootSig;
	float m_rotateAngle = 0.0f;
};