/*
 * sl_export.H
 *
 */


#ifndef SL_EXPORT_H
#define SL_EXPORT_H

#include "sl_vertex.H"
#include "sl_texture.H"
#include "sl_framebuffer.H"


void SL_SCTriangle_DDA(IS_Vertex_ZI *v1,
		       IS_Vertex_ZI *v2,
		       IS_Vertex_ZI *v3,
		       IS_CAttr_None *n,
		       FB_IntensityBuffer *buffer);




void SL_SCTriangle_DDA(IS_Vertex_ZRGB *v1,
		       IS_Vertex_ZRGB *v2,
		       IS_Vertex_ZRGB *v3,
		       IS_CAttr_None *n,
		       FB_UnifiedBuffer *buffer);
		   
void SL_SCTriangle_DDA(IS_Vertex_ZRGB *v1,
		       IS_Vertex_ZRGB *v2,
		       IS_Vertex_ZRGB *v3,
		       IS_CAttr_None *n,
		       FB_SplitBuffer *buffer);

void SL_SCTriangle_DDA(IS_Vertex_ZRGB *v1,
		       IS_Vertex_ZRGB *v2,
		       IS_Vertex_ZRGB *v3,
		       IS_CAttr_A *n,
		       FB_UnifiedBuffer *buffer);
		   
void SL_SCTriangle_DDA(IS_Vertex_ZRGB *v1,
		       IS_Vertex_ZRGB *v2,
		       IS_Vertex_ZRGB *v3,
		       IS_CAttr_A *n,
		       FB_SplitBuffer *buffer);


void SL_SCTriangle_DDA(IS_Vertex_Z *v1,
		       IS_Vertex_Z *v2,
		       IS_Vertex_Z *v3,
		       IS_CAttr_C *n,
		       FB_UnifiedBuffer *buffer);

void SL_SCTriangle_DDA(IS_Vertex_Z *v1,
		       IS_Vertex_Z *v2,
		       IS_Vertex_Z *v3,
		       IS_CAttr_C *n,
		       FB_SplitBuffer *buffer);

void SL_SCTriangle_DDA(IS_Vertex_None *v1,
		       IS_Vertex_None *v2,
		       IS_Vertex_None *v3,
		       IS_CAttr_C *n,
		       FB_UnifiedBuffer *buffer);

void SL_SCTriangle_DDA(IS_Vertex_None *v1,
		       IS_Vertex_None *v2,
		       IS_Vertex_None *v3,
		       IS_CAttr_C *n,
		       FB_SplitBuffer *buffer);

void SL_SCTriangle_DDA(IS_Vertex_ZRGBUVW *v1,
		       IS_Vertex_ZRGBUVW *v2,
		       IS_Vertex_ZRGBUVW *v3,
		       IS_CAttr_T *n,
		       FB_UnifiedBuffer *buffer);

void SL_SCTriangle_DDA(IS_Vertex_ZRGBUVW *v1,
		       IS_Vertex_ZRGBUVW *v2,
		       IS_Vertex_ZRGBUVW *v3,
		       IS_CAttr_T *n,
		       FB_SplitBuffer *buffer);





void SL_SCTriangle_DDX(IS_Vertex_ZRGB *v1,
		       IS_Vertex_ZRGB *v2,
		       IS_Vertex_ZRGB *v3,
		       IS_CAttr_None *n,
		       FB_UnifiedBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_ZRGB *v1,
		       IS_Vertex_ZRGB *v2,
		       IS_Vertex_ZRGB *v3,
		       IS_CAttr_None *n,
		       FB_SplitBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_ZRGB *v1,
		       IS_Vertex_ZRGB *v2,
		       IS_Vertex_ZRGB *v3,
		       IS_CAttr_A *n,
		       FB_UnifiedBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_ZRGB *v1,
		       IS_Vertex_ZRGB *v2,
		       IS_Vertex_ZRGB *v3,
		       IS_CAttr_A *n,
		       FB_SplitBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_Z *v1,
		       IS_Vertex_Z *v2,
		       IS_Vertex_Z *v3,
		       IS_CAttr_C *n,
		       FB_UnifiedBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_Z *v1,
		       IS_Vertex_Z *v2,
		       IS_Vertex_Z *v3,
		       IS_CAttr_C *n,
		       FB_SplitBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_None *v1,
		       IS_Vertex_None *v2,
		       IS_Vertex_None *v3,
		       IS_CAttr_C *n,
		       FB_UnifiedBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_None *v1,
		       IS_Vertex_None *v2,
		       IS_Vertex_None *v3,
		       IS_CAttr_C *n,
		       FB_SplitBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_ZRGBUVW *v1,
		       IS_Vertex_ZRGBUVW *v2,
		       IS_Vertex_ZRGBUVW *v3,
		       IS_CAttr_T *n,
		       FB_UnifiedBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_ZRGBUVW *v1,
		       IS_Vertex_ZRGBUVW *v2,
		       IS_Vertex_ZRGBUVW *v3,
		       IS_CAttr_T *n,
		       FB_SplitBuffer *buffer);





