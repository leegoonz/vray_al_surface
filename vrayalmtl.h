#ifndef __MTLSKEL__H
#define __MTLSKEL__H

#include "max.h"
#include <bmmlib.h>
#include "iparamm2.h"
#include "resource.h"

#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 6000
#include "IMtlRender_Compatibility.h"
#endif
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 13900
#include <IMaterialBrowserEntryInfo.h>
#endif

#include "vraybase.h"
#include "vrayplugins.h"
#include "brdfs.h"
#include "vraygeom.h"
#include "brdfpool.h"
#include "albrdf.h"

// IMPORTANT:
// The ClassID must be changed whenever a new project
// is created using this skeleton
#define	MTL_CLASSID	Class_ID(0x7d080943, 0x54fe7f7d)
#define STR_CLASSNAME _T("VRayALMtl")
#define STR_LIBDESC _T("VRayALMtl material")
#define STR_DLGTITLE _T("VRayALMtl Parameters")

extern ClassDesc* GetSkeletonMtlDesc();

// Paramblock2 name
enum { mtl_params, }; 

// Parameter map IDs
enum { map_basic, map_diffuse, map_sss1, map_sss2, map_sss3, map_reflect1, map_reflect2, map_textures, NUM_PMAPS };

// Paramblock2 parameter list
enum {
	mtl_glossiness,
	mtl_color,
	mtl_diffuse,
	mtl_opacity,

	mtl_sssMix,

	mtl_sssWeight1,
	mtl_sssColor1,
	mtl_sssRadius1,

	mtl_sssWeight2,
	mtl_sssColor2,
	mtl_sssRadius2,

	mtl_sssWeight3,
	mtl_sssColor3,
	mtl_sssRadius3,

	mtl_diffuseStrength,
	mtl_sssDensityScale,
};

/*===========================================================================*\
 |	The actual BRDF
\*===========================================================================*/

class MyALBSDF: public VR::MyBaseBSDF {
public:
	VR::Vector getGlossyReflectionDir(float uc, float vc, const VR::Vector &viewDir, float &rayProbability);
	VR::real getGlossyProbability(const VR::Vector &direction, const VR::Vector &viewDir);
	float remapGlossiness(float nk);
};

/*===========================================================================*\
 |	SkeletonMaterial class defn
\*===========================================================================*/

class SkeletonMaterial : public Mtl, public VR::VRenderMtl {
	VR::BRDFPool<MyALBSDF> bsdfPool;
	VR::LayeredBSDFRenderChannels renderChannels;

	VR::Color getBlend(ShadeContext &sc, int i);
public:
	// various variables
	Interval ivalid;

	// Cached parameters
	float glossiness;
	Color reflect, diffuse;
	Color opacity;
	float diffuseStrength;

	float sssMix;
	float sssDensityScale;
	float sssRadius1, sssRadius2, sssRadius3;
	float sssWeight1, sssWeight2, sssWeight3;
	Color sssColor1, sssColor2, sssColor3;

	// Parameter and UI management
	IParamBlock2 *pblock; 	
	ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
	void Update(TimeValue t, Interval& valid);
	Interval Validity(TimeValue t);
	void Reset();

	SkeletonMaterial(BOOL loading);
	Class_ID ClassID() { return MTL_CLASSID; }
	SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	void GetClassName(TSTR& s) { s=STR_CLASSNAME; }
	void DeleteThis() { delete this; }
	
	void NotifyChanged();

	// From MtlBase and Mtl
	void SetAmbient(Color c, TimeValue t);		
	void SetDiffuse(Color c, TimeValue t);		
	void SetSpecular(Color c, TimeValue t);
	void SetShininess(float v, TimeValue t);
	Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE);
	Color GetDiffuse(int mtlNum=0, BOOL backFace=FALSE);
	Color GetSpecular(int mtlNum=0, BOOL backFace=FALSE);
	float GetXParency(int mtlNum=0, BOOL backFace=FALSE);
	float GetShininess(int mtlNum=0, BOOL backFace=FALSE);		
	float GetShinStr(int mtlNum=0, BOOL backFace=FALSE);
	float WireSize(int mtlNum=0, BOOL backFace=FALSE);
			
	// Shade and displacement calculation
	void Shade(ShadeContext& sc);
	float EvalDisplacement(ShadeContext& sc); 
	Interval DisplacementValidity(TimeValue t); 

	// SubMaterial access methods
	int NumSubMtls() { return 0; }
	Mtl* GetSubMtl(int i) { return NULL; }
	void SetSubMtl(int i, Mtl *m) {}
	TSTR GetSubMtlSlotName(int i) { return _T(""); }
	TSTR GetSubMtlTVName(int i) { return _T(""); }

	// SubTexmap access methods
	int NumSubTexmaps() { return 0; }
	Texmap* GetSubTexmap(int i) { return NULL; }
	void SetSubTexmap(int i, Texmap *m) {}
	TSTR GetSubTexmapSlotName(int i) { return _T(""); }
	TSTR GetSubTexmapTVName(int i) { return _T(""); }

	// Number of subanims
	int NumSubs() { return 1; } 
	Animatable* SubAnim(int i);
	TSTR SubAnimName(int i);
	int SubNumToRefNum(int subNum) { return subNum; }

	// Number of references
 	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(NOTIFY_REF_CHANGED_ARGS);

	RefTargetHandle Clone(RemapDir& remap);

	IOResult Save(ISave *isave); 
	IOResult Load(ILoad *iload); 

	// Direct Paramblock2 access
	int	NumParamBlocks() { return 1; }
	IParamBlock2* GetParamBlock(int i) { return pblock; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } 
	BOOL SetDlgThing(ParamDlg* dlg);

	void* GetInterface(ULONG id) {
		if (id==I_VRAYMTL) return static_cast<VR::VRenderMtl*>(this);
		return Mtl::GetInterface(id);
	}

	// From VRenderMtl
	void renderBegin(TimeValue t, VR::VRayRenderer *vray) VRAY_OVERRIDE;
	void renderEnd(VR::VRayRenderer *vray) VRAY_OVERRIDE;

	VR::BSDFSampler* newBSDF(const VR::VRayContext &rc, VR::VRenderMtlFlags flags) VRAY_OVERRIDE;
	void deleteBSDF(const VR::VRayContext &rc, VR::BSDFSampler *bsdf) VRAY_OVERRIDE;
	
	void addRenderChannel(int index) VRAY_OVERRIDE;
	VR::VRayVolume* getVolume(const VR::VRayContext &rc) VRAY_OVERRIDE;
};

/*===========================================================================*\
 |	Dialog Processor
\*===========================================================================*/

#endif