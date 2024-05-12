// #version 330 core
//     layout (location = 0) in vec3 pos;
//     layout (location = 1) in float light;
//     layout (location = 2) in vec2 uv;

//     out vec3 ColourOut; //to send vertex colours to the fragment shader
//     out vec2 texCordsFrag; //to send texture cords to the fragment shader

//     uniform sampler2D texture; //the texture

//     uniform mat4 modelMat; //model matrix
//     uniform mat4 viewMat; //view matrix
//     uniform mat4 projectionMat; //projection matrix

//     void main()
//     {   
//         gl_Position = projectionMat * viewMat * modelMat * vec4(pos, 1.0f);
//         // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0f);
//         ColourOut = vec3(light, light, light);
//         texCordsFrag = uv;
//     }
#version 330 core
    #define CHUNK_SIZE_X 32
    #define CHUNK_SIZE_Y 32
    #define CHUNK_SIZE_Z 32

    layout (location = 0) in uint data;

    out vec3 ColourOut; //to send vertex colours to the fragment shader
    out vec2 texCordsFrag; //to send texture cords to the fragment shader

    uniform sampler2D texture; //the texture

    uniform mat4 modelMat; //model matrix
    uniform mat4 viewMat; //view matrix
    uniform mat4 projectionMat; //projection matrix
    uniform ivec3 chunkPos; //chunk position

    void main()
    {   
        //extracting data from packed vertex
        uint bz = data & 63u;
        uint by = (data >> 6u) & 63u;
        uint bx = (data >> 12u) & 63u;
        uint lightInt = (data >> 18u) & 3u;
        uint vcInt = (data >> 20u) & 15u;
        uint ucInt = (data >> 24u) & 15u;
        
        //getting world position
        vec3 pos = vec3(float(int(bx) + (chunkPos.x * CHUNK_SIZE_X)), float(int(by) + (chunkPos.y * CHUNK_SIZE_Y)), float(int(bz) + (chunkPos.z * CHUNK_SIZE_Z)));
        // vec3 pos = vec3(float(bx), float(by), float(bz));

        //converting light values
        float lightFloat = 0.4f;

        if(lightInt == 1u){
            lightFloat = 0.6f;
        } else if(lightInt == 2u){
            lightFloat = 0.8f;
        } else {
            lightFloat = 1.0f;
        }

        //converting uv cords
        float u = float(ucInt) / 15f;
        float v = float(vcInt) / 15f;

        gl_Position = projectionMat * viewMat * modelMat * vec4(pos, 1.0f);
        ColourOut = vec3(lightFloat, lightFloat, lightFloat);
        texCordsFrag = vec2(u, v);
    }