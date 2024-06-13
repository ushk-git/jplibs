#include "jpgfx.h"



static jpgfx_function DXGI_FORMAT
jpgfx_convert_format_to_d3d11_format(jpgfx_format format)
{
    switch (format)
    {
        case JPGFX_FORMAT_RGB32_FLOAT: {
            return DXGI_FORMAT_R32G32B32_FLOAT;
        }break;
        case JPGFX_FORMAT_RG32_FLOAT: {
            return DXGI_FORMAT_R32G32_FLOAT;
        }break;
        case JPGFX_FORMAT_RGBA32_FLOAT: {
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        }break;
        case JPGFX_FORMAT_RGBA8_UNORM : {
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        } break;
        case JPGFX_FORMAT_RGBA8_UNORM_SRGB : 
        {
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        } break;
        default: break;
    }
    
    return DXGI_FORMAT_R32G32B32A32_FLOAT; 
}

jpgfx_backend_exclusive jpgfx_function void
jpgfx_d3d11_init(jpgfx_renderer* renderer, 
                 HWND window)
{
    renderer->exclusive.window = window;
}


jpgfx_function jpgfx_result
jpgfx_create_renderer(jpgfx_renderer* renderer)
{
    jpgfx_result result = {0};
    
    
    DXGI_SWAP_CHAIN_DESC swapchain_desc =
    {
        .Windowed = 1,
        .SampleDesc.Count = 1,
        .BufferCount = 2,
        .BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .OutputWindow = renderer->exclusive.window,
    };
    
    HRESULT error;
    
    D3D_FEATURE_LEVEL feature_level[] = 
    {
        D3D_FEATURE_LEVEL_11_0 
    };
    
    
    error = D3D11CreateDeviceAndSwapChain(0,
                                          D3D_DRIVER_TYPE_HARDWARE,
                                          0,
                                          D3D11_CREATE_DEVICE_DEBUG,
                                          feature_level,
                                          sizeof(feature_level) /sizeof(D3D_FEATURE_LEVEL),
                                          D3D11_SDK_VERSION,
                                          &swapchain_desc,
                                          &renderer->exclusive.swapchain,
                                          &renderer->exclusive.device,
                                          0,
                                          &renderer->exclusive.device_context);
    
    if (FAILED(error))
    {
        result.error = 1;
        result.message = "direct3d - failed to create a device and swapchain.";
        
        return result;
    }
    
    
    ID3D11Texture2D* buffer;
    error = IDXGISwapChain_GetBuffer(renderer->exclusive.swapchain,
                                     0,
                                     &IID_ID3D11Texture2D,
                                     (LPVOID*)&buffer);
    
    if (FAILED(error))
    {
        result.error = 1;
        result.message = "direct3d - failed to get swapchain texture";
        return result;
    }
    
    error = ID3D11Device_CreateRenderTargetView(renderer->exclusive.device,
                                                (ID3D11Resource*)buffer,
                                                0,
                                                &renderer->exclusive.render_target_view);
    
    if (FAILED(error))
    {
        result.error = 1;
        result.message = "direct3d - failed to create render target view";
        return result;
    }
    
    
    ID3D11DeviceContext_OMSetRenderTargets(renderer->exclusive.device_context,
                                           1,
                                           &renderer->exclusive.render_target_view,
                                           0);
    
    ID3D11Texture2D_Release(buffer);
    
    // Sampler State
    D3D11_SAMPLER_DESC sampler_description =
    {
        .Filter =D3D11_FILTER_MIN_MAG_MIP_LINEAR,
        .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
        .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
        .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
        .ComparisonFunc = D3D11_COMPARISON_NEVER,
        .MaxAnisotropy  = 1,
        .MinLOD = 0,
        .MaxLOD = D3D11_FLOAT32_MAX
    };
    
    ID3D11Device_CreateSamplerState(renderer->exclusive.device,
                                    &sampler_description,
                                    &renderer->exclusive.sampler_state);
    
    
    
    ID3D11DeviceContext_PSSetSamplers(renderer->exclusive.device_context,
                                      renderer->exclusive.sampler_index,
                                      1,
                                      &renderer->exclusive.sampler_state);
    
    
    // Blend
    
    D3D11_BLEND_DESC blend_description = 
    {
        .RenderTarget = 
        {
            [0] = {
                .BlendEnable = 1,
                .SrcBlend = D3D11_BLEND_SRC_ALPHA,
                .DestBlend = D3D11_BLEND_INV_SRC_ALPHA,
                .BlendOp = D3D11_BLEND_OP_ADD,
                .SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA,
                .DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA,
                .BlendOpAlpha = D3D11_BLEND_OP_ADD,
                .RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL
            }
        }
    };
    
    ID3D11Device_CreateBlendState(renderer->exclusive.device,
                                  &blend_description,
                                  &renderer->exclusive.blend_state);
    
    ID3D11DeviceContext_OMSetBlendState(renderer->exclusive.device_context,
                                        renderer->exclusive.blend_state,
                                        0,
                                        ~0U);
    
    
    D3D11_RASTERIZER_DESC rasterizer_desc =
    {
        .FillMode = D3D11_FILL_SOLID,
        .CullMode = D3D11_CULL_NONE
    };
    
    ID3D11Device_CreateRasterizerState(renderer->exclusive.device,
                                       &rasterizer_desc,
                                       &renderer->exclusive.rasterizer_state);
    
    
    ID3D11DeviceContext_RSSetState(renderer->exclusive.device_context,
                                   renderer->exclusive.rasterizer_state);
    
    
    
    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {
        .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
        .DepthFunc = D3D11_COMPARISON_LESS,
        .StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
        .StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK
    };
    
    ID3D11Device_CreateDepthStencilState(renderer->exclusive.device,
                                         &depth_stencil_desc,
                                         &renderer->exclusive.ds_state);
    
    ID3D11DeviceContext_OMSetDepthStencilState(renderer->exclusive.device_context,
                                               renderer->exclusive.ds_state,
                                               0);
    
    
    D3D11_TEXTURE2D_DESC deptht2d_desc = {
        .Width = renderer->width,
        .Height = renderer->height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_D32_FLOAT,
        .SampleDesc.Count = 1,
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_DEPTH_STENCIL
    };
    
    ID3D11Texture2D* depth_t2d;
    ID3D11Device_CreateTexture2D(renderer->exclusive.device,
                                 &deptht2d_desc,
                                 0,
                                 &depth_t2d);
    ID3D11Device_CreateDepthStencilView(renderer->exclusive.device,
                                        (ID3D11Resource*)depth_t2d,
                                        0,
                                        &renderer->exclusive.ds_view);
    ID3D11Texture2D_Release(depth_t2d);
    
    ID3D11DeviceContext_OMSetRenderTargets(renderer->exclusive.device_context,
                                           1, &renderer->exclusive.render_target_view,
                                           renderer->exclusive.ds_view);
    
    
    D3D11_VIEWPORT viewport = 
    {
        .Width = renderer->width,
        .Height = renderer->height
    };
    
    
    ID3D11DeviceContext_RSSetViewports(renderer->exclusive.device_context,
                                       1,
                                       &viewport);
    
    return result;
}



