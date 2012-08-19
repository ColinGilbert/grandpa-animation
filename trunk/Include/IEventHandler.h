#ifndef __GRP_I_EVENT_HANDLER_H__
#define __GRP_I_EVENT_HANDLER_H__

namespace grp
{

class IModel;
class ISkeleton;
class IPart;
class IMaterial;
class IProperty;

//do not destroy corresponding IModel in any function below

class IEventHandler
{
public:
	virtual ~IEventHandler(){}

	//part has been built when this function is called
	//do not call IModel::setPart nor IModel::removePart nor IModel::removeAllParts within this function
	virtual void onPartBuilt(IModel* model, const Char* slot, IPart* part){}

	//part is still available when this function is called
	virtual void onPartDestroy(IModel* model, const Char* slot, IPart* part){}

	//do not call IModel::setPart nor IModel::removePart nor IModel::removeAllParts within this function
	virtual void onMaterialBuilt(IModel* model, IPart* part, size_t materialIndex){}

	//skeleton has been built when this function is called
	virtual void onSkeletonBuilt(IModel* model, ISkeleton* skeleton){}

	virtual void onAnimationStart(IModel* model, const Char* slot, const Char* eventName, IProperty* params){}
	virtual void onAnimationEnd(IModel* model, const Char* slot, const Char* eventName, IProperty* params){}
	virtual void onAnimationTime(IModel* model, const Char* slot, const Char* eventName, IProperty* params){}
};

}

#endif
