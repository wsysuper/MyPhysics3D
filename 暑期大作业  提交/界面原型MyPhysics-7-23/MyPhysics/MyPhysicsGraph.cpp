#include "stdafx.h"
//#include "MyPhysics.h"

#include "MyPhysicsDoc.h"
#include "MyPhysicsView.h"

void CMyPhysicsView::initMotionModel()
{
	view_xyz[0] = 2;
	view_xyz[1] = 0;
	view_xyz[2] = 1;
	view_hpr[0] = 180;
	view_hpr[1] = 0;
	view_hpr[2] = 0;
}

void CMyPhysicsView::wrapCameraAngles()
{
	for (int i=0; i<3; i++)
	{
		while (view_hpr[i] > 180) view_hpr[i] -= 360;
		while (view_hpr[i] < -180) view_hpr[i] += 360;
	}
}

// call this to update the current camera position. the bits in `mode' say
// if the left (1), middle (2) or right (4) mouse button is pressed, and
// (deltax,deltay) is the amount by which the mouse pointer has moved.

void CMyPhysicsView::dsMotion (int mode, int deltax, int deltay)
{
	float side = 0.01f * float(deltax);
	float fwd = (mode==4) ? (0.01f * float(deltay)) : 0.0f;
	float s = (float) sin (view_hpr[0]*DEG_TO_RAD);
	float c = (float) cos (view_hpr[0]*DEG_TO_RAD);

	if (mode==1) {
		view_hpr[0] += float (deltax) * 0.5f;
		view_hpr[1] += float (deltay) * 0.5f;
	}
	else {
		view_xyz[0] += -s*side + c*fwd;
		view_xyz[1] += c*side + s*fwd;
		if (mode==2 || mode==5) view_xyz[2] += 0.01f * float(deltay);
	}
	wrapCameraAngles();
}

//***************************************************************************
// drawing loop stuff

#if !defined(macintosh) || defined(ODE_PLATFORM_OSX)

void CMyPhysicsView::dsStartGraphics (int width, int height, dsFunctions *fn)
{

	const char *prefix = DEFAULT_PATH_TO_TEXTURES;
	if (fn->version >= 2 && fn->path_to_textures) prefix = fn->path_to_textures;
	char *s = (char*) alloca (strlen(prefix) + 20);

	strcpy (s,prefix);
	strcat (s,"/sky.ppm");
	texture[DS_SKY] = sky_texture = new Texture (s);

	strcpy (s,prefix);
	strcat (s,"/ground.ppm");
	texture[DS_GROUND] = ground_texture = new Texture (s);

	strcpy (s,prefix);
	strcat (s,"/wood.ppm");
	texture[DS_WOOD] = wood_texture = new Texture (s);

	strcpy (s,prefix);
	strcat (s,"/checkered.ppm");
	texture[DS_CHECKERED] = checkered_texture = new Texture (s);
}

#else // macintosh

void CMyPhysicsView::dsStartGraphics (int width, int height, dsFunctions *fn)
{

	// All examples build into the same dir
	char *prefix = "::::drawstuff:textures";
	char *s = (char*) alloca (strlen(prefix) + 20);

	strcpy (s,prefix);
	strcat (s,":sky.ppm");
	sky_texture = new Texture (s);

	strcpy (s,prefix);
	strcat (s,":ground.ppm");
	ground_texture = new Texture (s);

	strcpy (s,prefix);
	strcat (s,":wood.ppm");
	wood_texture = new Texture (s);
}

#endif


void CMyPhysicsView::dsStopGraphics()
{
	delete sky_texture;
	delete ground_texture;
	delete wood_texture;
	sky_texture = 0;
	ground_texture = 0;
	wood_texture = 0;
}


