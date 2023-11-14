//==============================================================================
//includes:

#include "GAPService.h"
//==============================================================================
//defines:

#define UID xSERVICE_UID
//==============================================================================
//variables:

/*static const ObjectDescriptionT privateObjectDescription =
{
	.Key = OBJECT_DESCRIPTION_KEY,
	.ObjectId = UID,
	.Type = nameof(GAPServiceT)
};*/
//==============================================================================
//functions:


//==============================================================================
//initialization:

xResult GAPServiceInit(GAPServiceT* service, GAPServiceInitT* init)
{
	xServiceInit((xServiceT*)service, (xServiceInitT*)init);

	service->Base.Info.Type = xServiceTypeGAP;

	//service->Base.Adapter.Interface = &privateInterface;
	//service->Base.Adapter.Description = nameof(GAPServiceT);

	return xResultAccept;
}
//==============================================================================
