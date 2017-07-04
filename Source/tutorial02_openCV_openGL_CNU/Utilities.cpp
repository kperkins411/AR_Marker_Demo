#include "Utilities.h"
#include <iostream>


// Function turn a cv::Mat into a texture, and return the texture ID as a GLuint for use
// REMEMBER TO DELETE THIS TEXTUREID USING 	glDeleteTextures(1,&textureID);
// OR ELSE YOU GET A MEMORY LEAK
GLuint matToTexture(cv::Mat &mat, GLenum minFilter, GLenum magFilter, GLenum wrapFilter)
{
	// Generate a number for our textureID's unique handle
	GLuint textureID;
	glGenTextures(1, &textureID);

	// Bind to our texture handle
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Catch silly-mistake texture interpolation method for magnification
	if (magFilter == GL_LINEAR_MIPMAP_LINEAR  ||
		magFilter == GL_LINEAR_MIPMAP_NEAREST ||
		magFilter == GL_NEAREST_MIPMAP_LINEAR ||
		magFilter == GL_NEAREST_MIPMAP_NEAREST)
	{
		std::cout << "You can't use MIPMAPs for magnification - setting filter to GL_LINEAR" << std::endl;
		magFilter = GL_LINEAR;
	}

	// Set texture interpolation methods for minification and magnification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	// Set texture clamping method
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapFilter);

	// Set incoming texture format to:
	// GL_BGR       for CV_CAP_OPENNI_BGR_IMAGE,
	// GL_LUMINANCE for CV_CAP_OPENNI_DISPARITY_MAP,
	// Work out other mappings as required ( there's a list in comments in main() )
	GLenum inputColourFormat = GL_BGR;
	if (mat.channels() == 1)
	{
		inputColourFormat = GL_LUMINANCE;
	}

	// Create the texture
	glTexImage2D(GL_TEXTURE_2D,     // Type of texture
		0,                 // Pyramid level (for mip-mapping) - 0 is the top level
		GL_RGB,            // Internal colour format to convert to
		mat.cols,          // Image width  i.e. 640 for Kinect in standard mode
		mat.rows,          // Image height i.e. 480 for Kinect in standard mode
		0,                 // Border width in pixels (can either be 1 or 0)
		inputColourFormat, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
		GL_UNSIGNED_BYTE,  // Image data type
		mat.ptr());        // The actual image data itself

	// If we're using mipmaps then generate them. Note: This requires OpenGL 3.0 or higher
	if (minFilter == GL_LINEAR_MIPMAP_LINEAR  ||
		minFilter == GL_LINEAR_MIPMAP_NEAREST ||
		minFilter == GL_NEAREST_MIPMAP_LINEAR ||
		minFilter == GL_NEAREST_MIPMAP_NEAREST)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	return textureID;
}

const char* GL_type_to_string (GLenum type) {
	switch (type) {
	case GL_BOOL: return "bool";
	case GL_INT: return "int";
	case GL_FLOAT: return "float";
	case GL_FLOAT_VEC2: return "vec2";
	case GL_FLOAT_VEC3: return "vec3";
	case GL_FLOAT_VEC4: return "vec4";
	case GL_FLOAT_MAT2: return "mat2";
	case GL_FLOAT_MAT3: return "mat3";
	case GL_FLOAT_MAT4: return "mat4";
	case GL_SAMPLER_2D: return "sampler2D";
	case GL_SAMPLER_3D: return "sampler3D";
	case GL_SAMPLER_CUBE: return "samplerCube";
	case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
	default: break;
	}
	return "other";
}
void printGLVersionInfo(){
	// get version info
	printf ("--------------------\nOpenGL VersionInfo");
	const GLubyte* renderer = glGetString (GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString (GL_VERSION); // version as a string
	printf ("Renderer: %s\n", renderer);
	printf ("OpenGL version supported %s\n", version);
	printf ("\n");
}
void _print_programme_info_log (GLuint programme) {
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetProgramInfoLog (programme, max_length, &actual_length, log);
	printf ("program info log for GL index %u:\n%s", programme, log);
}
void print_all (GLuint programme) {
	printGLVersionInfo();
	printf ("--------------------\nshader programme %i info:\n", programme);
	int params = -1;
	glGetProgramiv (programme, GL_LINK_STATUS, &params);
	printf ("GL_LINK_STATUS = %i\n", params);

	glGetProgramiv (programme, GL_ATTACHED_SHADERS, &params);
	printf ("GL_ATTACHED_SHADERS = %i\n", params);

	glGetProgramiv (programme, GL_ACTIVE_ATTRIBUTES, &params);
	printf ("GL_ACTIVE_ATTRIBUTES = %i\n", params);
	for (int i = 0; i < params; i++) {
		char name[64];
		int max_length = 64;
		int actual_length = 0;
		int size = 0;
		GLenum type;
		glGetActiveAttrib (
			programme,
			i,
			max_length,
			&actual_length,
			&size,
			&type,
			name
			);
		if (size > 1) {
			for (int j = 0; j < size; j++) {
				char long_name[64];
				sprintf (long_name, "%s[%i]", name, j);
				int location = glGetAttribLocation (programme, long_name);
				printf ("  %i) type:%s name:%s location:%i\n",
					i, GL_type_to_string (type), long_name, location);
			}
		} else {
			int location = glGetAttribLocation (programme, name);
			printf ("  %i) type:%s name:%s location:%i\n",
				i, GL_type_to_string (type), name, location);
		}
	}

	glGetProgramiv (programme, GL_ACTIVE_UNIFORMS, &params);
	printf ("GL_ACTIVE_UNIFORMS = %i\n", params);
	for (int i = 0; i < params; i++) {
		char name[64];
		int max_length = 64;
		int actual_length = 0;
		int size = 0;
		GLenum type;
		glGetActiveUniform (
			programme,
			i,
			max_length,
			&actual_length,
			&size,
			&type,
			name
			);
		if (size > 1) {
			for (int j = 0; j < size; j++) {
				char long_name[64];
				sprintf (long_name, "%s[%i]", name, j);
				int location = glGetUniformLocation (programme, long_name);
				printf ("  %i) type:%s name:%s location:%i\n",
					i, GL_type_to_string (type), long_name, location);
			}
		} else {
			int location = glGetUniformLocation (programme, name);
			printf ("  %i) type:%s name:%s location:%i\n",
				i, GL_type_to_string (type), name, location);
		}
	}

	_print_programme_info_log (programme);
}