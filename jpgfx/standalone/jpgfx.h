#ifndef JPGFX_H
#define JPGFX_H


#include <stdint.h>
#ifndef jpgfx_alloc
#include <stdlib.h>
#define jpgfx_alloc malloc
#endif

#if defined(JPGFX_D3D11)
#define JPGFX_BACKEND "D3D11"
#elif defined(JPGFX_OPENGL)
#define JPGFX_BACKEND "OpenGL"
#else
#define JPGFX_BACKEND "Unknown"
#endif
// To identify functions or structures that contain any backend-related code.
#define jpgfx_backend_exclusive


#if defined(_WIN32)
#define jpgfx_function_export __declspec(dllexport)
#define jpgfx_function_import __declspec(dllimport)
#if defined(jpgfx_export)
#define jpgfx_function jpgfx_function_export
#elif defined(jpgfx_import)
#define jpgfx_function jpgfx_function_import
#else
#define jpgfx_function
#endif
#else
#define jpgfx_function
#endif
#define STRUCT(name) typedef struct name name
#define UNION(name) typedef struct name name
#define ENUM(name) typedef enum name name

//////////////////////////////////////////////
/*
   jpgfx_exclusive_* are objects that relate to the backend you are using.
*/

STRUCT(jpgfx_exclusive_renderer);
STRUCT(jpgfx_exclusive_input_layout);
STRUCT(jpgfx_exclusive_shader);
STRUCT(jpgfx_exclusive_vbo);
STRUCT(jpgfx_exclusive_ebo);
STRUCT(jpgfx_exclusive_texture2d);
STRUCT(jpgfx_exclusive_framebuffer2d);
//////////////////////////////////////////////


/* D3D11-releated includes */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define COBJMACROS
#define D3D11_NO_HELPERS
#define CINTERFACE
#include <d3d11_1.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
// We require the following libaries...
#pragma comment (lib, "user32.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "d3dcompiler.lib")

// Renderer
//////////////////////////////////////////////
struct jpgfx_exclusive_renderer
{
    ID3D11DeviceContext* device_context;
    ID3D11Device* device;
    IDXGISwapChain* swapchain;
    HWND window;
    ID3D11RenderTargetView* render_target_view;
    
    uint32_t sampler_index;
    ID3D11SamplerState* sampler_state;
    ID3D11BlendState* blend_state;
    ID3D11RasterizerState* rasterizer_state;
    ID3D11DepthStencilState* ds_state;
    ID3D11DepthStencilView* ds_view;
};

// Input Layout
//////////////////////////////////////////////
struct jpgfx_exclusive_input_layout
{
    ID3D11InputLayout* layout;
};

// Shader
//////////////////////////////////////////////
struct jpgfx_exclusive_shader
{
    ID3D10Blob* vertex_shader_blob;
    ID3D10Blob* pixel_shader_blob;
    ID3D11VertexShader* vertex_shader;
    ID3D11PixelShader* pixel_shader;
};

// VBOs
//////////////////////////////////////////////
struct jpgfx_exclusive_vbo
{
    ID3D11Buffer* buffer;
    D3D11_MAPPED_SUBRESOURCE subresource;
};

// EBOs
//////////////////////////////////////////////
struct jpgfx_exclusive_ebo
{
    ID3D11Buffer* buffer;
};

// Texture2D
//////////////////////////////////////////////
struct jpgfx_exclusive_texture2d
{
    ID3D11ShaderResourceView* srv;
};

// Framebuffer2D
//////////////////////////////////////////////
struct jpgfx_exclusive_framebuffer2d
{
    ID3D11RenderTargetView* rtv;
    ID3D11Texture2D* rtv_texture;
    ID3D11ShaderResourceView* srv;
};




#else
// TODO: OpenGL, and possibly METAL
#endif

//////////////////////////////////////////////
// Structures and Enums of all objects.
STRUCT(jpgfx_result); 
STRUCT(jpgfx_renderer);
STRUCT(jpgfx_shader);
STRUCT(jpgfx_vbo);
STRUCT(jpgfx_ebo);
STRUCT(jpgfx_input_element);
STRUCT(jpgfx_input_layout);
STRUCT(jpgfx_texture2d);
STRUCT(jpgfx_framebuffer2d);
STRUCT(jpgfx_rgba);
ENUM(jpgfx_format);
ENUM(jpgfx_renderer_flags);
ENUM(jpgfx_buffer_usage);
ENUM(jpgfx_primitive_type);
//////////////////////////////////////////////
//  Generalized structures and enums.
//////////////////////////////////////////////
struct jpgfx_result
{
    uint8_t  error; 
    uint8_t* message;
};