void CMyPhysicsView::drawSky (float view_xyz[3])
{
	glDisable (GL_LIGHTING);
	if (use_textures) {
		glEnable (GL_TEXTURE_2D);
		sky_texture->bind (0);
	}
	else {
		glDisable (GL_TEXTURE_2D);
		glColor3f (0,0.5,1.0);
	}

	// make sure sky depth is as far back as possible
	glShadeModel (GL_FLAT);
	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);
	glDepthRange (1,1);

	const float ssize = 1000.0f;
	static float offset = 0.0f;

	float x = ssize*sky_scale;
	float z = view_xyz[2] + sky_height;

	glBegin (GL_QUADS);
	glNormal3f (0,0,-1);
	glTexCoord2f (-x+offset,-x+offset);
	glVertex3f (-ssize+view_xyz[0],-ssize+view_xyz[1],z);
	glTexCoord2f (-x+offset,x+offset);
	glVertex3f (-ssize+view_xyz[0],ssize+view_xyz[1],z);
	glTexCoord2f (x+offset,x+offset);
	glVertex3f (ssize+view_xyz[0],ssize+view_xyz[1],z);
	glTexCoord2f (x+offset,-x+offset);
	glVertex3f (ssize+view_xyz[0],-ssize+view_xyz[1],z);
	glEnd();

	offset = offset + 0.002f;
	if (offset > 1) offset -= 1;

	glDepthFunc (GL_LESS);
	glDepthRange (0,1);
}


void CMyPhysicsView::drawGround()
{
	glDisable (GL_LIGHTING);
	glShadeModel (GL_FLAT);
	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LESS);
	// glDepthRange (1,1);

	if (use_textures) {
		glEnable (GL_TEXTURE_2D);
		ground_texture->bind (0);
	}
	else {
		glDisable (GL_TEXTURE_2D);
		glColor3f (GROUND_R,GROUND_G,GROUND_B);
	}

	// ground fog seems to cause problems with TNT2 under windows
	/*
	GLfloat fogColor[4] = {0.5, 0.5, 0.5, 1};
	glEnable (GL_FOG);
	glFogi (GL_FOG_MODE, GL_EXP2);
	glFogfv (GL_FOG_COLOR, fogColor);
	glFogf (GL_FOG_DENSITY, 0.05f);
	glHint (GL_FOG_HINT, GL_NICEST); // GL_DONT_CARE);
	glFogf (GL_FOG_START, 1.0);
	glFogf (GL_FOG_END, 5.0);
	*/

	const float gsize = 100.0f;
	const float offset = 0; // -0.001f; ... polygon offsetting doesn't work well

	glBegin (GL_QUADS);
	glNormal3f (0,0,1);
	glTexCoord2f (-gsize*ground_scale + ground_ofsx,
		-gsize*ground_scale + ground_ofsy);
	glVertex3f (-gsize,-gsize,offset);
	glTexCoord2f (gsize*ground_scale + ground_ofsx,
		-gsize*ground_scale + ground_ofsy);
	glVertex3f (gsize,-gsize,offset);
	glTexCoord2f (gsize*ground_scale + ground_ofsx,
		gsize*ground_scale + ground_ofsy);
	glVertex3f (gsize,gsize,offset);
	glTexCoord2f (-gsize*ground_scale + ground_ofsx,
		gsize*ground_scale + ground_ofsy);
	glVertex3f (-gsize,gsize,offset);
	glEnd();

	glDisable (GL_FOG);
}


void CMyPhysicsView::drawPyramidGrid()
{
	// setup stuff
	glEnable (GL_LIGHTING);
	glDisable (GL_TEXTURE_2D);
	glShadeModel (GL_FLAT);
	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LESS);

	// draw the pyramid grid
	for (int i=-1; i<=1; i++) {
		for (int j=-1; j<=1; j++) {
			glPushMatrix();
			glTranslatef ((float)i,(float)j,(float)0);
			if (i==1 && j==0) setColor (1,0,0,1);
			else if (i==0 && j==1) setColor (0,0,1,1);
			else setColor (1,1,0,1);
			const float k = 0.03f;
			glBegin (GL_TRIANGLE_FAN);
			glNormal3f (0,-1,1);
			glVertex3f (0,0,k);
			glVertex3f (-k,-k,0);
			glVertex3f ( k,-k,0);
			glNormal3f (1,0,1);
			glVertex3f ( k, k,0);
			glNormal3f (0,1,1);
			glVertex3f (-k, k,0);
			glNormal3f (-1,0,1);
			glVertex3f (-k,-k,0);
			glEnd();
			glPopMatrix();
		}
	}
}


