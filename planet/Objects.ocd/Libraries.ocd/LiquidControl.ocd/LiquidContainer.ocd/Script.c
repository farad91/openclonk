/**
 * Liquid Container
 *
 * Basic interface for anything that can contain liquids.
 *
 * Author: Ringwaul, ST-DDT, Marky
 */

local liquid_container_item;

// -------------- Properties
//
// Simple properties that define the object as a liquid container,
// what kind of liquid it can hold, how much it can hold
//
// naming scheme: [verb]LiquidContainer[attribute], because it concerns the container

func IsLiquidContainer() { return true;}

func IsLiquidContainerForMaterial(string liquid_name)
{
	return false;
}

func GetLiquidContainerMaxFillLevel()
{
	return 0;
}

// -------------- Current Status
//
// Simple boolean status checks
//
// naming scheme: LiquidContainer[attribute/question]

func LiquidContainerIsEmpty()
{
	return (GetLiquidFillLevel() == 0);
}

func LiquidContainerIsFull()
{
	return GetLiquidFillLevel() == GetLiquidContainerMaxFillLevel();
}

func LiquidContainerAccepts(string liquid_name)
{
	return IsLiquidContainerForMaterial(liquid_name)
	   && (LiquidContainerIsEmpty() || GetLiquidType() == liquid_name);
}

// -------------- Getters
//
// Getters for stored liquid and amount
// - these should be used primarily by objects that include this library
//
// naming scheme: GetLiquid[attribute], because it concerns the liquid

func GetLiquidItem()
{
	return liquid_container_item;
}

func SetLiquidItem(object item)
{
	if (item && (item->~IsLiquid() || item.IsLiquid != nil))
	{
		liquid_container_item = item;
	}
	else
	{
		FatalError(Format("Object %v is not a liquid", item));
	}
}

func GetLiquidType()
{
	if (GetLiquidItem())
	{
		return GetLiquidItem()->IsLiquid();
	}
	return nil;
}

func GetLiquidFillLevel()
{
	if (GetLiquidItem())
	{
		return GetLiquidItem()->GetLiquidAmount();
	}
	return 0;
}

// -------------- Setters
//
// Setters for stored liquid and amount
// - these should be used primarily by objects that include this library
//
// naming scheme: SetLiquid[attribute], because it concerns the liquid

func SetLiquidType(string liquid_name)
{
	var amount = 0; // for new items only
	if (GetLiquidItem())
	{
		amount = GetLiquidItem()->GetLiquidAmount();
		if (!WildcardMatch(liquid_name, GetLiquidItem()->IsLiquid()))
			GetLiquidItem()->RemoveObject();
	}

	if (!GetLiquidItem())
	{
		var item = Library_Liquid->CreateLiquid(liquid_name, amount);
		SetLiquidItem(item);
		if (amount > 0) item->UpdateLiquidObject();
		// if not removed because of amount
		if (item) item->Enter(this);
	}
}

func SetLiquidFillLevel(int amount)
{
	if (!GetLiquidItem())
	{
		SetLiquidType(nil);
	}

	ChangeLiquidFillLevel(amount - GetLiquidFillLevel());
}

func ChangeLiquidFillLevel(int amount)
{
	if (GetLiquidItem())
	{
		GetLiquidItem()->DoLiquidAmount(amount);
	}
	
	this->~UpdateLiquidContainer();
}

// -------------- Interaction
//
// Interfaces for interaction with other objects

/**
Extracts liquid from the container.
@param liquid_name: Material to extract; Wildcardsupport
               Defaults to the current liquid if 'nil' is passed.
@param amount: Max Amount of liquid being extracted;
               Defaults to all contained liquid if 'nil' is passed.
@param destination: Object that extracts the liquid
@return [returned_liquid, returned_amount]
	   - returned_liquid: Material being extracted
	   - returned_amount: Amount being extracted
*/
func RemoveLiquid(string liquid_name, int amount, object destination)
{
	if (amount < 0)
	{
		FatalError(Format("You can remove positive amounts of liquid only, got %d", amount));
	}

	// default parameters if nothing is provided: the current material and level
	liquid_name = liquid_name ?? GetLiquidType();
	amount = amount ?? GetLiquidFillLevel();

	//Wrong material?
	if (!WildcardMatch(GetLiquidType(), liquid_name))
		return [GetLiquidType(), 0];

	amount = Min(amount, GetLiquidFillLevel());
	ChangeLiquidFillLevel(-amount);
	return [liquid_name, amount];
}

/** 
Inserts liquid into the container.
@param liquid_name: Material to insert
@param amount: Max Amount of Material being inserted 
@param source: Object which inserts the liquid
@return returned_amount: The inserted amount
*/
func PutLiquid(string liquid_name, int amount, object source)
{
	if (amount < 0)
	{
		FatalError(Format("You can insert positive amounts of liquid only, got %d", amount));
	}

	if (LiquidContainerAccepts(liquid_name))
	{
		SetLiquidType(liquid_name);
		amount = BoundBy(GetLiquidContainerMaxFillLevel() - GetLiquidFillLevel(), 0, amount);
		ChangeLiquidFillLevel(+amount);
		return amount;
	}
	else //Wrong material?
	{
		return 0;
	}
}

// --------------  Internals --------------
//
// Internal stuff


func SaveScenarioObject(props)
{
	if (!inherited(props, ...)) return false;
	if (GetLiquidType())
		props->AddCall("Fill", this, "SetLiquidContainer", Format("%v", GetLiquidType()), GetLiquidFillLevel());
	return true;
}

// set the current state, without sanity checks
func SetLiquidContainer(string liquid_name, int amount)
{
	SetLiquidType(liquid_name);
	SetLiquidFillLevel(amount);
}
