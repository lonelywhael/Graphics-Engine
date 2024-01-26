/* SHADOW MAP FRAGMENT SHADER
 *
 * The shadow map shader completes the processing of shadow maps by applying a bias to front facing polygons to avoid shadow acne, peter
 * panning, and other shadow artifacts.
 */

#version 330 core

#define BIAS 0.05

void main() {
    gl_FragDepth = gl_FragCoord.z + (gl_FrontFacing ? BIAS : 0.0);
}