void CMyPhysicsView::dsDrawFrame (int width, int height, dsFunctions *fn, int pause)
{
	if (current_state < 1) ASSERT ("internal error");
	current_state = 2;

	// setup stuff
	glEnable (GL_LIGHTING);
	glEnable (GL_LIGHT0);
	glDisable (GL_TEXTURE_2D);
	glDisable (GL_TEXTURE_GEN_S);
	glDisable (GL_TEXTURE_GEN_T);
	glShadeModel (GL_FLAT);
	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LESS);
	glEnable (GL_CULL_FACE);
	glCullFace (GL_BACK);
	glFrontFace (GL_CCW);

	// setup viewport
	glViewport (0,0,width,height);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	const float vnear = 0.1f;
	const float vfar = 100.0f;
	const float k = 0.8f;     // view scale, 1 = +/- 45 degrees
	if (width >= height) {
		float k2 = float(height)/float(width);
		glFrustum (-vnear*k,vnear*k,-vnear*k*k2,vnear*k*k2,vnear,vfar);
	}
	else {
		float k2 = float(width)/float(height);
		glFrustum (-vnear*k*k2,vnear*k*k2,-vnear*k,vnear*k,vnear,vfar);
	}

	// setup lights. it makes a difference whether this is done in the
	// GL_PROJECTION matrix mode (lights are scene relative) or the
	// GL_MODELVIEW matrix mode (lights are camera relative, bad!).
	static GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
	static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);
	glColor3f (1.0, 1.0, 1.0);

	// clear the window
	glClearColor (0.5,0.5,0.5,0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// snapshot camera position (in MS Windows it is changed by the GUI thread)
	float view2_xyz[3];
	float view2_hpr[3];
	memcpy (view2_xyz,view_xyz,sizeof(float)*3);
	memcpy (view2_hpr,view_hpr,sizeof(float)*3);

	// go to GL_MODELVIEW matrix mode and set the camera
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();
	setCamera (view2_xyz[0],view2_xyz[1],view2_xyz[2],
		view2_hpr[0],view2_hpr[1],view2_hpr[2]);

	// set the light position (for some reason we have to do this in model view.
	static GLfloat light_position[] = { LIGHTX, LIGHTY, 1.0, 0.0 };
	glLightfv (GL_LIGHT0, GL_POSITION, light_position);

	// draw the background (ground, sky etc)
	drawSky (view2_xyz);
	drawGround();

	// draw the little markers on the ground
	drawPyramidGrid();

	// leave openGL in a known state - flat shaded white, no textures
	glEnable (GL_LIGHTING);
	glDisable (GL_TEXTURE_2D);
	glShadeModel (GL_FLAT);
	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LESS);
	glColor3f (1,1,1);
	setColor (1,1,1,1);

	// draw the rest of the objects. set drawing state first.
	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;
	tnum = 0;
	if (fn->step) fn->step (pause);
}

////////////////////////////////////

void CMyPhysicsView::dsSetViewpoint (float xyz[3], float hpr[3])
{
	if (current_state < 1) ASSERT ("dsSetViewpoint() called before simulation started");
	if (xyz) {
		view_xyz[0] = xyz[0];
		view_xyz[1] = xyz[1];
		view_xyz[2] = xyz[2];
	}
	if (hpr) {
		view_hpr[0] = hpr[0];
		view_hpr[1] = hpr[1];
		view_hpr[2] = hpr[2];
		wrapCameraAngles();
	}
}

void CMyPhysicsView::dsGetViewpoint (float xyz[3], float hpr[3])
{
	if (current_state < 1) ASSERT ("dsGetViewpoint() called before simulation started");
	if (xyz) {
		xyz[0] = view_xyz[0];
		xyz[1] = view_xyz[1];
		xyz[2] = view_xyz[2];
	}
	if (hpr) {
		hpr[0] = view_hpr[0];
		hpr[1] = view_hpr[1];
		hpr[2] = view_hpr[2];
	}
}

void CMyPhysicsView::dsSetTexture (int texture_number)
{
	if (current_state != 2) ASSERT ("drawing function called outside simulation loop");
	tnum = texture_number;
}

void CMyPhysicsView::dsSetColor (float red, float green, float blue)
{
	if (current_state != 2) ASSERT ("drawing function called outside simulation loop");
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = 1;
}

