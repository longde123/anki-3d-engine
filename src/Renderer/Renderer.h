#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "Common.h"
#include "Math.h"
#include "Fbo.h"
#include "Texture.h"
#include "ShaderProg.h"
#include "Vbo.h"
#include "RsrcPtr.h"


class Camera;
class PointLight;
class SpotLight;
class RendererInitializer;
class SceneNode;



/**
 * Offscreen renderer
 *
 * It is a class and not a namespace because we may need external renderers for security cameras for example
 */
class Renderer
{
	//====================================================================================================================
	// nested                                                                                                            =
	//====================================================================================================================
	public:
		/**
		 * Rendering stage
		 */
		class RenderingStage
		{
			protected:
				Renderer& r; ///< Just so that the stage can know the father class

			public:
				RenderingStage(Renderer& r_): r(r_) {}
		};

		/**
		 * Material stage
		 */
		class Ms: private RenderingStage
		{
			friend class Renderer;

			public:
				/**
				 * EarlyZ pass
				 */
				class Ez: public RenderingStage
				{
					friend class Renderer;
					friend class Ms;

					PROPERTY_R(bool, enabled, isEnabled)

					public:
						Ez(Renderer& r_): RenderingStage(r_) {}

					private:
						Fbo fbo;

						void init();
						void run();
				};

			public:
				Ez ez;

				Ms(Renderer& r_): RenderingStage(r_), ez(r_) {}

			private:
				Texture normalFai;
				Texture diffuseFai;
				Texture specularFai;
				Texture depthFai;
				Fbo fbo;

				void init();
				void run();
		}; // end Ms

		/**
		 * Illumination stage
		 */
		class Is: private RenderingStage
		{
			friend class Renderer;
			friend class MainRenderer;

			public:
				/**
				 * Shadowmapping pass
				 */
				class Sm: private RenderingStage
				{
					friend class Is;
					friend class Renderer;

					PROPERTY_R(bool, enabled, isEnabled) ///< If false then disable
					PROPERTY_R(bool, pcfEnabled, isPcfEnabled) ///< Enable Percentage Closer Filtering
					PROPERTY_R(bool, bilinearEnabled, isBilinearEnabled) ///< Shadowmap bilinear filtering. Better quality
					PROPERTY_R(int, resolution, getResolution) ///< Shadowmap resolution. The higher the better but slower

					public:
						Sm(Renderer& r_): RenderingStage(r_) {}

					private:
						Fbo fbo; ///< Illumination stage shadowmapping FBO
						Texture shadowMap;

						void init();

						/**
						 * Render the scene only with depth and store the result in the shadowMap
						 * @param cam The light camera
						 */
						void run(const Camera& cam);
				}; // end Sm

				/**
				 * Stencil masking optimizations
				 */
				class Smo: public RenderingStage
				{
					friend class Is;
					friend class Renderer;

					public:
						Smo(Renderer& r_): RenderingStage(r_) {}

					private:
						static float sMOUvSCoords[]; ///< Illumination stage stencil masking optimizations UV sphere vertex positions
						static Vbo sMOUvSVbo; ///< Illumination stage stencil masking optimizations UV sphere VBO
						RsrcPtr<ShaderProg> sProg;
						const ShaderProg::UniVar* modelViewProjectionMatUniVar; ///< Opt

						void init();
						void run(const PointLight& light);
						void run(const SpotLight& light);
				}; // end Smo

			private:
				/// Illumination stage light pass uniform variables (opt)
				struct UniVars
				{
					const ShaderProg::UniVar* msNormalFai;
					const ShaderProg::UniVar* msDiffuseFai;
					const ShaderProg::UniVar* msSpecularFai;
					const ShaderProg::UniVar* msDepthFai;
					const ShaderProg::UniVar* planes;
					const ShaderProg::UniVar* lightPos;
					const ShaderProg::UniVar* lightInvRadius;
					const ShaderProg::UniVar* lightDiffuseCol;
					const ShaderProg::UniVar* lightSpecularCol;
					const ShaderProg::UniVar* lightTex;
					const ShaderProg::UniVar* texProjectionMat;
					const ShaderProg::UniVar* shadowMap;
				};