struct jpgfx_rgba
{
    float r;
    float g;
    float b;
    float a;
};
enum jpgfx_buffer_usage
{
    JPGFX_BUFFER_STATIC,
    JPGFX_BUFFER_DYNAMIC
};
enum jpgfx_format
{
    JPGFX_FORMAT_RGB32_FLOAT,
    JPGFX_FORMAT_RG32_FLOAT,
    JPGFX_FORMAT_RGBA32_FLOAT,
    JPGFX_FORMAT_RGBA8_UNORM,
    JPGFX_FORMAT_RGBA8_UNORM_SRGB
};
// Renderer
////////////////////////////////////////////////////////
enum jpgfx_renderer_flags
{
    JPGFX_RENDERER_VSYNC
};

struct jpgfx_renderer
{
    uint32_t width;
    uint32_t height;
    jpgfx_rgba clear_color;
    uint8_t flags;
    
    /*
 * This only relates to the platform we are using. 
     * Please do not modify unless you *know* what you are doing. 
*/
    jpgfx_backend_exclusive jpgfx_exclusive_renderer exclusive;
    
};


jpgfx_function jpgfx_result
jpgfx_create_renderer(jpgfx_renderer* renderer);
jpgfx_function void
jpgfx_resize_renderer(jpgfx_renderer* renderer,
                      float x,
                      float y,
                      float w,
                      float h);
jpgfx_function void
jpgfx_clear(jpgfx_renderer* renderer);

jpgfx_function void
jpgfx_present(jpgfx_renderer* renderer);
//////////////////////////////////////////////


// Shaders
////////////////////////////////////////////////////////
struct jpgfx_shader
{
    uint8_t* vertex_shader;
    uint8_t* pixel_shader;
    /*
 * This only relates to the platform we are using. 
     * Please do not modify unless you *know* what you are doing. 
*/
    jpgfx_backend_exclusive jpgfx_exclusive_shader exclusive;
};

jpgfx_function jpgfx_result
jpgfx_create_shader(jpgfx_renderer* renderer, 
                    jpgfx_shader* shader);
jpgfx_function void 
jpgfx_use_shader(jpgfx_renderer* renderer,
                 jpgfx_shader* shader);
jpgfx_function void 
jpgfx_disable_shader(jpgfx_renderer* renderer);

//////////////////////////////////////////////
// EBOs
//////////////////////////////////////////////
struct jpgfx_ebo
{
    uint32_t* data;
    size_t  size;
    
    jpgfx_backend_exclusive jpgfx_exclusive_ebo exclusive;
};

jpgfx_function void
jpgfx_create_ebo(jpgfx_renderer* renderer,
                 jpgfx_ebo* ebo);
//////////////////////////////////////////////
// VBOs
////////////////////////////////////////////////////////
enum jpgfx_primitive_type
{
    JPGFX_TRIANGLELIST
};
struct jpgfx_vbo
{
    jpgfx_buffer_usage usage;
    uint32_t size;   
    uint8_t insertion_mode;
	
    jpgfx_backend_exclusive jpgfx_exclusive_vbo exclusive;
};
jpgfx_function void
jpgfx_create_vbo(jpgfx_renderer* renderer, 
                 jpgfx_vbo* vbo);

jpgfx_function void
jpgfx_set_data_to_vbo(jpgfx_renderer* renderer, 
                      jpgfx_vbo* vbo,
                      void* data);

jpgfx_function void
jpgfx_start_vbo_insertion(jpgfx_renderer* renderer, 
                          jpgfx_vbo* vbo);
jpgfx_function void
jpgfx_add_data_to_vbo( jpgfx_vbo* vbo,
                      void* data, uint32_t offset);
jpgfx_function void
jpgfx_end_vbo_insertion(jpgfx_renderer* renderer, 
                        jpgfx_vbo* vbo);


jpgfx_function void
jpgfx_draw_vbo(jpgfx_renderer* renderer, 
               jpgfx_vbo* vbo, 
               uint32_t vertices,
               uint32_t stride,
               jpgfx_primitive_type primitive);


