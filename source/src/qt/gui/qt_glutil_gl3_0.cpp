/*
 * qt_glutil_gl3_0.cpp
 * (c) 2016 K.Ohta <whatisthis.sowhat@gmail.com>
 * License: GPLv2.
 * Renderer with OpenGL v3.0 (extend from renderer with OpenGL v2.0).
 * History:
 * Jan 22, 2016 : Initial.
 */

#include "qt_gldraw.h"
#include "qt_glpack.h"
#include "qt_glutil_gl3_0.h"
#include "menu_flags.h"

//extern USING_FLAGS *using_flags;

GLDraw_3_0::GLDraw_3_0(GLDrawClass *parent, USING_FLAGS *p, EMU *emu) : GLDraw_2_0(parent, p, emu)
{
	uTmpTextureID = 0;
	
	grids_shader = NULL;
	
	main_pass = NULL;
	std_pass = NULL;
	ntsc_pass1 = NULL;
	ntsc_pass2 = NULL;
	
	grids_horizonal_buffer = NULL;
	grids_horizonal_vertex = NULL;
	
	grids_vertical_buffer = NULL;
	grids_vertical_vertex = NULL;

}

GLDraw_3_0::~GLDraw_3_0()
{

	if(main_pass  != NULL) delete main_pass;
	if(std_pass   != NULL) delete std_pass;
	if(ntsc_pass1 != NULL) delete ntsc_pass1;
	if(ntsc_pass2 != NULL) delete ntsc_pass2;
	
	if(grids_horizonal_buffer != NULL) {
		if(grids_horizonal_buffer->isCreated()) grids_horizonal_buffer->destroy();
	}
	if(grids_horizonal_vertex != NULL) {
		if(grids_horizonal_vertex->isCreated()) grids_horizonal_vertex->destroy();
	}
	if(grids_vertical_buffer != NULL) {
		if(grids_vertical_buffer->isCreated()) grids_vertical_buffer->destroy();
	}
	if(grids_horizonal_vertex != NULL) {
		if(grids_vertical_vertex->isCreated()) grids_vertical_vertex->destroy();
	}

}

void GLDraw_3_0::setNormalVAO(QOpenGLShaderProgram *prg,
							   QOpenGLVertexArrayObject *vp,
							   QOpenGLBuffer *bp,
							   VertexTexCoord_t *tp,
							   int size)
{
	int vertex_loc = prg->attributeLocation("vertex");
	int texcoord_loc = prg->attributeLocation("texcoord");

	vp->bind();
	bp->bind();

	bp->write(0, tp, sizeof(VertexTexCoord_t) * size);
	prg->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(VertexTexCoord_t));
	prg->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3 * sizeof(GLfloat), 2, sizeof(VertexTexCoord_t));
	prg->setUniformValue("a_texture", 0);
			   
	extfunc->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord_t), 0); 
	extfunc->glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord_t), 
							       (char *)NULL + 3 * sizeof(GLfloat)); 
	bp->release();
	vp->release();
	prg->enableAttributeArray(vertex_loc);
	prg->enableAttributeArray(texcoord_loc);
}

void GLDraw_3_0::initGLObjects()
{
	GLDraw_2_0::initGLObjects();
	extfunc = new QOpenGLFunctions_3_0;
	extfunc->initializeOpenGLFunctions();
	extfunc->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texture_max_size);
}	