void SL_SCTriangle_DDA(IS_Vertex_ZRGB *v1,
		       IS_Vertex_ZRGB *v2,
		       IS_Vertex_ZRGB *v3,
		       IS_CAttr_None *n,
		       FB_FloatUnifiedBuffer *buffer);
		   
void SL_SCTriangle_DDA(IS_Vertex_ZRGB *v1,
		       IS_Vertex_ZRGB *v2,
		       IS_Vertex_ZRGB *v3,
		       IS_CAttr_None *n,
		       FB_FloatSplitBuffer *buffer);

void SL_SCTriangle_DDA(IS_Vertex_Z *v1,
		       IS_Vertex_Z *v2,
		       IS_Vertex_Z *v3,
		       IS_CAttr_None *n,
		       FB_FloatUnifiedBuffer *buffer);

void SL_SCTriangle_DDA(IS_Vertex_Z *v1,
		       IS_Vertex_Z *v2,
		       IS_Vertex_Z *v3,
		       IS_CAttr_None *n,
		       FB_FloatSplitBuffer *buffer);

void SL_SCTriangle_DDA(IS_Vertex_None *v1,
		       IS_Vertex_None *v2,
		       IS_Vertex_None *v3,
		       IS_CAttr_C *n,
		       FB_FloatUnifiedBuffer *buffer);

void SL_SCTriangle_DDA(IS_Vertex_None *v1,
		       IS_Vertex_None *v2,
		       IS_Vertex_None *v3,
		       IS_CAttr_C *n,
		       FB_FloatSplitBuffer *buffer);

void SL_SCTriangle_DDA(IS_Vertex_ZRGBUVW *v1,
		       IS_Vertex_ZRGBUVW *v2,
		       IS_Vertex_ZRGBUVW *v3,
		       IS_CAttr_T *n,
		       FB_FloatUnifiedBuffer *buffer);

void SL_SCTriangle_DDA(IS_Vertex_ZRGBUVW *v1,
		       IS_Vertex_ZRGBUVW *v2,
		       IS_Vertex_ZRGBUVW *v3,
		       IS_CAttr_T *n,
		       FB_FloatSplitBuffer *buffer);





void SL_SCTriangle_DDX(IS_Vertex_ZRGB *v1,
		       IS_Vertex_ZRGB *v2,
		       IS_Vertex_ZRGB *v3,
		       IS_CAttr_None *n,
		       FB_FloatUnifiedBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_ZRGB *v1,
		       IS_Vertex_ZRGB *v2,
		       IS_Vertex_ZRGB *v3,
		       IS_CAttr_None *n,
		       FB_FloatSplitBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_Z *v1,
		       IS_Vertex_Z *v2,
		       IS_Vertex_Z *v3,
		       IS_CAttr_C *n,
		       FB_FloatUnifiedBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_Z *v1,
		       IS_Vertex_Z *v2,
		       IS_Vertex_Z *v3,
		       IS_CAttr_C *n,
		       FB_FloatSplitBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_None *v1,
		       IS_Vertex_None *v2,
		       IS_Vertex_None *v3,
		       IS_CAttr_C *n,
		       FB_FloatUnifiedBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_None *v1,
		       IS_Vertex_None *v2,
		       IS_Vertex_None *v3,
		       IS_CAttr_C *n,
		       FB_FloatSplitBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_ZRGBUVW *v1,
		       IS_Vertex_ZRGBUVW *v2,
		       IS_Vertex_ZRGBUVW *v3,
		       IS_CAttr_T *n,
		       FB_FloatUnifiedBuffer *buffer);

void SL_SCTriangle_DDX(IS_Vertex_ZRGBUVW *v1,
		       IS_Vertex_ZRGBUVW *v2,
		       IS_Vertex_ZRGBUVW *v3,
		       IS_CAttr_T *n,
		       FB_FloatSplitBuffer *buffer);
		   


#endif /* SL_EXPORT_H */

