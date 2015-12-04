const char *kernel = 
"__constant sampler_t LINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;\n"
"__constant sampler_t NEAREST = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;\n"
"\n"
"__kernel void remap(\n"
"__read_only image2d_t src,		// CL_UNSIGNED_INT8 x 4\n"
"__read_only image2d_t mapX,		// CL_FLOAT\n"
"__read_only image2d_t mapY,		// CL_FLOAT\n"
"__write_only image2d_t	dst)	// CL_UNSIGNED_INT8 x 4\n"
"{\n"
"	int x = get_global_id(0);\n"
"	int y = get_global_id(1);\n"
"	float X = read_imagef(mapX, (int2)(x, y)).x;\n"
"	float Y = read_imagef(mapY, (int2)(x, y)).x;\n"
"	uint4 pixel = read_imageui(src, LINEAR, (float2)(X, Y));\n"
"	write_imageui(dst, (int2)(x, y), pixel);\n"
"}\n"
"\n"
"__kernel void demosaic(\n"
"		__read_only image2d_t src,	// CL_UNSIGNED_INT16\n"
"		__write_only image2d_t left,	// CL_UNSIGNED_INT8 x 4\n"
"		__write_only image2d_t right)	// CL_UNSIGNED_INT8 x 4\n"
"{\n"
"	// calculate 2 x 2 pixels block\n"
"	int x = get_global_id(0) * 2;\n"
"	int y = get_global_id(1) * 2;\n"
"	uint patch[4][4];\n"
"	uint4 rgb[2][2];\n"
"	uint p[4][4];\n"
"\n"
"	// read data\n"
"	patch[0][0] = read_imageui(src, NEAREST, (int2)(x - 1, y - 1)).x;\n"
"	patch[0][1] = read_imageui(src, NEAREST, (int2)(x    , y - 1)).x;\n"
"	patch[0][2] = read_imageui(src, NEAREST, (int2)(x + 1, y - 1)).x;\n"
"	patch[0][3] = read_imageui(src, NEAREST, (int2)(x + 2, y - 1)).x;\n"
"\n"
"	patch[1][0] = read_imageui(src, NEAREST, (int2)(x - 1, y)).x;\n"
"	patch[1][1] = read_imageui(src, NEAREST, (int2)(x    , y)).x;\n"
"	patch[1][2] = read_imageui(src, NEAREST, (int2)(x + 1, y)).x;\n"
"	patch[1][3] = read_imageui(src, NEAREST, (int2)(x + 2, y)).x;\n"
"\n"
"	patch[2][0] = read_imageui(src, NEAREST, (int2)(x - 1, y + 1)).x;\n"
"	patch[2][1] = read_imageui(src, NEAREST, (int2)(x    , y + 1)).x;\n"
"	patch[2][2] = read_imageui(src, NEAREST, (int2)(x + 1, y + 1)).x;\n"
"	patch[2][3] = read_imageui(src, NEAREST, (int2)(x + 2, y + 1)).x;\n"
"	\n"
"	patch[3][0] = read_imageui(src, NEAREST, (int2)(x - 1, y + 2)).x;\n"
"	patch[3][1] = read_imageui(src, NEAREST, (int2)(x    , y + 2)).x;\n"
"	patch[3][2] = read_imageui(src, NEAREST, (int2)(x + 1, y + 2)).x;\n"
"	patch[3][3] = read_imageui(src, NEAREST, (int2)(x + 2, y + 2)).x;\n"
"\n"
"	p[0][0] = patch[0][0] & 0xff;\n"
"	p[0][1] = patch[0][1] & 0xff;\n"
"	p[0][2] = patch[0][2] & 0xff;\n"
"	p[0][3] = patch[0][3] & 0xff;\n"
"	p[1][0] = patch[1][0] & 0xff;\n"
"	p[1][1] = patch[1][1] & 0xff;\n"
"	p[1][2] = patch[1][2] & 0xff;\n"
"	p[1][3] = patch[1][3] & 0xff;\n"
"	p[2][0] = patch[2][0] & 0xff;\n"
"	p[2][1] = patch[2][1] & 0xff;\n"
"	p[2][2] = patch[2][2] & 0xff;\n"
"	p[2][3] = patch[2][3] & 0xff;\n"
"	p[3][0] = patch[3][0] & 0xff;\n"
"	p[3][1] = patch[3][1] & 0xff;\n"
"	p[3][2] = patch[3][2] & 0xff;\n"
"	p[3][3] = patch[3][3] & 0xff;\n"
"\n"
"	if (x == 0)\n"
"	{\n"
"		p[0][0] = p[0][2];\n"
"		p[1][0] = p[1][2];\n"
"		p[2][0] = p[2][2];\n"
"		p[3][0] = p[3][2];\n"
"	}\n"
"	else if (x == get_image_width(src) - 2)\n"
"	{\n"
"		p[0][3] = p[0][1];\n"
"		p[1][3] = p[1][1];\n"
"		p[2][3] = p[2][1];\n"
"		p[3][3] = p[3][1];\n"
"	}\n"
"\n"
"\n"
"	if (y == 0)\n"
"	{\n"
"		p[0][0] = p[2][0];\n"
"		p[0][1] = p[2][1];\n"
"		p[0][2] = p[2][2];\n"
"		p[0][3] = p[2][3];\n"
"	}\n"
"	else if (y == get_image_height(src) - 2)\n"
"	{\n"
"		p[3][0] = p[1][0];\n"
"		p[3][1] = p[1][1];\n"
"		p[3][2] = p[1][2];\n"
"		p[3][3] = p[1][3];\n"
"	}\n"
"\n"
"	rgb[0][0].x = ((p[0][1] + p[2][1]) >> 1); // B \n"
"	rgb[0][0].y = (p[1][1]); // G\n"
"	rgb[0][0].z = ((p[1][0] + p[1][2]) >> 1); // R\n"
"	rgb[0][0].w = 255;\n"
"\n"
"	rgb[0][1].x = (p[2][1]); // B\n"
"	rgb[0][1].y = ((p[2][0] + p[2][2] + p[1][1] + p[3][1]) >> 2); // G\n"
"	rgb[0][1].z = ((p[1][0] + p[3][0] + p[1][2] + p[3][2]) >> 2); // R\n"
"	rgb[0][1].w = 255;\n"
"\n"
"	rgb[1][0].x = ((p[0][1] + p[2][1] + p[0][3] + p[2][3]) >> 2); // B\n"
"	rgb[1][0].y = ((p[0][2] + p[2][2] + p[1][1] + p[1][3]) >> 2); // G\n"
"	rgb[1][0].z = (p[1][2]); // R\n"
"	rgb[1][0].w = 255;\n"
"					\n"
"	rgb[1][1].x = ((p[2][1] + p[2][3]) >> 1); // B \n"
"	rgb[1][1].y = (p[2][2]); // G\n"
"	rgb[1][1].z = ((p[1][2] + p[3][2]) >> 1); // R\n"
"	rgb[1][1].w = 255;\n"
"\n"
"	// write pixels\n"
"	write_imageui(left, (int2)(x, y), rgb[0][0]);\n"
"	write_imageui(left, (int2)(x, y + 1), rgb[0][1]);\n"
"	write_imageui(left, (int2)(x + 1, y), rgb[1][0]);\n"
"	write_imageui(left, (int2)(x + 1, y + 1), rgb[1][1]);\n"
"\n"
"	p[0][0] = patch[0][0] >> 8;\n"
"	p[0][1] = patch[0][1] >> 8;\n"
"	p[0][2] = patch[0][2] >> 8;\n"
"	p[0][3] = patch[0][3] >> 8;\n"
"	p[1][0] = patch[1][0] >> 8;\n"
"	p[1][1] = patch[1][1] >> 8;\n"
"	p[1][2] = patch[1][2] >> 8;\n"
"	p[1][3] = patch[1][3] >> 8;\n"
"	p[2][0] = patch[2][0] >> 8;\n"
"	p[2][1] = patch[2][1] >> 8;\n"
"	p[2][2] = patch[2][2] >> 8;\n"
"	p[2][3] = patch[2][3] >> 8;\n"
"	p[3][0] = patch[3][0] >> 8;\n"
"	p[3][1] = patch[3][1] >> 8;\n"
"	p[3][2] = patch[3][2] >> 8;\n"
"	p[3][3] = patch[3][3] >> 8;\n"
"\n"
"	if (x == 0)\n"
"	{\n"
"		p[0][0] = p[0][2];\n"
"		p[1][0] = p[1][2];\n"
"		p[2][0] = p[2][2];\n"
"		p[3][0] = p[3][2];\n"
"	}\n"
"	else if (x == get_image_width(src) - 2)\n"
"	{\n"
"		p[0][3] = p[0][1];\n"
"		p[1][3] = p[1][1];\n"
"		p[2][3] = p[2][1];\n"
"		p[3][3] = p[3][1];\n"
"	}\n"
"\n"
"	if (y == 0)\n"
"	{\n"
"		p[0][0] = p[2][0];\n"
"		p[0][1] = p[2][1];\n"
"		p[0][2] = p[2][2];\n"
"		p[0][3] = p[2][3];\n"
"	}\n"
"	else if (y == get_image_height(src) - 2)\n"
"	{\n"
"		p[3][0] = p[1][0];\n"
"		p[3][1] = p[1][1];\n"
"		p[3][2] = p[1][2];\n"
"		p[3][3] = p[1][3];\n"
"	}\n"
"\n"
"	rgb[0][0].x = ((p[0][1] + p[2][1]) >> 1); // B \n"
"	rgb[0][0].y = (p[1][1]); // G\n"
"	rgb[0][0].z = ((p[1][0] + p[1][2]) >> 1); // R\n"
"	rgb[0][0].w = 255;\n"
"\n"
"	rgb[0][1].x = (p[2][1]); // B\n"
"	rgb[0][1].y = ((p[2][0] + p[2][2] + p[1][1] + p[3][1]) >> 2); // G\n"
"	rgb[0][1].z = ((p[1][0] + p[3][0] + p[1][2] + p[3][2]) >> 2); // R\n"
"	rgb[0][1].w = 255;\n"
"\n"
"	rgb[1][0].x = ((p[0][1] + p[2][1] + p[0][3] + p[2][3]) >> 2); // B\n"
"	rgb[1][0].y = ((p[0][2] + p[2][2] + p[1][1] + p[1][3]) >> 2); // G\n"
"	rgb[1][0].z = (p[1][2]); // R\n"
"	rgb[1][0].w = 255;\n"
"					\n"
"	rgb[1][1].x = ((p[2][1] + p[2][3]) >> 1); // B \n"
"	rgb[1][1].y = (p[2][2]); // G\n"
"	rgb[1][1].z = ((p[1][2] + p[3][2]) >> 1); // R\n"
"	rgb[1][1].w = 255;\n"
"\n"
"	// write pixels\n"
"	write_imageui(right, (int2)(x, y), rgb[0][0]);\n"
"	write_imageui(right, (int2)(x, y + 1), rgb[0][1]);\n"
"	write_imageui(right, (int2)(x + 1, y), rgb[1][0]);\n"
"	write_imageui(right, (int2)(x + 1, y + 1), rgb[1][1]);\n"
"}\n"
"\n"
"__kernel void grayscale( // TODO UNDER CONSTRUCTION\n"
"		__read_only image2d_t src,	// CL_UNSIGNED_INT8 x 4\n"
"		__write_only image2d_t dst,	// CL_UNSIGNED_INT8 x 1\n"
"		__read_only int scale)	// 1, 2, 4, 8\n"
"{\n"
"}\n"
"\n"
"__kernel void skincolor( // TODO UNDER CONSTRUCTION\n"
"		__read_only image2d_t src,	// CL_UNSIGNED_INT8 x 4\n"
"		__write_only image2d_t dst,	// CL_UNSIGNED_INT8 x 4\n"
"		__read_only image2d_t map,	// CL_FLOAT\n"
"		__read_only int scale)		// 1, 2, 4, 8\n"
"{\n"
"}\n"
"\n"
"__kernel void resize( // TODO UNDER CONSTRUCTION\n"
"		__read_only image2d_t src,	// CL_UNSIGNED_INT8 x 4\n"
"		__write_only image2d_t dst,	// CL_UNSIGNED_INT8 x 4\n"
"		__read_only int scale)		// 2, 4, 8\n"
"{\n"
"	// calculate 2 x 2 pixels block\n"
"	int x = get_global_id(0);\n"
"	int y = get_global_id(1);\n"
"	uint4 pixel = read_imageui(src, NEAREST, (int2)(x * scale, y * scale));\n"
"	write_imageui(dst, (int2)(x, y), pixel);\n"
"}\n"
"\n"
"__kernel void convertHSV( // TODO UNDER CONSTRUCTION\n"
"		__read_only image2d_t src,	// CL_UNSIGNED_INT8 x 4\n"
"		__write_only image2d_t dst)	// CL_UNSIGNED_INT8 x 4\n"
"{\n"
"}\n"
"\n"
"__kernel void convertGrayscale( // TODO UNDER CONSTRUCTION\n"
"		__read_only image2d_t src,	// CL_UNSIGNED_INT8 x 4\n"
"		__write_only image2d_t dst)	// CL_UNSIGNED_INT8\n"
"{\n"
"}\n"
;