			public:
				Sm sm;
				Smo smo;

				Is(Renderer& r_): RenderingStage(r_), sm(r_), smo(r_) {}

			private:
				Texture fai;
				Fbo fbo; ///< This FBO writes to the Is::fai
				uint stencilRb; ///< Illumination stage stencil buffer
				RsrcPtr<ShaderProg> ambientPassSProg; ///< Illumination stage ambient pass shader program
				RsrcPtr<ShaderProg> pointLightSProg; ///< Illumination stage point light shader program
				RsrcPtr<ShaderProg> spotLightNoShadowSProg; ///< Illumination stage spot light w/o shadow shader program
				RsrcPtr<ShaderProg> spotLightShadowSProg; ///< Illumination stage spot light w/ shadow shader program

				/**
				 * @name Ptrs to uniform variables
				 */
				//*@{*/
				const ShaderProg::UniVar* ambientColUniVar;
				const ShaderProg::UniVar* sceneColMapUniVar;
				UniVars pointLightSProgUniVars;
				UniVars spotLightNoShadowSProgUniVars;
				UniVars spotLightShadowSProgUniVars;
				//*@}*/

				Vec3 viewVectors[4];
				Vec2 planes;

				/**
				 * Calc the view vector that we will use inside the shader to calculate the frag pos in view space
				 */
				void calcViewVector();

				/**
				 * Calc the planes that we will use inside the shader to calculate the frag pos in view space
				 */
				void calcPlanes();
				void ambientPass(const Vec3& color);
				void pointLightPass(const PointLight& light);
				void spotLightPass(const SpotLight& light);
				void initFbo();
				void init();
				void run();
		}; // end Is

		/**
		 * Post-processing stage
		 *
		 * This stage is divided into 2 two parts. The first happens before blending stage and the second after.
		 */
		class Pps: private RenderingStage
		{
			friend class Renderer;
			friend class MainRenderer;

			public:
				/**
				 * High dynamic range lighting pass
				 */
				class Hdr: private RenderingStage
				{
					friend class Pps;
					friend class Renderer;

					PROPERTY_R(bool, enabled, isEnabled)
					PROPERTY_R(float, renderingQuality, getRenderingQuality)

					private:
						Fbo pass0Fbo;
						Fbo pass1Fbo;
						Fbo pass2Fbo;
						RsrcPtr<ShaderProg> pass0SProg;
						RsrcPtr<ShaderProg> pass1SProg;
						RsrcPtr<ShaderProg> pass2SProg;
						const ShaderProg::UniVar* pass0SProgFaiUniVar;
						const ShaderProg::UniVar* pass1SProgFaiUniVar;
						const ShaderProg::UniVar* pass2SProgFaiUniVar;

						void initFbos(Fbo& fbo, Texture& fai, int internalFormat);
						void init();
						void run();

					public:
						Texture pass0Fai; ///< Vertical blur pass FAI
						Texture pass1Fai; ///< pass0Fai with the horizontal blur FAI
						Texture fai; ///< The final FAI

						Hdr(Renderer& r_): RenderingStage(r_) {}
				}; // end Hrd

				/**
				 * Screen space ambient occlusion pass
				 *
				 * Three passes:
				 * - Calc ssao factor
				 * - Blur vertically
				 * - Blur horizontally
				 */
				class Ssao: private RenderingStage
				{
					friend class Pps;
					friend class Renderer;

					PROPERTY_R(bool, enabled, isEnabled)
					PROPERTY_R(float, renderingQuality, getRenderingQuality)
					PROPERTY_R(float, bluringQuality, getBluringQuality)

