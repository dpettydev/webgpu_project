//#version 330

#if defined GL_ES
precision highp float;
#endif
#pragma optimize(off)
#pragma debug(on)

// PS_UI

varying vec4 MaudeIO_Color0;
varying vec2 MaudeIO_TexUV0;

uniform sampler2D Flat_TextureSampler0;

void main() {
    vec4 Texture2D_Lookup_3_Color_Out = texture2D(Flat_TextureSampler0, MaudeIO_TexUV0);
    vec4 V4xV4_5_Result = MaudeIO_Color0 * Texture2D_Lookup_3_Color_Out;
    gl_FragData[0] = V4xV4_5_Result;
}

