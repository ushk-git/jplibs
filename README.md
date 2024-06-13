# jplibs
Single-file libaries.

### Libraries and their coorsponding version, and information
| Library Name | Version | Short description |
|:-------------|:--------|:------------------|
| jp_gfx        | 1.0     | A low-level **granular** library that unifies rendering backends. |

### Information about the libraries.

#### jp_gfx
- jp_gfx is a low-level granular library that unifies multiple rendering backends. 

- jp_gfx arrives in two formats 
  - standalone
      - You are given just the header files without any function implementations. 
      - If you choose to, you can use a rendering-backend that's external from the header file. 
  - full
      - everything required is in a single header file.

| Rendering Backends | Proficiency with functionality  | Activation definitions |
|:-------------------|:--------------------------------|:-----------------------|
| Direct3D 11        | Proficiency is at the standarized level. | JPGFX_D3D11 |
| OpenGL 3.3         | Pending development | JPGFX_OPENGL |    


If you are using the `full` version of jpgfx, you must define `JPGFX_IMPL` to add the implementation.
```c
#define JPGFX_IMPL
#include <jpgfx.h>
``` 


# Authors

[[github.com/jpshokar]]