					private:
						Fbo pass0Fbo;
						Fbo pass1Fbo;
						Fbo pass2Fbo;
						uint width, height, bwidth, bheight;
						RsrcPtr<Texture> noiseMap;
						RsrcPtr<ShaderProg> ssaoSProg;
						RsrcPtr<ShaderProg> blurSProg;
						RsrcPtr<ShaderProg> blurSProg2;
						const ShaderProg::UniVar* camerarangeUniVar;
						const ShaderProg::UniVar* msDepthFaiUniVar;
						const ShaderProg::UniVar* noiseMapUniVar;
						const ShaderProg::UniVar* msNormalFaiUniVar;
						const ShaderProg::UniVar* blurSProgFaiUniVar;
						const ShaderProg::UniVar* blurSProg2FaiUniVar;

						void initBlurFbo(Fbo& fbo, Texture& fai);
						void init();
						void run();

					public:
						Texture pass0Fai;
						Texture pass1Fai;
						Texture fai;  //< The final FAI

						Ssao(Renderer& r_): RenderingStage(r_) {}
				}; // end Ssao

				struct UniVars
				{
					const ShaderProg::UniVar* isFai;
					const ShaderProg::UniVar* ppsPrePassFai;
					const ShaderProg::UniVar* ppsSsaoFai;
					const ShaderProg::UniVar* ppsHdrFai;
				};

			PROPERTY_R(bool, enabled, isEnabled)
			PROPERTY_R(float, renderingQuality, getRenderingQuality)

			public:
				Hdr hdr;
				Ssao ssao;

				Pps(Renderer& r_): RenderingStage(r_), hdr(r_), ssao(r_) {}

			private:
				Texture prePassFai;
				Texture postPassFai;
				Fbo prePassFbo;
				Fbo postPassFbo;
				RsrcPtr<ShaderProg> prePassSProg;
				RsrcPtr<ShaderProg> postPassSProg;
				UniVars prePassSProgUniVars;
				UniVars postPassSProgUniVars;

				void initPassFbo(Fbo& fbo, Texture& fai, const char* msg);
				void initPrePassSProg();
				void initPostPassSProg();
				void init();
				void runPrePass();
				void runPostPass();
		}; // end Pps

		/**
		 * Blending stage
		 */
		class Bs: public RenderingStage
		{
			friend class Renderer;
			friend class MainRenderer;

			public:
				Bs(Renderer& r_): RenderingStage(r_) {}

			private:
				Fbo fbo;
				Fbo refractFbo;
				RsrcPtr<ShaderProg> refractSProg;
				Texture refractFai;

				void createFbo();
				void createRefractFbo();
				void init();
				void run();
		}; // end Bs

		/**
		 * Debugging stage
		 */
		class Dbg: public RenderingStage
		{
			friend class Renderer;

			PROPERTY_R(bool, enabled, isEnabled)
			PROPERTY_RW(bool, showAxisEnabled, setShowAxis, isShowAxisEnabled)
			PROPERTY_RW(bool, showLightsEnabled, setShowLights, isShowLightsEnabled)
			PROPERTY_RW(bool, showSkeletonsEnabled, setShowSkeletons, isShowSkeletonsEnabled)
			PROPERTY_RW(bool, showCamerasEnabled, setShowCameras, isShowCamerasEnabled)

			public:
				Dbg(Renderer& r_);
				void renderGrid();
				static void drawSphere(float radius, const Transform& trf, const Vec4& col, int complexity = 8);
				static void renderCube(float size = 1.0);

				static void setColor(const Vec4& color);
				static void setModelMat(const Mat4& modelMat);
				static void drawLine(const Vec3& from, const Vec3& to, const Vec4& color);

			private:
				Fbo fbo;
				static RsrcPtr<ShaderProg> sProg;
				static const ShaderProg::UniVar* colorUniVar;
				static const ShaderProg::UniVar* modelViewProjectionMat;
				static Mat4 viewProjectionMat;

				void init();
				void run();
		}; // end Dbg

