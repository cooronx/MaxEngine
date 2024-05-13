#ifndef MYD3D_H
#define MYD3D_H

#include <DirectXCollision.h>
#include <DirectXColors.h>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/src/Core/Matrix.h>
#include <combaseapi.h>
#include <d3d12.h>
#include <d3dcommon.h>
#include <memory>
#include <minwinbase.h>
#include <numbers>
#include <stdlib.h>
#include <windef.h>

#include "D3D12Util.h"
#include "D3DBasicSetUp.h"
#include "UpLoadBuffer.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "wrl/client.h"

using namespace Util;
using namespace DirectX;
using namespace Eigen;

struct TransMatrix
{
    /// mvp 矩阵
    float *mvp_mat;
};
struct Vertex
{
    RowVector3f Pos;
    RowVector4f Color;
};
// Defines a subrange of geometry in a MeshGeometry.  This is for when multiple
// geometries are stored in one vertex and index buffer.  It provides the offsets
// and data needed to draw a subset of geometry stores in the vertex and index
// buffers so that we can implement the technique described by Figure 6.3.
struct SubmeshGeometry
{
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    INT BaseVertexLocation = 0;

    // Bounding box of the geometry defined by this submesh.
    // This is used in later chapters of the book.
    DirectX::BoundingBox Bounds;
};
struct MeshGeometry
{
    // Give it a name so we can look it up by name.
    std::string Name;

    // System memory copies.  Use Blobs because the vertex/index format can be generic.
    // It is up to the client to cast appropriately.
    Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

    // Data about the buffers.
    UINT VertexByteStride = 0;
    UINT VertexBufferByteSize = 0;
    DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
    UINT IndexBufferByteSize = 0;

    // A MeshGeometry may store multiple geometries in one vertex/index buffer.
    // Use this container to define the Submesh geometries so we can draw
    // the Submeshes individually.
    std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
    {
        D3D12_VERTEX_BUFFER_VIEW vbv;
        vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
        vbv.StrideInBytes = VertexByteStride;
        vbv.SizeInBytes = VertexBufferByteSize;

        return vbv;
    }

    D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
    {
        D3D12_INDEX_BUFFER_VIEW ibv;
        ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
        ibv.Format = IndexFormat;
        ibv.SizeInBytes = IndexBufferByteSize;

        return ibv;
    }

    // We can free this memory after we finish upload to the GPU.
    void DisposeUploaders()
    {
        VertexBufferUploader = nullptr;
        IndexBufferUploader = nullptr;
    }
};

Matrix4f GetModelMatrix()
{
    auto res = Matrix4f::Identity();
    return res;
}

Matrix4f GetViewMatrix(Vector3f eye, Vector3f center, Vector3f up)
{
    // /// LF
    Matrix4f res{};
    // res.setIdentity();
    // // 摄像机方向向量
    Vector3f w = center - eye;
    w.normalize();
    // // 摄像机的右方向（对于D3D来说其实是左手系）
    Vector3f u = up.cross(w);
    u.normalize();
    // 摄像机的上方向
    Vector3f v = w.cross(u);
    v.normalize();
    res << u.x(), u.y(), u.z(), -1 * eye.dot(u), v.x(), v.y(), v.z(), -1 * eye.dot(v), w.x(), w.y(),
        w.z(), -1 * eye.dot(w), 0, 0, 0, 1;
    return {};
}
Matrix4f GetProjectionMatrix(float fov_y, float ratio, float n, float f)
{
    Matrix4f res{};
    res.setIdentity();
    fov_y = static_cast<float>(fov_y / 180.0 * std::numbers::pi);
    res << 1.0 / (ratio * std::tan(fov_y / 2)), 0, 0, 0, 0, 1.0 / (std::tan(fov_y / 2)), 0, 0, 0, 0,
        f / (f - n), 1, 0, 0, -n * f / (f - n), 0;
    return res;
}

