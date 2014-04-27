/*
   Hyllian's xBR v3.7c lq (squared) Shader
   
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

*/

precision mediump float;



/*
      Uniforms
      - rubyTexture: texture sampler
      - rubyTextureSize: size of the texture before rendering
    */
    
    uniform sampler2D rubyTexture;
    uniform vec2 rubyTextureSize;
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

    // Coefficient for weighted edge detection
    const float coef = 2.0;
    // Threshold for if luminance values are "equal"
    const float threshold = float(0.3125);

    // Conversion from RGB to Luminance (from ITU)
    const vec3 lum = vec3(0.299, 0.587, 0.114);

    // Gets the difference between 2 luminance vars
    float lum_df(float A, float B) {
      return abs(A - B);
    }

    // Determines if 2 4-value luminance vectors are "equal" based on threshold
    bool lum_eq(float A, float B) {
      return (lum_df(A, B) < threshold);
    }

    float lum_wd(float a, float b, float c, float d, float e, float f, float g, float h) {
      return lum_df(a, b) + lum_df(a, c) + lum_df(d, e) + lum_df(d, f) + 4.0 * lum_df(g, h);
    }


    void main() {
    
    
      /*
        Mask for algorhithm
        |B |C |
     |D |E |F |F4|
     |G |H |I |I4|
        |H5|I5|
      */

	float x = rubyTextureFract.x;
	float y = rubyTextureFract.y;

	vec4 t1 = vec4(0.0, -y, -x, 0.0);

      // Scale current texel coordinate to [0..1]
      vec2 pos = fract(tc * rubyTextureSize)-vec2(0.5, 0.5); // pos = pixel position
      vec2 dir = sign(pos); // dir = pixel direction

      bool fx = ( dot(dir,pos) > 0.5 );

      // Get mask values by performing texture lookup with the uniform sampler
      vec2 g1 = dir*t1.xy;
      vec2 g2 = dir*t1.zw;

      vec3 B = texture2D(rubyTexture, tc +g1   ).rgb;
      vec3 C = texture2D(rubyTexture, tc +g1-g2).rgb;
      vec3 D = texture2D(rubyTexture, tc    +g2).rgb;
      vec3 E = texture2D(rubyTexture, tc       ).rgb;
      vec3 F = texture2D(rubyTexture, tc    -g2).rgb;
      vec3 G = texture2D(rubyTexture, tc -g1+g2).rgb;
      vec3 H = texture2D(rubyTexture, tc -g1   ).rgb;
      vec3 I = texture2D(rubyTexture, tc -g1-g2).rgb;

      vec3 F4 = texture2D(rubyTexture,tc    -2.0*g2   ).rgb;
      vec3 I4 = texture2D(rubyTexture,tc -g1-2.0*g2   ).rgb;
      vec3 H5 = texture2D(rubyTexture,tc -2.0*g1      ).rgb;
      vec3 I5 = texture2D(rubyTexture,tc -2.0*g1-g2   ).rgb;

      float b = dot( B, lum );
      float c = dot( C, lum );
      float d = dot( D, lum );
      float e = dot( E, lum );
      float f = dot( F, lum );
      float g = dot( G, lum );
      float h = dot( H, lum );
      float i = dot( I, lum );

      float i4 = dot( I4, lum );
      float i5 = dot( I5, lum );
      float h5 = dot( H5, lum );
      float f4 = dot( F4, lum );


      // Perform edge weight calculations
      float e45   = lum_wd(e, c, g, i, h5, f4, h, f);
      float i45   = lum_wd(h, d, i5, f, i4, b, e, i);

      // Calculate rule results for interpolation
      bool r45_1   = e != f && e != h;
      bool r45_2   = !(lum_eq(f, b)) && !(lum_eq(f, c));
      bool r45_3   = !(lum_eq(h, d)) && !(lum_eq(h, g));
      bool r45_4_1 = !(lum_eq(f, f4)) && !(lum_eq(f, i4));
      bool r45_4_2 = !(lum_eq(h, h5)) && !(lum_eq(h, i5));
      bool r45_4   = lum_eq(e, i) && (r45_4_1 || r45_4_2);
      bool r45_5   = lum_eq(e, g) || lum_eq(e, c);
      bool r45     = r45_1 && (((r45_2 || r45_3) || r45_4) || r45_5);

      // Combine rules with edge weights
      bool edr45 = (e45 < i45) && r45;

      // Determine the color to mix with for each corner
      bool px = (lum_df(e, f) <= lum_df(e, h));

      bool nc = edr45 && fx;

      vec3 res = nc ? px ? F : H : E;

      gl_FragColor = vec4(res, 1.0);
    }
