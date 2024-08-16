#version 320 es
precision mediump float;

in vec2 UV;
flat in int instanceID;

uniform sampler2D atlasTexture;

out vec4 FragColor;

void main()
{
    int atlasIndex = instanceID % 16; // Cycle through 16 textures
    
    float atlasX = float(atlasIndex % 4) / 4.0;
    float atlasY = float(atlasIndex / 4) / 4.0;
    vec2 clampedUV = clamp(UV, 0.001, 0.999);  //  to hide the texture seem
    vec2 atlasCoords = vec2(atlasX + clampedUV.x / 4.0, 
                            atlasY + clampedUV.y / 4.0);
    FragColor = texture(atlasTexture, atlasCoords);
    // FragColor = vec4(1,0,0,1);
}



