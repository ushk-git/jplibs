
#include <torn.h>
#include <assert.h>
#include <jpgfx.h>
#define STB_IMAGE_IMPLEMENTATION
#include <third-party/stb/stb_image.h>
typedef struct 
{
    float pos[3];
    float color[4];
    float uv[2];
} vertex;

i32 main(void)
{
    TORN_Init();
    
    /////////////////////////////////////////////////////////////////
    OS_App* app = OS_ConstructApp("jpgfx - testing", 
                                  OS_AppDefault,
                                  OS_AppDefault,
                                  OS_AppStyleDefault);
    assert(app && "Failed to create window");
    /////////////////////////////////////////////////////////////////
    
    
    jpgfx_renderer renderer = (jpgfx_renderer) 
    {
        .width = app->size.w,
        .height = app->size.h,
        .flags = JPGFX_RENDERER_VSYNC
    };
    
    OS_Win32Handle* handle = (OS_Win32Handle*)OS_GetHandle(app);
    jpgfx_d3d11_init(&renderer, handle->window); 
    jpgfx_create_renderer(&renderer);
    
    const char* vertexsh = 
        "struct VOut\n"
        "{\n"
        "float4 position : SV_POSITION;\n"
        "float4 color : COLOR;\n"
        "float2 uv : TEXCOORD;\n"
        "};\n"
        
        "VOut jpgfx_vs(float4 position : POSITION, float4 color : COLOR, float2 uv : TEXCOORD)\n"
        "{\n"
        "VOut output;\n"
        "output.position = position;\n"
        "output.color = color;\n"
        "output.uv = uv;\n"
        "return output;\n"
        "}\n\0";
    
    
    const char* fragsh = 
        "SamplerState th_sampler;\n"
        "Texture2D th_texture2d;\n" 
        
        
        "float4 jpgfx_ps(float4 position : SV_POSITION, float4 color : COLOR, float2 uv : TEXCOORD) : SV_TARGET\n"
        "{\n"
        
        "float4 tex = th_texture2d.Sample(th_sampler, uv);\n"
        "return tex;\n"
        "}\n\0";
    
    
    jpgfx_shader shader = (jpgfx_shader)
    {
        .vertex_shader = vertexsh,
        .pixel_shader = fragsh
    };
    
    jpgfx_result res;
    res= jpgfx_create_shader(&renderer, &shader);
    if (res.error)
    {
        printf("%s\n", res.message);
    } else
        jpgfx_use_shader(&renderer, &shader);
    r32 size;
    size = 1.0f;
    vertex vertices[] = 
    { 
        {  {size,  size, 0.0f},     {1,0,0,1}, {0, 0}},
        {  {size, -size, 0.0f},     {0,1,0,1}, {0, 1}},
        { {-size, -size, 0.0f},     {0,0,1,1}, {1, 1}},
        { {-size,  size, 0.0f},     {1,0,0,1}, {1, 0}}
    };
    size = 0.5f;
    vertex vertices2[] = 
    { 
        {  {size,  size, 0.0f},     {1,0,0,1}, {0, 0}},
        {  {size, -size, 0.0f},     {0,1,0,1}, {0, 1}},
        { {-size, -size, 0.0f},     {0,0,1,1}, {1, 1}},
        { {-size,  size, 0.0f},     {1,0,0,1}, {1, 0}}
    };
    uint32_t indices[] = {  
        0, 1, 3,
        1, 2, 3 
    };  
    
    
    jpgfx_ebo ebo =   
    {
        .data = indices,
        .size = sizeof(indices)*sizeof(uint32_t)
    };
    
    jpgfx_create_ebo(&renderer, &ebo);
    
    jpgfx_vbo vbo = (jpgfx_vbo)
    {
        .size = 1024,
        .usage = JPGFX_BUFFER_DYNAMIC
    };
    
    jpgfx_create_vbo(&renderer, &vbo);
    
    jpgfx_vbo vbo2 = (jpgfx_vbo)
    {
        .size = 1024,
        .usage = JPGFX_BUFFER_DYNAMIC
    };
    
    jpgfx_create_vbo(&renderer, &vbo2);
    
    
    jpgfx_input_element elements[] =
    {
        {"POSITION", JPGFX_FORMAT_RGB32_FLOAT, D3D11_APPEND_ALIGNED_ELEMENT },
        {"COLOR", JPGFX_FORMAT_RGBA32_FLOAT,   D3D11_APPEND_ALIGNED_ELEMENT },
        {"TEXCOORD", JPGFX_FORMAT_RG32_FLOAT,  D3D11_APPEND_ALIGNED_ELEMENT }
    };
    
    jpgfx_input_layout layout =
    {
        .input_elements = elements,
        .length = 3
    };
    
    jpgfx_create_input_layout(&renderer,
                              &shader,
                              &layout);
    
    jpgfx_set_input_layout(&renderer,
                           &layout);
    
    
    
    jpgfx_start_vbo_insertion(&renderer, &vbo);
    {
        jpgfx_add_data_to_vbo(&vbo, &vertices,   0);
    }
    jpgfx_end_vbo_insertion(&renderer,   &vbo);
    
    
    jpgfx_start_vbo_insertion(&renderer, &vbo2);
    {
        jpgfx_add_data_to_vbo(&vbo2, &vertices2,   0);
    }
    jpgfx_end_vbo_insertion(&renderer,   &vbo2);
    
    int w, h, ch;
    unsigned char* pixels = stbi_load("W:\\jpgfx\\misc\\images.png",
                                      &w, &h,
                                      &ch, 4);
    
    jpgfx_texture2d texture =
    {
        .width = w,
        .height = h,
        .data = pixels,
        .pitch = w * 4,
        .format = JPGFX_FORMAT_RGBA8_UNORM_SRGB
    };
    
    jpgfx_create_texture2d(&renderer, &texture);
    
    
    
    pixels = stbi_load("W:\\jpgfx\\misc\\ff.jpg",
                       &w, &h,
                       &ch, 4);
    
    jpgfx_texture2d texture2 =
    {
        .width = w,
        .height = h,
        .data = pixels,
        .pitch = w * 4,
        .format = JPGFX_FORMAT_RGBA8_UNORM_SRGB
    };
    
    jpgfx_create_texture2d(&renderer, &texture2);
    
    
    jpgfx_framebuffer2d framebuffer = {
        .format = JPGFX_FORMAT_RGBA32_FLOAT
    };
    jpgfx_create_framebuffer(&renderer,
                             &framebuffer);
    
    while (app->running)
    {
        OS_PollEvents(app);
        jpgfx_clear(&renderer);
        
        
        jpgfx_set_framebuffer(&renderer,
                              &framebuffer);
        {
            
            jpgfx_use_texture2d(&renderer, &texture2);
            jpgfx_draw_vbo_indexed(&renderer,
                                   &vbo, 
                                   &ebo,
                                   8,
                                   sizeof(vertex),
                                   JPGFX_TRIANGLELIST);
            
            jpgfx_use_texture2d(&renderer, &texture);
            jpgfx_draw_vbo_indexed(&renderer,
                                   &vbo2, 
                                   &ebo,
                                   8,
                                   sizeof(vertex),
                                   JPGFX_TRIANGLELIST);
            
        } 
        jpgfx_unset_framebuffer(&renderer, &framebuffer);
        
        
        jpgfx_use_texture2d(&renderer, &framebuffer.texture);
        jpgfx_draw_vbo_indexed(&renderer,
                               &vbo2, 
                               &ebo,
                               8,
                               sizeof(vertex),
                               JPGFX_TRIANGLELIST);
        
        
        
        jpgfx_present(&renderer);
        
    }
    
    jpgfx_destroy_renderer(&renderer);
    jpgfx_destroy_shader(&shader);
    jpgfx_destroy_input_layout(&layout);
    jpgfx_destroy_vbo(&vbo);
    jpgfx_destroy_ebo(&ebo);
    jpgfx_destroy_texture2d(&texture);
    
    
    TORN_Destroy();
    return 0;
}
