// #version 330 core
// out vec4 FragColor;
// out vec4 BrightColor;

// in vec2 TexCoords;

// uniform sampler2D hdrBuffer;
// uniform bool hdr;
// uniform float exposure;

// void main()
// {             
//     const float gamma = 2.2;
//     vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
//     if(hdr)
//     {
//         vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
//         // also gamma correct while we're at it       
//         result = pow(result, vec3(1.0 / gamma));
//         FragColor = vec4(result, 1.0);
//         FragColor *= vec4(100.0,100.0,100.0,1.0);
//         float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
//         if(brightness > 1.0)
//             BrightColor = vec4(FragColor.rgb, 1.0);
//         else
//             BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
//     }
//     else
//     {
//         vec3 result = pow(hdrColor, vec3(1.0 / gamma));
//         FragColor = vec4(result, 1.0);
//         float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
//         if(brightness > 1.0)
//             BrightColor = vec4(FragColor.rgb, 1.0);
//         else
//             BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
//     }
// }
#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;
uniform float exposure;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(scene, TexCoords).rgb;      
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    if(bloom)
        hdrColor += bloomColor; // additive blending
    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    // also gamma correct while we're at it       
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
} 