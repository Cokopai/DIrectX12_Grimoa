#pragma once
#pragma comment (lib,"d3d12.lib")
#pragma comment (lib,"dxgi.lib")
#pragma comment (lib,"d3dcompiler.lib")

#include	<d3d12.h>
#include	<dxgi1_6.h>
#include	<wrl/client.h>
#include	<vector>
#include	<d3dcompiler.h>

using Microsoft::WRL::ComPtr;

class DirectX12_Graphics {
private:
	DirectX12_Graphics() {}

	//�f�o�C�X
	ComPtr<ID3D12Device>				 _dev = nullptr;
	//�t�@�N�g���[
	ComPtr<IDXGIFactory6>				 _dxgiFactory = nullptr;
	//�X���b�v�`�F�[��
	ComPtr<IDXGISwapChain4>			  	 _swapchain = nullptr;

	//�R�}���h���X�g
	ComPtr<ID3D12CommandAllocator>		 _cmdAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList>	 _cmdList = nullptr;
	ComPtr<ID3D12CommandQueue>			 _cmdQueue = nullptr;

	//�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap>		 _rtvHeaps = nullptr;//RenderTargetView Heaps

	ComPtr<ID3DBlob>					 _vsBlob;
	ComPtr<ID3DBlob>					 _psBlob;
	//�o�b�N�o�b�t�@
	std::vector<ComPtr<ID3D12Resource>>_backBuffers;
	//�t�F���X
	ComPtr<ID3D12Fence>					 _fence = nullptr;
	UINT64 _fenceVal = 0;

	ID3D12PipelineState*				_pipelinestate = nullptr;
	ID3D12RootSignature*				_rootSignature = nullptr;

	//��ʃN���A�@�J���[
	float clearColor[4] = { 0.6f,1.0f,0.8f,1.0f };

public:
	DirectX12_Graphics(const DirectX12_Graphics&) = delete;
	DirectX12_Graphics& operator=(const DirectX12_Graphics&) = delete;
	DirectX12_Graphics(DirectX12_Graphics&&) = delete;
	DirectX12_Graphics& operator=(DirectX12_Graphics&&) = delete;


	~DirectX12_Graphics() {
		Exit();
	}

	static DirectX12_Graphics* GetInstance() {
		static DirectX12_Graphics instance;
		return &instance;
	}

	// ��������
	bool Init(HWND hWnd,unsigned int Width, unsigned int Height, bool fullscreen);
	//�X�V����
	bool BeforeRender();
	void Draw();
	bool AfterRender();

	// �I������
	void Exit();

	// �f�o�C�XGET
	ID3D12Device*	GetDXDevice() const {
		return _dev.Get();
	}

	// SWAP�`�F�C��GET
	IDXGISwapChain* GetSwapChain()const {
		return _swapchain.Get();
	}

};