//void dsSetColorAlpha (float red, float green, float blue,
//								 float alpha)
//{
//	if (current_state != 2) dsError ("drawing function called outside simulation loop");
//	color[0] = red;
//	color[1] = green;
//	color[2] = blue;
//	color[3] = alpha;
//}

void CMyPhysicsView::dsDrawBox (const float pos[3], const float R[12],
								const float sides[3])
{
	if (current_state != 2) ASSERT ("drawing function called outside simulation loop");
	setupDrawingMode();
	glShadeModel (GL_FLAT);
	setTransform (pos,R);
	drawBox (sides);
	glPopMatrix();

	if (use_shadows) {
		setShadowDrawingMode();
		setShadowTransform();
		setTransform (pos,R);
		drawBox (sides);
		glPopMatrix();
		glPopMatrix();
		glDepthRange (0,1);
	}
}

void CMyPhysicsView::dsDrawConvex (const float pos[3], const float R[12],
								   float *_planes,unsigned int _planecount,
								   float *_points,
								   unsigned int _pointcount,
								   unsigned int *_polygons)
{
	if (current_state != 2) ASSERT ("drawing function called outside simulation loop");
	setupDrawingMode();
	glShadeModel (GL_FLAT);
	setTransform (pos,R);
	drawConvex(_planes,_planecount,_points,_pointcount,_polygons);
	glPopMatrix();
	if (use_shadows) {
		setShadowDrawingMode();
		setShadowTransform();
		setTransform (pos,R);
		drawConvex(_planes,_planecount,_points,_pointcount,_polygons);
		glPopMatrix();
		glPopMatrix();
		glDepthRange (0,1);
	}
}

void CMyPhysicsView::dsDrawSphere (const float pos[3], const float R[12],
								   float radius)
{
	if (current_state != 2) ASSERT ("drawing function called outside simulation loop");
	setupDrawingMode();
	glEnable (GL_NORMALIZE);
	glShadeModel (GL_SMOOTH);
	setTransform (pos,R);
	glScaled (radius,radius,radius);
	drawSphere();
	glPopMatrix();
	glDisable (GL_NORMALIZE);

	// draw shadows
	if (use_shadows) {
		glDisable (GL_LIGHTING);
		if (use_textures) {
			ground_texture->bind (1);
			glEnable (GL_TEXTURE_2D);
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
			glColor3f (SHADOW_INTENSITY,SHADOW_INTENSITY,SHADOW_INTENSITY);
		}
		else {
			glDisable (GL_TEXTURE_2D);
			glColor3f (GROUND_R*SHADOW_INTENSITY,GROUND_G*SHADOW_INTENSITY,
				GROUND_B*SHADOW_INTENSITY);
		}
		glShadeModel (GL_FLAT);
		glDepthRange (0,0.9999);
		drawSphereShadow (pos[0],pos[1],pos[2],radius);
		glDepthRange (0,1);
	}
}

void CMyPhysicsView::dsDrawTriangle (const float pos[3], const float R[12],
									 const float *v0, const float *v1,
									 const float *v2, int solid)
{
	if (current_state != 2) ASSERT ("drawing function called outside simulation loop");
	setupDrawingMode();
	glShadeModel (GL_FLAT);
	setTransform (pos,R);
	drawTriangle (v0, v1, v2, solid);
	glPopMatrix();
}

void CMyPhysicsView::dsDrawCylinder (const float pos[3], const float R[12],
									 float length, float radius)
{
	if (current_state != 2) ASSERT ("drawing function called outside simulation loop");
	setupDrawingMode();
	glShadeModel (GL_SMOOTH);
	setTransform (pos,R);
	drawCylinder (length,radius,0);
	glPopMatrix();

	if (use_shadows) {
		setShadowDrawingMode();
		setShadowTransform();
		setTransform (pos,R);
		drawCylinder (length,radius,0);
		glPopMatrix();
		glPopMatrix();
		glDepthRange (0,1);
	}
}