jpgfx_function void
jpgfx_resize_renderer(jpgfx_renderer* renderer,
                      float x,
                      float y,
                      float w,
                      float h)
{
    renderer->width = w;
    renderer->height = h;
    D3D11_VIEWPORT viewport = 
    {
        .Width = renderer->width,
        .Height = renderer->height,
        .TopLeftX = x,
        .TopLeftY = y
    };
    
    ID3D11DeviceContext_RSSetViewports(renderer->exclusive.device_context,
                                       1,
                                       &viewport);
    
}

jpgfx_function void
jpgfx_clear(jpgfx_renderer* renderer)
{
    
    float color[4] = {
        renderer->clear_color.r   / 255.0f,
        renderer->clear_color.g / 255.0f,
        renderer->clear_color.b  / 255.0f, 
        renderer->clear_color.a / 255.0f
    };
    
    ID3D11DeviceContext_ClearRenderTargetView(renderer->exclusive.device_context,
                                              renderer->exclusive.render_target_view,
                                              color);
    
    ID3D11DeviceContext_ClearDepthStencilView(renderer->exclusive.device_context,
                                              renderer->exclusive.ds_view,
                                              D3D11_CLEAR_DEPTH,
                                              1.0f,
                                              0);
    
    
    
    
}


jpgfx_function void
jpgfx_present(jpgfx_renderer* renderer)
{
    uint8_t vsync = renderer->flags & JPGFX_RENDERER_VSYNC;
    
    HRESULT window_status = IDXGISwapChain_Present(renderer->exclusive.swapchain,
                                                   vsync,0);
    if (window_status == DXGI_STATUS_OCCLUDED)
    {
        if (vsync) Sleep(10);
    }
}