	//====================================================================================================================
	// Properties                                                                                                        =
	//====================================================================================================================
	PROPERTY_R(uint, width, getWidth) ///< Width of the rendering. Dont confuse with the window width
	PROPERTY_R(uint, height, getHeight) ///< Height of the rendering. Dont confuse with the window width
	PROPERTY_R(uint, framesNum, getFramesNum) ///< Frame number
	PROPERTY_R(float, aspectRatio, getAspectRatio) ///< Just a precalculated value

	//====================================================================================================================
	// Public                                                                                                            =
	//====================================================================================================================
	public:
		/**
		 * @name Rendering stages
		 */
		/**@{*/
		Ms ms; ///< Material rendering stage
		Is is; ///< Illumination rendering stage
		Pps pps; ///< Postprocessing rendering stage
		Bs bs; ///< Blending stage
		Dbg dbg; ///< Debugging rendering stage
		/**@}*/

		Renderer();

		/**
		 * Init the renderer given an initialization class
		 * @param initializer The initializer class
		 */
		void init(const RendererInitializer& initializer);

		/**
		 * This function does all the rendering stages and produces a final FAI
		 * @param cam The camera from where the rendering will be done
		 */
		void render(Camera& cam);

		/**
		 * My version of gluUnproject
		 * @param windowCoords Window screen coords
		 * @param modelViewMat The modelview matrix
		 * @param projectionMat The projection matrix
		 * @param view The view vector
		 * @return The unprojected coords
		 */
		static Vec3 unproject(const Vec3& windowCoords, const Mat4& modelViewMat, const Mat4& projectionMat,
		                      const int view[4]);

		/**
		 * It returns an orthographic projection matrix
		 * @param left left vertical clipping plane
		 * @param right right vertical clipping plane
		 * @param bottom bottom horizontal clipping plane
		 * @param top top horizontal clipping plane
		 * @param near nearer distance of depth clipping plane
		 * @param far farther distance of depth clipping plane
		 * @return A 4x4 projection matrix
		 */
		static Mat4 ortho(float left, float right, float bottom, float top, float near, float far);


	//====================================================================================================================
	// Protected                                                                                                         =
	//====================================================================================================================
	protected:
		const Camera* cam; ///< Current camera
		static float quadVertCoords [][2];
		static int maxColorAtachments; ///< Max color attachments a FBO can accept
		Mat4 viewProjectionMat; ///< In case anyone needs it

		static void drawQuad(int vertCoordsUniLoc);
		void setupMaterial(const class Material& mtl, const SceneNode& sceneNode, const Camera& cam);
		static void setViewport(uint x, uint y, uint w, uint h) { glViewport(x,y,w,h); }

		// to be removed
	public:
		static void color3(const Vec3& v) { glColor3fv(&((Vec3&)v)[0]); } ///< OpenGL wrapper
		static void color4(const Vec4& v) { glColor4fv(&((Vec4&)v)[0]); } ///< OpenGL wrapper
		static void setProjectionMatrix(const Camera& cam);
		static void setViewMatrix(const Camera& cam);
		static void noShaders() { ShaderProg::unbind(); } ///< unbind shaders @todo remove this. From now on there will be only shaders
		static void setProjectionViewMatrices(const Camera& cam) { setProjectionMatrix(cam); setViewMatrix(cam); }
		static void multMatrix(const Mat4& m4) { glMultMatrixf(&(m4.getTransposed())(0, 0)); } ///< OpenGL wrapper
		static void multMatrix(const Transform& trf) { glMultMatrixf(&(Mat4(trf).getTransposed())(0, 0)); } ///< OpenGL wrapper
		static void loadMatrix(const Mat4& m4) { glLoadMatrixf(&(m4.getTransposed())(0, 0)); } ///< OpenGL wrapper
		static void loadMatrix(const Transform& trf) { glLoadMatrixf(&(Mat4(trf).getTransposed())(0, 0)); } ///< OpenGL wrapper
};

#endif