void CMyPhysicsView::dsDrawCapsule (const float pos[3], const float R[12],
									float length, float radius)
{
	if (current_state != 2) ASSERT ("drawing function called outside simulation loop");
	setupDrawingMode();
	glShadeModel (GL_SMOOTH);
	setTransform (pos,R);
	drawCapsule (length,radius);
	glPopMatrix();

	if (use_shadows) {
		setShadowDrawingMode();
		setShadowTransform();
		setTransform (pos,R);
		drawCapsule (length,radius);
		glPopMatrix();
		glPopMatrix();
		glDepthRange (0,1);
	}
}


void CMyPhysicsView::dsDrawLine (const float pos1[3], const float pos2[3])
{
	setupDrawingMode();
	glColor3f (color[0],color[1],color[2]);
	glDisable (GL_LIGHTING);
	glLineWidth (2);
	glShadeModel (GL_FLAT);
	glBegin (GL_LINES);
	glVertex3f (pos1[0],pos1[1],pos1[2]);
	glVertex3f (pos2[0],pos2[1],pos2[2]);
	glEnd();
}



void CMyPhysicsView::setupDrawingMode()
{
	glEnable (GL_LIGHTING);
	if (tnum)
	{
		if (use_textures)
		{
			glEnable (GL_TEXTURE_2D);
			texture[tnum]->bind (1);
			glEnable (GL_TEXTURE_GEN_S);
			glEnable (GL_TEXTURE_GEN_T);
			glTexGeni (GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
			glTexGeni (GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
			static GLfloat s_params[4] = {1.0f,1.0f,0.0f,1};
			static GLfloat t_params[4] = {0.817f,-0.817f,0.817f,1};
			glTexGenfv (GL_S,GL_OBJECT_PLANE,s_params);
			glTexGenfv (GL_T,GL_OBJECT_PLANE,t_params);
		}
		else
		{
			glDisable (GL_TEXTURE_2D);
		}
	}
	else
	{
		glDisable (GL_TEXTURE_2D);
	}
	setColor (color[0],color[1],color[2],color[3]);

	if (color[3] < 1)
	{
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glDisable (GL_BLEND);
	}
}


void CMyPhysicsView::setShadowDrawingMode()
{
	glDisable (GL_LIGHTING);
	if (use_textures)
	{
		glEnable (GL_TEXTURE_2D);
		ground_texture->bind (1);
		glColor3f (SHADOW_INTENSITY,SHADOW_INTENSITY,SHADOW_INTENSITY);
		glEnable (GL_TEXTURE_2D);
		glEnable (GL_TEXTURE_GEN_S);
		glEnable (GL_TEXTURE_GEN_T);
		glTexGeni (GL_S,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
		glTexGeni (GL_T,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
		static GLfloat s_params[4] = {ground_scale,0,0,ground_ofsx};
		static GLfloat t_params[4] = {0,ground_scale,0,ground_ofsy};
		glTexGenfv (GL_S,GL_EYE_PLANE,s_params);
		glTexGenfv (GL_T,GL_EYE_PLANE,t_params);
	}
	else
	{
		glDisable (GL_TEXTURE_2D);
		glColor3f (GROUND_R*SHADOW_INTENSITY,GROUND_G*SHADOW_INTENSITY,
			GROUND_B*SHADOW_INTENSITY);
	}
	glDepthRange (0,0.9999);
}

////////////////////////////////////////////////////////////////////////

void CMyPhysicsView::start()
{
	// adjust the starting viewpoint a bit
	float xyz[3],hpr[3];
	dsGetViewpoint (xyz,hpr);
	hpr[0] += 7;
	xyz[0] += 1;
	xyz[1] += 0.5;
	xyz[2] += 0.1;
	dsSetViewpoint (xyz,hpr);
}


void CMyPhysicsView::simLoop (int pause)
{
	float pos[3];
	float R[12];
	static float a = 0;

	if (!pause) a += 0.02f;
	if (a > (2*M_PI)) a -= (float) (2*M_PI);
	float ca = (float) cos(a);
	float sa = (float) sin(a);
	if(bshowSphere)
	{
		dsSetTexture (DS_WOOD);

		float b = (a > M_PI) ? (2*(a-(float)M_PI)) : a*2;
		pos[0] = -0.3f;
		pos[1] = 0.3f;
		pos[2] = (float) (0.1f*(2*M_PI*b - b*b) + 0.3f);
		R[0] = ca; R[1] = 0; R[2] = -sa;
		R[4] = 0;  R[5] = 1; R[6] = 0;
		R[8] = sa; R[9] = 0; R[10] = ca;
		dsSetColor (1,0.8f,0.6f);
		dsDrawSphere (pos,R,0.3f);
		if(bshow_force)
		{
			float pos2[3];
			pos2[0] = pos[0];
			pos2[1] = pos[1];
			pos2[2] = pos[2] - 1;
			dsSetColor (0,0,0);
			dsDrawLine(pos,pos2);
			float pos3[3];
			pos3[0] = pos2[0]+0.1*sin(a*5);
			pos3[1] = pos2[1]+0.1*cos(a*5);
			pos3[2] = pos2[2]+0.2;
			dsDrawLine(pos3,pos2);
			pos3[0] = pos2[0]-0.1*sin(a*5);
			pos3[1] = pos2[1]-0.1*cos(a*5);
			dsDrawLine(pos3,pos2);
			if(a-M_PI<0.05 && M_PI-a < 0.05 || a<0.05 || a>2*M_PI-0.05)
			{
				pos2[2] = 3;
				dsSetColor (1,0,1);
				dsDrawLine(pos,pos2);
				pos3[0] = pos2[0];
				pos3[1] = pos2[1]+0.1;
				pos3[2] = pos2[2]-0.2;
				dsDrawLine(pos3,pos2);
				pos3[1] = pos2[1]-0.1;
				dsDrawLine(pos3,pos2);
			}
		}
	}
	if(bshowCube)
	{
		dsSetTexture (DS_NONE);

		pos[0] = ca*2;
		pos[1] = sa*2;
		pos[2] = 0.4f;
		R[0] = ca; R[1] = -sa; R[2] = 0;
		R[4] = sa; R[5] = ca;  R[6] = 0;
		R[8] = 0;  R[9] = 0;   R[10] = 1;
		float sides[3] = {0.1f,0.4f,0.8f};
		dsSetColor (0.6f,0.6f,1);
		dsDrawBox (pos,R,sides);
		if(bshow_force)
		{

			float pos2[3];
			pos2[0] = pos[0]/1.5;
			pos2[1] = pos[1]/1.5;
			pos2[2] = pos[2];
			dsSetColor (0,0,1);
			dsDrawLine(pos,pos2);
			float pos3[3];
			pos3[0] = 1.55*cos(a+0.07);
			pos3[1] = 1.55*sin(a+0.07);
			pos3[2] = pos2[2];
			dsDrawLine(pos3,pos2);
			pos3[0] = 1.55*cos(a-0.07);
			pos3[1] = 1.55*sin(a-0.07);
			dsDrawLine(pos3,pos2);
			// gravity
			pos2[0] = pos[0];
			pos2[1] = pos[1];
			pos2[2] = pos[2] - 1;
			dsSetColor (0,0,0);
			dsDrawLine(pos,pos2);
			pos3[0] = pos2[0]+0.1*sin(a*5);
			pos3[1] = pos2[1]+0.1*cos(a*5);
			pos3[2] = pos2[2]+0.2;
			dsDrawLine(pos3,pos2);
			pos3[0] = pos2[0]-0.1*sin(a*5);
			pos3[1] = pos2[1]-0.1*cos(a*5);
			dsDrawLine(pos3,pos2);
			//
			pos2[2] = pos[2] + 1;
			dsSetColor (1,0,1);
			dsDrawLine(pos,pos2);
			pos3[0] = pos2[0]+0.1*sin(a*5);
			pos3[1] = pos2[1]+0.1*cos(a*5);
			pos3[2] = pos2[2]-0.2;
			dsDrawLine(pos3,pos2);
			pos3[0] = pos2[0]-0.1*sin(a*5);
			pos3[1] = pos2[1]-0.1*cos(a*5);
			dsDrawLine(pos3,pos2);
		}
	}
	if(bshowothers)
	{
		dsSetTexture (DS_WOOD);

		float r = 0.3f;		      // cylinder radius
		float d = (float)cos(a*2) * 0.4f;
		float cd = (float)cos(-d/r);
		float sd = (float)sin(-d/r);
		pos[0] = -0.2f;
		pos[1] = -1 + d;
		pos[2] = 0.3f;
		R[0] = 0;   R[1] = 0;  R[2] = -1;
		R[4] = -sd; R[5] = cd; R[6] =  0;
		R[8] =  cd; R[9] = sd; R[10] = 0;
		dsSetColor (0.4f,1,1);
		dsDrawCylinder (pos,R,0.8f,r);
		if(bshow_force)
		{
			float pos2[3];
			pos2[0] = pos[0];
			pos2[1] = pos[1]-d*3;
			pos2[2] = pos[2];
			dsSetColor (0,0,1);
			dsDrawLine(pos,pos2);
			float pos3[3];
			pos3[0] = pos2[0]+0.1*cos(a*5);
			pos3[1] = pos2[1]+d/abs(d)*0.2;
			pos3[2] = pos2[2]+0.1*sin(a*5);
			dsDrawLine(pos3,pos2);
			pos3[0] = pos2[0]-0.1*cos(a*5);
			pos3[2] = pos2[2]-0.1*sin(a*5);
			dsDrawLine(pos3,pos2);
			// gravity
			pos2[0] = pos[0];
			pos2[1] = pos[1];
			pos2[2] = pos[2] - 1;
			dsSetColor (0,0,0);
			dsDrawLine(pos,pos2);
			pos3[0] = pos2[0]+0.1*sin(a*5);
			pos3[1] = pos2[1]+0.1*cos(a*5);
			pos3[2] = pos2[2]+0.2;
			dsDrawLine(pos3,pos2);
			pos3[0] = pos2[0]-0.1*sin(a*5);
			pos3[1] = pos2[1]-0.1*cos(a*5);
			dsDrawLine(pos3,pos2);
			//
			pos2[2] = pos[2] + 1;
			dsSetColor (1,0,1);
			dsDrawLine(pos,pos2);
			pos3[0] = pos2[0]+0.1*sin(a*5);
			pos3[1] = pos2[1]+0.1*cos(a*5);
			pos3[2] = pos2[2]-0.2;
			dsDrawLine(pos3,pos2);
			pos3[0] = pos2[0]-0.1*sin(a*5);
			pos3[1] = pos2[1]-0.1*cos(a*5);
			dsDrawLine(pos3,pos2);

		}
		pos[0] = 0.7f;
		pos[1] = 0.7f;
		pos[2] = 0.2f;
		R[0] = 0; R[1] = sa; R[2] = -ca;
		R[4] = 0; R[5] = ca; R[6] = sa;
		R[8] = 1; R[9] = 0;  R[10] = 0;
		dsSetColor (1,0.9f,0.2f);
		dsDrawCappedCylinder (pos,R,0.8f,0.2f);
		if(bshow_force)
		{
			float pos2[3];
			float pos3[3];

			// gravity
			pos2[0] = pos[0];
			pos2[1] = pos[1];
			pos2[2] = pos[2] - 1;
			dsSetColor (0,0,0);
			dsDrawLine(pos,pos2);
			pos3[0] = pos2[0]+0.1*sin(a*5);
			pos3[1] = pos2[1]+0.1*cos(a*5);
			pos3[2] = pos2[2]+0.2;
			dsDrawLine(pos3,pos2);
			pos3[0] = pos2[0]-0.1*sin(a*5);
			pos3[1] = pos2[1]-0.1*cos(a*5);
			dsDrawLine(pos3,pos2);
			//
			pos2[2] = pos[2] + 1;
			dsSetColor (1,0,1);
			dsDrawLine(pos,pos2);
			pos3[0] = pos2[0]+0.1*sin(a*5);
			pos3[1] = pos2[1]+0.1*cos(a*5);
			pos3[2] = pos2[2]-0.2;
			dsDrawLine(pos3,pos2);
			pos3[0] = pos2[0]-0.1*sin(a*5);
			pos3[1] = pos2[1]-0.1*cos(a*5);
			dsDrawLine(pos3,pos2);
		}
	}

	//pos[0]=-0.3;
	//pos[1]=0;
	//pos[2]=0;
	//float pos1[3]={-0.3,0,2};
	//float pos2[3]={1,0,2};
	//float pos3[3]={-0.3,1,1};
	//drawTriangle(pos,pos2,pos3,1);
}

