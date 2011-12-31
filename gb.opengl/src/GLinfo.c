/***************************************************************************

  GLinfo.c

  (c) 2005-2007 Laurent Carlier <lordheavy@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __GLINFO_C

#include "GL.h"

int checkSize(GLenum value)
{
	int retSize = 0;
	
	if (value == GL_COMPRESSED_TEXTURE_FORMATS)
	{
		GLint size;
		glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &size);
		return size;
	}

	switch(value)
	{
		case GL_ACCUM_ALPHA_BITS:
		case GL_ACCUM_BLUE_BITS:
		case GL_ACCUM_GREEN_BITS:
		case GL_ACCUM_RED_BITS:
		case GL_ACTIVE_TEXTURE:
		case GL_ALPHA_BIAS:
		case GL_ALPHA_BITS:
		case GL_ALPHA_SCALE:
		case GL_ALPHA_TEST:
		case GL_ALPHA_TEST_FUNC:
		case GL_ALPHA_TEST_REF:
		case GL_ARRAY_BUFFER_BINDING:
		case GL_ATTRIB_STACK_DEPTH:
		case GL_AUTO_NORMAL:
		case GL_AUX_BUFFERS:
		case GL_BLEND:
		case GL_BLEND_DST:
		case GL_BLEND_DST_ALPHA:
		case GL_BLEND_DST_RGB:
		case GL_BLEND_EQUATION_ALPHA:
		case GL_BLEND_EQUATION_RGB:
		case GL_BLEND_SRC:
		case GL_BLEND_SRC_ALPHA:
		case GL_BLEND_SRC_RGB:
		case GL_BLUE_BIAS:
		case GL_BLUE_BITS:
		case GL_BLUE_SCALE:
		case GL_CLIENT_ACTIVE_TEXTURE:
		case GL_CLIENT_ATTRIB_STACK_DEPTH:
		case GL_CLIP_PLANE0:
		case GL_CLIP_PLANE1:
		case GL_CLIP_PLANE2:
		case GL_CLIP_PLANE3:
		case GL_CLIP_PLANE4:
		case GL_CLIP_PLANE5:
		case GL_COLOR_ARRAY:
		case GL_COLOR_ARRAY_BUFFER_BINDING:
		case GL_COLOR_ARRAY_SIZE:
		case GL_COLOR_ARRAY_STRIDE:
		case GL_COLOR_ARRAY_TYPE:
		case GL_COLOR_LOGIC_OP:
		case GL_COLOR_MATERIAL:
		case GL_COLOR_MATERIAL_FACE:
		case GL_COLOR_MATERIAL_PARAMETER:
		case GL_COLOR_MATRIX_STACK_DEPTH:
		case GL_COLOR_SUM:
		case GL_COLOR_TABLE:
		case GL_CONVOLUTION_1D:
		case GL_CONVOLUTION_2D:
		case GL_CULL_FACE:
		case GL_CULL_FACE_MODE:
		case GL_CURRENT_FOG_COORD:
		case GL_CURRENT_INDEX:
		case GL_CURRENT_PROGRAM:
		case GL_CURRENT_RASTER_DISTANCE:
		case GL_CURRENT_RASTER_INDEX:
		case GL_CURRENT_RASTER_POSITION_VALID:
		case GL_DEPTH_BIAS:
		case GL_DEPTH_BITS:
		case GL_DEPTH_CLEAR_VALUE:
		case GL_DEPTH_FUNC:
		case GL_DEPTH_SCALE:
		case GL_DEPTH_TEST:
		case GL_DEPTH_WRITEMASK:
		case GL_DITHER:
		case GL_DOUBLEBUFFER:
		case GL_DRAW_BUFFER:
		case GL_EDGE_FLAG:
		case GL_EDGE_FLAG_ARRAY:
		case GL_EDGE_FLAG_ARRAY_BUFFER_BINDING:
		case GL_EDGE_FLAG_ARRAY_STRIDE:
		case GL_ELEMENT_ARRAY_BUFFER_BINDING:
		case GL_FEEDBACK_BUFFER_SIZE:
		case GL_FEEDBACK_BUFFER_TYPE:
		case GL_FOG:
		case GL_FOG_COORD_ARRAY:
		case GL_FOG_COORD_ARRAY_BUFFER_BINDING:
		case GL_FOG_COORD_ARRAY_STRIDE:
		case GL_FOG_COORD_ARRAY_TYPE:
		case GL_FOG_COORD_SRC:
		case GL_FOG_DENSITY:
		case GL_FOG_END:
		case GL_FOG_HINT:
		case GL_FOG_INDEX:
		case GL_FOG_MODE:
		case GL_FOG_START:
		case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
		case GL_FRONT_FACE:
		case GL_GENERATE_MIPMAP_HINT:
		case GL_GREEN_BIAS:
		case GL_GREEN_BITS:
		case GL_GREEN_SCALE:
		case GL_HISTOGRAM:
		case GL_INDEX_ARRAY:
		case GL_INDEX_ARRAY_BUFFER_BINDING:
		case GL_INDEX_ARRAY_STRIDE:
		case GL_INDEX_ARRAY_TYPE:
		case GL_INDEX_BITS:
		case GL_INDEX_CLEAR_VALUE:
		case GL_INDEX_LOGIC_OP:
		case GL_INDEX_MODE:
		case GL_INDEX_OFFSET:
		case GL_INDEX_SHIFT:
		case GL_INDEX_WRITEMASK:
		case GL_LIGHT0:
		case GL_LIGHT1:
		case GL_LIGHT2:
		case GL_LIGHT3:
		case GL_LIGHT4:
		case GL_LIGHT5:
		case GL_LIGHT6:
		case GL_LIGHT7:
		case GL_LIGHTING:
		case GL_LIGHT_MODEL_COLOR_CONTROL:
		case GL_LIGHT_MODEL_LOCAL_VIEWER:
		case GL_LIGHT_MODEL_TWO_SIDE:
		case GL_LINE_SMOOTH:
		case GL_LINE_SMOOTH_HINT:
		case GL_LINE_STIPPLE:
		case GL_LINE_STIPPLE_PATTERN:
		case GL_LINE_STIPPLE_REPEAT:
		case GL_LINE_WIDTH:
		// GL_LINE_WIDTH_GRANULARITY deprecated see GL_SMOOTH_LINE_WIDTH_GRANULARITY
		case GL_LIST_BASE:
		case GL_LIST_INDEX:
		case GL_LIST_MODE:
		// GL_LOGIC_OP same as GL_LOGIC_OP_INDEX
		case GL_LOGIC_OP_MODE:
		case GL_MAP1_COLOR_4:
		case GL_MAP1_GRID_SEGMENTS:
		case GL_MAP1_INDEX:
		case GL_MAP1_NORMAL:
		case GL_MAP1_TEXTURE_COORD_1:
		case GL_MAP1_TEXTURE_COORD_2:
		case GL_MAP1_TEXTURE_COORD_3:
		case GL_MAP1_TEXTURE_COORD_4:
		case GL_MAP1_VERTEX_3:
		case GL_MAP1_VERTEX_4:
		case GL_MAP2_COLOR_4:
		case GL_MAP2_INDEX:
		case GL_MAP2_NORMAL:
		case GL_MAP2_TEXTURE_COORD_1:
		case GL_MAP2_TEXTURE_COORD_2:
		case GL_MAP2_TEXTURE_COORD_3:
		case GL_MAP2_TEXTURE_COORD_4:
		case GL_MAP2_VERTEX_3:
		case GL_MAP2_VERTEX_4:
		case GL_MAP_COLOR:
		case GL_MAP_STENCIL:
		case GL_MATRIX_MODE:
		case GL_MAX_3D_TEXTURE_SIZE:
		case GL_MAX_ATTRIB_STACK_DEPTH:
		case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
		case GL_MAX_CLIP_PLANES:
		case GL_MAX_COLOR_MATRIX_STACK_DEPTH:
		case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
		case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
		case GL_MAX_DRAW_BUFFERS:
		case GL_MAX_ELEMENTS_INDICES:
		case GL_MAX_ELEMENTS_VERTICES:
		case GL_MAX_EVAL_ORDER:
		case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
		case GL_MAX_LIGHTS:
		case GL_MAX_LIST_NESTING:
		case GL_MAX_MODELVIEW_STACK_DEPTH:
		case GL_MAX_NAME_STACK_DEPTH:
		case GL_MAX_PIXEL_MAP_TABLE:
		case GL_MAX_PROJECTION_STACK_DEPTH:
		case GL_MAX_TEXTURE_COORDS:
		case GL_MAX_TEXTURE_IMAGE_UNITS:
		case GL_MAX_TEXTURE_LOD_BIAS:
		case GL_MAX_TEXTURE_SIZE:
		case GL_MAX_TEXTURE_STACK_DEPTH:
		case GL_MAX_TEXTURE_UNITS:
		case GL_MAX_VARYING_FLOATS:
		case GL_MAX_VERTEX_ATTRIBS:
		case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
		case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
		case GL_MINMAX:
		case GL_MODELVIEW_STACK_DEPTH:
		case GL_NAME_STACK_DEPTH:
		case GL_NORMAL_ARRAY:
		case GL_NORMAL_ARRAY_BUFFER_BINDING:
		case GL_NORMAL_ARRAY_STRIDE:
		case GL_NORMAL_ARRAY_TYPE:
		case GL_NORMALIZE:
		case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
		case GL_PACK_ALIGNMENT:
		case GL_PACK_IMAGE_HEIGHT:
		case GL_PACK_LSB_FIRST:
		case GL_PACK_ROW_LENGTH:
		case GL_PACK_SKIP_IMAGES:
		case GL_PACK_SKIP_PIXELS:
		case GL_PACK_SKIP_ROWS:
		case GL_PACK_SWAP_BYTES:
		case GL_PERSPECTIVE_CORRECTION_HINT:
		case GL_PIXEL_MAP_A_TO_A_SIZE:
		case GL_PIXEL_MAP_B_TO_B_SIZE:
		case GL_PIXEL_MAP_G_TO_G_SIZE:
		case GL_PIXEL_MAP_I_TO_A_SIZE:
		case GL_PIXEL_MAP_I_TO_B_SIZE:
		case GL_PIXEL_MAP_I_TO_G_SIZE:
		case GL_PIXEL_MAP_I_TO_I_SIZE:
		case GL_PIXEL_MAP_I_TO_R_SIZE:
		case GL_PIXEL_MAP_R_TO_R_SIZE:
		case GL_PIXEL_MAP_S_TO_S_SIZE:
		case GL_PIXEL_PACK_BUFFER_BINDING:
		case GL_PIXEL_UNPACK_BUFFER_BINDING:
		case GL_POINT_FADE_THRESHOLD_SIZE:
		case GL_POINT_SIZE:
		// GL_POINT_SIZE_GRANULARITY deprecated see GL_SMOOTH_POINT_SIZE_GRANULARITY
		case GL_POINT_SIZE_MAX:
		case GL_POINT_SIZE_MIN:
		case GL_POINT_SMOOTH:
		case GL_POINT_SMOOTH_HINT:
		case GL_POINT_SPRITE:
		case GL_POLYGON_OFFSET_FACTOR:
		case GL_POLYGON_OFFSET_UNITS:
		case GL_POLYGON_OFFSET_FILL:
		case GL_POLYGON_OFFSET_LINE:
		case GL_POLYGON_OFFSET_POINT:
		case GL_POLYGON_SMOOTH:
		case GL_POLYGON_SMOOTH_HINT:
		case GL_POLYGON_STIPPLE:
		case GL_POST_COLOR_MATRIX_COLOR_TABLE:
		case GL_POST_COLOR_MATRIX_RED_BIAS:
		case GL_POST_COLOR_MATRIX_GREEN_BIAS:
		case GL_POST_COLOR_MATRIX_BLUE_BIAS:
		case GL_POST_COLOR_MATRIX_ALPHA_BIAS:
		case GL_POST_COLOR_MATRIX_RED_SCALE:
		case GL_POST_COLOR_MATRIX_GREEN_SCALE:
		case GL_POST_COLOR_MATRIX_BLUE_SCALE:
		case GL_POST_COLOR_MATRIX_ALPHA_SCALE:
		case GL_POST_CONVOLUTION_COLOR_TABLE:
		case GL_POST_CONVOLUTION_RED_BIAS:
		case GL_POST_CONVOLUTION_GREEN_BIAS:
		case GL_POST_CONVOLUTION_BLUE_BIAS:
		case GL_POST_CONVOLUTION_ALPHA_BIAS:
		case GL_POST_CONVOLUTION_RED_SCALE:
		case GL_POST_CONVOLUTION_GREEN_SCALE:
		case GL_POST_CONVOLUTION_BLUE_SCALE:
		case GL_POST_CONVOLUTION_ALPHA_SCALE:
		case GL_PROJECTION_STACK_DEPTH:
		case GL_READ_BUFFER:
		case GL_RED_BIAS:
		case GL_RED_BITS:
		case GL_RED_SCALE:
		case GL_RENDER_MODE:
		case GL_RESCALE_NORMAL:
		case GL_RGBA_MODE:
		case GL_SAMPLE_BUFFERS:
		case GL_SAMPLE_COVERAGE_VALUE:
		case GL_SAMPLE_COVERAGE_INVERT:
		case GL_SAMPLES:
		case GL_SCISSOR_TEST:
		case GL_SECONDARY_COLOR_ARRAY:
		case GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING:
		case GL_SECONDARY_COLOR_ARRAY_SIZE:
		case GL_SECONDARY_COLOR_ARRAY_STRIDE:
		case GL_SECONDARY_COLOR_ARRAY_TYPE:
		case GL_SELECTION_BUFFER_SIZE:
		case GL_SEPARABLE_2D:
		case GL_SMOOTH_LINE_WIDTH_GRANULARITY:
		case GL_SMOOTH_POINT_SIZE_GRANULARITY:
		case GL_SHADE_MODEL:
		case GL_STENCIL_BACK_FAIL:
		case GL_STENCIL_BACK_FUNC:
		case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
		case GL_STENCIL_BACK_PASS_DEPTH_PASS:
		case GL_STENCIL_BACK_REF:
		case GL_STENCIL_BACK_VALUE_MASK:
		case GL_STENCIL_BACK_WRITEMASK:
		case GL_STENCIL_BITS:
		case GL_STENCIL_CLEAR_VALUE:
		case GL_STENCIL_FAIL:
		case GL_STENCIL_FUNC:
		case GL_STENCIL_PASS_DEPTH_FAIL:
		case GL_STENCIL_PASS_DEPTH_PASS:
		case GL_STENCIL_REF:
		case GL_STENCIL_TEST:
		case GL_STENCIL_VALUE_MASK:
		case GL_STENCIL_WRITEMASK:
		case GL_STEREO:
		case GL_SUBPIXEL_BITS:
		case GL_TEXTURE_1D:
		case GL_TEXTURE_BINDING_1D:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_BINDING_2D:
		case GL_TEXTURE_3D:
		case GL_TEXTURE_BINDING_3D:
		case GL_TEXTURE_BINDING_CUBE_MAP:
		case GL_TEXTURE_COMPRESSION_HINT:
		case GL_TEXTURE_COORD_ARRAY:
		case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
		case GL_TEXTURE_COORD_ARRAY_SIZE:
		case GL_TEXTURE_COORD_ARRAY_STRIDE:
		case GL_TEXTURE_COORD_ARRAY_TYPE:
		case GL_TEXTURE_CUBE_MAP:
		case GL_TEXTURE_ENV_MODE:
		case GL_TEXTURE_GEN_S:
		case GL_TEXTURE_GEN_R:
		case GL_TEXTURE_GEN_T:
		case GL_TEXTURE_GEN_Q:
		case GL_TEXTURE_STACK_DEPTH:
		case GL_UNPACK_ALIGNMENT:
		case GL_UNPACK_IMAGE_HEIGHT:
		case GL_UNPACK_LSB_FIRST:
		case GL_UNPACK_ROW_LENGTH:
		case GL_UNPACK_SKIP_IMAGES:
		case GL_UNPACK_SKIP_PIXELS:
		case GL_UNPACK_SKIP_ROWS:
		case GL_UNPACK_SWAP_BYTES:
		case GL_VERTEX_ARRAY:
		case GL_VERTEX_ARRAY_BUFFER_BINDING:
		case GL_VERTEX_ARRAY_SIZE:
		case GL_VERTEX_ARRAY_STRIDE:
		case GL_VERTEX_ARRAY_TYPE:
		case GL_VERTEX_PROGRAM_POINT_SIZE:
		case GL_VERTEX_PROGRAM_TWO_SIDE:
		case GL_ZOOM_X:
		case GL_ZOOM_Y:
			retSize = 1;
			break;
		case GL_ALIASED_POINT_SIZE_RANGE:
		case GL_ALIASED_LINE_WIDTH_RANGE:
		case GL_DEPTH_RANGE:
		// GL_LINE_WIDTH_RANGE deprecated see GL_SMOOTH_LINE_WIDTH_RANGE
		case GL_MAP1_GRID_DOMAIN:
		case GL_MAP2_GRID_SEGMENTS:
		case GL_MAX_VIEWPORT_DIMS:
		// GL_POINT_SIZE_RANGE deprecated see GL_SMOOTH_POINT_SIZE_RANGE
		case GL_POLYGON_MODE:
		case GL_SMOOTH_LINE_WIDTH_RANGE:
		case GL_SMOOTH_POINT_SIZE_RANGE:
			retSize = 2;
			break;
		case GL_CURRENT_NORMAL:
		case GL_POINT_DISTANCE_ATTENUATION:
			retSize = 3;
			break;
		case GL_ACCUM_CLEAR_VALUE:
		case GL_BLEND_COLOR:
		case GL_COLOR_CLEAR_VALUE:
		case GL_COLOR_WRITEMASK:
		case GL_CURRENT_COLOR:
		case GL_CURRENT_RASTER_COLOR:
		case GL_CURRENT_RASTER_POSITION:
		case GL_CURRENT_RASTER_SECONDARY_COLOR:
		case GL_CURRENT_RASTER_TEXTURE_COORDS:
		case GL_CURRENT_SECONDARY_COLOR:
		case GL_CURRENT_TEXTURE_COORDS:
		case GL_FOG_COLOR:
		case GL_LIGHT_MODEL_AMBIENT:
		case GL_MAP2_GRID_DOMAIN:
		case GL_SCISSOR_BOX:
		case GL_TEXTURE_ENV_COLOR:
		case GL_VIEWPORT:
			retSize = 4;
			break;
		case GL_COLOR_MATRIX:
		case GL_MODELVIEW_MATRIX:
		case GL_PROJECTION_MATRIX:
		case GL_TEXTURE_MATRIX:
		case GL_TRANSPOSE_COLOR_MATRIX:
		case GL_TRANSPOSE_MODELVIEW_MATRIX:
		case GL_TRANSPOSE_PROJECTION_MATRIX:
		case GL_TRANSPOSE_TEXTURE_MATRIX:
			retSize = 16;
			break;
	}
	
	return retSize;
}

/**************************************************************************/