jpgfx_function jpgfx_result
jpgfx_create_shader(jpgfx_renderer* renderer, jpgfx_shader* shader)
{
    jpgfx_result result = {0};
    
    jpgfx_exclusive_shader* e_shader = &shader->exclusive;
    
    ID3D10Blob* error;
    HRESULT hr;
    
    uint32_t shader_len = 0;
    while (shader->vertex_shader[shader_len++] != '\0');
    
    hr= D3DCompile(shader->vertex_shader, 
                   shader_len,
                   0,0,0,
                   "jpgfx_vs","vs_5_0",
                   0,0,
                   &e_shader->vertex_shader_blob, &error);
    
    if (FAILED(hr))
    {
        result.error = 1;
        result.message = ID3D10Blob_GetBufferPointer(error);
#ifdef JPGFX_SHADER_ERROR_MESSAGEBOX
        MessageBox(0, result.message, "error: vertex-shader", MB_ICONWARNING);
#endif
        return result;
    }
    
    shader_len = 0;
    while (shader->pixel_shader[shader_len++] != '\0');
    
    hr= D3DCompile(shader->pixel_shader, 
                   shader_len,
                   0,0,0,
                   "jpgfx_ps","ps_5_0",
                   0,0,
                   &e_shader->pixel_shader_blob, &error);
    
    if (FAILED(hr))
    {
        result.error = 1;
        result.message = ID3D10Blob_GetBufferPointer(error);
#ifdef JPGFX_SHADER_ERROR_MESSAGEBOX
        MessageBox(0, result.message, "error: pixel-shader", MB_ICONWARNING);
#endif
        return result;
    }
    
    ID3D11Device_CreateVertexShader(renderer->exclusive.device,
                                    ID3D10Blob_GetBufferPointer(e_shader->vertex_shader_blob),
                                    ID3D10Blob_GetBufferSize(e_shader->vertex_shader_blob),
                                    0,
                                    &e_shader->vertex_shader);
    
    
    ID3D11Device_CreatePixelShader(renderer->exclusive.device,
                                   ID3D10Blob_GetBufferPointer(e_shader->pixel_shader_blob),
                                   ID3D10Blob_GetBufferSize(e_shader->pixel_shader_blob),
                                   0,
                                   &e_shader->pixel_shader);
    
    
    return result;
}


jpgfx_function void 
jpgfx_use_shader(jpgfx_renderer* renderer, 
                 jpgfx_shader* shader)
{
    jpgfx_exclusive_shader* e_shader = &shader->exclusive;
    
    ID3D11DeviceContext_VSSetShader(renderer->exclusive.device_context, e_shader->vertex_shader, 0, 0);
    
    ID3D11DeviceContext_PSSetShader(renderer->exclusive.device_context, e_shader->pixel_shader, 0, 0);
}


jpgfx_function void 
jpgfx_disable_shader(jpgfx_renderer* renderer)
{
    ID3D11DeviceContext_VSSetShader(renderer->exclusive.device_context, 0, 0, 0);
    ID3D11DeviceContext_PSSetShader(renderer->exclusive.device_context, 0, 0, 0);
}



jpgfx_backend_exclusive jpgfx_function void
jpgfx_hlsl_cbuffer(jpgfx_renderer* renderer,
                   jpgfx_shader* shader, 
                   uint32_t slot,
                   uint32_t size,
                   void* data)
{
    D3D11_BUFFER_DESC buffer_desc = 
    {
        .ByteWidth = size,
        .Usage = D3D11_USAGE_DYNAMIC,
        .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
        .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE
    };
    D3D11_SUBRESOURCE_DATA resource = { data };
    
    ID3D11Buffer* c_buffer;
    ID3D11Device_CreateBuffer(renderer->exclusive.device,
                              &buffer_desc,
                              &resource,
                              &c_buffer);
    jpgfx_use_shader(renderer, shader);
    
    ID3D11DeviceContext_VSSetConstantBuffers(renderer->exclusive.device_context,
                                             slot, 1, &c_buffer);
    
}


// VBOs
/////////////////////////////////////////////////////////////////////

