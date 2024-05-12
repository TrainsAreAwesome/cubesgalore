#version 330 core
    out vec4 FragColor;
    in vec3 ColourOut;
    in vec4 pos;
    in vec2 texCordsFrag;

    uniform sampler2D texture1;
    uniform sampler2D texture2;
    void main()
    {
        FragColor = mix(texture(texture1, texCordsFrag) * vec4(ColourOut, 1.0), texture(texture2, texCordsFrag) * vec4(ColourOut, 1.0), 0.2);
    }