/*
   Hyllian's 3xBR v3.8c (squared) Shader
   
   Copyright (C) 2011/2012 Hyllian/Jararaca - sergiogdb@gmail.com

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


   Incorporates some of the ideas from SABR shader. Thanks to Joshua Street.
*/

precision mediump float;



/*
      Uniforms
      - rubyTexture: texture sampler
      - rubyTextureSize: size of the texture before rendering
      - rubyInputSize: size of the actual game input screen before rendering (input resolution)
      - rubyOutputSize: size of the actual game output screen after rendering (output viewport)
    */
    
    uniform sampler2D rubyTexture;
    uniform vec2 rubyTextureSize;
    uniform vec2 rubyInputSize;
    uniform vec2 rubyOutputSize;
    uniform vec2 rubyTextureFract;

    /*
      Varying attributes
      - tc: coordinate of the texel being processed
      - xyp_[]_[]_[]: a packed coordinate for 3 areas within the texture
    */
    
    varying vec2 tc;

    /*
      Constants
    */
    /*
      Inequation coefficients for interpolation
        Equations are in the form: Ay + Bx = C
        45, 30, and 60 denote the angle from x each line the cooeficient variable set builds
    */
    const vec4 Ao  = vec4( 1.0, -1.0, -1.0,  1.0);
    const vec4 Bo  = vec4( 1.0,  1.0, -1.0, -1.0);
    const vec4 Co  = vec4( 1.5,  0.5, -0.5,  0.5);
    const vec4 Bx  = vec4( 0.5,  2.0, -0.5, -2.0);
    const vec4 Cx  = vec4( 1.0,  1.0, -0.5,  0.0);
    const vec4 By  = vec4( 2.0,  0.5, -2.0, -0.5);
    const vec4 Cy  = vec4( 2.0,  0.0, -1.0,  0.5);

    const vec4 delta  = vec4(0.4, 0.4, 0.4, 0.4);
    const vec4 deltaL = vec4(0.2, 0.4, 0.2, 0.4);
    const vec4 deltaU = vec4(0.4, 0.2, 0.4, 0.2);

    const vec4 zero = vec4(0.0, 0.0, 0.0, 0.0);
    const vec4 um   = vec4(1.0, 1.0, 1.0, 1.0);


    // Coefficient for weighted edge detection
    const float coef = 2.0;
    // Threshold for if luminance values are "equal"
    const vec4 threshold = vec4(0.3125);

    // Conversion from RGB to Luminance (from GIMP)
    const vec3 lum = vec3(0.299, 0.587, 0.114);

    // Performs same logic operation as && for vectors
    bvec4 _and_(bvec4 A, bvec4 B) {
      return bvec4(A.x && B.x, A.y && B.y, A.z && B.z, A.w && B.w);
    }

    // Performs same logic operation as || for vectors
    bvec4 _or_(bvec4 A, bvec4 B) {
      return bvec4(A.x || B.x, A.y || B.y, A.z || B.z, A.w || B.w);
    }

    // Converts 4 3-color vectors into 1 4-value luminance vector
    vec4 lum_to(vec3 v0, vec3 v1, vec3 v2, vec3 v3) {
      return vec4(dot(lum, v0), dot(lum, v1), dot(lum, v2), dot(lum, v3));
  
  //		return mat4(v0.x, v1.x, v2.x, v3.x, v0.y, v1.y, v2.y, v3.y, v0.z, v1.z, v2.z, v3.z, 0.0, 0.0, 0.0, 0.0) * vec4(lum, 0.0);
    }

    // Gets the difference between 2 4-value luminance vectors
    vec4 lum_df(vec4 A, vec4 B) {
      return abs(A - B);
    }

    // Determines if 2 4-value luminance vectors are "equal" based on threshold
    bvec4 lum_eq(vec4 A, vec4 B) {
      return lessThan(lum_df(A, B), threshold);
    }

    vec4 lum_wd(vec4 a, vec4 b, vec4 c, vec4 d, vec4 e, vec4 f, vec4 g, vec4 h) {
      return lum_df(a, b) + lum_df(a, c) + lum_df(d, e) + lum_df(d, f) + 4.0 * lum_df(g, h);
    }

    // Gets the difference between 2 3-value rgb colors
    float c_df(vec3 c1, vec3 c2) {
      vec3 df = abs(c1 - c2);
      return df.r + df.g + df.b;
    }

    void main() {
    
    
      /*
        Mask for algorhithm
	//    A1 B1 C1
	// A0  A  B  C C4
	// D0  D  E  F F4
	// G0  G  H  I I4
	//    G5 H5 I5
      */

	float x = rubyTextureFract.x;
	float y = rubyTextureFract.y;

	vec4 t1 = tc.xxxy + vec4(-x, 0.0, x, -2.0 * y);
	vec4 t2 = tc.xxxy + vec4(-x, 0.0, x, -y);
	vec4 t3 = tc.xxxy + vec4(-x, 0.0, x, 0.0);
	vec4 t4 = tc.xxxy + vec4(-x, 0.0, x, y);
	vec4 t5 = tc.xxxy + vec4(-x, 0.0, x, 2.0 * y);
	vec4 t6 = tc.xyyy + vec4(-2.0 * x, -y, 0.0, y);
	vec4 t7 = tc.xyyy + vec4(2.0 * x, -y, 0.0, y);

      // Get mask values by performing texture lookup with the uniform sampler
      vec3 A1  = texture2D(rubyTexture, t1.xw   ).rgb;
      vec3 B1  = texture2D(rubyTexture, t1.yw   ).rgb;
      vec3 C1  = texture2D(rubyTexture, t1.zw   ).rgb;

      vec3 A   = texture2D(rubyTexture, t2.xw   ).rgb;
      vec3 B   = texture2D(rubyTexture, t2.yw   ).rgb;
      vec3 C   = texture2D(rubyTexture, t2.zw   ).rgb;

      vec3 D   = texture2D(rubyTexture, t3.xw).rgb;
      vec3 E   = texture2D(rubyTexture, t3.yw).rgb;
      vec3 F   = texture2D(rubyTexture, t3.zw).rgb;

      vec3 G   = texture2D(rubyTexture, t4.xw).rgb;
      vec3 H   = texture2D(rubyTexture, t4.yw).rgb;
      vec3 I   = texture2D(rubyTexture, t4.zw).rgb;

      vec3 G5  = texture2D(rubyTexture, t5.xw).rgb;
      vec3 H5  = texture2D(rubyTexture, t5.yw).rgb;
      vec3 I5  = texture2D(rubyTexture, t5.zw).rgb;

      vec3 A0  = texture2D(rubyTexture, t6.xy ).rgb;
      vec3 D0  = texture2D(rubyTexture, t6.xz ).rgb;
      vec3 G0  = texture2D(rubyTexture, t6.xw ).rgb;

      vec3 C4  = texture2D(rubyTexture, t7.xy  ).rgb;
      vec3 F4  = texture2D(rubyTexture, t7.xz  ).rgb;
      vec3 I4  = texture2D(rubyTexture, t7.xw  ).rgb;

      // Store luminance values of each point in groups of 4
      // so that we may operate on all four corners at once
      vec4 b = lum_to(B, D, H, F);
      vec4 c = lum_to(C, A, G, I);
      vec4 e = lum_to(E, E, E, E);
      vec4 d = b.yzwx;
      vec4 f = b.wxyz;
      vec4 g = c.zwxy;
      vec4 h = b.zwxy;
      vec4 i = c.wxyz;

      vec4 i4 = lum_to(I4, C1, A0, G5);
      vec4 i5 = lum_to(I5, C4, A1, G0);
      vec4 h5 = lum_to(H5, F4, B1, D0);
      vec4 f4 = h5.yzwx;

      // Scale current texel coordinate to [0..1]
      vec2 fp = fract(tc * rubyTextureSize);

      // Determine amount of "smoothing" or mixing that could be done on texel corners
      vec4 AoMulFpy = Ao * fp.y;
      vec4 BoMulFpx = Bo * fp.x;

      vec2 delta = vec2(rubyInputSize.x/rubyOutputSize.x, 0.5*rubyInputSize.x/rubyOutputSize.x);

      vec4 fx45 = max(zero, min(um, (AoMulFpy + BoMulFpx  + delta.xxxx -Co)/(2.0*delta.xxxx)));
      vec4 fx30 = max(zero, min(um, (AoMulFpy + Bx * fp.x + delta.yxyx -Cx)/(2.0*delta.yxyx)));
      vec4 fx60 = max(zero, min(um, (AoMulFpy + By * fp.x + delta.xyxy -Cy)/(2.0*delta.xyxy)));

      // Perform edge weight calculations
      vec4 e45   = lum_wd(e, c, g, i, h5, f4, h, f);
      vec4 econt = lum_wd(h, d, i5, f, i4, b, e, i);
      vec4 e30   = lum_df(f, g);
      vec4 e60   = lum_df(h, c);

      // Calculate rule results for interpolation
      bvec4 r45_1   = _and_(notEqual(e, f), notEqual(e, h));
      bvec4 r45_2   = _and_(not(lum_eq(f, b)), not(lum_eq(f, c)));
      bvec4 r45_3   = _and_(not(lum_eq(h, d)), not(lum_eq(h, g)));
      bvec4 r45_4_1 = _and_(not(lum_eq(f, f4)), not(lum_eq(f, i4)));
      bvec4 r45_4_2 = _and_(not(lum_eq(h, h5)), not(lum_eq(h, i5)));
      bvec4 r45_4   = _and_(lum_eq(e, i), _or_(r45_4_1, r45_4_2));
      bvec4 r45_5   = _or_(lum_eq(e, g), lum_eq(e, c));
      bvec4 r45     = _and_(r45_1, _or_(_or_(_or_(r45_2, r45_3), r45_4), r45_5));
      bvec4 r30 = _and_(notEqual(e, g), notEqual(d, g));
      bvec4 r60 = _and_(notEqual(e, c), notEqual(b, c));

      // Combine rules with edge weights
      bvec4 edr45 = _and_(lessThan(e45, econt), r45);
      bvec4 edr30 = _and_(_and_(lessThanEqual(coef * e30, e60), r30), edr45);
      bvec4 edr60 = _and_(_and_(lessThanEqual(coef * e60, e30), r60), edr45);

      fx45 = vec4(edr45)*fx45;
      fx30 = vec4(edr30)*fx30;
      fx60 = vec4(edr60)*fx60;


      // Determine the color to mix with for each corner
      vec4 px = step(lum_df(e, f), lum_df(e, h));

      // Determine the mix amounts by combining the final rule result and corresponding
      // mix amount for the rule in each corner
      vec4 mac = max(max(fx30, fx60), fx45);

      /*
        Calculate the resulting color by traversing clockwise and counter-clockwise around
        the corners of the texel

        Finally choose the result that has the largest difference from the texel's original
        color
      */

      vec3 pix1 = mix(E, mix(H, F, px.x), mac.x);
      vec3 pix2 = mix(E, mix(F, B, px.y), mac.y);
      vec3 pix3 = mix(E, mix(B, D, px.z), mac.z);
      vec3 pix4 = mix(E, mix(D, H, px.w), mac.w);

      vec4 pixel = lum_to( pix1, pix2, pix3, pix4 );
      vec4 diff = lum_df(pixel,e);
      vec3 res = pix1;
      float mx = diff.x;
      float mx2 = diff.y;
      float mx3 = diff.z;

      if (diff.y > mx) {res = pix2;}
      if (diff.z > mx2) {res = pix3;}
      if (diff.w > mx3) {res = pix4;}

      gl_FragColor = vec4(res, 1.0);
    }