jpgfx_function void
jpgfx_create_vbo(jpgfx_renderer* renderer, jpgfx_vbo* vbo)
{
    
    jpgfx_exclusive_vbo * e_vbo = &vbo->exclusive;
    
    D3D11_USAGE usage = {0};
    switch (vbo->usage)
    {
        case JPGFX_BUFFER_DYNAMIC: 
        {
            usage = D3D11_USAGE_DYNAMIC;
        } break;
        case JPGFX_BUFFER_STATIC: 
        {
            usage = D3D11_USAGE_IMMUTABLE;
        } break;
        default:
        {
            usage =  D3D11_USAGE_DEFAULT;
        } break;
    }
    
    D3D11_BUFFER_DESC b_desc = {
        .Usage = usage,
        .ByteWidth = vbo->size,
        .BindFlags = D3D11_BIND_VERTEX_BUFFER,
        .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE
    };
    
    ID3D11Device_CreateBuffer(renderer->exclusive.device,
                              &b_desc,
                              0,
                              &e_vbo->buffer);
    
}

jpgfx_function void
jpgfx_set_data_to_vbo(jpgfx_renderer* renderer, 
                      jpgfx_vbo* vbo,
                      void* data)
{
    
    
    jpgfx_exclusive_vbo* e_vbo = &vbo->exclusive;
    
    ID3D11DeviceContext_Map(renderer->exclusive.device_context,
                            (ID3D11Resource*)e_vbo->buffer,
                            0,
                            D3D11_MAP_WRITE_NO_OVERWRITE, 
                            0,
                            &e_vbo->subresource);
    
    
    CopyMemory((uint8_t*)e_vbo->subresource.pData ,
               data,
               vbo->size);
    ID3D11DeviceContext_Unmap(renderer->exclusive.device_context, 
                              (ID3D11Resource*)e_vbo->buffer,
                              0);
}


jpgfx_function void
jpgfx_add_data_to_vbo( jpgfx_vbo* vbo,
                      void* data, uint32_t offset)
{
    CopyMemory((uint8_t*)vbo->exclusive.subresource.pData + offset,
               data,
               vbo->size);
}
jpgfx_function void
jpgfx_start_vbo_insertion(jpgfx_renderer* renderer, 
                          jpgfx_vbo* vbo)
{
    vbo->insertion_mode = 1;
    ID3D11DeviceContext_Map(renderer->exclusive.device_context,
                            (ID3D11Resource*)vbo->exclusive.buffer,
                            0,
                            D3D11_MAP_WRITE_NO_OVERWRITE, 
                            0,
                            &vbo->exclusive.subresource);
}