jpgfx_function void
jpgfx_draw_vbo_indexed(jpgfx_renderer* renderer, 
                       jpgfx_vbo* vbo, 
                       jpgfx_ebo* ebo,
                       uint32_t vertices,
                       uint32_t stride,
                       jpgfx_primitive_type primitive);
////////////////////////////////////////////////////////
// LAYOUTs
////////////////////////////////////////////////////////
struct jpgfx_input_element
{
    uint8_t* name;
    jpgfx_format format;
    uint32_t offset;
};

struct jpgfx_input_layout
{
    jpgfx_input_element* input_elements;
    size_t length;
    jpgfx_backend_exclusive jpgfx_exclusive_input_layout exclusive;
};

jpgfx_function void
jpgfx_create_input_layout(jpgfx_renderer* renderer, 
                          jpgfx_shader* shader,
                          jpgfx_input_layout* input_layout);


jpgfx_function void
jpgfx_set_input_layout(jpgfx_renderer* renderer,
                       jpgfx_input_layout* input_layout);

jpgfx_function void
jpgfx_unset_input_layout(jpgfx_renderer* renderer);





// Texture2D
////////////////////////////////////////////////////////
struct jpgfx_texture2d
{
    float width;
    float height;
    void* data;
    uint32_t pitch;
    
    jpgfx_format format;
    
    
    jpgfx_backend_exclusive jpgfx_exclusive_texture2d exclusive;
};


jpgfx_function void
jpgfx_create_texture2d(jpgfx_renderer* renderer,
                       jpgfx_texture2d* texture2d);


jpgfx_function void
jpgfx_use_texture2d(jpgfx_renderer* renderer, 
                    jpgfx_texture2d* texture2d);
// Framebuffers
////////////////////////////////////////////////////////
struct jpgfx_framebuffer2d
{
    jpgfx_texture2d texture;
    jpgfx_format format;
    
    
    jpgfx_backend_exclusive jpgfx_exclusive_framebuffer2d exclusive;
};

jpgfx_function void
jpgfx_create_framebuffer(jpgfx_renderer* renderer,
                         jpgfx_framebuffer2d* framebuffer);

jpgfx_function void
jpgfx_set_framebuffer(jpgfx_renderer* renderer, 
                      jpgfx_framebuffer2d* framebuffer);

jpgfx_function void
jpgfx_unset_framebuffer(jpgfx_renderer* renderer, 
                        jpgfx_framebuffer2d* framebuffer);
// Clean-up
////////////////////////////////////////////////////////
jpgfx_function void
jpgfx_destroy_renderer(jpgfx_renderer* renderer);

jpgfx_function void
jpgfx_destroy_input_layout(jpgfx_input_layout* input_layout);

jpgfx_function void
jpgfx_destroy_shader(jpgfx_shader* shader);

jpgfx_function void
jpgfx_destroy_vbo(jpgfx_vbo* vbo);

jpgfx_function void
jpgfx_destroy_ebo(jpgfx_ebo* ebo);

jpgfx_function void
jpgfx_destroy_texture2d(jpgfx_texture2d* texture2d);

jpgfx_function void
jpgfx_destroy_framebuffer2d(jpgfx_framebuffer2d* framebuffer2d);

#if defined(JPGFX_D3D11)
// D3D11
////////////////////////////////////////////////////////
jpgfx_backend_exclusive jpgfx_function void
jpgfx_d3d11_init(jpgfx_renderer* renderer, 
                 HWND window);

// Shaders (HLSL)
////////////////////////////////////////////////////////
jpgfx_backend_exclusive jpgfx_function void 
jpgfx_hlsl_cbuffer(jpgfx_renderer* renderer,
                   jpgfx_shader* shader, 
                   uint32_t slot,
                   uint32_t size,
                   void* data);
#endif

// Information releated functions.
////////////////////////////////////////////////////////
jpgfx_function uint8_t*
jpgfx_get_backend(void);
#if defined (JPGFX_IMPL)

jpgfx_function uint8_t*
jpgfx_get_backend(void)
{
    return JPGFX_BACKEND;
}


#if defined(JPGFX_D3D11)

// jpgfx_d3d11.c should be here!

#endif // JPGFX_D3D11


#endif // JPGFX_IMPL



#undef STRUCT
#undef UNION
#undef ENUM