class MYD3D : public D3DBasicSetUp
{
    public:
    MYD3D() : D3DBasicSetUp()
    {
    }
    ~MYD3D()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
    /**
     * @brief 创建Imgui的描述符堆（用在imgui自己的渲染）
     *
     */
    void createImguiSRVHeap()
    {
        D3D12_DESCRIPTOR_HEAP_DESC srv_desc;
        srv_desc.NumDescriptors = 1;
        srv_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srv_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; /// 必须要设置为shader visible
        srv_desc.NodeMask = 0;
        ThrowIfFailed(d3d_device_->CreateDescriptorHeap(
            &srv_desc, IID_PPV_ARGS(imgui_srv_heap_.GetAddressOf())));
    }
    /**
     * @brief 初始化Imgui
     *
     */
    void InitImgui()
    {
        /// 加入imgui初始化
        /// 设置imgui上下文
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        ImGui_ImplWin32_Init(handle_mainWnd_);
        ImGui_ImplDX12_Init(d3d_device_.Get(), swap_chain_buffer_cnt_, back_buffer_format_,
                            imgui_srv_heap_.Get(),
                            imgui_srv_heap_->GetCPUDescriptorHandleForHeapStart(),
                            imgui_srv_heap_->GetGPUDescriptorHandleForHeapStart());
    }
    /**
     * @brief 创建常量堆
     *
     */
    void CreateCBVHeaps()
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc;
        desc.NumDescriptors = 1;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 0;
        ThrowIfFailed(
            d3d_device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(cbv_heap_.GetAddressOf())));
    }
    /**
     * @brief 创建常量缓冲区描述符（PS:放在上传堆中）
     *
     */
    void CreateConstBuffer()
    {
        const_buffer_obj_ = std::make_shared<UpLoadBuffer<TransMatrix>>(d3d_device_.Get(), 1, true);
        UINT const_buffer_obj_byte_size =
            Util::HelperFuncs::CalculateConstantBufferByteSize(sizeof(TransMatrix));
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        desc.SizeInBytes = const_buffer_obj_byte_size;
        desc.BufferLocation = const_buffer_obj_->Resource()->GetGPUVirtualAddress();
        d3d_device_->CreateConstantBufferView(&desc,
                                              cbv_heap_->GetCPUDescriptorHandleForHeapStart());
    }
    /**
     * @brief 创建根签名
     *
     */
    void CreateRootSignature()
    {
        // 根签名就和函数的形参一样，着色器就相当于一个函数，在这个例子中，我们只有一个输入参数也就是一个常量缓冲
        // Root parameter can be a table, root descriptor or root constants.
        CD3DX12_ROOT_PARAMETER slotRootParameter[1];
        CD3DX12_DESCRIPTOR_RANGE cbvTable;
        cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

        // A root signature is an array of root parameters.
        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
            1, slotRootParameter, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        // create a root signature with a single slot which points to a descriptor range consisting
        // of a single constant buffer
        Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
        HRESULT hr =
            D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

        if (errorBlob != nullptr)
        {
            ::OutputDebugStringA((char *)errorBlob->GetBufferPointer());
        }
        ThrowIfFailed(hr);

        ThrowIfFailed(d3d_device_->CreateRootSignature(0, serializedRootSig->GetBufferPointer(),
                                                       serializedRootSig->GetBufferSize(),
                                                       IID_PPV_ARGS(&root_signature_)));
    }
    /**
     * @brief 编译着色器并设置输入布局
     *
     */
    void CompileShaderAndBuildInputLayout()
    {
        vs_bytecode_ =
            Util::HelperFuncs::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
        ps_bytecode_ =
            Util::HelperFuncs::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

        input_layout_ = {{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                         {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
                          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
    }
    /**
     * @brief 构建物体的顶点和索引
     *
     */
    void BuildBoxGeometry()
    {

        std::array<Vertex, 8> vertices = {
            Vertex({RowVector3f(-1.0f, -1.0f, -1.0f), RowVector4f(Colors::White)}),
            Vertex({RowVector3f(-1.0f, +1.0f, -1.0f), RowVector4f(Colors::Black)}),
            Vertex({RowVector3f(+1.0f, +1.0f, -1.0f), RowVector4f(Colors::Red)}),
            Vertex({RowVector3f(+1.0f, -1.0f, -1.0f), RowVector4f(Colors::Green)}),
            Vertex({RowVector3f(-1.0f, -1.0f, +1.0f), RowVector4f(Colors::Blue)}),
            Vertex({RowVector3f(-1.0f, +1.0f, +1.0f), RowVector4f(Colors::Yellow)}),
            Vertex({RowVector3f(+1.0f, +1.0f, +1.0f), RowVector4f(Colors::Cyan)}),
            Vertex({RowVector3f(+1.0f, -1.0f, +1.0f), RowVector4f(Colors::Magenta)})};

        std::array<std::uint16_t, 36> indices = {// front face
                                                 0, 1, 2, 0, 2, 3,

                                                 // back face
                                                 4, 6, 5, 4, 7, 6,

                                                 // left face
                                                 4, 5, 1, 4, 1, 0,

                                                 // right face
                                                 3, 2, 6, 3, 6, 7,

                                                 // top face
                                                 1, 5, 6, 1, 6, 2,

                                                 // bottom face
                                                 4, 0, 3, 4, 3, 7};

        const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
        const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

        box_geo_ = std::make_unique<MeshGeometry>();
        box_geo_->Name = "boxGeo";

        ThrowIfFailed(D3DCreateBlob(vbByteSize, &box_geo_->VertexBufferCPU));
        CopyMemory(box_geo_->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

        ThrowIfFailed(D3DCreateBlob(ibByteSize, &box_geo_->IndexBufferCPU));
        CopyMemory(box_geo_->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

        box_geo_->VertexBufferGPU =
            HelperFuncs::CreateDefaultBuffer(d3d_device_.Get(), direct_list_.Get(), vertices.data(),
                                             vbByteSize, box_geo_->VertexBufferUploader);

        box_geo_->IndexBufferGPU =
            HelperFuncs::CreateDefaultBuffer(d3d_device_.Get(), direct_list_.Get(), indices.data(),
                                             ibByteSize, box_geo_->IndexBufferUploader);

        box_geo_->VertexByteStride = sizeof(Vertex);
        box_geo_->VertexBufferByteSize = vbByteSize;
        box_geo_->IndexFormat = DXGI_FORMAT_R16_UINT;
        box_geo_->IndexBufferByteSize = ibByteSize;

        SubmeshGeometry submesh;
        submesh.IndexCount = (UINT)indices.size();
        submesh.StartIndexLocation = 0;
        submesh.BaseVertexLocation = 0;

        box_geo_->DrawArgs["box"] = submesh;
    }
    void BuildPSO()
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
        ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
        psoDesc.InputLayout = {input_layout_.data(), (UINT)input_layout_.size()};
        psoDesc.pRootSignature = root_signature_.Get();
        psoDesc.VS = {reinterpret_cast<BYTE *>(vs_bytecode_->GetBufferPointer()),
                      vs_bytecode_->GetBufferSize()};
        psoDesc.PS = {reinterpret_cast<BYTE *>(ps_bytecode_->GetBufferPointer()),
                      ps_bytecode_->GetBufferSize()};
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = back_buffer_format_;
        psoDesc.SampleDesc.Count = MSAA4x_state_ ? 4 : 1;
        psoDesc.SampleDesc.Quality = MSAA4x_state_ ? (MSAA4x_quality_level_ - 1) : 0;
        psoDesc.DSVFormat = depth_stencil_format_;
        ThrowIfFailed(d3d_device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso_)));
    }
    virtual bool Initialize() override
    {
        if (!D3DBasicSetUp::Initialize())
            return false;
        ThrowIfFailed(direct_list_->Reset(direct_allocator_.Get(), nullptr));
        createImguiSRVHeap();
        InitImgui();
        CreateCBVHeaps();
        CreateConstBuffer();
        CompileShaderAndBuildInputLayout();
        BuildBoxGeometry();
        BuildPSO();
        ThrowIfFailed(direct_list_->Close());
        ID3D12CommandList *lists[] = {direct_list_.Get()};
        command_queue_->ExecuteCommandLists(_countof(lists), lists);
        return true;
    }
    void Update()
    {
        Vector3f eye_pos = {0, 0, -10};
        Vector3f target_pos = {0, 0, 0};
        Vector3f world_up = {0, 1, 0};
        Matrix4f mvp = proj_mat_ * GetViewMatrix(eye_pos, target_pos, world_up) * GetModelMatrix();
        TransMatrix trans_mat = {mvp.data()};
    }
    /**
     * @brief 当窗口大小发生变化时
     *
     * @param width 宽度
     * @param height 高度
     */
    void Resize(LONG width, LONG height)
    {
        client_width_ = width;
        client_height_ = height;
        proj_mat_ = GetProjectionMatrix(45.0f, width * 1.0f / height, 1.0f, 100.0f);
        D3DBasicSetUp::OnResize();
    }
    /**
     * @brief 绘制
     *
     */
    void Draw(const GameTimer &) override
    {
        // 复用命令列表和分配器
        ThrowIfFailed(direct_allocator_->Reset());
        ThrowIfFailed(direct_list_->Reset(direct_allocator_.Get(), nullptr));

        /// imgui渲染
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
        // {
        //     static int counter = 0;
        //     ImGui::Begin(
        //         "Hello, world!"); // Create a window called "Hello, world!" and append into it.
        //     ImGui::Text("This is some useful text."); // Display some text (you can use a format
        //                                               // strings too)
        //     ImGui::Checkbox("Demo Window",
        //                     &show_demo_window); // Edit bools storing our window open/close state
        //     ImGui::SliderFloat("float", &test_input, 0.1f,
        //                        1.0f);    // Edit 1 float using a slider from 0.0f to 1.0f
        //     if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return
        //                                  // true when edited/activated)
        //         counter++;
        //     ImGui::SameLine();
        //     ImGui::Text("counter = %d", counter);
        //     ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        //                 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        //     ImGui::End();
        // }

        // Indicate a state transition on the resource usage.
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        direct_list_->ResourceBarrier(1, &barrier);

        // Set the viewport and scissor rect.  This needs to be reset whenever the
        // command list is reset.
        direct_list_->RSSetViewports(1, &screen_viewport_);
        direct_list_->RSSetScissorRects(1, &scissor_rect_);

        // Clear the back buffer and depth buffer.
        direct_list_->ClearRenderTargetView(CurrentBackBufferDesc(), Colors::LightSteelBlue, 0,
                                            nullptr);
        direct_list_->ClearDepthStencilView(DepthStencilDesc(),
                                            D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f,
                                            0, 0, nullptr);
        // Specify the buffers we are going to render to.
        auto back_buffer_desc = CurrentBackBufferDesc();
        auto depth_desc = DepthStencilDesc();
        direct_list_->OMSetRenderTargets(1, &back_buffer_desc, true, &depth_desc);
        /// imgui绘制
        ID3D12DescriptorHeap *heaps[] = {imgui_srv_heap_.Get()};
        direct_list_->SetDescriptorHeaps(_countof(heaps), heaps);
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), direct_list_.Get());
        //  Indicate a state transition on the resource usage.
        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        direct_list_->ResourceBarrier(1, &barrier);

        // Done recording commands.
        ThrowIfFailed(direct_list_->Close());

        // Add the command list to the queue for execution.
        ID3D12CommandList *cmdsLists[] = {direct_list_.Get()};
        command_queue_->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

        // swap the back and front buffers
        ThrowIfFailed(swap_chain_->Present(0, 0));
        current_back_buffer_idx_ = (current_back_buffer_idx_ + 1) % swap_chain_buffer_cnt_;

        // Wait until frame commands are complete.  This waiting is inefficient and
        // is done for simplicity.  Later we will show how to organize our rendering
        // code so we do not have to wait per frame.
        FlushCommandQueue();
    }
    /**
     * @brief 获取Imgui上下文
     *
     * @return ImGuiContext* 上下文指针
     */
    ImGuiContext *GetImguiCtx()
    {
        return ImGui::GetCurrentContext();
    }

    private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> imgui_srv_heap_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbv_heap_;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_;
    Microsoft::WRL::ComPtr<ID3DBlob> vs_bytecode_;
    Microsoft::WRL::ComPtr<ID3DBlob> ps_bytecode_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso_;
    std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout_;
    std::shared_ptr<UpLoadBuffer<TransMatrix>> const_buffer_obj_ = nullptr;
    std::unique_ptr<MeshGeometry> box_geo_;
    Matrix4f proj_mat_;
};

#endif