BEGIN_METHOD(GLGETBOOLEANV, GB_INTEGER parameter)

	GB_ARRAY bArray;
	int size = checkSize(VARG(parameter));

	if (!size)
	{
		GB.Error("Unknown parameter !");
		return;
	}

	int i;
	GLboolean boolArray[size];
	
	GB.Array.New(&bArray , GB_T_BOOLEAN , size);
	glGetBooleanv(VARG(parameter), boolArray);
	
	for (i=0;i<size; i++)
		*((int *)GB.Array.Get(bArray, i)) = boolArray[i];
	
	GB.ReturnObject(bArray);

END_METHOD

BEGIN_METHOD(GLGETFLOATV, GB_INTEGER parameter)

	GB_ARRAY fArray;
	int size = checkSize(VARG(parameter));

	if (!size)
	{
		GB.Error("Unknown parameter !");
		return;
	}

	int i;
	GLdouble floatArray[size];

	GB.Array.New(&fArray , GB_T_FLOAT , size);
	glGetDoublev(VARG(parameter), floatArray);
	
	for (i=0;i<size; i++)
		*((double *)GB.Array.Get(fArray, i)) = floatArray[i];
	
	GB.ReturnObject(fArray);

END_METHOD

BEGIN_METHOD(GLGETINTEGERV, GB_INTEGER parameter)

	GB_ARRAY iArray;
	int size = checkSize(VARG(parameter));

	if (!size)
	{
		GB.Error("Unknown parameter !");
		return;
	}

	int i;
	GLint intArray[size];

	GB.Array.New(&iArray , GB_T_INTEGER , size);
	glGetIntegerv(VARG(parameter), intArray);
	
	for (i=0;i<size; i++)
		*((int *)GB.Array.Get(iArray, i)) = intArray[i];
	
	GB.ReturnObject(iArray);

END_METHOD

BEGIN_METHOD(GLGETSTRING, GB_INTEGER name)

	const GLubyte *str = glGetString(VARG(name));
	
	if (!str)
		 return;
	 
	GB.ReturnNewZeroString((char *)str);
	 
END_METHOD
