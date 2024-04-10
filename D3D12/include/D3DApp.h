#ifndef D3DAPP_H 
#define D3DAPP_H


#ifndef EXPORTTING
#define DECLSPEC __declspec(dllimport)
#else
#define DECLSPEC __declspec(dllexport)
#endif // EXPORTTING

#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
#include <iostream>

#include "D3D12Util.h"
#include "minwindef.h"
#include "dxgi.h"
#include "dxgi1_4.h"

class D3DApp {
private:
	/// ��������
	Microsoft::WRL::ComPtr <IDXGIFactory4> dxgi_factory_;
	/// ����������
	Microsoft::WRL::ComPtr <IDXGISwapChain> swap_chain_;
	/// Ӳ������
	Microsoft::WRL::ComPtr <ID3D12Device> d3d_device_;
	/// CPU��GPU֮������϶���
	Microsoft::WRL::ComPtr <ID3D12Fence> fence_;
	/// �������
	Microsoft::WRL::ComPtr <ID3D12CommandQueue> command_queue_;
	/// �����б�
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> direct_list_;
	/// ���������
	Microsoft::WRL::ComPtr <ID3D12CommandAllocator> direct_allocator_;
	/// rtv��
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> rtv_heap_;
	/// dsv��
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsv_heap_;
	/// �������Ĵ�С
	/// RenderTargetView ֻд����Ⱦ����
	UINT rtv_size_{};
	/// DepthStencilView ֻд����Ȼ������
	UINT dsv_size_{};
	/// UnorderedAccessView ConstantBufferView ShaderResourceView ��������С��һ�µ�
	UINT cbv_srv_uav_size_{};
	/// ���ƿ����
	bool MSAA4x_state_ = false;
private:
	/**
	 * ��ʼ��D3D12.
	 * 
	 * \return �Ƿ��ʼ���ɹ�
	 */
	bool InitializeD3D();
	/**
	 * @brief ����������У������б�����������.
	 */
	void CreateCommandObjects();
	/**
	 * @brief ����������.
	 * 
	 */
	void CreateSwapChain();
	/**
	 * @brief ������������.
	 * 
	 */
	void CreateDescriptorHeaps();
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferDesc()const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilDesc()const;
protected:
	/// Ӧ�ó����ʵ�����
	HINSTANCE instance_ = nullptr;
	/// �����ڵ�ʵ�����
	HWND handle_mainWnd_ = nullptr;
	/// ʹ�ü���������
	static const int swap_chain_buffer_cnt_ = 2;
	/// ��ǰ���ĸ����������Ա�������֪�����ڸû�����һ����
	int current_back_buffer_idx_ = 0;
	UINT client_width_ = 800;
	UINT client_height_ = 600;
	/// ��̨����������ʾ��ʽ
	DXGI_FORMAT back_buffer_format_ = DXGI_FORMAT_R8G8B8A8_UNORM;
public:
	D3DApp(HINSTANCE);
	bool initialize();
	~D3DApp();
};



#endif
