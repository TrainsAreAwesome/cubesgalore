#version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 Colour;
    layout (location = 2) in vec2 textureCords;

    out vec3 ColourOut; //to send vertex colours to the fragment shader
    out vec2 texCordsFrag; //to send texture cords to the fragment shader

    uniform mat4 transform; //custom transforms

    uniform sampler2D texture; //the texture

    uniform mat4 modelMat; //model matrix
    uniform mat4 viewMat; //view matrix
    uniform mat4 projectionMat; //projection matrix
    
    void main()
    {
        gl_Position = projectionMat * viewMat * modelMat * vec4(aPos, 1.0f);
        // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0f);
        ColourOut = Colour;
        texCordsFrag = textureCords;
    }