jpgfx_function void
jpgfx_end_vbo_insertion(jpgfx_renderer* renderer, 
                        jpgfx_vbo* vbo)
{
    vbo->insertion_mode = 0;
    ID3D11DeviceContext_Unmap(renderer->exclusive.device_context, 
                              (ID3D11Resource*)vbo->exclusive.buffer,
                              0);
}
jpgfx_function void
jpgfx_draw_vbo(jpgfx_renderer* renderer, 
               jpgfx_vbo* vbo,
               uint32_t vertices,
               uint32_t stride,
               jpgfx_primitive_type primitive)
{
    uint32_t offset = 0;
    ID3D11DeviceContext_IASetVertexBuffers(renderer->exclusive.device_context, 
                                           0, 1,
                                           &vbo->exclusive.buffer,
                                           &stride, &offset);
    if (primitive == JPGFX_TRIANGLELIST)
        ID3D11DeviceContext_IASetPrimitiveTopology(renderer->exclusive.device_context,D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ID3D11DeviceContext_Draw(renderer->exclusive.device_context,
                             vertices, 0);
    
    
}
jpgfx_function void
jpgfx_draw_vbo_indexed(jpgfx_renderer* renderer, 
                       jpgfx_vbo* vbo, 
                       jpgfx_ebo* ebo,
                       uint32_t vertices,
                       uint32_t stride,
                       jpgfx_primitive_type primitive)
{
    ID3D11DeviceContext_IASetIndexBuffer(renderer->exclusive.device_context,
                                         ebo->exclusive.buffer,
                                         DXGI_FORMAT_R32_UINT,
                                         0);
    uint32_t offset = 0;
    ID3D11DeviceContext_IASetVertexBuffers(renderer->exclusive.device_context, 
                                           0, 1,
                                           &vbo->exclusive.buffer,
                                           &stride, &offset);
    if (primitive == JPGFX_TRIANGLELIST)
        ID3D11DeviceContext_IASetPrimitiveTopology(renderer->exclusive.device_context,D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    ID3D11DeviceContext_DrawIndexed(renderer->exclusive.device_context,
                                    ebo->size, 0, 0);
    
    
    ID3D11DeviceContext_IASetIndexBuffer(renderer->exclusive.device_context,
                                         0,
                                         DXGI_FORMAT_R32_UINT,
                                         0);
}




// LAYOUTS
////////////////////////////////////////////////////////


jpgfx_function void
jpgfx_create_input_layout(jpgfx_renderer* renderer, 
                          jpgfx_shader* shader,
                          jpgfx_input_layout* input_layout) 
{
    
    
    D3D11_INPUT_ELEMENT_DESC* input_elements = jpgfx_alloc(sizeof(D3D11_INPUT_ELEMENT_DESC)*input_layout->length);
    
    for (int i=0;i<input_layout->length;i++)
    {
        input_elements[i].SemanticName  = input_layout->input_elements[i].name;
        input_elements[i].SemanticIndex = 0;
        input_elements[i].Format = jpgfx_convert_format_to_d3d11_format(input_layout->input_elements[i].format);
        
        input_elements[i].InputSlot = 0;
        input_elements[i].AlignedByteOffset = input_layout->input_elements[i].offset;
        input_elements[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        input_elements[i].InstanceDataStepRate  = 0;
    }
    
    
    ID3D11Device_CreateInputLayout(renderer->exclusive.device,
                                   input_elements,
                                   input_layout->length,
                                   ID3D10Blob_GetBufferPointer(shader->exclusive.vertex_shader_blob),
                                   ID3D10Blob_GetBufferSize(shader->exclusive.vertex_shader_blob),
                                   &input_layout->exclusive.layout);
    
    
    
    
}


jpgfx_function void
jpgfx_unset_input_layout(jpgfx_renderer* renderer)
{
    ID3D11DeviceContext_IASetInputLayout(renderer->exclusive.device_context,
                                         0);
}

jpgfx_function void
jpgfx_set_input_layout(jpgfx_renderer* renderer,
                       jpgfx_input_layout* input_layout)
{
    ID3D11DeviceContext_IASetInputLayout(renderer->exclusive.device_context,
                                         input_layout->exclusive.layout);
}


// EBO


jpgfx_function void
jpgfx_create_ebo(jpgfx_renderer* renderer,
                 jpgfx_ebo* ebo)
{
    
    
    D3D11_BUFFER_DESC buffer_description = 
    {
        .ByteWidth = ebo->size,
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_INDEX_BUFFER
    };
    
    
    D3D11_SUBRESOURCE_DATA subresource = {
        ebo->data
    };
    
    
    ID3D11Device_CreateBuffer(renderer->exclusive.device,
                              &buffer_description,
                              &subresource,
                              &ebo->exclusive.buffer);
    
    
}



// Clean-up


jpgfx_function void
jpgfx_destroy_renderer(jpgfx_renderer* renderer)
{
    
    ID3D11DeviceContext_Release(renderer->exclusive.device_context);
    ID3D11Device_Release(renderer->exclusive.device);
    IDXGISwapChain_Release(renderer->exclusive.swapchain);
    ID3D11RenderTargetView_Release(renderer->exclusive.render_target_view);
    
    
    
}

jpgfx_function void
jpgfx_destroy_input_layout(jpgfx_input_layout* input_layout)
{
    ID3D11InputLayout_Release(input_layout->exclusive.layout);
}

jpgfx_function void
jpgfx_destroy_shader(jpgfx_shader* shader)
{
    ID3D10Blob_Release(shader->exclusive.vertex_shader_blob);
    ID3D10Blob_Release(shader->exclusive.pixel_shader_blob);
    ID3D11VertexShader_Release(shader->exclusive.vertex_shader);
    ID3D11PixelShader_Release(shader->exclusive.pixel_shader);
}

jpgfx_function void
jpgfx_destroy_vbo(jpgfx_vbo* vbo)
{
    ID3D11Buffer_Release(vbo->exclusive.buffer);
}

jpgfx_function void
jpgfx_destroy_ebo(jpgfx_ebo* ebo)
{
    ID3D11Buffer_Release(ebo->exclusive.buffer);
}


// Texture2D

jpgfx_function void
jpgfx_create_texture2d(jpgfx_renderer* renderer,
                       jpgfx_texture2d* texture2d)
{
    D3D11_TEXTURE2D_DESC t2d_d = 
    {
        .Width = texture2d->width,
        .Height = texture2d->height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = jpgfx_convert_format_to_d3d11_format(texture2d->format),
        .SampleDesc = {1,0},
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_SHADER_RESOURCE
    };
    
    D3D11_SUBRESOURCE_DATA srd =
    {
        .pSysMem = texture2d->data,
        .SysMemPitch = texture2d->pitch
    };
    
    
    ID3D11Texture2D* texture;
    
    ID3D11Device_CreateTexture2D(renderer->exclusive.device,
                                 &t2d_d,
                                 &srd,
                                 &texture);
    
    ID3D11Device_CreateShaderResourceView(renderer->exclusive.device,
                                          (ID3D11Resource*)texture, 
                                          0,
                                          &texture2d->exclusive.srv);
    
    /*ID3D11DeviceContext_GenerateMips(renderer->exclusive.device_context,
                                     texture2d->exclusive.srv);
    */
    ID3D11Texture2D_Release(texture);
    
    
    
    
}


jpgfx_function void
jpgfx_use_texture2d(jpgfx_renderer* renderer,
                    jpgfx_texture2d* texture2d)
{
    
    ID3D11DeviceContext_PSSetShaderResources(renderer->exclusive.device_context,
                                             0,
                                             1,
                                             &texture2d->exclusive.srv);
}

jpgfx_function void
jpgfx_destroy_texture2d(jpgfx_texture2d* texture2d)
{
    ID3D11ShaderResourceView_Release(texture2d->exclusive.srv);
    
}

// Framebuffer

jpgfx_function void
jpgfx_create_framebuffer(jpgfx_renderer* renderer,
                         jpgfx_framebuffer2d* framebuffer)
{
    
    D3D11_TEXTURE2D_DESC t2d_d = 
    {
        .Width = renderer->width,
        .Height =renderer->height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = jpgfx_convert_format_to_d3d11_format(framebuffer->format),
        .SampleDesc = {1,0},
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE
    };
    
    ID3D11Device_CreateTexture2D(renderer->exclusive.device,
                                 &t2d_d,
                                 0,
                                 &framebuffer->exclusive.rtv_texture);
    
    
    D3D11_RENDER_TARGET_VIEW_DESC rtv_d =
    {
        .Format = t2d_d.Format,
        .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D
    };
    
    ID3D11Device_CreateRenderTargetView(renderer->exclusive.device,
                                        (ID3D11Resource*)framebuffer->exclusive.rtv_texture,
                                        &rtv_d,
                                        &framebuffer->exclusive.rtv);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_d =
    {
        .Format = t2d_d.Format,
        .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
        .Texture2D =
        {
            .MipLevels = t2d_d.MipLevels
        }
    };
    
    
    ID3D11Device_CreateShaderResourceView(renderer->exclusive.device,
                                          (ID3D11Resource*)framebuffer->exclusive.rtv_texture,
                                          &srv_d,
                                          &framebuffer->exclusive.srv);
    
    framebuffer->texture.exclusive.srv = framebuffer->exclusive.srv;
    framebuffer->texture.width = renderer->width;
    framebuffer->texture.height = renderer->height;
    // Data is immutable. Please do not try to access void* data in this context. (jpgfx_texture2d)
    
    
    
}

jpgfx_function void
jpgfx_set_framebuffer(jpgfx_renderer* renderer, 
                      jpgfx_framebuffer2d* framebuffer)
{
    ID3D11DeviceContext_OMSetRenderTargets(renderer->exclusive.device_context,
                                           1,
                                           &framebuffer->exclusive.rtv,
                                           renderer->exclusive.ds_view);
}
jpgfx_function void
jpgfx_unset_framebuffer(jpgfx_renderer* renderer, 
                        jpgfx_framebuffer2d* framebuffer)
{
    
    ID3D11DeviceContext_OMSetRenderTargets(renderer->exclusive.device_context,
                                           1,
                                           &renderer->exclusive.render_target_view,
                                           0);
}

jpgfx_function void
jpgfx_destroy_framebuffer2d(jpgfx_framebuffer2d* framebuffer2d)
{
    ID3D11RenderTargetView_Release(framebuffer2d->exclusive.rtv);
    ID3D11Texture2D_Release(framebuffer2d->exclusive.rtv_texture);
    ID3D11ShaderResourceView_Release(framebuffer2d->exclusive.srv);
}