void GLDraw_3_0::initLocalGLObjects(void)
{

	int _width = using_flags->get_screen_width();
	int _height = using_flags->get_screen_height();
	if((_width * 4) <= texture_max_size) {
		_width = _width * 4;
		low_resolution_screen = true;
	} else {
		_width = _width * 2;
	}
	p_wid->makeCurrent();
	
	vertexFormat[0].x = -0.5f;
	vertexFormat[0].y = -0.5f;
	vertexFormat[0].z = -0.9f;
	vertexFormat[0].s = 0.0f;
	vertexFormat[0].t = 1.0f;
	
	vertexFormat[1].x = +0.5f;
	vertexFormat[1].y = -0.5f;
	vertexFormat[1].z = -0.9f;
	vertexFormat[1].s = 1.0f;
	vertexFormat[1].t = 1.0f;
	
	vertexFormat[2].x = +0.5f;
	vertexFormat[2].y = +0.5f;
	vertexFormat[2].z = -0.9f;
	vertexFormat[2].s = 1.0f;
	vertexFormat[2].t = 0.0f;
	
	vertexFormat[3].x = -0.5f;
	vertexFormat[3].y = +0.5f;
	vertexFormat[3].z = -0.9f;
	vertexFormat[3].s = 0.0f;
	vertexFormat[3].t = 0.0f;

	main_pass = new GLScreenPack(_width, _height * 2, p_wid);
	main_pass->initialize(_width, _height * 2, ":/vertex_shader.glsl", ":/fragment_shader.glsl");

	std_pass = new GLScreenPack(_width, _height * 2, p_wid);
	std_pass->initialize(_width, _height * 2,":/tmp_vertex_shader.glsl", ":/chromakey_fragment_shader.glsl");

	ntsc_pass1 = new GLScreenPack(_width, _height * 2, p_wid);
	ntsc_pass1->initialize(_width, _height * 2,":/tmp_vertex_shader.glsl", ":/ntsc_pass1.glsl");

	ntsc_pass2 = new GLScreenPack(_width, _height * 2, p_wid);
	ntsc_pass2->initialize(_width, _height * 2,":/tmp_vertex_shader.glsl", ":/ntsc_pass2.glsl");
	
	grids_shader = new QOpenGLShaderProgram(p_wid);
	if(using_flags->is_use_screen_rotate()) {
		if(grids_shader != NULL) {
			grids_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/grids_vertex_shader.glsl");
			grids_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/grids_fragment_shader.glsl");
			grids_shader->link();
		}
	} else {
		if(grids_shader != NULL) {
			grids_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/grids_vertex_shader_fixed.glsl");
			grids_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/grids_fragment_shader.glsl");
			grids_shader->link();
		}
	}
	grids_horizonal_buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	grids_horizonal_vertex = new QOpenGLVertexArrayObject;
	grids_horizonal_vertex->create();
	updateGridsVAO(grids_horizonal_buffer, grids_horizonal_vertex,
				   glHorizGrids, using_flags->get_screen_height() + 2);
	
	grids_vertical_buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	grids_vertical_vertex = new QOpenGLVertexArrayObject;
	grids_vertical_vertex->create();
	updateGridsVAO(grids_vertical_buffer, grids_vertical_vertex,
				   glVertGrids, using_flags->get_screen_width() + 2);

			
	p_wid->doneCurrent();
}

void GLDraw_3_0::updateGridsVAO(QOpenGLBuffer *bp,
								QOpenGLVertexArrayObject *vp,
								GLfloat *tp,
								int number)

{
	bool checkf = false;
	if(bp != NULL) {
		if(bp->isCreated()) {
			if(bp->size() != (number * sizeof(GLfloat) * 3 * 2)) {
				bp->destroy();
				bp->create();
				checkf = true;
			}
		} else {
			bp->create();
			checkf = true;
		}
		if(checkf) {
			bp->bind();
			bp->allocate((number + 1) * sizeof(GLfloat) * 3 * 2);
			if(tp != NULL) {
				bp->write(0, tp, (number + 1) * sizeof(GLfloat) * 3 * 2);
			}
			bp->release();
		}
	}
}
void GLDraw_3_0::drawGridsMain_3(QOpenGLShaderProgram *prg,
								 QOpenGLBuffer *bp,
								 QOpenGLVertexArrayObject *vp,
								 int number,
								 GLfloat lineWidth,
								 QVector4D color)
{
	if(number <= 0) return;
	extfunc->glDisable(GL_TEXTURE_2D);
	extfunc->glDisable(GL_DEPTH_TEST);
	extfunc->glDisable(GL_BLEND);

	if((bp == NULL) || (vp == NULL) || (prg == NULL)) return;
	if((!bp->isCreated()) || (!vp->isCreated()) || (!prg->isLinked())) return;
	{
		int i, p;
		bp->bind();
		vp->bind();
		prg->bind();
		
		prg->setUniformValue("color", color);
		prg->enableAttributeArray("vertex");
		int vertex_loc = prg->attributeLocation("vertex");
		extfunc->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0); 
		extfunc->glEnableVertexAttribArray(vertex_loc);
		
		extfunc->glEnableClientState(GL_VERTEX_ARRAY);
		extfunc->glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		extfunc->glLineWidth(lineWidth);
		extfunc->glVertexPointer(3, GL_FLOAT, 0, 0);
		extfunc->glDrawArrays(GL_LINES, 0, (number + 1) * 2);
		extfunc->glDisableClientState(GL_VERTEX_ARRAY);
		prg->release();
		vp->release();
		bp->release();
	}
}

