#pragma once
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <string>
#include <cassert>
#include <dxgi1_6.h>
#include <wrl.h>
#include <format>
#include "WinApp.h"
#include <dxcapi.h>
#include "StringUtility.h"
#include <chrono>
#include <fstream>
#include <sstream>
#include <thread>
#include "Logger.h"

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/DirectXTex/DirectXTex.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWmd, UINT msg, WPARAM wParam, LPARAM lParam);


struct Vector2 {
	float x;
	float y;
};

struct Vector3 {
	float x;
	float y;
	float z;
};

struct Vector4 {
	float w;
	float x;
	float y;
	float z;
};

struct Matrix4x4 {
	float m[4][4] = {};
};

struct MaterialData {
	std::string textureFilePath;
};

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};


// DirectX基盤
class DirectXCommon {
public: // メンバ関数
	// 初期化
	void Initialize(WinApp* winApp);

	// 描画前処理
	void PreDraw();

	// 描画後処理
	void PostDraw();

	// 解放
	void DXCRelease();

	/// <summary>
	/// バッファリソースの生成
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	// シェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		const std::wstring& filePath, const wchar_t* profile);

	/// <summary>
	///	テクスチャリソースの生成
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(
		ID3D12Device* device, const DirectX::TexMetadata& metadata);

	/// <summary>
	/// テクスチャデータの転送
	/// </summary>
	void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, const DirectX::TexMetadata& metadata);

	/// <summary>
	/// テクスチャファイルの読み込み
	/// </summary>
	/// <param name="filePath">テクスチャファイルのパス</param>
	/// <returns>画像イメージデータ</returns>
	static DirectX::ScratchImage LoadTexture(const std::string& filePath);


	/// <summary>
	/// SRVの指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	/// <summary>
	/// SRVの指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

	// getter
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const { return device.Get(); }
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return commandList_.Get(); }


private:

	// デバイスの初期化
	void DeviceInitialize();

	// コマンド関連の初期化
	void CommandInitialize();

	// スワップチェーンの生成
	void MakeSwapChain();

	// 深度バッファの生成
	void CreateDepthStencilTextureResource(int32_t width, int32_t height);

	// 各種デスクリプタヒープの生成
	void CreateVariousDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	
	// レンダーターゲットビューの初期化
	void RTVInitialize();

	// デスクリプタヒープを生成する
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	
	// 深度ステンシルビューの初期化
	void DepthStencilViewInitialize();

	// フェンスの生成
	void CreateFence();

	// ビューポート矩形の初期化
	void ViewportInitialize();

	// シザリング矩形の初期化
	void ScissorRectInitialize();

	// DXCコンパイラの生成
	void CreateDxcCompiler();

	// ImGuiの初期化
	void ImGuiInitialize();

	// FPS固定初期化
	void InitializeFixFPS();

	// FPS固定更新
	void UpdateFixFPS();

	// mtlファイルを読む関数
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

private:
	// DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;

	// デバッグコントローラ
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;

	// コマンドアロケータ
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
	// コマンドキュー
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;

	// スワップチェーン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	// WindowsAPI
	WinApp* winApp = nullptr;

	// RTV用のDescriptorHeap
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RTVDescriptorHeap = nullptr;
	// SRV用のDescriptorHeap
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SRVDescriptorHeap = nullptr;
	// DSV用のDescriptorHeap
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DSVDescriptorHeap = nullptr;
	// RTV用のDescriptorSizeを取得しておく
	uint32_t descriptorSizeRTV;
	// SRV用のDescriptorSizeを取得しておく
	uint32_t descriptorSizeSRV;
	// DSV用のDescriptorSizeを取得しておく
	uint32_t descriptorSizeDSV;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU;

	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	// RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	// depthStencilResourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;

	// ビューポート
	D3D12_VIEWPORT viewport{};
	// シザー矩形
	D3D12_RECT scissorRect{};

	// DXCユーティリティの生成
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
	// DXCコンパイラの生成
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	// デフォルトインクルードハンドラの生成
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler = nullptr;

	// Fenceを生成する
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	// FenceのSignalを待つためのイベントを作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	// フェンス値
	UINT64 fenceValue = 0;

	// 使用するアダプタ用の変数
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter;

	// 記録時間(FPS固定用)
	std::chrono::steady_clock::time_point reference_;

	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	UINT backBufferIndex;

	ID3D12DescriptorHeap* descriptorHeaps[1];
	ID3D12CommandList* commandLists[1];


	/// <summary>
	/// 指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
		const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap,
		uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// 指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(
		const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap,
		uint32_t descriptorSize, uint32_t index);
};