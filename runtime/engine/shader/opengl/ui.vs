//#version 330

#if defined GL_ES
precision highp float;
#endif
#pragma optimize(off)
#pragma debug(on)

// VS_UI

uniform sampler2D Flat_TextureSampler0;

attribute vec3 MaudeInput_Position;
attribute vec4 MaudeInput_Color0;
attribute vec2 MaudeInput_TexUV0;

varying vec4 MaudeIO_Color0;
varying vec2 MaudeIO_TexUV0;

void main() {
	// Create a screen aligned quad
    gl_Position = vec4((2.0 * MaudeInput_Position.x) - 1.0, (-2.0 * MaudeInput_Position.y) + 1.0, MaudeInput_Position.z, 1.0);
    MaudeIO_Color0 = MaudeInput_Color0;
    MaudeIO_TexUV0 = MaudeInput_TexUV0;
}