void GLDraw_3_0::drawGridsHorizonal(void)
{
	QVector4D c= QVector4D(0.0f, 0.0f, 0.0f, 1.0f);
	updateGridsVAO(grids_horizonal_buffer,
				   grids_horizonal_vertex,
				   glHorizGrids,
				   vert_lines);
	drawGridsMain_3(grids_shader,
					grids_horizonal_buffer,
					grids_horizonal_vertex,
					vert_lines,
					0.15f,
					c);
}

void GLDraw_3_0::drawGridsVertical(void)
{
	QVector4D c= QVector4D(0.0f, 0.0f, 0.0f, 1.0f);
	updateGridsVAO(grids_vertical_buffer,
				   grids_vertical_vertex,
				   glVertGrids,
				   horiz_pixels);
	drawGridsMain_3(grids_shader,
					grids_vertical_buffer,
					grids_vertical_vertex,
					horiz_pixels,
					0.5f,
					c);
}

void GLDraw_3_0::renderToTmpFrameBuffer_nPass(GLuint src_texture,
											  GLuint src_w,
											  GLuint src_h,
											  GLScreenPack *renderObject,
											  GLuint dst_w,
											  GLuint dst_h,
											  bool use_chromakey)
{
#if 0
	// OLD_THREE_PHASE
	const float luma_filter[24 + 1] = {
		-0.000071070,
		-0.000032816,
		0.000128784,
		0.000134711,
		-0.000226705,
		-0.000777988,
		-0.000997809,
		-0.000522802,
		0.000344691,
		0.000768930,
		0.000275591,
		-0.000373434,
		0.000522796,
		0.003813817,
		0.007502825,
		0.006786001,
		-0.002636726,
		-0.019461182,
		-0.033792479,
		-0.029921972,
		0.005032552,
		0.071226466,
		0.151755921,
		0.218166470,
		0.243902439
	};
	const float chroma_filter[24 + 1] = {
		0.001845562,
		0.002381606,
		0.003040177,
		0.003838976,
		0.004795341,
		0.005925312,
		0.007242534,
		0.008757043,
		0.010473987,
		0.012392365,
		0.014503872,
		0.016791957,
		0.019231195,
		0.021787070,
		0.024416251,
		0.027067414,
		0.029682613,
		0.032199202,
		0.034552198,
		0.036677005,
		0.038512317,
		0.040003044,
		0.041103048,
		0.041777517,
		0.042004791
	};
#else
#if 1
	// THREE_PHASE
	const float luma_filter[24 + 1] = {
		-0.000012020,
		-0.000022146,
		-0.000013155,
		-0.000012020,
		-0.000049979,
		-0.000113940,
		-0.000122150,
		-0.000005612,
		0.000170516,
		0.000237199,
		0.000169640,
		0.000285688,
		0.000984574,
		0.002018683,
		0.002002275,
		-0.000909882,
		-0.007049081,
		-0.013222860,
		-0.012606931,
		0.002460860,
		0.035868225,
		0.084016453,
		0.135563500,
		0.175261268,
		0.190176552
	};
	const float chroma_filter[24 + 1] = {
		-0.000118847,
		-0.000271306,
		-0.000502642,
		-0.000930833,
		-0.001451013,
		-0.002064744,
		-0.002700432,
		-0.003241276,
		-0.003524948,
		-0.003350284,
		-0.002491729,
		-0.000721149,
		0.002164659,
		0.006313635,
		0.011789103,
		0.018545660,
		0.026414396,
		0.035100710,
		0.044196567,
		0.053207202,
		0.061590275,
		0.068803602,
		0.074356193,
		0.077856564,
		0.079052396
	};
// END "ntsc-decode-filter-3phase.inc" //
#else
				// TWO_PHASE
	const float luma_filter[24 + 1] = {
		-0.000012020,
		-0.000022146,
		-0.000013155,
		-0.000012020,
		-0.000049979,
		-0.000113940,
		-0.000122150,
		-0.000005612,
		0.000170516,
		0.000237199,
		0.000169640,
		0.000285688,
		0.000984574,
		0.002018683,
		0.002002275,
		-0.000909882,
		-0.007049081,
		-0.013222860,
		-0.012606931,
		0.002460860,
		0.035868225,
		0.084016453,
		0.135563500,
		0.175261268,
		0.190176552
	};
	
	const float chroma_filter[24 + 1] = {
		-0.000118847,
		-0.000271306,
		-0.000502642,
		-0.000930833,
		-0.001451013,
		-0.002064744,
		-0.002700432,
		-0.003241276,
		-0.003524948,
		-0.003350284,
		-0.002491729,
		-0.000721149,
		0.002164659,
		0.006313635,
		0.011789103,
		0.018545660,
		0.026414396,
		0.035100710,
		0.044196567,
		0.053207202,
		0.061590275,
		0.068803602,
		0.074356193,
		0.077856564,
		0.079052396
	};
// END "ntsc-decode-filter-3phase.inc" //
#endif
#endif
	{
		QOpenGLShaderProgram *shader = renderObject->getShader();
		int ii;
		// NTSC_PASSx

		extfunc->glClearColor(0.0, 0.0, 0.0, 1.0);
		extfunc->glClearDepth(1.0f);
		extfunc->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		{
			if((src_texture != 0) && (shader != NULL)) {
				extfunc->glEnable(GL_TEXTURE_2D);
				renderObject->bind();
				//extfunc->glViewport(0, 0, src_w, src_h);
				extfunc->glViewport(0, 0, dst_w, dst_h);
				extfunc->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
				extfunc->glActiveTexture(GL_TEXTURE0);
				extfunc->glBindTexture(GL_TEXTURE_2D, src_texture);
				extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				//extfunc->glColor4f(1.0, 1.0, 1.0, 1.0);
				shader->setUniformValue("a_texture", 0);
				//shader->setUniformValue("a_texture", src_texture);
				{
					ii = shader->uniformLocation("source_size");
					if(ii >= 0) {
						QVector4D source_size = QVector4D((float)src_w, (float)src_h, 0, 0);
						shader->setUniformValue(ii, source_size);
					}
					ii = shader->uniformLocation("target_size");
					if(ii >= 0) {
						QVector4D target_size = QVector4D((float)dst_w, (float)dst_h, 0, 0);
						shader->setUniformValue(ii, target_size);
					}
					ii = shader->uniformLocation("phase");
					if(ii >= 0) {
						float phase = 0.1;
						shader->setUniformValue(ii,  phase);
					}
					ii = shader->uniformLocation("luma_filter");
					if(ii >= 0) {
						shader->setUniformValueArray(ii, luma_filter, 24 + 1, 1);
					}
					ii = shader->uniformLocation("chroma_filter");
					if(ii >= 0) {
						shader->setUniformValueArray(ii, chroma_filter, 24 + 1, 1);
					}
				}
				{
					QVector4D c(fBrightR, fBrightG, fBrightB, 1.0);
					QVector3D chromakey(0.0, 0.0, 0.0);
		
					ii = shader->uniformLocation("color");
					if(ii >= 0) {
						shader->setUniformValue(ii, c);
					}
					ii = shader->uniformLocation("do_chromakey");
					if(ii >= 0) {
						if(use_chromakey) {
							int ij;
							ij = shader->uniformLocation("chromakey");
							if(ij >= 0) {
								shader->setUniformValue(ij, chromakey);
							}
							shader->setUniformValue(ii, GL_TRUE);
						} else {
							shader->setUniformValue(ii, GL_FALSE);
						}
					}
					//shader->setUniformValue("tex_width",  (float)w); 
					//shader->setUniformValue("tex_height", (float)h);
				}
				shader->enableAttributeArray("texcoord");
				shader->enableAttributeArray("vertex");
				
				int vertex_loc = shader->attributeLocation("vertex");
				int texcoord_loc = shader->attributeLocation("texcoord");
				extfunc->glEnableVertexAttribArray(vertex_loc);
				extfunc->glEnableVertexAttribArray(texcoord_loc);
				extfunc->glEnable(GL_VERTEX_ARRAY);

				extfunc->glDrawArrays(GL_POLYGON, 0, 4);
				
				extfunc->glViewport(0, 0, dst_w, dst_h);
				extfunc->glOrtho(0.0f, (float)dst_w, 0.0f, (float)dst_h, -1.0, 1.0);
				renderObject->release();
				extfunc->glBindTexture(GL_TEXTURE_2D, 0);
				extfunc->glDisable(GL_TEXTURE_2D);
			}
		}
	}
}


