/*
 * qt_glutil_gl2_0.h
 * (c) 2016 K.Ohta <whatisthis.sowhat@gmail.com>
 * License: GPLv2.
 * Renderer with OpenGL v2.0 .
 * History:
 * Jan 21, 2016 : Initial.
 */

#ifndef _QT_COMMON_GLUTIL_2_0_H
#define _QT_COMMON_GLUTIL_2_0_H

#include <QtGui>
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QGLWidget>
#include <QImage>
#include <QOpenGLFunctions_2_0>
#include <QTimer>
#include <QList>

#include "common.h"

#include "qt_glpack.h"

class EMU;
class QEvent;
class GLDrawClass;
class QOpenGLFramebufferObject;
class QOpenGLFramebufferObjectFormat;
class USING_FLAGS;
class CSP_Logger;
class DLL_PREFIX GLDraw_2_0 : public QObject
{
	Q_OBJECT
private:
	QOpenGLFunctions_2_0 *extfunc_2;
protected:
	GLDrawClass *p_wid;
	USING_FLAGS *using_flags;
	QImage *imgptr;
	CSP_Logger *csp_logger;
	bool smoosing;
	bool gl_grid_horiz;
	bool gl_grid_vert;
	
	int  vert_lines;
	int  horiz_pixels;
	GLfloat *glVertGrids;
	GLfloat *glHorizGrids;
	float screen_multiply;

	float screen_width;
	float screen_height;
	
	int screen_texture_width;
	int screen_texture_width_old;
	int screen_texture_height;
	int screen_texture_height_old;

	GLuint icon_texid[9][8];

	int rec_count;
	int rec_width;
	int rec_height;

	VertexTexCoord_t vertexFormat[4];
	
	QOpenGLShaderProgram *main_shader;
	
	QOpenGLVertexArrayObject *vertex_screen;
	QOpenGLBuffer *buffer_screen_vertex;
	
	VertexTexCoord_t vertexBitmap[4];
	QOpenGLShaderProgram *bitmap_shader;
	QOpenGLBuffer *buffer_bitmap_vertex;
	QOpenGLVertexArrayObject *vertex_bitmap;
	QOpenGLVertexArrayObject *vertex_button[128];
	QOpenGLBuffer *buffer_button_vertex[128];
	QOpenGLShaderProgram *button_shader;
	VertexTexCoord_t vertexOSD[32][4];
	QOpenGLVertexArrayObject *vertex_osd[32];
	QOpenGLBuffer *buffer_osd[32];
	QOpenGLShaderProgram *osd_shader;

	virtual void initButtons(void);
	virtual void initBitmapVertex(void);
	virtual void initBitmapVAO(void);

	GLuint uVramTextureID;
	GLuint uButtonTextureID[128];
	GLfloat fButtonX[128];
	GLfloat fButtonY[128];
	GLfloat fButtonWidth[128];
	GLfloat fButtonHeight[128];
	QVector<VertexTexCoord_t> *vertexButtons;

	QVector<QImage> ButtonImages;
	bool button_updated;
	void updateButtonTexture(void);
	

	GLfloat fBrightR;
	GLfloat fBrightG;
	GLfloat fBrightB;
	bool set_brightness;
	bool InitVideo;
	GLuint uBitmapTextureID;
	bool bitmap_uploaded;
	virtual void setNormalVAO(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
							  QOpenGLBuffer *bp, VertexTexCoord_t *tp, int size = 4);
	
	virtual void resizeGL_Screen(void);
	virtual void drawGridsHorizonal(void);
	virtual void drawGridsVertical(void);
	void resizeGL_SetVertexs(void);
	
	void drawGridsMain(GLfloat *tp,
					   int number,
					   GLfloat lineWidth = 0.2f,
					   QVector4D color = QVector4D(0.0f, 0.0f, 0.0f, 1.0f));
	void drawButtons();
	bool button_drawn;
	void drawBitmapTexture(void);
	bool crt_flag;
	bool redraw_required;

	virtual void drawOsdLeds();
	virtual void drawOsdIcons();
	virtual void set_osd_vertex(int xbit);

	QOpenGLFramebufferObject *offscreen_frame_buffer;
	QOpenGLFramebufferObjectFormat *offscreen_frame_buffer_format;
	QImage offscreen_image;
	GLint texture_max_size;
	
	bool low_resolution_screen;
	bool emu_launched;

	uint32_t osd_led_status;
	uint32_t osd_led_status_bak;
	int osd_led_bit_width;
	bool osd_onoff;
public:
	GLDraw_2_0(GLDrawClass *parent, USING_FLAGS *p, CSP_Logger *logger, EMU *emu = 0);
	~GLDraw_2_0();

	virtual void initGLObjects();
	virtual void initFBO(void);
	virtual void initLocalGLObjects(void);
	virtual void initOsdObjects(void);

	virtual void uploadMainTexture(QImage *p, bool chromakey);

	virtual void drawScreenTexture(void);
	void drawGrids(void);
	void uploadBitmapTexture(QImage *p);

	virtual void drawMain(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
						  QOpenGLBuffer *bp,
						  VertexTexCoord_t *vertex_data,
						  GLuint texid,
						  QVector4D color, bool f_smoosing,
						  bool do_chromakey = false,
						  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f));
	virtual void doSetGridsHorizonal(int lines, bool force);
	virtual void doSetGridsVertical(int pixels, bool force);
public slots:
	virtual void setBrightness(GLfloat r, GLfloat g, GLfloat b);
	virtual void do_set_texture_size(QImage *p, int w, int h);
	virtual void do_set_screen_multiply(float mul);
	virtual void uploadIconTexture(QPixmap *p, int icon_type, int localnum);
	
	void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int width, int height);

	void setImgPtr(QImage *p);
	void setSmoosing(bool);
	void setDrawGLGridVert(bool);
	void setDrawGLGridHoriz(bool);
	void setVirtualVramSize(int ,int);	
	void setChangeBrightness(bool);
	void updateBitmap(QImage *);
	void paintGL_OffScreen(int count, int w, int h);
	void set_emu_launched(void);
	void do_set_display_osd(bool onoff);
	void do_display_osd_leds(int lednum, bool onoff);
	void do_set_led_width(int bitwidth);
signals:
	int sig_push_image_to_movie(int, int, int, QImage *);
};
#endif // _QT_COMMON_GLUTIL_2_0_H
