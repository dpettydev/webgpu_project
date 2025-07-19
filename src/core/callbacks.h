#ifndef __DRG_CALLBACKS_H__
#define __DRG_CALLBACKS_H__

//
//Forward Declarations
//
class drgObjectBase;
class string16;

//
//Callback Definitions
//

typedef void(*drgCallback) (void* data);
typedef void(*drgCallbackStr) (string16 str, void* data);
typedef void(*drgCallbackObj) (drgObjectBase* obj, void* data);

#endif // __DRG_CALLBACKS_H__