void GLDraw_3_0::uploadMainTexture(QImage *p, bool use_chromakey)
{
	// set vertex
	redraw_required = true;
	if(p == NULL) return;
	//redraw_required = true;
	imgptr = p;
	if(uVramTextureID == 0) {
		uVramTextureID = p_wid->bindTexture(*p);
	}
	{
		// Upload to main texture
		extfunc->glBindTexture(GL_TEXTURE_2D, uVramTextureID);
		extfunc->glTexSubImage2D(GL_TEXTURE_2D, 0,
							 0, 0,
							 p->width(), p->height(),
							 GL_BGRA, GL_UNSIGNED_BYTE, p->constBits());
		extfunc->glBindTexture(GL_TEXTURE_2D, 0);
	}
	if(using_flags->is_support_tv_render() && (using_flags->get_config_ptr()->rendering_type == CONFIG_RENDER_TYPE_TV)) {
		renderToTmpFrameBuffer_nPass(uVramTextureID,
									 (low_resolution_screen) ? (screen_texture_width * 4) : (screen_texture_width * 2),
									 screen_texture_height,
									 ntsc_pass1,
									 (low_resolution_screen) ? (screen_texture_width * 4) : (screen_texture_width * 2),
									 screen_texture_height * 2);
		renderToTmpFrameBuffer_nPass(ntsc_pass1->getTexture(),
									 (low_resolution_screen) ? (screen_texture_width * 4) : (screen_texture_width * 2),
									 screen_texture_height,
									 ntsc_pass2,
									 (low_resolution_screen) ? (screen_texture_width * 4) : (screen_texture_width * 2),
									 screen_texture_height * 2);
		uTmpTextureID = ntsc_pass2->getTexture();
	} else {
		renderToTmpFrameBuffer_nPass(uVramTextureID,
									 (low_resolution_screen) ? (screen_texture_width * 4) : (screen_texture_width * 2),
									 screen_texture_height * 2,
									 std_pass,
									 (low_resolution_screen) ? (screen_texture_width * 4) : (screen_texture_width * 2),
									 screen_texture_height * 2);

		uTmpTextureID = std_pass->getTexture();
	}
	crt_flag = true;
}

void GLDraw_3_0::drawScreenTexture(void)
{
	if(using_flags->is_use_one_board_computer()) {
		if(uBitmapTextureID != 0) {
			extfunc->glEnable(GL_BLEND);
			extfunc->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	} else {
		extfunc->glDisable(GL_BLEND);
	}
	
	QVector4D color;
	smoosing = using_flags->get_config_ptr()->use_opengl_filters;
	if(set_brightness) {
		color = QVector4D(fBrightR, fBrightG, fBrightB, 1.0);
	} else {
		color = QVector4D(1.0, 1.0, 1.0, 1.0);
	}			
	main_pass->getShader()->setUniformValue("color", color);
	drawMain(main_pass->getShader(), main_pass->getVAO(),
			 main_pass->getVertexBuffer(),
			 vertexFormat,
			 uTmpTextureID, // v2.0
			 color, smoosing);
	if(using_flags->is_use_one_board_computer()) {
		extfunc->glDisable(GL_BLEND);
	}
}


void GLDraw_3_0::drawMain(QOpenGLShaderProgram *prg,
						  QOpenGLVertexArrayObject *vp,
						  QOpenGLBuffer *bp,
						  VertexTexCoord_t *vertex_data,
						  GLuint texid,
						  QVector4D color,
						  bool f_smoosing,
						  bool do_chromakey,
						  QVector3D chromakey)
						   
{
	if(texid != 0) {
		extfunc->glEnable(GL_TEXTURE_2D);
		vp->bind();
		bp->bind();
		prg->bind();
		extfunc->glViewport(0, 0, p_wid->width(), p_wid->height());
		extfunc->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
		extfunc->glActiveTexture(GL_TEXTURE0);
		extfunc->glBindTexture(GL_TEXTURE_2D, texid);
		if(!f_smoosing) {
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		} else {
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		prg->setUniformValue("a_texture", 0);
		prg->setUniformValue("color", color);
		prg->setUniformValue("tex_width",  (float)screen_texture_width); 
		prg->setUniformValue("tex_height", (float)screen_texture_height);
		
		if(using_flags->is_use_screen_rotate()) {
			if(using_flags->get_config_ptr()->rotate_type) {
				prg->setUniformValue("rotate", GL_TRUE);
			} else {
				prg->setUniformValue("rotate", GL_FALSE);
			}
		} else {
			prg->setUniformValue("rotate", GL_FALSE);
		}
		if(do_chromakey) {
			prg->setUniformValue("chromakey", chromakey);
			prg->setUniformValue("do_chromakey", GL_TRUE);
		} else {
			prg->setUniformValue("do_chromakey", GL_FALSE);
		}			
		prg->enableAttributeArray("texcoord");
		prg->enableAttributeArray("vertex");
		int vertex_loc = prg->attributeLocation("vertex");
		int texcoord_loc = prg->attributeLocation("texcoord");
		extfunc->glEnableVertexAttribArray(vertex_loc);
		extfunc->glEnableVertexAttribArray(texcoord_loc);
		extfunc->glEnable(GL_VERTEX_ARRAY);
		extfunc->glDrawArrays(GL_POLYGON, 0, 4);
		bp->release();
		vp->release();
		
		prg->release();
		extfunc->glBindTexture(GL_TEXTURE_2D, 0);
		extfunc->glDisable(GL_TEXTURE_2D);
	}
}


void GLDraw_3_0::setBrightness(GLfloat r, GLfloat g, GLfloat b)
{
	fBrightR = r;
	fBrightG = g;
	fBrightB = b;

	if(imgptr != NULL) {
		p_wid->makeCurrent();
		if(uVramTextureID != 0) {
			uVramTextureID = p_wid->bindTexture(*imgptr);
		}
		if(using_flags->is_use_one_board_computer() || (using_flags->get_max_button() > 0)) {
			uploadMainTexture(imgptr, true);
		} else {
			uploadMainTexture(imgptr, false);
		}
		crt_flag = true;
		p_wid->doneCurrent();
	}
}

void GLDraw_3_0::set_texture_vertex(QImage *p, int w_wid, int h_wid, int w, int h, float wmul, float hmul)
{
	int ww = (int)(w * wmul);
	int hh = (int)(h * hmul);
	float wfactor = 1.0f;
	float hfactor = 1.0f;
	float iw, ih;
	
	//if(screen_multiply < 1.0f) {
	if(p != NULL) {
		iw = (float)p->width();
		ih = (float)p->height();
	} else {
		iw = (float)using_flags->get_screen_width();
		ih = (float)using_flags->get_screen_height();
	}
#if 0
	if((ww > w_wid) || (hh > h_wid)) {
		ww = (int)(screen_multiply * (float)w * wmul);
		hh = (int)(screen_multiply * (float)h * hmul);
		wfactor = screen_multiply * 2.0f - 1.0f;
		hfactor = -screen_multiply * 2.0f + 1.0f;
	}
#endif
	vertexTmpTexture[0].x = -1.0f;
	vertexTmpTexture[0].y = -1.0f;
	vertexTmpTexture[0].z = -0.1f;
	vertexTmpTexture[0].s = 0.0f;
	vertexTmpTexture[0].t = 0.0f;
	
	vertexTmpTexture[1].x = wfactor;
	vertexTmpTexture[1].y = -1.0f;
	vertexTmpTexture[1].z = -0.1f;
	vertexTmpTexture[1].s = ((float)w * wmul) / iw;
	vertexTmpTexture[1].t = 0.0f;
	
	vertexTmpTexture[2].x = wfactor;
	vertexTmpTexture[2].y = hfactor;
	vertexTmpTexture[2].z = -0.1f;
	vertexTmpTexture[2].s = ((float)w * wmul) / iw;
	vertexTmpTexture[2].t = ((float)h * hmul) / ih;
	
	vertexTmpTexture[3].x = -1.0f;
	vertexTmpTexture[3].y = hfactor;
	vertexTmpTexture[3].z = -0.1f;
	vertexTmpTexture[3].s = 0.0f;
	vertexTmpTexture[3].t = ((float)h * hmul) / ih;
}

void GLDraw_3_0::do_set_texture_size(QImage *p, int w, int h)
{
	if(w <= 0) w = using_flags->get_screen_width();
	if(h <= 0) h = using_flags->get_screen_height();
	imgptr = p;
	if(p != NULL) {
		screen_texture_width = w;
		screen_texture_height = h;

		p_wid->makeCurrent();
		{
			set_texture_vertex(p, p_wid->width(), p_wid->height(), w, h);
			setNormalVAO(std_pass->getShader(), std_pass->getVAO(),
						 std_pass->getVertexBuffer(),
						 vertexTmpTexture, 4);
			set_texture_vertex(p, p_wid->width(), p_wid->height(), w, h);
			setNormalVAO(ntsc_pass1->getShader(), ntsc_pass1->getVAO(),
						 ntsc_pass1->getVertexBuffer(),
						 vertexTmpTexture, 4);

			set_texture_vertex(p, p_wid->width(), p_wid->height(), w, h);
			setNormalVAO(ntsc_pass2->getShader(), ntsc_pass2->getVAO(),
						 ntsc_pass2->getVertexBuffer(),
						 vertexTmpTexture, 4);
			
		}
		{
			p_wid->deleteTexture(uVramTextureID);
			uVramTextureID = p_wid->bindTexture(*p);
		}
		int ww = w;
		int hh = h;
		float iw, ih;
		
		//if(screen_multiply < 1.0f) {
		if(p != NULL) {
			iw = (float)p->width();
			ih = (float)p->height();
		} else {
			iw = (float)using_flags->get_screen_width();
			ih = (float)using_flags->get_screen_height();
		}
		vertexFormat[0].x = -1.0f;
		vertexFormat[0].y = -1.0f;
		vertexFormat[0].z = -0.9f;
		vertexFormat[1].x =  1.0f;
		vertexFormat[1].y = -1.0f;
		vertexFormat[1].z = -0.9f;
		vertexFormat[2].x =  1.0f;
		vertexFormat[2].y =  1.0f;
		vertexFormat[2].z = -0.9f;
		vertexFormat[3].x = -1.0f;
		vertexFormat[3].y =  1.0f;
		vertexFormat[3].z = -0.9f;

		vertexFormat[0].s = 0.0f;
		vertexFormat[0].t = (float)hh / ih;
		vertexFormat[1].s = (float)ww / iw;
		vertexFormat[1].t = (float)hh / ih;
		vertexFormat[2].s = (float)ww / iw;
		vertexFormat[2].t = 0.0f;
		vertexFormat[3].s = 0.0f;
		vertexFormat[3].t = 0.0f;
		
		setNormalVAO(main_pass->getShader(), main_pass->getVAO(),
					 main_pass->getVertexBuffer(),
					 vertexFormat, 4);
		p_wid->doneCurrent();

		this->doSetGridsHorizonal(h, true);
		this->doSetGridsVertical(w, true);
	}
}

void GLDraw_3_0::resizeGL_Screen(void)
{
	if(main_pass != NULL) {
		setNormalVAO(main_pass->getShader(), main_pass->getVAO(),
					 main_pass->getVertexBuffer(),
					 vertexFormat, 4);
	}
}	

